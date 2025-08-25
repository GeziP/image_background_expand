cmake_minimum_required(VERSION 3.16)

# Project configuration
project(ImageBackgroundExpander VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required Qt components
find_package(Qt6 REQUIRED COMPONENTS Core Widgets)

# Ensure Qt MOC is enabled
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# Source files
set(SOURCES
    main.cpp
    mainwindow.cpp
    imageviewer.cpp
    imageprocessor.cpp
)

# Header files
set(HEADERS
    mainwindow.h
    imageviewer.h
    imageprocessor.h
)

# UI files
set(UI_FILES
    mainwindow.ui
)

# Create executable
add_executable(${PROJECT_NAME}
    ${SOURCES}
    ${HEADERS}
    ${UI_FILES}
)

# Link Qt libraries
target_link_libraries(${PROJECT_NAME} Qt6::Core Qt6::Widgets)

# Windows specific settings
if(WIN32)
    set_target_properties(${PROJECT_NAME} PROPERTIES
        WIN32_EXECUTABLE TRUE
        VERSION ${PROJECT_VERSION}
        COMPILE_DEFINITIONS QT_DEPRECATED_WARNINGS
    )
    
    # Set application icon (if available)
    # set_target_properties(${PROJECT_NAME} PROPERTIES
    #     WIN32_EXECUTABLE TRUE
    #     LINK_FLAGS "/SUBSYSTEM:WINDOWS"
    # )
endif()

# Installation rules
install(TARGETS ${PROJECT_NAME}
    BUNDLE DESTINATION .
    RUNTIME DESTINATION bin
)

# Qt deployment
if(WIN32)
    # This will be handled by windeployqt in the GitHub Actions workflow
    find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS ${Qt6_DIR}/../../../bin)
    if(WINDEPLOYQT_EXECUTABLE)
        message(STATUS "Found windeployqt: ${WINDEPLOYQT_EXECUTABLE}")
    endif()
endif()
