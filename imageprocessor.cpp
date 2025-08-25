#include "imageprocessor.h"
#include <QApplication>
#include <QDebug>
#include <QMutexLocker>
#include <QtAlgorithms>
#include <QHash>
#include <QTimer>
#include <cmath>

ImageProcessor::ImageProcessor(QObject *parent)
    : QObject(parent)
    , m_processTimer(new QTimer(this))
    , m_processing(false)
    , m_processingEnabled(true)
    , m_cancelRequested(false)
    , m_currentTopExpansion(0)
    , m_currentBottomExpansion(0)
    , m_currentLeftExpansion(0)
    , m_currentRightExpansion(0)
    , m_blendDistance(DEFAULT_BLEND_DISTANCE)
    , m_enableGradient(true)
    , m_gradientStrength(DEFAULT_GRADIENT_STRENGTH)
{
    // 设置单次触发定时器，用于延迟处理
    m_processTimer->setSingleShot(true);
    m_processTimer->setInterval(PROGRESS_UPDATE_INTERVAL);
    connect(m_processTimer, &QTimer::timeout, this, &ImageProcessor::processInBackground);
}

ImageProcessor::~ImageProcessor()
{
    cancelProcessing();
}

void ImageProcessor::expandBackground(const QImage &originalImage,
                                    const QColor &backgroundColor,
                                    int topExpansion,
                                    int bottomExpansion, 
                                    int leftExpansion,
                                    int rightExpansion)
{
    if (!m_processingEnabled || originalImage.isNull()) {
        return;
    }
    
    // 如果参数没有变化且正在处理，则不重复处理
    QMutexLocker locker(&m_processMutex);
    
    if (m_processing && 
        m_currentOriginalImage == originalImage &&
        m_currentBackgroundColor == backgroundColor &&
        m_currentTopExpansion == topExpansion &&
        m_currentBottomExpansion == bottomExpansion &&
        m_currentLeftExpansion == leftExpansion &&
        m_currentRightExpansion == rightExpansion) {
        return;
    }
    
    // 保存处理参数
    m_currentOriginalImage = originalImage;
    m_currentBackgroundColor = backgroundColor;
    m_currentTopExpansion = topExpansion;
    m_currentBottomExpansion = bottomExpansion;
    m_currentLeftExpansion = leftExpansion;
    m_currentRightExpansion = rightExpansion;
    
    // 重启定时器进行延迟处理（避免频繁处理）
    m_processTimer->stop();
    m_processTimer->start();
}

void ImageProcessor::processInBackground()
{
    QMutexLocker locker(&m_processMutex);
    
    if (m_processing) {
        return;
    }
    
    m_processing = true;
    m_cancelRequested = false;
    locker.unlock();
    
    emit processingStarted();
    emit progressChanged(0);
    
    try {
        // 执行图像处理
        QImage result = createExpandedImage(
            m_currentOriginalImage,
            m_currentBackgroundColor,
            m_currentTopExpansion,
            m_currentBottomExpansion,
            m_currentLeftExpansion,
            m_currentRightExpansion
        );
        
        if (!m_cancelRequested && !result.isNull()) {
            // 应用渐变混合（如果启用）
            if (m_enableGradient && 
                (m_currentTopExpansion > 0 || m_currentBottomExpansion > 0 || 
                 m_currentLeftExpansion > 0 || m_currentRightExpansion > 0)) {
                
                emit progressChanged(70);
                if (!m_cancelRequested) {
                    result = applyGradientBlending(
                        result,
                        m_currentOriginalImage,
                        m_currentTopExpansion,
                        m_currentBottomExpansion,
                        m_currentLeftExpansion,
                        m_currentRightExpansion
                    );
                }
            }
            
            emit progressChanged(100);
            
            if (!m_cancelRequested) {
                emit imageProcessed(result);
            }
        }
        
    } catch (const std::exception &e) {
        emit errorOccurred(QString("处理图像时发生错误: %1").arg(e.what()));
    } catch (...) {
        emit errorOccurred("处理图像时发生未知错误");
    }
    
    locker.relock();
    m_processing = false;
    emit processingFinished();
}

QImage ImageProcessor::createExpandedImage(const QImage &originalImage,
                                          const QColor &backgroundColor,
                                          int topExpansion,
                                          int bottomExpansion,
                                          int leftExpansion, 
                                          int rightExpansion) const
{
    if (originalImage.isNull()) {
        return QImage();
    }
    
    // 计算新图像尺寸
    int newWidth = originalImage.width() + leftExpansion + rightExpansion;
    int newHeight = originalImage.height() + topExpansion + bottomExpansion;
    
    if (newWidth <= 0 || newHeight <= 0) {
        return QImage();
    }
    
    // 创建新图像
    QImage expandedImage(newWidth, newHeight, QImage::Format_ARGB32);
    expandedImage.fill(backgroundColor);
    
    updateProgress(10, 100);
    if (m_cancelRequested) return QImage();
    
    // 计算原始图像在新图像中的位置
    int offsetX = leftExpansion;
    int offsetY = topExpansion;
    
    // 复制原始图像到新图像中心
    for (int y = 0; y < originalImage.height(); ++y) {
        if (m_cancelRequested) return QImage();
        
        const QRgb *sourceLine = reinterpret_cast<const QRgb*>(originalImage.constScanLine(y));
        QRgb *destLine = reinterpret_cast<QRgb*>(expandedImage.scanLine(y + offsetY));
        
        for (int x = 0; x < originalImage.width(); ++x) {
            destLine[x + offsetX] = sourceLine[x];
        }
        
        // 更新进度（10% - 60%）
        if (y % 10 == 0) {
            updateProgress(10 + (50 * y) / originalImage.height(), 100);
        }
    }
    
    updateProgress(60, 100);
    return expandedImage;
}

