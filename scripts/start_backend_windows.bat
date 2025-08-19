@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 车身域控制器后端服务启动脚本 (Windows)
echo ========================================
echo.

:: 设置颜色
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

:: 设置工作目录
set "SCRIPT_DIR=%~dp0"
set "PROJECT_ROOT=%SCRIPT_DIR%.."
cd /d "%PROJECT_ROOT%"

echo %BLUE%[INFO]%NC% 项目根目录: %PROJECT_ROOT%
echo.

:: 检查build目录
if not exist "build" (
    echo %RED%[ERROR]%NC% build目录不存在
    echo 请先运行编译脚本: scripts\build_windows.bat
    pause
    exit /b 1
)

cd build

:: 查找可执行文件
echo %BLUE%[INFO]%NC% 查找可执行文件...

set "API_SERVER="
set "WS_SERVER="
set "POSSIBLE_PATHS=bin Release\bin Debug\bin ."

for %%p in (%POSSIBLE_PATHS%) do (
    if exist "%%p\body_controller_web_server.exe" (
        set "API_SERVER=%%p\body_controller_web_server.exe"
        echo %GREEN%[FOUND]%NC% REST API服务器: !API_SERVER!
    )
    if exist "%%p\body_controller_websocket_server.exe" (
        set "WS_SERVER=%%p\body_controller_websocket_server.exe"
        echo %GREEN%[FOUND]%NC% WebSocket服务器: !WS_SERVER!
    )
)

:: 检查是否找到可执行文件
if "!API_SERVER!"=="" (
    echo %RED%[ERROR]%NC% 未找到 body_controller_web_server.exe
    echo 请确认编译是否成功
    echo.
    echo 当前目录内容:
    dir /s *.exe
    pause
    exit /b 1
)

if "!WS_SERVER!"=="" (
    echo %RED%[ERROR]%NC% 未找到 body_controller_websocket_server.exe
    echo 请确认编译是否成功
    pause
    exit /b 1
)

echo.

:: 检查端口是否被占用
echo %BLUE%[INFO]%NC% 检查端口占用情况...

netstat -an | findstr ":8080" >nul
if !errorlevel! == 0 (
    echo %YELLOW%[WARNING]%NC% 端口8080已被占用
    netstat -ano | findstr ":8080"
    set /p "CONTINUE=是否继续启动? (y/N): "
    if /i not "!CONTINUE!"=="y" (
        echo 启动已取消
        pause
        exit /b 1
    )
)

netstat -an | findstr ":8081" >nul
if !errorlevel! == 0 (
    echo %YELLOW%[WARNING]%NC% 端口8081已被占用
    netstat -ano | findstr ":8081"
    set /p "CONTINUE=是否继续启动? (y/N): "
    if /i not "!CONTINUE!"=="y" (
        echo 启动已取消
        pause
        exit /b 1
    )
)

echo.
echo ========================================
echo 启动后端服务...
echo ========================================
echo.

:: 启动REST API服务器
echo %BLUE%[INFO]%NC% 启动REST API服务器 (端口8080)...
start "Body Controller - REST API Server" cmd /k "title Body Controller - REST API Server && echo. && echo ========================================== && echo 车身域控制器 REST API 服务器 && echo ========================================== && echo. && echo 服务器地址: http://localhost:8080 && echo API端点: http://localhost:8080/api && echo Web界面: http://localhost:8080 && echo. && echo 按 Ctrl+C 停止服务 && echo. && "!API_SERVER!""

:: 等待3秒让API服务器启动
echo %BLUE%[INFO]%NC% 等待API服务器启动...
timeout /t 3 /nobreak >nul

:: 启动WebSocket服务器
echo %BLUE%[INFO]%NC% 启动WebSocket服务器 (端口8081)...
start "Body Controller - WebSocket Server" cmd /k "title Body Controller - WebSocket Server && echo. && echo ========================================== && echo 车身域控制器 WebSocket 服务器 && echo ========================================== && echo. && echo 服务器地址: ws://localhost:8081 && echo 实时事件推送服务 && echo. && echo 按 Ctrl+C 停止服务 && echo. && "!WS_SERVER!""

:: 等待2秒让WebSocket服务器启动
echo %BLUE%[INFO]%NC% 等待WebSocket服务器启动...
timeout /t 2 /nobreak >nul

echo.
echo %GREEN%[SUCCESS]%NC% 后端服务启动完成！
echo.
echo ========================================
echo 服务信息
echo ========================================
echo %BLUE%REST API服务器:%NC%    http://localhost:8080
echo %BLUE%WebSocket服务器:%NC%   ws://localhost:8081
echo %BLUE%Web管理界面:%NC%       http://localhost:8080
echo %BLUE%API文档:%NC%           http://localhost:8080/api/docs
echo.
echo ========================================
echo 下一步操作
echo ========================================
echo 1. 测试连接:        scripts\test_connection_windows.bat
echo 2. 启动前端:        scripts\start_frontend.bat
echo 3. 集成测试:        打开 scripts\test_integration.html
echo 4. 查看日志:        查看服务器窗口输出
echo.
echo %YELLOW%注意:%NC% 请保持此窗口打开，关闭将停止服务器
echo.
echo 按任意键退出...
pause >nul
