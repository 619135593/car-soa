# Windows环境配置指南

## 🛠️ 开发工具安装

### 1. Visual Studio 2022 Community (推荐)

**下载地址**: https://visualstudio.microsoft.com/zh-hans/vs/community/

**安装组件**:
- ✅ 使用C++的桌面开发
- ✅ CMake工具
- ✅ Git for Windows
- ✅ Windows 10/11 SDK

**验证安装**:
```cmd
# 检查编译器
cl
# 检查CMake
cmake --version
# 检查Git
git --version
```

### 2. 替代方案：MinGW-w64 + CMake

如果不想安装Visual Studio，可以使用轻量级方案：

**安装MinGW-w64**:
1. 下载MSYS2: https://www.msys2.org/
2. 安装后运行MSYS2终端
3. 执行以下命令：
```bash
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-make
```

**安装CMake** (独立版本):
- 下载: https://cmake.org/download/
- 选择Windows x64 Installer
- 安装时选择"Add CMake to system PATH"

### 3. Python环境

**下载Python 3.8+**: https://www.python.org/downloads/windows/

**安装时注意**:
- ✅ 勾选"Add Python to PATH"
- ✅ 勾选"Install pip"

**验证安装**:
```cmd
python --version
pip --version
```

## 🔧 环境变量配置

### 添加到系统PATH
```
C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Tools\MSVC\14.xx.xxxxx\bin\Hostx64\x64
C:\Program Files\CMake\bin
C:\Program Files\Git\bin
C:\Users\%USERNAME%\AppData\Local\Programs\Python\Python3x
```

### 设置开发环境变量
```cmd
# 创建环境变量批处理文件
echo @echo off > setup_env.bat
echo set VCPKG_ROOT=C:\vcpkg >> setup_env.bat
echo set CMAKE_TOOLCHAIN_FILE=C:\vcpkg\scripts\buildsystems\vcpkg.cmake >> setup_env.bat
echo call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat" >> setup_env.bat
```

## 📦 依赖管理 (可选)

### 使用vcpkg管理C++依赖

**安装vcpkg**:
```cmd
git clone https://github.com/Microsoft/vcpkg.git C:\vcpkg
cd C:\vcpkg
.\bootstrap-vcpkg.bat
.\vcpkg integrate install
```

**安装常用库**:
```cmd
.\vcpkg install nlohmann-json:x64-windows
.\vcpkg install httplib:x64-windows
.\vcpkg install websocketpp:x64-windows
```

## ✅ 验证开发环境

创建测试文件 `test_env.cpp`:
```cpp
#include <iostream>
int main() {
    std::cout << "C++ environment is ready!" << std::endl;
    return 0;
}
```

编译测试:
```cmd
# 使用Visual Studio
cl test_env.cpp

# 使用MinGW
g++ test_env.cpp -o test_env.exe
```

## 🚨 常见问题

### 问题1: 找不到编译器
**解决方案**: 运行Visual Studio Developer Command Prompt

### 问题2: CMake找不到编译器
**解决方案**: 
```cmd
cmake -G "Visual Studio 17 2022" -A x64 ..
```

### 问题3: 权限不足
**解决方案**: 以管理员身份运行命令提示符

### 问题4: 路径包含空格
**解决方案**: 使用引号包围路径
```cmd
"C:\Program Files\CMake\bin\cmake.exe" --version
```
