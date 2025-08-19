@echo off
setlocal enabledelayedexpansion

echo 🚀 Starting Body Controller VSOMEIP Services...
echo.

REM 检查可执行文件是否存在
if not exist "build\bin\body_controller_services.exe" (
    echo ❌ Error: Executable not found. Please build the project first:
    echo    build.bat
    exit /b 1
)

REM 检查配置文件是否存在
if not exist "config\vsomeip_services.json" (
    echo ❌ Error: Configuration file not found: config\vsomeip_services.json
    exit /b 1
)

REM 设置环境变量
set "VSOMEIP_CONFIGURATION=%cd%\config\vsomeip_services.json"
set "VSOMEIP_APPLICATION_NAME=body_controller_services"

echo 📋 Configuration:
echo    Config File: %VSOMEIP_CONFIGURATION%
echo    Application: %VSOMEIP_APPLICATION_NAME%
echo    Working Dir: %cd%\build
echo.

REM 切换到构建目录
cd build

echo 🎯 Starting services...
echo    Press Ctrl+C to stop
echo.

REM 运行服务
.\bin\body_controller_services.exe

echo.
echo Services stopped.
pause
