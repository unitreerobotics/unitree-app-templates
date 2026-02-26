#!/bin/bash
set -e  # 出错时立即退出

# 清理旧文件
rm -rf build out

# 配置 CMake
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release

# 编译
cmake --build build -j

# 安装到 out 目录
cmake --install build --prefix out

echo "✔ Build & Install finished. Output in ./out"
