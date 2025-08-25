#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QWidget>
#include <QPixmap>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QWheelEvent>
#include <QPaintEvent>
#include <QResizeEvent>
#include <QScrollArea>
#include <QScrollBar>

class ImageViewer : public QWidget
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    
    // 图像操作
    bool loadImage(const QString &fileName);
    bool saveImage(const QString &fileName);
    bool hasImage() const;
    QSize imageSize() const;
    
    // 获取图像数据
    QImage originalImage() const { return m_originalImage; }
    QImage processedImage() const { return m_processedImage; }
    
    // 预览控制
    void setProcessedImage(const QImage &image);
    void clearPreview();
    
    // 显示控制
    void fitToWindow();
    void resetZoom();
    void zoomIn();
    void zoomOut();
    void setZoom(double factor);
    
    // 获取状态
    double zoomFactor() const { return m_scaleFactor; }
    bool isPreviewMode() const { return m_showProcessed; }

signals:
    void colorPicked(const QColor &color);
    void imageChanged();
    void zoomChanged(double factor);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void wheelEvent(QWheelEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateImageSize();
    void scaleImage(double factor);
    QPoint imagePointFromWidget(const QPoint &widgetPoint) const;
    QColor getPixelColor(const QPoint &imagePoint) const;
    void drawCheckerboard(QPainter &painter, const QRect &rect) const;
    
    // 图像数据
    QImage m_originalImage;
    QImage m_processedImage;
    QPixmap m_scaledPixmap;
    
    // 显示状态
    double m_scaleFactor;
    bool m_showProcessed;
    QPoint m_lastPanPoint;
    bool m_dragging;
    
    // 显示区域
    QRect m_imageRect;
    QSize m_scaledImageSize;
    
    // 常量
    static constexpr double MIN_SCALE_FACTOR = 0.1;
    static constexpr double MAX_SCALE_FACTOR = 10.0;
    static constexpr double ZOOM_STEP = 1.25;
};

#endif // IMAGEVIEWER_H
