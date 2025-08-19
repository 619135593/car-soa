@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 车身域控制器连接测试 (Windows)
echo ========================================
echo.

:: 设置颜色
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

:: 检查必要工具
echo %BLUE%[INFO]%NC% 检查测试工具...

:: 检查curl
curl --version >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%[WARNING]%NC% curl 未安装，将使用PowerShell进行测试
    set "USE_CURL=0"
) else (
    echo %GREEN%[OK]%NC% curl 可用
    set "USE_CURL=1"
)

:: 检查Python
python --version >nul 2>&1
if errorlevel 1 (
    echo %YELLOW%[WARNING]%NC% Python 未安装，WebSocket测试将跳过
    set "USE_PYTHON=0"
) else (
    echo %GREEN%[OK]%NC% Python 可用
    set "USE_PYTHON=1"
)

echo.

:: 测试1: 检查端口监听状态
echo ========================================
echo 测试1: 检查端口监听状态
echo ========================================

echo %BLUE%[INFO]%NC% 检查端口8080 (REST API)...
netstat -an | findstr ":8080.*LISTENING" >nul
if !errorlevel! == 0 (
    echo %GREEN%[PASS]%NC% 端口8080正在监听
    set "PORT_8080=1"
) else (
    echo %RED%[FAIL]%NC% 端口8080未监听
    set "PORT_8080=0"
)

echo %BLUE%[INFO]%NC% 检查端口8081 (WebSocket)...
netstat -an | findstr ":8081.*LISTENING" >nul
if !errorlevel! == 0 (
    echo %GREEN%[PASS]%NC% 端口8081正在监听
    set "PORT_8081=1"
) else (
    echo %RED%[FAIL]%NC% 端口8081未监听
    set "PORT_8081=0"
)

echo.

:: 测试2: REST API连接测试
echo ========================================
echo 测试2: REST API连接测试
echo ========================================

if !PORT_8080! == 0 (
    echo %RED%[SKIP]%NC% 端口8080未监听，跳过API测试
    goto :websocket_test
)

if !USE_CURL! == 1 (
    :: 使用curl测试
    echo %BLUE%[INFO]%NC% 使用curl测试API连接...
    
    echo %BLUE%[TEST]%NC% 测试健康检查端点...
    curl -s -o nul -w "HTTP状态码: %%{http_code}, 响应时间: %%{time_total}s" http://localhost:8080/api/health
    echo.
    
    echo %BLUE%[TEST]%NC% 测试系统信息端点...
    curl -s -o nul -w "HTTP状态码: %%{http_code}, 响应时间: %%{time_total}s" http://localhost:8080/api/system/info
    echo.
    
    echo %BLUE%[TEST]%NC% 获取健康检查详细信息...
    curl -s http://localhost:8080/api/health
    echo.
    
) else (
    :: 使用PowerShell测试
    echo %BLUE%[INFO]%NC% 使用PowerShell测试API连接...
    
    powershell -Command "try { $response = Invoke-WebRequest -Uri 'http://localhost:8080/api/health' -TimeoutSec 5; Write-Host '[PASS] API健康检查成功 - 状态码:' $response.StatusCode -ForegroundColor Green; Write-Host '响应内容:' $response.Content } catch { Write-Host '[FAIL] API健康检查失败:' $_.Exception.Message -ForegroundColor Red }"
    echo.
)

:websocket_test
:: 测试3: WebSocket连接测试
echo ========================================
echo 测试3: WebSocket连接测试
echo ========================================

if !PORT_8081! == 0 (
    echo %RED%[SKIP]%NC% 端口8081未监听，跳过WebSocket测试
    goto :browser_test
)

