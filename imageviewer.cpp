#include "imageviewer.h"
#include <QApplication>
#include <QFileInfo>
#include <QImageReader>
#include <QImageWriter>
#include <QMessageBox>
#include <QMimeData>
#include <QDragEnterEvent>
#include <QDropEvent>
#include <QUrl>
#include <QPainterPath>
#include <QDebug>
#include <QPainter>
#include <QPixmap>
#include <QScrollBar>

ImageViewer::ImageViewer(QWidget *parent)
    : QWidget(parent)
    , m_scaleFactor(1.0)
    , m_showProcessed(false)
    , m_dragging(false)
{
    setAcceptDrops(true);
    setMouseTracking(true);
    setFocusPolicy(Qt::StrongFocus);
    
    // 设置最小尺寸
    setMinimumSize(200, 200);
    
    // 设置背景色
    setAutoFillBackground(true);
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, QColor(64, 64, 64));
    setPalette(palette);
}

bool ImageViewer::loadImage(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    
    QImage image = reader.read();
    if (image.isNull()) {
        qDebug() << "无法加载图像:" << reader.errorString();
        return false;
    }
    
    // 确保图像格式支持颜色拾取
    if (image.format() != QImage::Format_ARGB32 && 
        image.format() != QImage::Format_RGB32) {
        image = image.convertToFormat(QImage::Format_ARGB32);
    }
    
    m_originalImage = image;
    m_processedImage = QImage(); // 清除处理后的图像
    m_showProcessed = false;
    
    // 重置显示参数
    m_scaleFactor = 1.0;
    updateImageSize();
    fitToWindow();
    
    emit imageChanged();
    emit zoomChanged(m_scaleFactor);
    
    update();
    return true;
}

bool ImageViewer::saveImage(const QString &fileName)
{
    if (!hasImage()) {
        return false;
    }
    
    // 选择要保存的图像（原始或处理后）
    const QImage &imageToSave = m_showProcessed && !m_processedImage.isNull() 
                                ? m_processedImage : m_originalImage;
    
    QImageWriter writer(fileName);
    if (!writer.write(imageToSave)) {
        qDebug() << "保存图像失败:" << writer.errorString();
        return false;
    }
    
    return true;
}

bool ImageViewer::hasImage() const
{
    return !m_originalImage.isNull();
}

QSize ImageViewer::imageSize() const
{
    if (!hasImage()) {
        return QSize();
    }
    
    const QImage &currentImage = m_showProcessed && !m_processedImage.isNull() 
                                ? m_processedImage : m_originalImage;
    return currentImage.size();
}

void ImageViewer::setProcessedImage(const QImage &image)
{
    m_processedImage = image;
    m_showProcessed = true;
    updateImageSize();
    update();
}

void ImageViewer::clearPreview()
{
    m_showProcessed = false;
    updateImageSize();
    update();
}

void ImageViewer::fitToWindow()
{
    if (!hasImage()) {
        return;
    }
    
    QSize imageSize = this->imageSize();
    QSize widgetSize = size() - QSize(20, 20); // 留出边距
    
    double scaleX = static_cast<double>(widgetSize.width()) / imageSize.width();
    double scaleY = static_cast<double>(widgetSize.height()) / imageSize.height();
    
    double scale = qMin(scaleX, scaleY);
    scale = qBound(MIN_SCALE_FACTOR, scale, MAX_SCALE_FACTOR);
    
    setZoom(scale);
}

void ImageViewer::resetZoom()
{
    setZoom(1.0);
}

void ImageViewer::zoomIn()
{
    setZoom(m_scaleFactor * ZOOM_STEP);
}

void ImageViewer::zoomOut()
{
    setZoom(m_scaleFactor / ZOOM_STEP);
}

void ImageViewer::setZoom(double factor)
{
    factor = qBound(MIN_SCALE_FACTOR, factor, MAX_SCALE_FACTOR);
    
    if (qAbs(factor - m_scaleFactor) < 0.001) {
        return;
    }
    
    m_scaleFactor = factor;
    updateImageSize();
    emit zoomChanged(m_scaleFactor);
    update();
}

