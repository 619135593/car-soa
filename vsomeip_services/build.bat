@echo off
setlocal enabledelayedexpansion

echo 🏗️  Building Body Controller VSOMEIP Services...
echo.

REM 检查是否在正确的目录
if not exist "CMakeLists.txt" (
    echo ❌ Error: CMakeLists.txt not found. Please run this script from the vsomeip_services directory.
    exit /b 1
)

REM 创建构建目录
echo 📁 Creating build directory...
if not exist "build" mkdir build
cd build

REM 清理之前的构建
echo 🧹 Cleaning previous build...
del /q /s * 2>nul

REM 配置项目
echo ⚙️  Configuring project...
cmake .. -DCMAKE_BUILD_TYPE=Release
if errorlevel 1 (
    echo ❌ CMake configuration failed!
    exit /b 1
)

REM 编译项目
echo 🔨 Compiling project...
cmake --build . --config Release -j 4
if errorlevel 1 (
    echo ❌ Compilation failed!
    exit /b 1
)

REM 检查编译结果
if exist "bin\body_controller_services.exe" (
    echo.
    echo ✅ Build completed successfully!
    echo.
    echo 📋 Build Results:
    echo    Executable: .\bin\body_controller_services.exe
    echo    Config:     ..\config\vsomeip_services.json
    echo.
    echo 🚀 To run the services:
    echo    cd build
    echo    set VSOMEIP_CONFIGURATION=..\config\vsomeip_services.json
    echo    set VSOMEIP_APPLICATION_NAME=body_controller_services
    echo    .\bin\body_controller_services.exe
    echo.
) else (
    echo.
    echo ❌ Build failed! Executable not found.
    echo Please check the compilation errors above.
    exit /b 1
)

echo Press any key to continue...
pause >nul
