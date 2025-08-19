#!/bin/bash

# Body Controller VSOMEIP Services 运行脚本

set -e

echo "🚀 Starting Body Controller VSOMEIP Services..."
echo ""

# 检查可执行文件是否存在
if [ ! -f "build/bin/body_controller_services" ]; then
    echo "❌ Error: Executable not found. Please build the project first:"
    echo "   ./build.sh"
    exit 1
fi

# 检查配置文件是否存在
if [ ! -f "config/vsomeip_services.json" ]; then
    echo "❌ Error: Configuration file not found: config/vsomeip_services.json"
    exit 1
fi

# 设置环境变量
export VSOMEIP_CONFIGURATION="$(pwd)/config/vsomeip_services.json"
export VSOMEIP_APPLICATION_NAME="body_controller_services"

echo "📋 Configuration:"
echo "   Config File: $VSOMEIP_CONFIGURATION"
echo "   Application: $VSOMEIP_APPLICATION_NAME"
echo "   Working Dir: $(pwd)/build"
echo ""

# 切换到构建目录
cd build

echo "🎯 Starting services..."
echo "   Press Ctrl+C to stop"
echo ""

# 运行服务
./bin/body_controller_services
