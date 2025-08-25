#include "mainwindow.h"
#include "imageviewer.h"
#include "imageprocessor.h"
#include <QApplication>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QSplitter>
#include <QScrollArea>
#include <QGroupBox>
#include <QSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QColorDialog>
#include <QFileDialog>
#include <QMessageBox>
#include <QMenuBar>
#include <QToolBar>
#include <QStatusBar>
#include <QProgressBar>
#include <QKeySequence>
#include <QSettings>
#include <QStandardPaths>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_mainSplitter(nullptr)
    , m_scrollArea(nullptr)
    , m_imageViewer(nullptr)
    , m_controlPanel(nullptr)
    , m_imageProcessor(nullptr)
    , m_selectedColor(Qt::white)
    , m_previewEnabled(true)
{
    setWindowTitle("图像背景扩展工具");
    setMinimumSize(800, 600);
    resize(1200, 800);
    
    // 创建核心组件
    m_imageProcessor = new ImageProcessor(this);
    
    // 创建界面
    createCentralWidget();
    createMenus();
    createToolBars();
    createStatusBar();
    
    // 连接信号槽
    connectSignals();
    
    // 加载上次选择的路径
    loadLastImageDirectory();
    
    // 初始状态
    setControlsEnabled(false);
    updateStatusBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createCentralWidget()
{
    m_centralWidget = new QWidget;
    setCentralWidget(m_centralWidget);
    
    // 创建主分割器
    m_mainSplitter = new QSplitter(Qt::Horizontal);
    
    // 创建图像显示区域
    m_imageViewer = new ImageViewer;
    m_scrollArea = new QScrollArea;
    m_scrollArea->setWidget(m_imageViewer);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setAlignment(Qt::AlignCenter);
    m_scrollArea->setMinimumSize(400, 300);
    
    // 创建控制面板
    createControlPanel();
    
    // 添加到分割器
    m_mainSplitter->addWidget(m_scrollArea);
    m_mainSplitter->addWidget(m_controlPanel);
    m_mainSplitter->setStretchFactor(0, 3);
    m_mainSplitter->setStretchFactor(1, 1);
    
    // 设置布局
    QHBoxLayout *layout = new QHBoxLayout(m_centralWidget);
    layout->addWidget(m_mainSplitter);
    layout->setContentsMargins(0, 0, 0, 0);
}

void MainWindow::createControlPanel()
{
    m_controlPanel = new QWidget;
    m_controlPanel->setMaximumWidth(300);
    m_controlPanel->setMinimumWidth(250);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(m_controlPanel);
    
    // 扩展控制组
    m_expansionGroup = new QGroupBox("扩展大小 (像素)");
    QGridLayout *expansionLayout = new QGridLayout(m_expansionGroup);
    
    // 创建方向控制
    expansionLayout->addWidget(new QLabel("上:"), 0, 0);
    m_topSpinBox = new QSpinBox;
    m_topSpinBox->setRange(0, 5000);
    m_topSpinBox->setValue(0);
    m_topSpinBox->setSuffix(" px");
    expansionLayout->addWidget(m_topSpinBox, 0, 1);
    
    expansionLayout->addWidget(new QLabel("下:"), 1, 0);
    m_bottomSpinBox = new QSpinBox;
    m_bottomSpinBox->setRange(0, 5000);
    m_bottomSpinBox->setValue(0);
    m_bottomSpinBox->setSuffix(" px");
    expansionLayout->addWidget(m_bottomSpinBox, 1, 1);
    
    expansionLayout->addWidget(new QLabel("左:"), 2, 0);
    m_leftSpinBox = new QSpinBox;
    m_leftSpinBox->setRange(0, 5000);
    m_leftSpinBox->setValue(0);
    m_leftSpinBox->setSuffix(" px");
    expansionLayout->addWidget(m_leftSpinBox, 2, 1);
    
    expansionLayout->addWidget(new QLabel("右:"), 3, 0);
    m_rightSpinBox = new QSpinBox;
    m_rightSpinBox->setRange(0, 5000);
    m_rightSpinBox->setValue(0);
    m_rightSpinBox->setSuffix(" px");
    expansionLayout->addWidget(m_rightSpinBox, 3, 1);
    
    mainLayout->addWidget(m_expansionGroup);
    
    // 颜色选择组
    m_colorGroup = new QGroupBox("背景颜色");
    QVBoxLayout *colorLayout = new QVBoxLayout(m_colorGroup);
    
    m_colorButton = new QPushButton("选择颜色");
    m_colorButton->setMinimumHeight(40);
    updateColorButton();
    
    m_colorLabel = new QLabel("点击图像选取颜色或使用按钮选择");
    m_colorLabel->setWordWrap(true);
    m_colorLabel->setStyleSheet("color: gray;");
    
    colorLayout->addWidget(m_colorButton);
    colorLayout->addWidget(m_colorLabel);
    mainLayout->addWidget(m_colorGroup);
    
    // 预览控制组
    m_previewGroup = new QGroupBox("预览设置");
    QVBoxLayout *previewLayout = new QVBoxLayout(m_previewGroup);
    
    m_previewCheckBox = new QCheckBox("实时预览");
    m_previewCheckBox->setChecked(true);
    
    m_resetButton = new QPushButton("重置扩展");
    
    previewLayout->addWidget(m_previewCheckBox);
    previewLayout->addWidget(m_resetButton);
    mainLayout->addWidget(m_previewGroup);
    
    // 添加弹性空间
    mainLayout->addStretch();
}

void MainWindow::createMenus()
{
    m_menuBar = menuBar();
    
    // 文件菜单
    m_fileMenu = m_menuBar->addMenu("文件(&F)");
    
    QAction *openAction = new QAction("打开图像(&O)...", this);
    openAction->setShortcut(QKeySequence::Open);
    openAction->setStatusTip("打开图像文件");
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    m_fileMenu->addAction(openAction);
    
    m_fileMenu->addSeparator();
    
    QAction *saveAction = new QAction("保存(&S)...", this);
    saveAction->setShortcut(QKeySequence::Save);
    saveAction->setStatusTip("保存当前图像");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);
    m_fileMenu->addAction(saveAction);
    
    QAction *exportAction = new QAction("导出为(&E)...", this);
    exportAction->setShortcut(QKeySequence::SaveAs);
    exportAction->setStatusTip("导出图像为新文件");
    connect(exportAction, &QAction::triggered, this, &MainWindow::exportImage);
    m_fileMenu->addAction(exportAction);
    
    m_fileMenu->addSeparator();
    
    QAction *exitAction = new QAction("退出(&X)", this);
    exitAction->setShortcut(QKeySequence::Quit);
    exitAction->setStatusTip("退出应用程序");
    connect(exitAction, &QAction::triggered, this, &QWidget::close);
    m_fileMenu->addAction(exitAction);
    
    // 编辑菜单
    m_editMenu = m_menuBar->addMenu("编辑(&E)");
    
    QAction *resetAction = new QAction("重置扩展(&R)", this);
    resetAction->setStatusTip("重置所有扩展设置");
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetExpansion);
    m_editMenu->addAction(resetAction);
    
    // 视图菜单
    m_viewMenu = m_menuBar->addMenu("视图(&V)");
    
    QAction *previewAction = new QAction("实时预览(&P)", this);
    previewAction->setCheckable(true);
    previewAction->setChecked(true);
    previewAction->setStatusTip("切换实时预览");
    connect(previewAction, &QAction::toggled, this, &MainWindow::onPreviewToggled);
    m_viewMenu->addAction(previewAction);
    
    // 帮助菜单
    m_helpMenu = m_menuBar->addMenu("帮助(&H)");
    
    QAction *aboutAction = new QAction("关于(&A)", this);
    aboutAction->setStatusTip("关于本程序");
    connect(aboutAction, &QAction::triggered, [this]() {
        QMessageBox::about(this, "关于",
            "图像背景扩展工具 v1.0.0\\n\\n"
            "用于扩展图像背景的实用工具\\n"
            "基于 Qt 5.14 开发");
    });
    m_helpMenu->addAction(aboutAction);
}

