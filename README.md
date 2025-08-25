# 图像背景扩展工具

一个基于Qt 5.14的图像背景扩展工具，用于向图像四个方向扩展指定的背景色。

## 功能特性

- 🖼️ 支持多种图像格式（PNG、JPG、BMP、GIF、TIFF）
- 🎨 点击图像或使用颜色对话框选取背景色
- 📏 独立控制四个方向的扩展大小（0-5000像素）
- 👁️ 实时预览扩展效果
- 🌈 智能渐变混合，自然过渡效果
- 💾 多格式导出支持
- 🔍 图像缩放和平移查看

## 系统要求

- Qt 5.14 或更高版本
- C++11 支持的编译器
- Windows 10/Linux/macOS

## 编译方法

### 使用qmake（推荐）

```bash
# 生成Makefile
qmake ImageBackgroundExpander.pro

# 编译
make  # Linux/macOS
nmake # Windows (Visual Studio)
mingw32-make # Windows (MinGW)
```

### 使用Qt Creator

1. 打开Qt Creator
2. 文件 -> 打开文件或项目
3. 选择 `ImageBackgroundExpander.pro`
4. 配置项目
5. 构建 -> 构建项目

## 使用方法

### 1. 加载图像
- 点击"文件"菜单 -> "打开图像"
- 或使用工具栏的"打开"按钮
- 支持拖拽图像文件到窗口

### 2. 选择背景色
- **方法1**：直接点击图像上的像素选取颜色
- **方法2**：点击"选择颜色"按钮使用颜色对话框

### 3. 设置扩展大小
- 在右侧控制面板调整"扩展大小"
- 分别设置上、下、左、右四个方向的像素数
- 范围：0-5000像素

### 4. 预览和导出
- 勾选"实时预览"查看效果
- 点击"文件"菜单 -> "导出为"保存结果

### 5. 快捷操作
- **鼠标滚轮**：缩放图像
- **右键拖拽**：移动图像
- **Ctrl+O**：打开文件
- **Ctrl+S**：保存文件
- **Ctrl+Shift+S**：另存为

## 技术特性

### 图像处理算法
- **基础扩展**：使用指定颜色填充扩展区域
- **渐变混合**：边缘自然过渡，避免生硬边界
- **高性能处理**：多线程背景处理，实时进度显示

### 界面特性
- **现代化设计**：直观的分割窗口布局
- **响应式UI**：支持窗口缩放和自适应
- **状态反馈**：详细的状态栏信息显示

### 兼容性
- **Qt版本**：针对Qt 5.14优化，支持Qt 5.12+
- **平台支持**：Windows、Linux、macOS
- **格式支持**：PNG、JPG、JPEG、BMP、GIF、TIFF

## 项目结构

```
ImageBackgroundExpander/
├── main.cpp                 # 程序入口
├── mainwindow.h/cpp         # 主窗口类
├── imageviewer.h/cpp        # 自定义图像显示组件
├── imageprocessor.h/cpp     # 图像处理算法
├── mainwindow.ui            # UI界面文件
├── resources.qrc            # 资源文件
├── ImageBackgroundExpander.pro  # qmake项目文件
└── README.md                # 说明文档
```

## 开发说明

### 核心类说明

- **MainWindow**：主窗口，负责UI布局和用户交互
- **ImageViewer**：自定义图像显示组件，支持缩放、取色
- **ImageProcessor**：图像处理引擎，负责背景扩展算法

### 扩展开发

项目采用模块化设计，可以轻松扩展新功能：

- 添加新的图像处理算法到 `ImageProcessor`
- 扩展 `ImageViewer` 支持更多交互功能
- 在 `MainWindow` 中增加新的控制选项

## 许可证

本项目基于MIT许可证开源。

## 贡献

欢迎提交Issue和Pull Request来改进项目！
