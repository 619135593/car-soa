@echo off
setlocal enabledelayedexpansion

echo ========================================
echo 第二阶段：前端界面集成验证
echo ========================================
echo.

:: 设置颜色
set "RED=[91m"
set "GREEN=[92m"
set "YELLOW=[93m"
set "BLUE=[94m"
set "NC=[0m"

echo %BLUE%[阶段2]%NC% 验证前端界面与后端的完整交互
echo.

:: 步骤1: 打开浏览器进行手动测试
echo %BLUE%[步骤1]%NC% 启动浏览器界面测试...

echo %BLUE%[信息]%NC% 正在打开主界面...
start http://localhost:8080

timeout /t 3 /nobreak >nul

echo %BLUE%[信息]%NC% 正在打开集成测试页面...
start "%~dp0test_integration.html"

echo.
echo %YELLOW%[指导]%NC% 请在浏览器中进行以下测试：
echo.
echo 1. 主界面测试 (http://localhost:8080):
echo    - 检查页面是否正常加载
echo    - 查看连接状态指示器（应显示"已连接"）
echo    - 测试主题切换功能
echo    - 检查所有控制面板是否显示正常
echo.
echo 2. 功能交互测试:
echo    - 车门控制：点击锁定/解锁按钮
echo    - 车窗控制：拖动位置滑块
echo    - 灯光控制：切换灯光状态
echo    - 座椅控制：测试调节和记忆功能
echo.
echo 3. 实时更新测试:
echo    - 观察状态徽章是否有动画效果
echo    - 检查活动日志是否记录操作
echo    - 验证WebSocket连接状态
echo.

:: 步骤2: 等待用户完成手动测试
echo %BLUE%[步骤2]%NC% 等待用户完成界面测试...
echo.

set /p "UI_TEST_RESULT=主界面是否正常显示? (y/N): "
if /i not "!UI_TEST_RESULT!"=="y" (
    echo %RED%[失败]%NC% 主界面显示异常
    goto :troubleshoot_ui
)

set /p "CONNECTION_TEST_RESULT=连接状态是否显示"已连接"? (y/N): "
if /i not "!CONNECTION_TEST_RESULT!"=="y" (
    echo %RED%[失败]%NC% 连接状态异常
    goto :troubleshoot_connection
)

set /p "INTERACTION_TEST_RESULT=控制功能是否响应正常? (y/N): "
if /i not "!INTERACTION_TEST_RESULT!"=="y" (
    echo %RED%[失败]%NC% 控制功能异常
    goto :troubleshoot_interaction
)

set /p "REALTIME_TEST_RESULT=实时更新是否正常工作? (y/N): "
if /i not "!REALTIME_TEST_RESULT!"=="y" (
    echo %RED%[失败]%NC% 实时更新异常
    goto :troubleshoot_realtime
)

:: 步骤3: 自动化API测试
echo.
echo %BLUE%[步骤3]%NC% 执行自动化API测试...

echo %BLUE%[测试]%NC% 车门控制API
powershell -Command "$body = @{doorID=0; command=1} | ConvertTo-Json; try { $r = Invoke-WebRequest -Uri 'http://localhost:8080/api/door/lock' -Method POST -Body $body -ContentType 'application/json' -TimeoutSec 5; if($r.StatusCode -eq 200) { Write-Host '[✓] 车门锁定API成功' -ForegroundColor Green; exit 0 } else { Write-Host '[✗] 车门锁定API失败 - 状态码:' $r.StatusCode -ForegroundColor Red; exit 1 } } catch { Write-Host '[✗] 车门锁定API异常:' $_.Exception.Message -ForegroundColor Red; exit 1 }"
set "DOOR_API_OK=!errorlevel!"

echo %BLUE%[测试]%NC% 车窗控制API
powershell -Command "$body = @{windowID=0; position=50} | ConvertTo-Json; try { $r = Invoke-WebRequest -Uri 'http://localhost:8080/api/window/position' -Method POST -Body $body -ContentType 'application/json' -TimeoutSec 5; if($r.StatusCode -eq 200) { Write-Host '[✓] 车窗位置API成功' -ForegroundColor Green; exit 0 } else { Write-Host '[✗] 车窗位置API失败 - 状态码:' $r.StatusCode -ForegroundColor Red; exit 1 } } catch { Write-Host '[✗] 车窗位置API异常:' $_.Exception.Message -ForegroundColor Red; exit 1 }"
set "WINDOW_API_OK=!errorlevel!"

echo %BLUE%[测试]%NC% 灯光控制API
powershell -Command "$body = @{command=1} | ConvertTo-Json; try { $r = Invoke-WebRequest -Uri 'http://localhost:8080/api/light/headlight' -Method POST -Body $body -ContentType 'application/json' -TimeoutSec 5; if($r.StatusCode -eq 200) { Write-Host '[✓] 灯光控制API成功' -ForegroundColor Green; exit 0 } else { Write-Host '[✗] 灯光控制API失败 - 状态码:' $r.StatusCode -ForegroundColor Red; exit 1 } } catch { Write-Host '[✗] 灯光控制API异常:' $_.Exception.Message -ForegroundColor Red; exit 1 }"
set "LIGHT_API_OK=!errorlevel!"

echo %BLUE%[测试]%NC% 座椅控制API
powershell -Command "$body = @{axis=0; direction=0} | ConvertTo-Json; try { $r = Invoke-WebRequest -Uri 'http://localhost:8080/api/seat/adjust' -Method POST -Body $body -ContentType 'application/json' -TimeoutSec 5; if($r.StatusCode -eq 200) { Write-Host '[✓] 座椅调节API成功' -ForegroundColor Green; exit 0 } else { Write-Host '[✗] 座椅调节API失败 - 状态码:' $r.StatusCode -ForegroundColor Red; exit 1 } } catch { Write-Host '[✗] 座椅调节API异常:' $_.Exception.Message -ForegroundColor Red; exit 1 }"
set "SEAT_API_OK=!errorlevel!"

echo.

:: 阶段2结果汇总
echo ========================================
echo 第二阶段验证结果
echo ========================================

set "TOTAL_TESTS=8"
set "PASSED_TESTS=0"

if /i "!UI_TEST_RESULT!"=="y" (
    echo %GREEN%[✓]%NC% 主界面显示
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 主界面显示
)

if /i "!CONNECTION_TEST_RESULT!"=="y" (
    echo %GREEN%[✓]%NC% 连接状态显示
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 连接状态显示
)

if /i "!INTERACTION_TEST_RESULT!"=="y" (
    echo %GREEN%[✓]%NC% 控制功能交互
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 控制功能交互
)

if /i "!REALTIME_TEST_RESULT!"=="y" (
    echo %GREEN%[✓]%NC% 实时更新功能
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 实时更新功能
)

if !DOOR_API_OK! == 0 (
    echo %GREEN%[✓]%NC% 车门控制API
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 车门控制API
)

if !WINDOW_API_OK! == 0 (
    echo %GREEN%[✓]%NC% 车窗控制API
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 车窗控制API
)

if !LIGHT_API_OK! == 0 (
    echo %GREEN%[✓]%NC% 灯光控制API
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 灯光控制API
)

if !SEAT_API_OK! == 0 (
    echo %GREEN%[✓]%NC% 座椅控制API
    set /a PASSED_TESTS+=1
) else (
    echo %RED%[✗]%NC% 座椅控制API
)

echo.
echo 测试结果: !PASSED_TESTS!/!TOTAL_TESTS! 通过

if !PASSED_TESTS! geq 6 (
    echo.
    echo %GREEN%[成功]%NC% 第二阶段验证基本通过！
    echo.
    echo 下一步操作:
    echo 1. 进行第三阶段准备: scripts\stage3_vsomeip_preparation.bat
    echo 2. 继续功能测试和优化
    echo 3. 准备集成真实硬件服务
    
    set /p "NEXT_STAGE=是否继续第三阶段准备? (y/N): "
    if /i "!NEXT_STAGE!"=="y" (
        echo.
        echo 启动第三阶段准备...
        call "%~dp0stage3_vsomeip_preparation.bat"
    )
) else (
    echo.
    echo %YELLOW%[警告]%NC% 第二阶段验证部分通过，建议检查失败项目
    goto :troubleshoot_summary
)

goto :end

:troubleshoot_ui
echo.
echo %YELLOW%[故障排除]%NC% 主界面显示问题:
echo 1. 检查浏览器控制台是否有JavaScript错误
echo 2. 确认CSS文件是否正确加载
echo 3. 验证HTML文件是否完整
echo 4. 尝试强制刷新页面 (Ctrl+F5)
goto :end

:troubleshoot_connection
echo.
echo %YELLOW%[故障排除]%NC% 连接状态问题:
echo 1. 检查WebSocket服务器是否正常运行
echo 2. 查看浏览器Network标签的WebSocket连接
echo 3. 确认防火墙没有阻止WebSocket连接
echo 4. 检查WebSocket URL配置是否正确
goto :end

:troubleshoot_interaction
echo.
echo %YELLOW%[故障排除]%NC% 控制功能问题:
echo 1. 检查浏览器控制台的API请求错误
echo 2. 确认后端API端点是否正确实现
echo 3. 验证请求参数格式是否正确
echo 4. 检查CORS设置是否正确
goto :end

:troubleshoot_realtime
echo.
echo %YELLOW%[故障排除]%NC% 实时更新问题:
echo 1. 确认WebSocket消息订阅是否成功
echo 2. 检查事件处理函数是否正确
echo 3. 验证状态更新逻辑是否正常
echo 4. 查看WebSocket消息流
goto :end

:troubleshoot_summary
echo.
echo %YELLOW%[故障排除总结]%NC%
echo 1. 查看详细错误信息和日志
echo 2. 使用浏览器开发者工具调试
echo 3. 运行完整测试: scripts\test_api_windows.ps1
echo 4. 检查后端服务器实现
goto :end

:end
echo.
echo 按任意键退出...
pause >nul
