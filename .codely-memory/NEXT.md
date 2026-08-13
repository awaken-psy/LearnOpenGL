# Next Session

> 记忆路径: `C:\Users\chang.wei\cw\LearnOpenGL\.codely-memory\`

Updated: 2026-08-11 17:37

## 上次做了什么

完成了 LearnOpenGL 第二章 Lighting 全章学习。修复了 dev.sh 构建工具链在 Nano 机器上的兼容性问题（VS 2022 Build Tools 安装、generator 自动检测、GLM 路径、Debug 子目录运行路径、C4819 警告）。将 2.4（观察空间光照）和 2.5（Gouraud 着色）从 `#if 0` 注释块改造为完整可运行项目。已 commit (`a55bb76`) 并 push。

## 从这里继续

- 开始第三章 **Model Loading**（`src/3.model_loading/1.model_loading/`）
- 用 `./dev.sh run 3/1` 运行
- 第三章引入 Assimp 模型导入，使用 `includes/learnopengl/model.h` 和 `mesh.h`

## 未完成

- 无

## 注意事项

- 这台机器（Nano）用 VS 2022 Build Tools 构建，`dev.sh` 会自动检测
- 如果换到另一台机器（MinGW），`dev.sh` 会自动 fallback
- `CODELY.md` 和 `.codely-cli/` 已加入 `.gitignore`，不会被提交

## 相关文件

- `dev.sh` — 构建脚本，所有 demo 的 configure/build/run 入口
- `CMakeLists.txt` — 项目构建配置，CHAPTERS 和 demo 列表
- `src/3.model_loading/1.model_loading/` — 下一章入口
- `includes/learnopengl/model.h` — Assimp 模型加载器
- `includes/learnopengl/mesh.h` — Mesh 结构体
