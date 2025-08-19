#!/bin/bash

echo "=== 测试CMake配置 ==="

# 清理之前的构建
echo "1. 清理之前的构建目录..."
rm -rf build
mkdir build
cd build

echo ""
echo "2. 运行CMake配置..."
cmake .. -DCMAKE_BUILD_TYPE=Debug

echo ""
echo "3. 检查CMake配置结果..."
if [ $? -eq 0 ]; then
    echo "✅ CMake配置成功"
    
    echo ""
    echo "4. 尝试构建一个测试目标..."
    make test_door_client -j$(nproc)
    
    if [ $? -eq 0 ]; then
        echo "✅ 构建成功！"
        echo ""
        echo "5. 检查生成的可执行文件..."
        ls -la bin/test_door_client 2>/dev/null && echo "✅ test_door_client 生成成功" || echo "❌ test_door_client 未生成"
    else
        echo "❌ 构建失败"
    fi
else
    echo "❌ CMake配置失败"
fi

echo ""
echo "=== 测试完成 ==="