void MainWindow::createToolBars()
{
    m_mainToolBar = addToolBar("主工具栏");
    m_mainToolBar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    
    // 添加常用操作到工具栏
    QAction *openAction = new QAction("打开", this);
    openAction->setStatusTip("打开图像文件");
    connect(openAction, &QAction::triggered, this, &MainWindow::openImage);
    m_mainToolBar->addAction(openAction);
    
    QAction *saveAction = new QAction("保存", this);
    saveAction->setStatusTip("保存图像");
    connect(saveAction, &QAction::triggered, this, &MainWindow::saveImage);
    m_mainToolBar->addAction(saveAction);
    
    m_mainToolBar->addSeparator();
    
    QAction *resetAction = new QAction("重置", this);
    resetAction->setStatusTip("重置扩展设置");
    connect(resetAction, &QAction::triggered, this, &MainWindow::resetExpansion);
    m_mainToolBar->addAction(resetAction);
}

void MainWindow::createStatusBar()
{
    m_statusBar = statusBar();
    
    m_imageInfoLabel = new QLabel("就绪");
    m_colorInfoLabel = new QLabel();
    m_progressBar = new QProgressBar;
    m_progressBar->setVisible(false);
    
    m_statusBar->addWidget(m_imageInfoLabel, 1);
    m_statusBar->addWidget(m_colorInfoLabel);
    m_statusBar->addWidget(m_progressBar);
}

