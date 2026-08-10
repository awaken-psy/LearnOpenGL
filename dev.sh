#!/usr/bin/env bash
# ============================================================
# LearnOpenGL 开发脚本
#
# 简写规则（按优先级）：
#   ./dev.sh run 1/1.1        章节号/编号前缀，精确无歧义
#   ./dev.sh run 1.1          全局搜索，唯一直接用，歧义就列出选项
#   ./dev.sh run hello_window 文本片段匹配
#   ./dev.sh run 1.getting_started__1.1.hello_window  完整 target 名
# ============================================================
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "$0")" && pwd)"
BUILD_DIR="$REPO_ROOT/build"

RED='\033[0;31m'; GREEN='\033[0;32m'; YELLOW='\033[1;33m'; CYAN='\033[0;36m'; NC='\033[0m'
info()  { echo -e "${GREEN}[INFO]${NC}  $*"; }
warn()  { echo -e "${YELLOW}[WARN]${NC}  $*"; }
err()   { echo -e "${RED}[ERR]${NC}   $*" >&2; }

# ---------- 配置 ----------
detect_glm() {
  for candidate in \
    "${GLM_ROOT_DIR:-}" \
    "E:/tools/scoop/apps/glm/current/include" \
    "/e/tools/scoop/apps/glm/current/include" \
    "/usr/local/include" \
    "/usr/include"; do
    if [[ -f "$candidate/glm/glm.hpp" ]]; then echo "$candidate"; return 0; fi
  done
  local found; found=$(find /e/tools/scoop -path "*/glm/glm.hpp" 2>/dev/null | head -1)
  if [[ -n "$found" ]]; then echo "$(dirname "$(dirname "$found")")"; return 0; fi
  err "找不到 GLM，请设置 GLM_ROOT_DIR"
  return 1
}
detect_generator() {
  [[ "$OSTYPE" == msys || "$OSTYPE" == win32 || -n "${WINDIR:-}" ]] && echo "MinGW Makefiles" || echo "Unix Makefiles"
}

# ---------- 索引 ----------
# 输出格式: 章节序号 章节名 demo全名
# 章节序号 = CMakeLists.txt 中 CHAPTERS 列表的位置（1-based）
build_index() {
  awk '
  BEGIN {
    split("1.getting_started 2.lighting 3.model_loading 4.advanced_opengl 5.advanced_lighting 6.pbr 7.in_practice", ch_list)
    for (i in ch_list) ch_num[ch_list[i]] = i
    ch_num["8.guest"] = 8
  }
  /^[ \t]*set\([0-9]+\.[a-z_]+/ {
    chapter=$0; gsub(/.*set\(|\).*/,"",chapter); in_block=1; next
  }
  in_block && /^[ \t]*\)/ { in_block=0; next }
  in_block {
    gsub(/^[ \t]+|[ \t]+$/,"")
    if ($0 ~ /^[0-9]/ && chapter in ch_num) print ch_num[chapter] " " chapter " " $0
  }
  ' "$REPO_ROOT/CMakeLists.txt"
  awk '
  /^[ \t]*set\(GUEST_ARTICLES/,/^[ \t]*\)/ {
    gsub(/^[ \t]+|[ \t]+$/,"")
    if ($0~/^8\.guest/) print "8 8.guest " $0
  }' "$REPO_ROOT/CMakeLists.txt"
}

