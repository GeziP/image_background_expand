#include <QApplication>
#include <QStyleFactory>
#include <QDir>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // 设置应用程序信息
    app.setApplicationName("Image Background Expander");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("Image Tools");
    app.setApplicationDisplayName("图像背景扩展工具");
    
    // 设置现代化样式（如果可用）
    QStringList availableStyles = QStyleFactory::keys();
    if (availableStyles.contains("Fusion")) {
        app.setStyle("Fusion");
    }
    
    // 创建并显示主窗口
    MainWindow window;
    window.show();
    
    return app.exec();
}