void MainWindow::connectSignals()
{
    // 图像查看器信号
    connect(m_imageViewer, &ImageViewer::colorPicked,
            this, &MainWindow::onColorSelected);
    
    // 扩展控制信号
    connect(m_topSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onExpansionChanged);
    connect(m_bottomSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onExpansionChanged);
    connect(m_leftSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onExpansionChanged);
    connect(m_rightSpinBox, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &MainWindow::onExpansionChanged);
    
    // 颜色选择信号
    connect(m_colorButton, &QPushButton::clicked, [this]() {
        QColor color = QColorDialog::getColor(m_selectedColor, this, "选择背景颜色");
        if (color.isValid()) {
            onColorSelected(color);
        }
    });
    
    // 预览控制信号
    connect(m_previewCheckBox, &QCheckBox::toggled,
            this, &MainWindow::onPreviewToggled);
    connect(m_resetButton, &QPushButton::clicked,
            this, &MainWindow::resetExpansion);
    
    // 图像处理器信号
    connect(m_imageProcessor, &ImageProcessor::imageProcessed,
            m_imageViewer, &ImageViewer::setProcessedImage);
    connect(m_imageProcessor, &ImageProcessor::progressChanged,
            m_progressBar, &QProgressBar::setValue);
}

void MainWindow::openImage()
{
    QString fileName = QFileDialog::getOpenFileName(this,
        "打开图像文件", m_lastImageDirectory,
        "图像文件 (*.png *.jpg *.jpeg *.bmp *.gif *.tiff);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        if (m_imageViewer->loadImage(fileName)) {
            m_currentImagePath = fileName;
            
            // 保存选择的目录路径
            QFileInfo fileInfo(fileName);
            saveLastImageDirectory(fileInfo.absolutePath());
            
            setControlsEnabled(true);
            updateStatusBar();
            
            // 重置扩展设置
            resetExpansion();
        } else {
            QMessageBox::warning(this, "错误", "无法加载图像文件：" + fileName);
        }
    }
}

void MainWindow::saveImage()
{
    if (m_currentImagePath.isEmpty()) {
        exportImage();
        return;
    }
    
    if (m_imageViewer->saveImage(m_currentImagePath)) {
        statusBar()->showMessage("图像已保存", 2000);
    } else {
        QMessageBox::warning(this, "错误", "保存图像失败");
    }
}