resolve_target() {
  local arg="${1:-}" arg2="${2:-}"
  local index; index="$(build_index)"
  local m

  # 两个参数: ./dev.sh run 1.getting_started 2.1 （旧格式兼容）
  if [[ -n "$arg2" ]]; then
    m=$(echo "$index" | awk -v ch="$arg" -v pat="$arg2" '$2==ch && $3~"^"pat"\\." {print $2"__"$3}')
    local c; c=$(echo "$m" | grep -c . 2>/dev/null || true)
    if   [[ "${c:-0}" -eq 0 ]]; then err "章节 \"$arg\" 中找不到 \"$arg2\""; return 1
    elif [[ "${c:-0}" -eq 1 ]]; then echo "$m"; return 0
    else err "$arg $arg2 匹配多个:"; echo "$m" | while read -r t; do [[ -n "$t" ]] && err "    $t"; done; return 1; fi
  fi

  # target 格式: chapter__demo
  if echo "$arg" | grep -q '__'; then echo "$arg"; return 0; fi

  # 章节/编号: ./dev.sh run 1/2.1 → 第1章中以 2.1 开头的 demo
  if echo "$arg" | grep -q '/'; then
    local ch_num="${arg%%/*}"
    local pat="${arg#*/}"
    m=$(echo "$index" | awk -v cn="$ch_num" -v pat="$pat" '$1==cn && $3~"^"pat"\\." {print $2"__"$3}')
    local c; c=$(echo "$m" | grep -c . 2>/dev/null || true)
    if   [[ "${c:-0}" -eq 0 ]]; then err "\"$arg\" 无匹配"; return 1
    elif [[ "${c:-0}" -eq 1 ]]; then echo "$m"; return 0
    else err "\"$arg\" 匹配多个:"; echo "$m" | while read -r t; do [[ -n "$t" ]] && err "    $t"; done; return 1; fi
  fi

  # 精确 demo 名
  m=$(echo "$index" | awk -v pat="$arg" '$3==pat {print $2"__"$3}')
  local c; c=$(echo "$m" | grep -c . 2>/dev/null || true)
  if [[ "${c:-0}" -eq 1 ]]; then echo "$m"; return 0; fi

  # 前缀匹配（数字简写）
  m=$(echo "$index" | awk -v pat="$arg" '$3~"^"pat"\\." {print $2"__"$3}')
  c=$(echo "$m" | grep -c . 2>/dev/null || true)
  if   [[ "${c:-0}" -eq 1 ]]; then echo "$m"; return 0
  elif [[ "${c:-0}" -gt 1 ]]; then err "\"$arg\" 匹配多个:"; echo "$m" | while read -r t; do [[ -n "$t" ]] && err "    $t"; done; return 1; fi

  # 包含匹配
  m=$(echo "$index" | awk -v pat="$arg" 'index($3,pat) {print $2"__"$3}')
  c=$(echo "$m" | grep -c . 2>/dev/null || true)
  if   [[ "${c:-0}" -eq 0 ]]; then err "\"$arg\" 无匹配，运行 ./dev.sh list"; return 1
  elif [[ "${c:-0}" -eq 1 ]]; then echo "$m"; return 0
  else err "\"$arg\" 匹配多个:"; echo "$m" | while read -r t; do [[ -n "$t" ]] && err "    $t"; done; return 1; fi
}

# ---------- 命令 ----------
cmd_configure() {
  local glm_root; glm_root="$(detect_glm)" || exit 1
  info "GLM: $glm_root"
  cmake -S "$REPO_ROOT" -B "$BUILD_DIR" \
    -DCMAKE_BUILD_TYPE=Debug -DCMAKE_POLICY_VERSION_MINIMUM=3.5 \
    -DGLM_ROOT_DIR="$glm_root" -G "$(detect_generator)"
  info "配置完成"
}

cmd_build() {
  [[ ! -d "$BUILD_DIR" ]] && { warn "自动 configure..."; cmd_configure; }
  local njobs; njobs=$(nproc 2>/dev/null || echo 4)
  if [[ $# -eq 0 ]]; then
    info "编译全部（j$njobs）..."
    cmake --build "$BUILD_DIR" -j"$njobs"
  else
    local t; t="$(resolve_target "$@")" || exit 1
    info "编译: $t"
    cmake --build "$BUILD_DIR" --target "$t" -j"$njobs"
  fi
}

cmd_run() {
  [[ $# -eq 0 ]] && { err "用法: ./dev.sh run <简写>"; err "  ./dev.sh run 1/1.1 | 1.1 | hello_window"; exit 1; }
  local t; t="$(resolve_target "$@")" || exit 1
  local ch="${t%%__*}"
  local exe_dir="$REPO_ROOT/bin/$ch"
  local exe="$exe_dir/${t}"
  [[ -f "$exe.exe" ]] && exe="$exe.exe"
  cmd_build "$t"
  info "运行: $exe"
  # 切到 exe 所在目录，shader 文件靠相对路径查找
  cd "$exe_dir" && "$exe"
}

cmd_list() {
  echo ""; echo -e "${CYAN}demo 列表（编号 = 章节序号/编号前缀）${NC}"; echo "=============================="
  local prev=""
  build_index | while read -r cn ch demo; do
    if [[ "$ch" != "$prev" ]]; then echo ""; echo -e "${YELLOW}$cn  $ch${NC}"; prev="$ch"; fi
    printf "    %-40s → %s\n" "$demo" "$cn/$(echo "$demo" | grep -oP '^\d+\.\d+')"
  done
  echo ""
}

# ---------- 入口 ----------
case "${1:-}" in
  configure) shift; cmd_configure "$@" ;;
  build)     shift; cmd_build "$@" ;;
  run)       shift; cmd_run "$@" ;;
  list|ls)   cmd_list ;;
  *) echo ""; echo -e "${CYAN}用法${NC}"; echo "===="
     echo "  ./dev.sh run 1/2.1       章节序号/编号前缀（无歧义）"
     echo "  ./dev.sh run 2.1         全局搜索"
     echo "  ./dev.sh run hello_window 文本匹配"
     echo "  ./dev.sh build | list | configure"
     echo "" ;;
esac