if !USE_PYTHON! == 1 (
    echo %BLUE%[INFO]%NC% 使用Python测试WebSocket连接...
    
    :: 创建临时Python测试脚本
    echo import websocket > temp_ws_test.py
    echo import json >> temp_ws_test.py
    echo import time >> temp_ws_test.py
    echo import threading >> temp_ws_test.py
    echo. >> temp_ws_test.py
    echo def on_message(ws, message): >> temp_ws_test.py
    echo     print(f"收到消息: {message}") >> temp_ws_test.py
    echo. >> temp_ws_test.py
    echo def on_error(ws, error): >> temp_ws_test.py
    echo     print(f"WebSocket错误: {error}") >> temp_ws_test.py
    echo. >> temp_ws_test.py
    echo def on_close(ws, close_status_code, close_msg): >> temp_ws_test.py
    echo     print("WebSocket连接已关闭") >> temp_ws_test.py
    echo. >> temp_ws_test.py
    echo def on_open(ws): >> temp_ws_test.py
    echo     print("[PASS] WebSocket连接成功") >> temp_ws_test.py
    echo     ws.send(json.dumps({"type": "ping", "timestamp": time.time()})) >> temp_ws_test.py
    echo     time.sleep(1) >> temp_ws_test.py
    echo     ws.close() >> temp_ws_test.py
    echo. >> temp_ws_test.py
    echo try: >> temp_ws_test.py
    echo     ws = websocket.WebSocketApp("ws://localhost:8081", >> temp_ws_test.py
    echo                                on_open=on_open, >> temp_ws_test.py
    echo                                on_message=on_message, >> temp_ws_test.py
    echo                                on_error=on_error, >> temp_ws_test.py
    echo                                on_close=on_close) >> temp_ws_test.py
    echo     ws.run_forever() >> temp_ws_test.py
    echo except Exception as e: >> temp_ws_test.py
    echo     print(f"[FAIL] WebSocket连接失败: {e}") >> temp_ws_test.py
    
    python temp_ws_test.py
    del temp_ws_test.py
    
) else (
    echo %YELLOW%[SKIP]%NC% Python不可用，跳过WebSocket测试
    echo %BLUE%[INFO]%NC% 可以手动测试WebSocket连接:
    echo 1. 安装Python: https://www.python.org/downloads/
    echo 2. 安装websocket-client: pip install websocket-client
    echo 3. 重新运行此脚本
)

echo.

:browser_test
:: 测试4: 浏览器测试指导
echo ========================================
echo 测试4: 浏览器测试指导
echo ========================================

echo %BLUE%[INFO]%NC% 浏览器测试步骤:
echo.
echo 1. 打开浏览器，访问: http://localhost:8080
echo 2. 按F12打开开发者工具
echo 3. 切换到Network(网络)标签
echo 4. 刷新页面，检查是否有404或500错误
echo 5. 切换到Console(控制台)标签，检查JavaScript错误
echo 6. 在Console中输入: fetch('/api/health').then(r=>r.json()).then(console.log)
echo 7. 检查WebSocket连接状态指示器
echo.

set /p "OPEN_BROWSER=是否现在打开浏览器进行测试? (y/N): "
if /i "!OPEN_BROWSER!"=="y" (
    echo %BLUE%[INFO]%NC% 正在打开浏览器...
    start http://localhost:8080
    timeout /t 2 /nobreak >nul
    
    echo %BLUE%[INFO]%NC% 正在打开集成测试页面...
    start "%~dp0test_integration.html"
)

echo.

:: 测试总结
echo ========================================
echo 测试总结
echo ========================================

set "TOTAL_TESTS=0"
set "PASSED_TESTS=0"

set /a TOTAL_TESTS+=1
if !PORT_8080! == 1 (
    set /a PASSED_TESTS+=1
    echo %GREEN%[PASS]%NC% REST API端口监听
) else (
    echo %RED%[FAIL]%NC% REST API端口监听
)

set /a TOTAL_TESTS+=1
if !PORT_8081! == 1 (
    set /a PASSED_TESTS+=1
    echo %GREEN%[PASS]%NC% WebSocket端口监听
) else (
    echo %RED%[FAIL]%NC% WebSocket端口监听
)

echo.
echo 测试结果: !PASSED_TESTS!/!TOTAL_TESTS! 通过

if !PASSED_TESTS! == !TOTAL_TESTS! (
    echo %GREEN%[SUCCESS]%NC% 所有基础连接测试通过！
    echo.
    echo 下一步操作:
    echo 1. 启动前端界面: scripts\start_frontend.bat
    echo 2. 进行功能测试: 打开 scripts\test_integration.html
    echo 3. 查看API文档: http://localhost:8080/api/docs
) else (
    echo %RED%[ERROR]%NC% 部分测试失败，请检查:
    echo 1. 后端服务是否正确启动
    echo 2. 防火墙是否阻止了端口访问
    echo 3. 是否有其他程序占用了端口
    echo.
    echo 故障排除:
    echo 1. 重新启动后端服务: scripts\start_backend_windows.bat
    echo 2. 检查服务器日志输出
    echo 3. 确认编译是否成功
)

echo.
echo 按任意键退出...
pause >nul
