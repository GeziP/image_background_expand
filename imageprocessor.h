#ifndef IMAGEPROCESSOR_H
#define IMAGEPROCESSOR_H

#include <QObject>
#include <QImage>
#include <QColor>
#include <QThread>
#include <QMutex>
#include <QTimer>

class ImageProcessor : public QObject
{
    Q_OBJECT

public:
    explicit ImageProcessor(QObject *parent = nullptr);
    ~ImageProcessor();

    // 主要处理函数
    void expandBackground(const QImage &originalImage, 
                         const QColor &backgroundColor,
                         int topExpansion, 
                         int bottomExpansion, 
                         int leftExpansion, 
                         int rightExpansion);

    // 颜色分析工具
    QColor getDominantColor(const QImage &image, const QRect &region = QRect()) const;
    QColor getAverageColor(const QImage &image, const QRect &region = QRect()) const;
    
    // 边缘检测（用于智能扩展）
    QColor getEdgeColor(const QImage &image, Qt::Edge edge) const;
    
    // 智能比例计算
    struct ExpansionValues {
        int top;
        int bottom;
        int left;
        int right;
        bool isValid;
        QString errorMessage;
        QString expansionType; // "width" 或 "height"
        QString description;   // 扩展描述
    };
    
    ExpansionValues calculateSmartExpansion(const QImage &originalImage,
                                          const QString &targetRatio,
                                          const QString &distribution) const;
    
    // 取消当前处理
    void cancelProcessing();
    
    // 获取处理状态
    bool isProcessing() const { return m_processing; }

public slots:
    void setProcessingEnabled(bool enabled) { m_processingEnabled = enabled; }

signals:
    void imageProcessed(const QImage &processedImage);
    void progressChanged(int percentage);
    void processingStarted();
    void processingFinished();
    void errorOccurred(const QString &error);

private slots:
    void processInBackground();

private:
    // 核心算法函数
    QImage createExpandedImage(const QImage &originalImage,
                              const QColor &backgroundColor,
                              int topExpansion,
                              int bottomExpansion, 
                              int leftExpansion,
                              int rightExpansion) const;
    
    QImage applyGradientBlending(const QImage &expandedImage,
                                const QImage &originalImage,
                                int topExpansion,
                                int bottomExpansion,
                                int leftExpansion, 
                                int rightExpansion) const;
    
    // 辅助函数
    QColor blendColors(const QColor &color1, const QColor &color2, double factor) const;
    double calculateBlendFactor(int distance, int maxDistance) const;
    void updateProgress(int current, int total) const;
    
    // 颜色分析辅助函数
    QVector<QRgb> extractPixels(const QImage &image, const QRect &region) const;
    QColor calculateDominantColor(const QVector<QRgb> &pixels) const;
    QColor calculateAverageColor(const QVector<QRgb> &pixels) const;
    
    // 智能比例计算辅助函数
    QPair<double, double> parseRatioString(const QString &ratio) const;
    ExpansionValues calculateOptimalExpansion(const QSize &originalSize,
                                             const QPair<double, double> &ratio,
                                             const QString &distribution) const;
    QPair<int, int> distributeExpansion(int totalExpansion, 
                                       const QString &distribution,
                                       const QString &expansionType) const;

    // 成员变量
    QTimer *m_processTimer;
    mutable QMutex m_processMutex;
    
    // 处理状态
    bool m_processing;
    bool m_processingEnabled;
    bool m_cancelRequested;
    
    // 当前处理参数
    QImage m_currentOriginalImage;
    QColor m_currentBackgroundColor;
    int m_currentTopExpansion;
    int m_currentBottomExpansion;
    int m_currentLeftExpansion;
    int m_currentRightExpansion;
    
    // 配置参数
    int m_blendDistance;        // 混合距离（像素）
    bool m_enableGradient;      // 是否启用渐变混合
    double m_gradientStrength;  // 渐变强度
    
    // 常量
    static constexpr int DEFAULT_BLEND_DISTANCE = 10;
    static constexpr double DEFAULT_GRADIENT_STRENGTH = 0.3;
    static constexpr int PROGRESS_UPDATE_INTERVAL = 100; // 进度更新间隔（毫秒）
};

#endif // IMAGEPROCESSOR_H
