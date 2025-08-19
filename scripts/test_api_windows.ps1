# 车身域控制器 Windows API 测试脚本
# PowerShell 版本

param(
    [string]$ApiBase = "http://localhost:8080/api",
    [string]$WsUrl = "ws://localhost:8081",
    [switch]$Verbose
)

# 设置错误处理
$ErrorActionPreference = "Continue"

# 颜色函数
function Write-ColorOutput {
    param(
        [string]$Message,
        [string]$Color = "White"
    )
    
    switch ($Color) {
        "Red" { Write-Host $Message -ForegroundColor Red }
        "Green" { Write-Host $Message -ForegroundColor Green }
        "Yellow" { Write-Host $Message -ForegroundColor Yellow }
        "Blue" { Write-Host $Message -ForegroundColor Blue }
        "Cyan" { Write-Host $Message -ForegroundColor Cyan }
        default { Write-Host $Message }
    }
}

# 日志函数
function Write-Log {
    param(
        [string]$Message,
        [string]$Level = "INFO"
    )
    
    $timestamp = Get-Date -Format "yyyy-MM-dd HH:mm:ss"
    $colorMap = @{
        "INFO" = "Blue"
        "SUCCESS" = "Green"
        "WARNING" = "Yellow"
        "ERROR" = "Red"
    }
    
    $color = $colorMap[$Level]
    Write-ColorOutput "[$timestamp] [$Level] $Message" $color
}

# API测试函数
function Test-ApiEndpoint {
    param(
        [string]$Endpoint,
        [string]$Method = "GET",
        [hashtable]$Body = $null,
        [int]$TimeoutSec = 10
    )
    
    $url = "$ApiBase$Endpoint"
    Write-Log "测试 $Method $url" "INFO"
    
    try {
        $params = @{
            Uri = $url
            Method = $Method
            TimeoutSec = $TimeoutSec
            Headers = @{
                "Content-Type" = "application/json"
            }
        }
        
        if ($Body) {
            $params.Body = $Body | ConvertTo-Json
        }
        
        $startTime = Get-Date
        $response = Invoke-WebRequest @params
        $endTime = Get-Date
        $duration = ($endTime - $startTime).TotalMilliseconds
        
        Write-Log "成功 - 状态码: $($response.StatusCode), 响应时间: ${duration}ms" "SUCCESS"
        
        if ($Verbose) {
            Write-Log "响应内容: $($response.Content)" "INFO"
        }
        
        return @{
            Success = $true
            StatusCode = $response.StatusCode
            Content = $response.Content
            Duration = $duration
        }
    }
    catch {
        Write-Log "失败 - $($_.Exception.Message)" "ERROR"
        return @{
            Success = $false
            Error = $_.Exception.Message
        }
    }
}

