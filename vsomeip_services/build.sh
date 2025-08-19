#!/bin/bash

# Body Controller VSOMEIP Services 构建脚本

set -e  # 遇到错误立即退出

echo "🏗️  Building Body Controller VSOMEIP Services..."
echo ""

# 检查是否在正确的目录
if [ ! -f "CMakeLists.txt" ]; then
    echo "❌ Error: CMakeLists.txt not found. Please run this script from the vsomeip_services directory."
    exit 1
fi

# 创建构建目录
echo "📁 Creating build directory..."
mkdir -p build
cd build

# 清理之前的构建
echo "🧹 Cleaning previous build..."
rm -rf *

# 配置项目
echo "⚙️  Configuring project..."
cmake .. -DCMAKE_BUILD_TYPE=Release

# 编译项目
echo "🔨 Compiling project..."
make -j$(nproc)

# 检查编译结果
if [ -f "bin/body_controller_services" ]; then
    echo ""
    echo "✅ Build completed successfully!"
    echo ""
    echo "📋 Build Results:"
    echo "   Executable: ./bin/body_controller_services"
    echo "   Config:     ../config/vsomeip_services.json"
    echo ""
    echo "🚀 To run the services:"
    echo "   cd build"
    echo "   export VSOMEIP_CONFIGURATION=../config/vsomeip_services.json"
    echo "   export VSOMEIP_APPLICATION_NAME=body_controller_services"
    echo "   ./bin/body_controller_services"
    echo ""
else
    echo ""
    echo "❌ Build failed! Executable not found."
    echo "Please check the compilation errors above."
    exit 1
fi
