@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 第一阶段：基础连接验证
echo ========================================
echo.

:: 设置颜色
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

echo %BLUE%[阶段1]%NC% 验证HTTP API和WebSocket基础连接
echo.

:: 步骤1: 检查后端服务状态
echo %BLUE%[步骤1]%NC% 检查后端服务状态...

echo %BLUE%[检查]%NC% REST API服务器 (端口8080)
netstat -an | findstr ":8080.*LISTENING" >nul
if !errorlevel! == 0 (
    echo %GREEN%[✓]%NC% REST API服务器正在运行
    set "API_RUNNING=1"
) else (
    echo %RED%[✗]%NC% REST API服务器未运行
    set "API_RUNNING=0"
)

echo %BLUE%[检查]%NC% WebSocket服务器 (端口8081)
netstat -an | findstr ":8081.*LISTENING" >nul
if !errorlevel! == 0 (
    echo %GREEN%[✓]%NC% WebSocket服务器正在运行
    set "WS_RUNNING=1"
) else (
    echo %RED%[✗]%NC% WebSocket服务器未运行
    set "WS_RUNNING=0"
)

if !API_RUNNING! == 0 (
    echo.
    echo %RED%[错误]%NC% REST API服务器未运行，请先启动后端服务
    echo 运行命令: scripts\start_backend_windows.bat
    pause
    exit /b 1
)

if !WS_RUNNING! == 0 (
    echo.
    echo %RED%[错误]%NC% WebSocket服务器未运行，请先启动后端服务
    echo 运行命令: scripts\start_backend_windows.bat
    pause
    exit /b 1
)

echo.

:: 步骤2: 测试基础API连接
echo %BLUE%[步骤2]%NC% 测试基础API连接...

echo %BLUE%[测试]%NC% 健康检查端点
powershell -Command "try { $r = Invoke-WebRequest -Uri 'http://localhost:8080/api/health' -TimeoutSec 5; if($r.StatusCode -eq 200) { Write-Host '[✓] 健康检查成功' -ForegroundColor Green; exit 0 } else { Write-Host '[✗] 健康检查失败 - 状态码:' $r.StatusCode -ForegroundColor Red; exit 1 } } catch { Write-Host '[✗] 健康检查失败:' $_.Exception.Message -ForegroundColor Red; exit 1 }"
if !errorlevel! == 0 (
    set "HEALTH_OK=1"
) else (
    set "HEALTH_OK=0"
)

echo %BLUE%[测试]%NC% 系统信息端点
powershell -Command "try { $r = Invoke-WebRequest -Uri 'http://localhost:8080/api/system/info' -TimeoutSec 5; if($r.StatusCode -eq 200) { Write-Host '[✓] 系统信息获取成功' -ForegroundColor Green; exit 0 } else { Write-Host '[✗] 系统信息获取失败 - 状态码:' $r.StatusCode -ForegroundColor Red; exit 1 } } catch { Write-Host '[✗] 系统信息获取失败:' $_.Exception.Message -ForegroundColor Red; exit 1 }"
if !errorlevel! == 0 (
    set "SYSINFO_OK=1"
) else (
    set "SYSINFO_OK=0"
)

echo.

:: 步骤3: 测试WebSocket连接
echo %BLUE%[步骤3]%NC% 测试WebSocket连接...

:: 创建临时PowerShell脚本测试WebSocket
echo $ws = New-Object System.Net.WebSockets.ClientWebSocket > temp_ws_test.ps1
echo $uri = [System.Uri]::new("ws://localhost:8081") >> temp_ws_test.ps1
echo $cts = New-Object System.Threading.CancellationTokenSource >> temp_ws_test.ps1
echo try { >> temp_ws_test.ps1
echo   $task = $ws.ConnectAsync($uri, $cts.Token) >> temp_ws_test.ps1
echo   $task.Wait(5000) >> temp_ws_test.ps1
echo   if ($ws.State -eq [System.Net.WebSockets.WebSocketState]::Open) { >> temp_ws_test.ps1
echo     Write-Host "[✓] WebSocket连接成功" -ForegroundColor Green >> temp_ws_test.ps1
echo     $ws.CloseAsync([System.Net.WebSockets.WebSocketCloseStatus]::NormalClosure, "Test complete", $cts.Token).Wait(1000) >> temp_ws_test.ps1
echo     exit 0 >> temp_ws_test.ps1
echo   } else { >> temp_ws_test.ps1
echo     Write-Host "[✗] WebSocket连接失败 - 状态:" $ws.State -ForegroundColor Red >> temp_ws_test.ps1
echo     exit 1 >> temp_ws_test.ps1
echo   } >> temp_ws_test.ps1
echo } catch { >> temp_ws_test.ps1
echo   Write-Host "[✗] WebSocket连接异常:" $_.Exception.Message -ForegroundColor Red >> temp_ws_test.ps1
echo   exit 1 >> temp_ws_test.ps1
echo } finally { >> temp_ws_test.ps1
echo   $ws.Dispose() >> temp_ws_test.ps1
echo } >> temp_ws_test.ps1