void ImageViewer::paintEvent(QPaintEvent *event)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);
    
    // 绘制背景
    painter.fillRect(event->rect(), palette().window());
    
    if (!hasImage()) {
        // 绘制提示文字
        painter.setPen(QPen(Qt::white));
        painter.drawText(rect(), Qt::AlignCenter, 
                        QString::fromUtf8("点击\"文件\"菜单打开图像\n或拖拽图像文件到此处"));
        return;
    }
    
    // 计算图像绘制区域
    QRect imageRect = m_imageRect;
    if (imageRect.isEmpty()) {
        return;
    }
    
    // 如果图像有透明通道，先绘制棋盘背景
    const QImage &currentImage = m_showProcessed && !m_processedImage.isNull() 
                                ? m_processedImage : m_originalImage;
    
    if (currentImage.hasAlphaChannel()) {
        drawCheckerboard(painter, imageRect);
    }
    
    // 绘制图像
    if (!m_scaledPixmap.isNull()) {
        painter.drawPixmap(imageRect, m_scaledPixmap);
    }
    
    // 如果是预览模式，绘制提示
    if (m_showProcessed && !m_processedImage.isNull()) {
        painter.setPen(QPen(Qt::yellow, 2));
        painter.drawRect(imageRect.adjusted(-1, -1, 1, 1));
        
        QRect textRect = imageRect;
        textRect.setHeight(30);
        textRect.moveTop(imageRect.top() - 35);
        
        painter.fillRect(textRect, QColor(0, 0, 0, 128));
        painter.setPen(Qt::white);
        painter.drawText(textRect, Qt::AlignCenter, "预览模式");
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (!hasImage()) {
        return;
    }
    
    if (event->button() == Qt::LeftButton) {
        QPoint imagePoint = imagePointFromWidget(event->pos());
        if (!imagePoint.isNull()) {
            QColor color = getPixelColor(imagePoint);
            if (color.isValid()) {
                emit colorPicked(color);
            }
        }
    } else if (event->button() == Qt::RightButton) {
        // 右键开始拖拽
        m_lastPanPoint = event->pos();
        m_dragging = true;
        setCursor(Qt::ClosedHandCursor);
    }
    
    QWidget::mousePressEvent(event);
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::RightButton)) {
        // 处理拖拽移动
        QPoint delta = event->pos() - m_lastPanPoint;
        m_imageRect.translate(delta);
        m_lastPanPoint = event->pos();
        update();
    }
    
    QWidget::mouseMoveEvent(event);
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (hasImage()) {
        const int numDegrees = event->angleDelta().y() / 8;
        const int numSteps = numDegrees / 15;
        
        if (numSteps > 0) {
            zoomIn();
        } else if (numSteps < 0) {
            zoomOut();
        }
    }
    
    QWidget::wheelEvent(event);
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    updateImageSize();
    QWidget::resizeEvent(event);
}

void ImageViewer::updateImageSize()
{
    if (!hasImage()) {
        m_scaledPixmap = QPixmap();
        m_imageRect = QRect();
        m_scaledImageSize = QSize();
        return;
    }
    
    const QImage &currentImage = m_showProcessed && !m_processedImage.isNull() 
                                ? m_processedImage : m_originalImage;
    
    // 计算缩放后的尺寸
    m_scaledImageSize = currentImage.size() * m_scaleFactor;
    
    // 创建缩放后的像素图
    if (m_scaleFactor == 1.0) {
        m_scaledPixmap = QPixmap::fromImage(currentImage);
    } else {
        m_scaledPixmap = QPixmap::fromImage(currentImage.scaled(
            m_scaledImageSize, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
    
    // 计算居中位置
    QPoint center = rect().center();
    m_imageRect = QRect(QPoint(), m_scaledImageSize);
    m_imageRect.moveCenter(center);
    
    update();
}

QPoint ImageViewer::imagePointFromWidget(const QPoint &widgetPoint) const
{
    if (!hasImage() || !m_imageRect.contains(widgetPoint)) {
        return QPoint();
    }
    
    // 将窗口坐标转换为图像坐标
    QPoint relativePoint = widgetPoint - m_imageRect.topLeft();
    
    // 考虑缩放因子
    double x = relativePoint.x() / m_scaleFactor;
    double y = relativePoint.y() / m_scaleFactor;
    
    const QImage &currentImage = m_showProcessed && !m_processedImage.isNull() 
                                ? m_processedImage : m_originalImage;
    
    // 确保坐标在图像范围内
    if (x >= 0 && x < currentImage.width() && y >= 0 && y < currentImage.height()) {
        return QPoint(static_cast<int>(x), static_cast<int>(y));
    }
    
    return QPoint();
}

QColor ImageViewer::getPixelColor(const QPoint &imagePoint) const
{
    if (!hasImage() || imagePoint.isNull()) {
        return QColor();
    }
    
    const QImage &currentImage = m_showProcessed && !m_processedImage.isNull() 
                                ? m_processedImage : m_originalImage;
    
    if (imagePoint.x() >= 0 && imagePoint.x() < currentImage.width() &&
        imagePoint.y() >= 0 && imagePoint.y() < currentImage.height()) {
        return QColor(currentImage.pixel(imagePoint));
    }
    
    return QColor();
}

void ImageViewer::drawCheckerboard(QPainter &painter, const QRect &rect) const
{
    const int tileSize = 16;
    const QColor light(240, 240, 240);
    const QColor dark(200, 200, 200);
    
    painter.save();
    painter.setClipRect(rect);
    
    for (int y = rect.top(); y < rect.bottom(); y += tileSize) {
        for (int x = rect.left(); x < rect.right(); x += tileSize) {
            QRect tileRect(x, y, tileSize, tileSize);
            
            // 交替绘制明暗方块
            bool isLight = ((x / tileSize) + (y / tileSize)) % 2 == 0;
            painter.fillRect(tileRect, isLight ? light : dark);
        }
    }
    
    painter.restore();
}
