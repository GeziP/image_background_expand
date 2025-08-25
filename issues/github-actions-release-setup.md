# GitHub Actions 自动发布配置

## 概述

已为项目配置了自动化的 GitHub Actions workflow，当创建 release tag 时会自动构建并发布 Windows 版本。

## 文件位置

- `.github/workflows/release.yml` - 主要的 workflow 配置文件

## 工作流程

### 触发条件
- 当在 GitHub 上发布新的 Release 时自动触发
- 触发事件：`release: types: [published]`

### 构建过程

1. **环境准备**
   - 使用 Windows Server 最新版本
   - 安装 Qt 5.15.2 (win64_msvc2019_64)
   - 设置 MSVC 2019 编译环境

2. **项目构建**
   - 检出源代码
   - 使用 qmake 生成 Makefile
   - 使用 nmake 编译项目

3. **依赖打包**
   - 使用 windeployqt 自动收集 Qt 依赖
   - 创建完整的可独立运行的程序包

4. **发布文件**
   - 标准版本：包含所有必要文件
   - 便携版本：相同内容的副本，便于区分

## 如何使用

### 1. 推送代码到 GitHub
```bash
git add .
git commit -m "准备发布 v1.0.0"
git push origin main
```

### 2. 创建 Release Tag
```bash
# 创建并推送 tag
git tag v1.0.0
git push origin v1.0.0
```

### 3. 在 GitHub 上创建 Release
1. 访问 GitHub 仓库页面
2. 点击 "Releases" -> "Create a new release"
3. 选择刚才创建的 tag (v1.0.0)
4. 填写 Release 标题和描述
5. 点击 "Publish release"

### 4. 自动构建
- 发布 Release 后，GitHub Actions 会自动开始构建
- 构建完成后，会自动上传两个 zip 文件到 Release 页面：
  - `ImageBackgroundExpander-v1.0.0-Windows-x64.zip`
  - `ImageBackgroundExpander-v1.0.0-Windows-x64-Portable.zip`

## 生成的文件包含

- `ImageBackgroundExpander.exe` - 主程序
- Qt 运行时库（由 windeployqt 自动收集）
- `README.md` - 项目说明
- `README.txt` - 版本信息

## 注意事项

1. **Qt 版本**：使用 Qt 5.15.2，向下兼容项目要求的 5.14
2. **编译器**：使用 MSVC 2019，确保最佳兼容性
3. **架构**：仅构建 64 位版本 (x64)
4. **依赖**：自动包含所有必要的 Qt 库和依赖

## 故障排除

如果构建失败，可以：
1. 检查 GitHub Actions 的构建日志
2. 确认项目代码在本地能正常编译
3. 检查 Qt 版本兼容性
4. 验证 .pro 文件配置

## 后续扩展

可以考虑添加的功能：
- Linux 版本构建
- macOS 版本构建
- 自动化测试
- 代码签名
- 安装程序制作

---

配置完成时间：$(date)
负责人：AI Assistant
