#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QWidget>
#include <QColor>

QT_BEGIN_NAMESPACE
class QAction;
class QMenu;
class QMenuBar;
class QToolBar;
class QStatusBar;
class QVBoxLayout;
class QHBoxLayout;
class QSpinBox;
class QPushButton;
class QCheckBox;
class QGroupBox;
class QSplitter;
class QScrollArea;
class QProgressBar;
QT_END_NAMESPACE

class ImageViewer;
class ImageProcessor;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 文件操作
    void openImage();
    void saveImage();
    void exportImage();
    
    // 图像处理
    void onColorSelected(const QColor &color);
    void onExpansionChanged();
    void onPreviewToggled(bool enabled);
    void resetExpansion();
    
    // 界面更新
    void updateStatusBar();
    void updatePreview();

private:
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void createCentralWidget();
    void createControlPanel();
    void connectSignals();
    
    void updateColorButton();
    void setControlsEnabled(bool enabled);

    // UI组件
    QWidget *m_centralWidget;
    QSplitter *m_mainSplitter;
    QScrollArea *m_scrollArea;
    ImageViewer *m_imageViewer;
    QWidget *m_controlPanel;
    
    // 菜单和工具栏
    QMenuBar *m_menuBar;
    QMenu *m_fileMenu;
    QMenu *m_editMenu;
    QMenu *m_viewMenu;
    QMenu *m_helpMenu;
    QToolBar *m_mainToolBar;
    
    // 控制组件
    QGroupBox *m_expansionGroup;
    QSpinBox *m_topSpinBox;
    QSpinBox *m_bottomSpinBox;
    QSpinBox *m_leftSpinBox;
    QSpinBox *m_rightSpinBox;
    
    QGroupBox *m_colorGroup;
    QPushButton *m_colorButton;
    QLabel *m_colorLabel;
    
    QGroupBox *m_previewGroup;
    QCheckBox *m_previewCheckBox;
    QPushButton *m_resetButton;
    
    // 状态栏
    QStatusBar *m_statusBar;
    QLabel *m_imageInfoLabel;
    QLabel *m_colorInfoLabel;
    QProgressBar *m_progressBar;
    
    // 核心功能
    ImageProcessor *m_imageProcessor;
    
    // 状态变量
    QString m_currentImagePath;
    QColor m_selectedColor;
    bool m_previewEnabled;
};

#endif // MAINWINDOW_H
