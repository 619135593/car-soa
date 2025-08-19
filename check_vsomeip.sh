#!/bin/bash

echo "=== 检查vsomeip安装状态 ==="

echo "1. 检查/usr/local/lib中的vsomeip库文件："
ls -la /usr/local/lib/ | grep vsomeip || echo "未找到vsomeip库文件"

echo ""
echo "2. 检查/usr/local/include中的vsomeip头文件："
ls -la /usr/local/include/ | grep vsomeip || echo "未找到vsomeip头文件目录"

echo ""
echo "3. 检查vsomeip.hpp头文件是否存在："
if [ -f "/usr/local/include/vsomeip/vsomeip.hpp" ]; then
    echo "✅ 找到 /usr/local/include/vsomeip/vsomeip.hpp"
else
    echo "❌ 未找到 /usr/local/include/vsomeip/vsomeip.hpp"
fi

echo ""
echo "4. 检查pkg-config配置："
pkg-config --exists vsomeip3 && echo "✅ 找到vsomeip3 pkg-config" || echo "❌ 未找到vsomeip3 pkg-config"
pkg-config --exists vsomeip && echo "✅ 找到vsomeip pkg-config" || echo "❌ 未找到vsomeip pkg-config"

echo ""
echo "5. 检查CMake配置文件："
find /usr/local -name "*vsomeip*Config.cmake" 2>/dev/null || echo "未找到vsomeip CMake配置文件"

echo ""
echo "6. 检查库文件详情："
find /usr/local/lib -name "*vsomeip*" -type f 2>/dev/null || echo "未找到vsomeip库文件"

echo ""
echo "7. 检查动态链接库缓存："
ldconfig -p | grep vsomeip || echo "动态链接库缓存中未找到vsomeip"
