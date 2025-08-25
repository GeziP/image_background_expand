QT += core widgets

CONFIG += c++11

TARGET = ImageBackgroundExpander
TEMPLATE = app

# Qt version compatibility
QT_VERSION = 5.14
lessThan(QT_MAJOR_VERSION, 5) {
    error("Qt 5.0 or newer is required")
}
equals(QT_MAJOR_VERSION, 5) : lessThan(QT_MINOR_VERSION, 14) {
    warning("This application is optimized for Qt 5.14 or newer")
}

# Source files
SOURCES += \
    main.cpp \
    mainwindow.cpp \
    imageviewer.cpp \
    imageprocessor.cpp

# Header files  
HEADERS += \
    mainwindow.h \
    imageviewer.h \
    imageprocessor.h

# UI files
FORMS += \
    mainwindow.ui

# Resource files (commented out until needed)
# RESOURCES += \
#     resources.qrc

# Windows specific settings
win32 {
    VERSION = 1.0.0.0
    QMAKE_TARGET_COMPANY = "Image Tools"
    QMAKE_TARGET_PRODUCT = "Image Background Expander"
    QMAKE_TARGET_DESCRIPTION = "Tool for expanding image backgrounds"
    QMAKE_TARGET_COPYRIGHT = "Copyright 2024"
}

# Deployment settings
target.path = $$[QT_INSTALL_EXAMPLES]/widgets/imagebackgroundexpander
INSTALLS += target