# 主测试函数
function Start-ApiTests {
    Write-ColorOutput "========================================" "Cyan"
    Write-ColorOutput "车身域控制器 API 测试 (PowerShell)" "Cyan"
    Write-ColorOutput "========================================" "Cyan"
    Write-Host ""
    
    Write-Log "API基础地址: $ApiBase" "INFO"
    Write-Log "WebSocket地址: $WsUrl" "INFO"
    Write-Host ""
    
    $testResults = @()
    
    # 测试1: 健康检查
    Write-ColorOutput "测试1: 健康检查" "Yellow"
    $result = Test-ApiEndpoint -Endpoint "/health"
    $testResults += @{ Name = "健康检查"; Result = $result }
    Write-Host ""
    
    # 测试2: 系统信息
    Write-ColorOutput "测试2: 系统信息" "Yellow"
    $result = Test-ApiEndpoint -Endpoint "/system/info"
    $testResults += @{ Name = "系统信息"; Result = $result }
    Write-Host ""
    
    # 测试3: 车门状态查询
    Write-ColorOutput "测试3: 车门状态查询" "Yellow"
    for ($i = 0; $i -lt 4; $i++) {
        $result = Test-ApiEndpoint -Endpoint "/door/$i/status"
        $testResults += @{ Name = "车门$i状态"; Result = $result }
    }
    Write-Host ""
    
    # 测试4: 车门控制
    Write-ColorOutput "测试4: 车门控制" "Yellow"
    $lockBody = @{ doorID = 0; command = 1 }
    $result = Test-ApiEndpoint -Endpoint "/door/lock" -Method "POST" -Body $lockBody
    $testResults += @{ Name = "车门锁定"; Result = $result }
    
    Start-Sleep -Seconds 1
    
    $unlockBody = @{ doorID = 0; command = 0 }
    $result = Test-ApiEndpoint -Endpoint "/door/lock" -Method "POST" -Body $unlockBody
    $testResults += @{ Name = "车门解锁"; Result = $result }
    Write-Host ""
    
    # 测试5: 车窗控制
    Write-ColorOutput "测试5: 车窗控制" "Yellow"
    $windowBody = @{ windowID = 0; position = 50 }
    $result = Test-ApiEndpoint -Endpoint "/window/position" -Method "POST" -Body $windowBody
    $testResults += @{ Name = "车窗位置设置"; Result = $result }
    Write-Host ""
    
    # 测试6: 灯光控制
    Write-ColorOutput "测试6: 灯光控制" "Yellow"
    $lightBody = @{ command = 1 }
    $result = Test-ApiEndpoint -Endpoint "/light/headlight" -Method "POST" -Body $lightBody
    $testResults += @{ Name = "前大灯控制"; Result = $result }
    Write-Host ""
    
    # 测试7: 座椅控制
    Write-ColorOutput "测试7: 座椅控制" "Yellow"
    $seatBody = @{ axis = 0; direction = 0 }
    $result = Test-ApiEndpoint -Endpoint "/seat/adjust" -Method "POST" -Body $seatBody
    $testResults += @{ Name = "座椅调节"; Result = $result }
    Write-Host ""
    
    # 测试结果汇总
    Write-ColorOutput "========================================" "Cyan"
    Write-ColorOutput "测试结果汇总" "Cyan"
    Write-ColorOutput "========================================" "Cyan"
    
    $totalTests = $testResults.Count
    $passedTests = ($testResults | Where-Object { $_.Result.Success }).Count
    $failedTests = $totalTests - $passedTests
    
    foreach ($test in $testResults) {
        if ($test.Result.Success) {
            Write-ColorOutput "[PASS] $($test.Name)" "Green"
        } else {
            Write-ColorOutput "[FAIL] $($test.Name) - $($test.Result.Error)" "Red"
        }
    }
    
    Write-Host ""
    Write-Log "总测试数: $totalTests" "INFO"
    Write-Log "通过: $passedTests" "SUCCESS"
    Write-Log "失败: $failedTests" "ERROR"
    
    $successRate = [math]::Round(($passedTests / $totalTests) * 100, 2)
    Write-Log "成功率: $successRate%" "INFO"
    
    if ($passedTests -eq $totalTests) {
        Write-ColorOutput "🎉 所有测试通过！" "Green"
    } elseif ($passedTests -gt 0) {
        Write-ColorOutput "⚠️ 部分测试通过，请检查失败的测试项" "Yellow"
    } else {
        Write-ColorOutput "❌ 所有测试失败，请检查后端服务状态" "Red"
    }
    
    return @{
        Total = $totalTests
        Passed = $passedTests
        Failed = $failedTests
        SuccessRate = $successRate
    }
}

# WebSocket测试函数
function Test-WebSocketConnection {
    Write-ColorOutput "========================================" "Cyan"
    Write-ColorOutput "WebSocket 连接测试" "Cyan"
    Write-ColorOutput "========================================" "Cyan"
    
    Write-Log "WebSocket地址: $WsUrl" "INFO"
    
    # 检查是否安装了WebSocket模块
    try {
        Import-Module -Name "WebSocketClient" -ErrorAction Stop
        Write-Log "WebSocket模块已加载" "SUCCESS"
    }
    catch {
        Write-Log "WebSocket模块未安装，跳过WebSocket测试" "WARNING"
        Write-Log "可以通过以下方式安装:" "INFO"
        Write-Log "Install-Module -Name WebSocketClient" "INFO"
        return
    }
    
    # 这里可以添加WebSocket测试代码
    # 由于PowerShell的WebSocket支持有限，建议使用Python或JavaScript进行测试
    Write-Log "建议使用Python或浏览器JavaScript进行WebSocket测试" "INFO"
}

