# 构建工具链 (dev.sh / CMake)

Last updated: 2026-08-11 17:37

> 记忆路径: `C:\Users\chang.wei\cw\LearnOpenGL\.codely-memory\`

## 概述

LearnOpenGL 项目的构建脚本 `dev.sh` 和 `CMakeLists.txt` 的配置。支持 Windows（MSVC + MinGW）和 macOS/Linux。项目自带 MSVC 编译的 `.lib` 文件，需要用 Visual Studio generator 构建才能链接。

## 当前状态

- **Nano 机器（当前）**：安装了 VS 2022 Build Tools，`dev.sh` 自动检测并使用 "Visual Studio 17 2022" generator。GLM 从项目 `includes/` 目录加载。
- **另一台机器**：使用 MinGW (GCC)，`lib/` 里的库是 MinGW 编译的，兼容。
- `dev.sh` 的 `detect_generator` 函数自动选择：有 VS 2022 就用 MSVC，没有就 fallback 到 MinGW。

## 关键知识

- `lib/` 下的 `.lib` 文件是 MSVC 编译的，MinGW 的 `ld` 无法链接（缺 `__security_cookie` 等符号）
- VS 生成器把 exe 放到 `bin/<chapter>/Debug/` 子目录，MinGW 直接放 `bin/<chapter>/`
- `dev.sh` 的 `cmd_run` 必须在编译后检测 Debug/ 路径（编译前 exe 不存在）
- MSVC 需要 `/utf-8` 编译选项消除 C4819 警告（源文件含中文字符）
- `/ignore:4099` 是 MSVC 专用链接选项，不能放在非 MSVC 分支
- `cmake/modules/` 下有自定义 Find 模块：FindGLM.cmake、FindGLFW3.cmake、FindASSIMP.cmake
- GLM 路径检测顺序：`$GLM_ROOT_DIR` → 项目 `includes/` → Scoop 路径 → 系统路径
