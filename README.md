# WordMemorizer - 单词记忆助手

## 项目简介

WordMemorizer 是一款基于 Qt 框架开发的单词记忆辅助工具，帮助用户高效地管理和记忆单词。该应用程序提供了直观的界面，支持添加、删除、修改单词及其释义，设置单词优先级，搜索单词以及灵活的单词列表排序功能。

## 功能特点

-   **单词管理**：添加、删除和修改单词及其释义
-   **优先级设置**：为单词设置优先级（1-20），便于重点记忆
-   **搜索功能**：支持快速搜索单词，带有自动补全功能
-   **灵活排序**：支持按创建时间、修改时间和优先级进行升序或降序排列
-   **用户友好界面**：简洁直观的界面设计，提供良好的用户体验
-   **数据持久化**：单词数据保存到 JSON 文件中，保证数据不丢失

## 项目结构

```
├── main.cpp                   # 程序入口
├── wordmemorizer.hpp          # 主窗口类头文件
├── wordmemorizer.cpp          # 主窗口类实现
├── worddatabase.hpp           # 单词数据库类头文件
├── worddatabase.cpp           # 单词数据库类实现
├── resources.qrc              # Qt资源文件
├── styles/                    # 样式表文件
│   └── main.qss               # 主样式表
├── icons/                     # 图标资源
│   ├── logo.png               # 应用程序图标
│   ├── up_arrow_normal.png    # 上箭头正常状态图标
│   ├── up_arrow_pressed.png   # 上箭头按下状态图标
│   ├── up_arrow_hover.png     # 上箭头悬停状态图标
│   ├── down_arrow_normal.png  # 下箭头正常状态图标
│   ├── down_arrow_pressed.png # 下箭头按下状态图标
│   ├── down_arrow_hover.png   # 下箭头悬停状态图标
│   └── dropdown.png           # 下拉菜单图标
└── CMakeLists.txt             # CMake构建文件
```

## 技术栈

-   C++ 17
-   Qt 6（或 Qt 5）
-   CMake 3.16+

## 安装与配置

### 前提条件

-   已安装 Qt 开发环境（Qt 5 或 Qt 6）
-   已安装 CMake 3.16 或更高版本
-   兼容的 C++编译器（支持 C++17 标准）

### 构建步骤

#### 使用 Qt Creator 构建

1. 打开 Qt Creator
2. 选择"文件" > "打开文件或项目"
3. 选择项目目录下的`CMakeLists.txt`文件
4. 按照向导完成项目配置
5. 点击"构建" > "构建项目 'WordMemorizer'"
6. 构建完成后，可以直接运行应用程序

#### 使用命令行构建

```bash
# 克隆项目
git clone https://github.com/pzzzyda/WordMemorizer.git

# 进入项目目录
cd WordMemorizer

# 创建构建目录
mkdir build
cd build

# 配置项目
cmake ..

# 构建项目
cmake --build . --config Release

# 运行应用程序
# 在Windows上
Release\WordMemorizer.exe
# 在Linux/macOS上
./WordMemorizer
```

## 使用说明

### 添加单词

1. 在"单词"输入框中输入单词
2. 在"输入释义"框中输入单词的释义，然后点击"添加释义"按钮
3. 可以添加多个释义
4. 设置单词的优先级（1-20，数字越大优先级越高）
5. 点击"添加单词"按钮保存单词

### 管理单词

-   **删除单词**：在单词列表中选择要删除的单词，然后点击"删除选中单词"按钮
-   **修改单词**：通过右键菜单选择"编辑单词"选项
-   **查看单词详情**：在单词列表中选中单词，即可查看其完整释义

### 搜索单词

1. 在搜索框中输入要查找的单词或部分内容
2. 搜索结果会实时显示在单词列表中
3. 点击"取消"按钮可返回完整单词列表

### 排序单词

-   使用单词列表上方的下拉菜单选择排序方式
-   支持按创建时间、修改时间和优先级进行升序或降序排列

## 数据存储

-   单词数据以 JSON 格式存储在应用程序的数据目录中
-   在 Windows 系统上，数据文件通常位于`%APPDATA%\WordMemorizer\words.json`
-   在 Linux 系统上，数据文件通常位于`~/.local/share/WordMemorizer/words.json`
-   在 macOS 系统上，数据文件通常位于`~/Library/Application Support/WordMemorizer/words.json`

## 开发说明

### 代码风格

项目使用了基于 Clang-Format 的代码风格，配置文件为`.clang-format`。

### 资源管理

项目使用 Qt 的资源系统（`.qrc`文件）管理图标和样式表等资源。

### 项目依赖

-   Qt Widgets 模块：用于创建 GUI 界面
-   C++标准库：用于基本数据结构和算法

## 许可证

[MIT License](https://opensource.org/licenses/MIT)

## 致谢

-   感谢 Qt 框架提供的强大 GUI 开发支持
-   感谢所有为该项目做出贡献的开发者

## 联系方式

-   项目维护者：[pzzzyda](https://github.com/pzzzyda)
-   邮箱：[zhongyuan03@outlook.com](mailto:zhongyuan03@outlook.com)

如有任何问题或建议，请随时联系项目维护者。