# 性能测试函数
function Test-ApiPerformance {
    param(
        [int]$RequestCount = 10,
        [int]$ConcurrentRequests = 3
    )
    
    Write-ColorOutput "========================================" "Cyan"
    Write-ColorOutput "API 性能测试" "Cyan"
    Write-ColorOutput "========================================" "Cyan"
    
    Write-Log "并发请求数: $ConcurrentRequests" "INFO"
    Write-Log "总请求数: $RequestCount" "INFO"
    Write-Host ""
    
    $jobs = @()
    $startTime = Get-Date
    
    for ($i = 0; $i -lt $RequestCount; $i++) {
        $job = Start-Job -ScriptBlock {
            param($ApiBase)
            try {
                $response = Invoke-WebRequest -Uri "$ApiBase/health" -TimeoutSec 5
                return @{
                    Success = $true
                    StatusCode = $response.StatusCode
                    Duration = (Measure-Command { 
                        Invoke-WebRequest -Uri "$ApiBase/health" -TimeoutSec 5 
                    }).TotalMilliseconds
                }
            }
            catch {
                return @{
                    Success = $false
                    Error = $_.Exception.Message
                }
            }
        } -ArgumentList $ApiBase
        
        $jobs += $job
        
        # 控制并发数
        if (($jobs | Where-Object { $_.State -eq "Running" }).Count -ge $ConcurrentRequests) {
            $jobs | Wait-Job -Any | Out-Null
        }
    }
    
    # 等待所有任务完成
    $jobs | Wait-Job | Out-Null
    $endTime = Get-Date
    
    # 收集结果
    $results = $jobs | Receive-Job
    $jobs | Remove-Job
    
    $totalDuration = ($endTime - $startTime).TotalMilliseconds
    $successCount = ($results | Where-Object { $_.Success }).Count
    $failCount = $RequestCount - $successCount
    $avgDuration = ($results | Where-Object { $_.Success } | Measure-Object -Property Duration -Average).Average
    
    Write-Log "总耗时: ${totalDuration}ms" "INFO"
    Write-Log "成功请求: $successCount" "SUCCESS"
    Write-Log "失败请求: $failCount" "ERROR"
    Write-Log "平均响应时间: ${avgDuration}ms" "INFO"
    Write-Log "吞吐量: $([math]::Round($RequestCount / ($totalDuration / 1000), 2)) 请求/秒" "INFO"
}

# 主执行逻辑
if ($MyInvocation.InvocationName -ne '.') {
    # 检查PowerShell版本
    if ($PSVersionTable.PSVersion.Major -lt 5) {
        Write-ColorOutput "需要PowerShell 5.0或更高版本" "Red"
        exit 1
    }
    
    # 执行测试
    $apiResults = Start-ApiTests
    Write-Host ""
    
    # 性能测试（可选）
    $performanceTest = Read-Host "是否进行性能测试? (y/N)"
    if ($performanceTest -eq "y" -or $performanceTest -eq "Y") {
        Test-ApiPerformance
        Write-Host ""
    }
    
    # WebSocket测试（可选）
    $wsTest = Read-Host "是否进行WebSocket测试? (y/N)"
    if ($wsTest -eq "y" -or $wsTest -eq "Y") {
        Test-WebSocketConnection
        Write-Host ""
    }
    
    Write-ColorOutput "测试完成！" "Green"
    Write-Host "按任意键退出..."
    $null = $Host.UI.RawUI.ReadKey("NoEcho,IncludeKeyDown")
}