void MainWindow::exportImage()
{
    if (!m_imageViewer->hasImage()) {
        QMessageBox::information(this, "提示", "请先打开一个图像文件");
        return;
    }
    
    QString fileName = QFileDialog::getSaveFileName(this,
        "导出图像", m_lastImageDirectory,
        "PNG 文件 (*.png);;JPEG 文件 (*.jpg);;BMP 文件 (*.bmp);;所有文件 (*)");
    
    if (!fileName.isEmpty()) {
        if (m_imageViewer->saveImage(fileName)) {
            // 保存选择的目录路径
            QFileInfo fileInfo(fileName);
            saveLastImageDirectory(fileInfo.absolutePath());
            
            statusBar()->showMessage("图像已导出到: " + fileName, 3000);
        } else {
            QMessageBox::warning(this, "错误", "导出图像失败");
        }
    }
}

void MainWindow::onColorSelected(const QColor &color)
{
    m_selectedColor = color;
    updateColorButton();
    updatePreview();
    updateStatusBar();
}

void MainWindow::onExpansionChanged()
{
    updatePreview();
}

void MainWindow::onPreviewToggled(bool enabled)
{
    m_previewEnabled = enabled;
    if (enabled) {
        updatePreview();
    } else {
        m_imageViewer->clearPreview();
    }
}

void MainWindow::resetExpansion()
{
    m_topSpinBox->setValue(0);
    m_bottomSpinBox->setValue(0);
    m_leftSpinBox->setValue(0);
    m_rightSpinBox->setValue(0);
    
    updatePreview();
}

void MainWindow::updateStatusBar()
{
    if (m_imageViewer->hasImage()) {
        QSize size = m_imageViewer->imageSize();
        m_imageInfoLabel->setText(QString("图像: %1x%2").arg(size.width()).arg(size.height()));
        
        if (m_selectedColor.isValid()) {
            m_colorInfoLabel->setText(QString("颜色: RGB(%1,%2,%3)")
                .arg(m_selectedColor.red())
                .arg(m_selectedColor.green())
                .arg(m_selectedColor.blue()));
        }
    } else {
        m_imageInfoLabel->setText("就绪");
        m_colorInfoLabel->clear();
    }
}

void MainWindow::updatePreview()
{
    if (!m_previewEnabled || !m_imageViewer->hasImage()) {
        return;
    }
    
    // 获取扩展参数
    int top = m_topSpinBox->value();
    int bottom = m_bottomSpinBox->value();
    int left = m_leftSpinBox->value();
    int right = m_rightSpinBox->value();
    
    // 如果没有扩展，清除预览
    if (top == 0 && bottom == 0 && left == 0 && right == 0) {
        m_imageViewer->clearPreview();
        return;
    }
    
    // 处理图像
    m_progressBar->setVisible(true);
    m_imageProcessor->expandBackground(
        m_imageViewer->originalImage(),
        m_selectedColor,
        top, bottom, left, right
    );
}

void MainWindow::updateColorButton()
{
    QString styleSheet = QString(
        "QPushButton {"
        "    background-color: rgb(%1, %2, %3);"
        "    border: 2px solid #888888;"
        "    border-radius: 4px;"
        "    color: %4;"
        "    font-weight: bold;"
        "}"
        "QPushButton:hover {"
        "    border: 2px solid #555555;"
        "}"
    ).arg(m_selectedColor.red())
     .arg(m_selectedColor.green())
     .arg(m_selectedColor.blue())
     .arg(m_selectedColor.lightness() > 128 ? "black" : "white");
    
    m_colorButton->setStyleSheet(styleSheet);
}

void MainWindow::setControlsEnabled(bool enabled)
{
    m_expansionGroup->setEnabled(enabled);
    m_colorGroup->setEnabled(enabled);
    m_previewGroup->setEnabled(enabled);
}

void MainWindow::loadLastImageDirectory()
{
    QSettings settings;
    m_lastImageDirectory = settings.value("lastImageDirectory", 
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation)).toString();
}

void MainWindow::saveLastImageDirectory(const QString &directory)
{
    if (!directory.isEmpty()) {
        m_lastImageDirectory = directory;
        QSettings settings;
        settings.setValue("lastImageDirectory", directory);
    }
}