QImage ImageProcessor::applyGradientBlending(const QImage &expandedImage,
                                           const QImage &originalImage,
                                           int topExpansion,
                                           int bottomExpansion,
                                           int leftExpansion,
                                           int rightExpansion) const
{
    if (expandedImage.isNull() || originalImage.isNull()) {
        return expandedImage;
    }
    
    QImage result = expandedImage;
    int blendDist = qMin(m_blendDistance, 
                        qMin(qMin(topExpansion, bottomExpansion), 
                             qMin(leftExpansion, rightExpansion)));
    
    if (blendDist <= 0) {
        return result;
    }
    
    // 获取原始图像在扩展图像中的区域
    QRect originalRect(leftExpansion, topExpansion, 
                      originalImage.width(), originalImage.height());
    
    updateProgress(70, 100);
    if (m_cancelRequested) return QImage();
    
    // 对每个扩展区域应用渐变
    const QColor bgColor = QColor(expandedImage.pixel(0, 0));
    
    // 顶部渐变
    if (topExpansion > 0) {
        for (int y = 0; y < qMin(topExpansion, blendDist); ++y) {
            if (m_cancelRequested) return QImage();
            
            double factor = static_cast<double>(y) / blendDist;
            factor = calculateBlendFactor(y, blendDist);
            
            QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(topExpansion - 1 - y));
            const QRgb *edgeLine = reinterpret_cast<const QRgb*>(originalImage.constScanLine(0));
            
            for (int x = leftExpansion; x < leftExpansion + originalImage.width(); ++x) {
                QColor edgeColor(edgeLine[x - leftExpansion]);
                QColor blendedColor = blendColors(bgColor, edgeColor, factor);
                line[x] = blendedColor.rgba();
            }
        }
    }
    
    updateProgress(80, 100);
    if (m_cancelRequested) return QImage();
    
    // 底部渐变
    if (bottomExpansion > 0) {
        int bottomStart = topExpansion + originalImage.height();
        for (int y = 0; y < qMin(bottomExpansion, blendDist); ++y) {
            if (m_cancelRequested) return QImage();
            
            double factor = static_cast<double>(y) / blendDist;
            factor = calculateBlendFactor(y, blendDist);
            
            QRgb *line = reinterpret_cast<QRgb*>(result.scanLine(bottomStart + y));
            const QRgb *edgeLine = reinterpret_cast<const QRgb*>(
                originalImage.constScanLine(originalImage.height() - 1));
            
            for (int x = leftExpansion; x < leftExpansion + originalImage.width(); ++x) {
                QColor edgeColor(edgeLine[x - leftExpansion]);
                QColor blendedColor = blendColors(bgColor, edgeColor, factor);
                line[x] = blendedColor.rgba();
            }
        }
    }
    
    updateProgress(90, 100);
    if (m_cancelRequested) return QImage();
    
    // 左侧渐变
    if (leftExpansion > 0) {
        for (int x = 0; x < qMin(leftExpansion, blendDist); ++x) {
            if (m_cancelRequested) return QImage();
            
            double factor = static_cast<double>(x) / blendDist;
            factor = calculateBlendFactor(x, blendDist);
            
            for (int y = topExpansion; y < topExpansion + originalImage.height(); ++y) {
                QRgb *pixel = reinterpret_cast<QRgb*>(result.scanLine(y));
                QColor edgeColor(originalImage.pixel(0, y - topExpansion));
                QColor blendedColor = blendColors(bgColor, edgeColor, factor);
                pixel[leftExpansion - 1 - x] = blendedColor.rgba();
            }
        }
    }
    
    // 右侧渐变
    if (rightExpansion > 0) {
        int rightStart = leftExpansion + originalImage.width();
        for (int x = 0; x < qMin(rightExpansion, blendDist); ++x) {
            if (m_cancelRequested) return QImage();
            
            double factor = static_cast<double>(x) / blendDist;
            factor = calculateBlendFactor(x, blendDist);
            
            for (int y = topExpansion; y < topExpansion + originalImage.height(); ++y) {
                QRgb *pixel = reinterpret_cast<QRgb*>(result.scanLine(y));
                QColor edgeColor(originalImage.pixel(originalImage.width() - 1, y - topExpansion));
                QColor blendedColor = blendColors(bgColor, edgeColor, factor);
                pixel[rightStart + x] = blendedColor.rgba();
            }
        }
    }
    
    updateProgress(95, 100);
    return result;
}