powershell -ExecutionPolicy Bypass -File temp_ws_test.ps1
if !errorlevel! == 0 (
    set "WS_OK=1"
) else (
    set "WS_OK=0"
)

del temp_ws_test.ps1 >nul 2>&1

echo.

:: 步骤4: 测试前端静态文件访问
echo %BLUE%[步骤4]%NC% 测试前端静态文件访问...

echo %BLUE%[测试]%NC% 主页面访问
powershell -Command "try { $r = Invoke-WebRequest -Uri 'http://localhost:8080/' -TimeoutSec 5; if($r.StatusCode -eq 200 -and $r.Content -like '*车身域控制器*') { Write-Host '[✓] 主页面访问成功' -ForegroundColor Green; exit 0 } else { Write-Host '[✗] 主页面内容异常' -ForegroundColor Red; exit 1 } } catch { Write-Host '[✗] 主页面访问失败:' $_.Exception.Message -ForegroundColor Red; exit 1 }"
if !errorlevel! == 0 (
    set "FRONTEND_OK=1"
) else (
    set "FRONTEND_OK=0"
)

echo.

:: 阶段1结果汇总
echo ========================================
echo 第一阶段验证结果
echo ========================================

set "TOTAL_TESTS=5"
set "PASSED_TESTS=0"

if !API_RUNNING! == 1 (
    echo %GREEN%[✓]%NC% REST API服务器运行状态
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% REST API服务器运行状态
)

if !HEALTH_OK! == 1 (
    echo %GREEN%[✓]%NC% 健康检查端点
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 健康检查端点
)

if !SYSINFO_OK! == 1 (
    echo %GREEN%[✓]%NC% 系统信息端点
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 系统信息端点
)

if !WS_OK! == 1 (
    echo %GREEN%[✓]%NC% WebSocket连接
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% WebSocket连接
)

if !FRONTEND_OK! == 1 (
    echo %GREEN%[✓]%NC% 前端页面访问
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 前端页面访问
)

echo.
echo 测试结果: !PASSED_TESTS!/!TOTAL_TESTS! 通过

if !PASSED_TESTS! == !TOTAL_TESTS! (
    echo.
    echo %GREEN%[成功]%NC% 第一阶段验证通过！
    echo.
    echo 下一步操作:
    echo 1. 进行第二阶段测试: scripts\stage2_frontend_integration.bat
    echo 2. 或直接打开浏览器: http://localhost:8080
    echo 3. 查看详细测试: scripts\test_integration.html
    
    set /p "NEXT_STAGE=是否继续第二阶段测试? (y/N): "
    if /i "!NEXT_STAGE!"=="y" (
        echo.
        echo 启动第二阶段测试...
        call "%~dp0stage2_frontend_integration.bat"
    )
) else (
    echo.
    echo %RED%[失败]%NC% 第一阶段验证未完全通过
    echo.
    echo 故障排除建议:
    if !API_RUNNING! == 0 echo - 启动REST API服务器
    if !WS_RUNNING! == 0 echo - 启动WebSocket服务器
    if !HEALTH_OK! == 0 echo - 检查API服务器配置和日志
    if !SYSINFO_OK! == 0 echo - 检查系统信息端点实现
    if !WS_OK! == 0 echo - 检查WebSocket服务器配置
    if !FRONTEND_OK! == 0 echo - 检查前端文件部署
    echo.
    echo 详细调试:
    echo 1. 查看服务器日志输出
    echo 2. 运行: scripts\test_connection_windows.bat
    echo 3. 检查防火墙设置
)

echo.
echo 按任意键退出...
pause >nul