QColor ImageProcessor::blendColors(const QColor &color1, const QColor &color2, double factor) const
{
    factor = qBound(0.0, factor, 1.0);
    
    int r = static_cast<int>(color1.red() * (1.0 - factor) + color2.red() * factor);
    int g = static_cast<int>(color1.green() * (1.0 - factor) + color2.green() * factor);
    int b = static_cast<int>(color1.blue() * (1.0 - factor) + color2.blue() * factor);
    int a = static_cast<int>(color1.alpha() * (1.0 - factor) + color2.alpha() * factor);
    
    return QColor(r, g, b, a);
}

double ImageProcessor::calculateBlendFactor(int distance, int maxDistance) const
{
    if (maxDistance <= 0) {
        return 0.0;
    }
    
    double normalizedDistance = static_cast<double>(distance) / maxDistance;
    
    // 使用平滑步函数创建更自然的渐变
    double factor = normalizedDistance * normalizedDistance * (3.0 - 2.0 * normalizedDistance);
    
    // 应用渐变强度
    factor *= m_gradientStrength;
    
    return qBound(0.0, factor, 1.0);
}

void ImageProcessor::updateProgress(int current, int total) const
{
    if (total > 0) {
        int percentage = (current * 100) / total;
        // 由于这是const函数，我们需要通过const_cast来发射信号
        const_cast<ImageProcessor*>(this)->progressChanged(percentage);
    }
}

QColor ImageProcessor::getDominantColor(const QImage &image, const QRect &region) const
{
    if (image.isNull()) {
        return QColor();
    }
    
    QVector<QRgb> pixels = extractPixels(image, region);
    return calculateDominantColor(pixels);
}

QColor ImageProcessor::getAverageColor(const QImage &image, const QRect &region) const
{
    if (image.isNull()) {
        return QColor();
    }
    
    QVector<QRgb> pixels = extractPixels(image, region);
    return calculateAverageColor(pixels);
}

QColor ImageProcessor::getEdgeColor(const QImage &image, Qt::Edge edge) const
{
    if (image.isNull()) {
        return QColor();
    }
    
    QRect region;
    switch (edge) {
    case Qt::TopEdge:
        region = QRect(0, 0, image.width(), 1);
        break;
    case Qt::BottomEdge:
        region = QRect(0, image.height() - 1, image.width(), 1);
        break;
    case Qt::LeftEdge:
        region = QRect(0, 0, 1, image.height());
        break;
    case Qt::RightEdge:
        region = QRect(image.width() - 1, 0, 1, image.height());
        break;
    }
    
    return getAverageColor(image, region);
}

void ImageProcessor::cancelProcessing()
{
    QMutexLocker locker(&m_processMutex);
    m_cancelRequested = true;
    m_processTimer->stop();
}

QVector<QRgb> ImageProcessor::extractPixels(const QImage &image, const QRect &region) const
{
    QRect actualRegion = region.isValid() ? region : image.rect();
    actualRegion = actualRegion.intersected(image.rect());
    
    QVector<QRgb> pixels;
    pixels.reserve(actualRegion.width() * actualRegion.height());
    
    for (int y = actualRegion.top(); y <= actualRegion.bottom(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb*>(image.constScanLine(y));
        for (int x = actualRegion.left(); x <= actualRegion.right(); ++x) {
            pixels.append(line[x]);
        }
    }
    
    return pixels;
}

QColor ImageProcessor::calculateDominantColor(const QVector<QRgb> &pixels) const
{
    if (pixels.isEmpty()) {
        return QColor();
    }
    
    // 使用颜色直方图找到最常见的颜色
    QHash<QRgb, int> colorCount;
    
    for (QRgb pixel : pixels) {
        // 稍微降低精度以减少颜色数量
        QColor color(pixel);
        int r = (color.red() / 8) * 8;
        int g = (color.green() / 8) * 8;
        int b = (color.blue() / 8) * 8;
        QRgb quantizedColor = qRgb(r, g, b);
        
        colorCount[quantizedColor]++;
    }
    
    // 找到出现次数最多的颜色
    QRgb dominantColor = 0;
    int maxCount = 0;
    
    for (auto it = colorCount.constBegin(); it != colorCount.constEnd(); ++it) {
        if (it.value() > maxCount) {
            maxCount = it.value();
            dominantColor = it.key();
        }
    }
    
    return QColor(dominantColor);
}

QColor ImageProcessor::calculateAverageColor(const QVector<QRgb> &pixels) const
{
    if (pixels.isEmpty()) {
        return QColor();
    }
    
    quint64 totalR = 0, totalG = 0, totalB = 0, totalA = 0;
    
    for (QRgb pixel : pixels) {
        QColor color(pixel);
        totalR += color.red();
        totalG += color.green();
        totalB += color.blue();
        totalA += color.alpha();
    }
    
    int count = pixels.size();
    return QColor(
        static_cast<int>(totalR / count),
        static_cast<int>(totalG / count),
        static_cast<int>(totalB / count),
        static_cast<int>(totalA / count)
    );
}
