# 构建和测试指南

## 第一步：vsomeip客户端实现完成

我们已经成功实现了第一步计划：开发vsomeip客户端。具体完成的内容包括：

### ✅ 已实现的组件

1. **基类 SomeipClient** (`src/communication/someip_client.cpp`)
   - 完整的vsomeip应用程序生命周期管理
   - 标准的初始化、启动、停止流程
   - 消息发送和事件订阅的基础功能
   - 错误处理和日志记录

2. **DoorServiceClient 实现** (`src/communication/door_service_client.cpp`) ⭐ **独立文件**
   - 完整的车门服务客户端功能
   - SetLockState() 和 GetLockState() 方法调用
   - OnLockStateChanged 和 OnDoorStateChanged 事件处理
   - 正确的vsomeip API使用（send方法、事件订阅流程）

3. **WindowServiceClient 实现** (`src/communication/window_service_client.cpp`) ⭐ **独立文件**
   - 完整的车窗服务客户端功能
   - SetWindowPosition()、ControlWindow() 和 GetWindowPosition() 方法调用
   - OnWindowPositionChanged 事件处理
   - 支持车窗位置设置、升降控制和实时状态监控

4. **LightServiceClient 实现** (`src/communication/light_service_client.cpp`) ⭐ **独立文件**
   - 完整的灯光服务客户端功能
   - SetHeadlightState()、SetIndicatorState() 和 SetPositionLightState() 方法调用
   - OnLightStateChanged 事件处理
   - 支持前大灯（关闭/近光/远光）、转向灯（左转/右转/双闪）、位置灯控制

5. **SeatServiceClient 实现** (`src/communication/seat_service_client.cpp`) ⭐ **新增独立文件**
   - 完整的座椅服务客户端功能
   - AdjustSeat()、RecallMemoryPosition() 和 SaveMemoryPosition() 方法调用
   - OnSeatPositionChanged 和 OnMemorySaveConfirm 事件处理
   - 支持座椅前后/靠背调节、记忆位置保存/恢复（1-3号位置）

6. **测试程序**
   - `src/test_door_client.cpp` - 车门服务功能测试
   - `src/test_window_client.cpp` - 车窗服务功能测试
   - `src/test_light_client.cpp` - 灯光服务功能测试
   - `src/test_seat_client.cpp` - 座椅服务功能测试 ⭐ **新增**
   - 完整的功能测试用例和事件处理器演示
   - 优雅的启动和关闭流程

5. **构建系统** (`CMakeLists.txt`)
   - 跨平台构建支持
   - 自动依赖检测
   - 模块化源文件管理 ⭐ **新增**
   - 测试和安装规则
   - 自动生成测试脚本

6. **代码组织** ⭐ **新增模块化结构**
   - 每个服务客户端独立源文件
   - 提高代码可维护性和扩展性
   - 便于团队协作和并行开发

## 构建步骤

### 1. 环境准备

**Linux (Ubuntu/Debian):**
```bash
# 安装基础开发工具
sudo apt update
sudo apt install -y build-essential cmake git pkg-config

# 安装vsomeip库 (如果系统包管理器中有)
sudo apt install -y libvsomeip3-dev

# 或者从源码编译vsomeip (如果包管理器中没有)
cd vsomeip
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
sudo ldconfig
```

**Windows:**
```cmd
# 使用vcpkg安装依赖
vcpkg install vsomeip:x64-windows
# 或者手动编译vsomeip库
```

### 2. 构建项目

```bash
# 在项目根目录
mkdir build && cd build

# 配置项目
cmake ..

# 编译
make -j$(nproc)

# 检查构建结果
ls -la bin/
# 应该看到: test_door_client
```

### 3. 配置环境

```bash
# 设置vsomeip配置文件路径
export VSOMEIP_CONFIGURATION=$(pwd)/config/vsomeip.json

# 设置应用程序名称
export VSOMEIP_APPLICATION_NAME=door_test_client

# 验证配置文件
cat config/vsomeip.json
```

## 测试步骤

### 1. 网络配置验证

```bash
# 检查网络接口
ip addr show

# 配置测试网络 (如果需要)
sudo ip addr add 192.168.3.10/24 dev eth1
sudo ip link set eth1 up

# 测试与STM32H7的连通性
ping 192.168.3.11
```

### 2. 运行测试程序

#### 2.1 车门服务测试

**方法1：使用提供的脚本**
```bash
cd build
./run_door_test.sh
```

**方法2：手动运行**
```bash
cd build
export VSOMEIP_CONFIGURATION=./config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=door_test_client
./bin/test_door_client
```

#### 2.2 车窗服务测试

**方法1：使用提供的脚本**
```bash
cd build
./run_window_test.sh
```

**方法2：手动运行**
```bash
cd build
export VSOMEIP_CONFIGURATION=./config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=window_test_client
./bin/test_window_client
```

#### 2.3 灯光服务测试

**方法1：使用提供的脚本**
```bash
cd build
./run_light_test.sh
```

**方法2：手动运行**
```bash
cd build
export VSOMEIP_CONFIGURATION=./config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=light_test_client
./bin/test_light_client
```

#### 2.4 座椅服务测试 ⭐ **新增**

**方法1：使用提供的脚本**
```bash
cd build
./run_seat_test.sh
```

**方法2：手动运行**
```bash
cd build
export VSOMEIP_CONFIGURATION=./config/vsomeip.json
export VSOMEIP_APPLICATION_NAME=seat_test_client
./bin/test_seat_client
```

### 3. 预期输出

**正常启动输出：**
```
=== Door Service Client Test ===
This test program demonstrates the DoorServiceClient functionality
Make sure STM32H7 is running and providing door services
Press Ctrl+C to exit
========================================
[SomeipClient] Created application: door_test_client
[DoorServiceClient] Created door service client
[SomeipClient] Application initialized successfully
[DoorServiceClient] Door service client initialized
[TestDoorClient] Door service client initialized successfully
[TestDoorClient] Starting door service client...
[SomeipClient] State changed to: 1
[SomeipClient] Application registered successfully
```

**服务发现成功输出：**
```
[SomeipClient] Service 0x1002 Instance 0x1002 is available
[DoorServiceClient] Door service is available, subscribing to events...
[SomeipClient] Subscribed to event - Service: 0x1002 Event: 0x8001 EventGroup: 0x1
[SomeipClient] Subscribed to event - Service: 0x1002 Event: 0x8002 EventGroup: 0x1
```

**车门服务测试命令执行输出：**
```
[TestDoorClient] Starting test command sequence...

=== Test 1: Get front left door lock state ===
[DoorServiceClient] Getting lock state for door: 0
[SomeipClient] Sent request - Service: 0x1002 Method: 0x2 Payload size: 1

=== Test 2: Lock front left door ===
[DoorServiceClient] Setting lock state for door: 0 Command: 0
[SomeipClient] Sent request - Service: 0x1002 Method: 0x1 Payload size: 2
```

**车窗服务测试命令执行输出：**
```
[TestWindowClient] Starting test command sequence...

=== Test 1: Get front left window position ===
[WindowServiceClient] Getting window position for window: 0
[SomeipClient] Sent request - Service: 0x1001 Method: 0x3 Payload size: 1

=== Test 2: Set front left window position to 50% ===
[WindowServiceClient] Setting window position for window: 0 Position: 50%
[SomeipClient] Sent request - Service: 0x1001 Method: 0x1 Payload size: 2

=== Test 3: Move front left window up ===
[WindowServiceClient] Controlling window: 0 Command: 0
[SomeipClient] Sent request - Service: 0x1001 Method: 0x2 Payload size: 2

[WindowServiceClient] WindowPositionChanged event - Window: 0 New Position: 75%
[WindowServiceClient] WindowPositionChanged event - Window: 0 New Position: 100%
```

**灯光服务测试命令执行输出：**
```
[TestLightClient] Starting test command sequence...

=== Test 1: Turn on position lights ===
[LightServiceClient] Setting position light state: ON
[SomeipClient] Sent request - Service: 0x1003 Method: 0x3 Payload size: 1

=== Test 2: Turn on low beam headlights ===
[LightServiceClient] Setting headlight state: LOW_BEAM
[SomeipClient] Sent request - Service: 0x1003 Method: 0x1 Payload size: 1

=== Test 5: Turn on left indicator ===
[LightServiceClient] Setting indicator state: LEFT
[SomeipClient] Sent request - Service: 0x1003 Method: 0x2 Payload size: 1

[LightServiceClient] LightStateChanged event - Light Type: HEADLIGHT New State: 1
[LightServiceClient] LightStateChanged event - Light Type: INDICATOR New State: 1
```

**座椅服务测试命令执行输出：** ⭐ **新增**
```
[TestSeatClient] Starting test command sequence...

=== Test 1: Move seat forward ===
[SeatServiceClient] Adjusting seat - Axis: FORWARD_BACKWARD Direction: POSITIVE
[SomeipClient] Sent request - Service: 0x1004 Method: 0x1 Payload size: 2

=== Test 2: Stop forward/backward adjustment ===
[SeatServiceClient] Adjusting seat - Axis: FORWARD_BACKWARD Direction: STOP
[SomeipClient] Sent request - Service: 0x1004 Method: 0x1 Payload size: 2

=== Test 9: Save current position to memory slot 1 ===
[SeatServiceClient] Saving current position to memory slot: 1
[SomeipClient] Sent request - Service: 0x1004 Method: 0x3 Payload size: 1

=== Test 14: Recall memory position 1 ===
[SeatServiceClient] Recalling memory position: 1
[SomeipClient] Sent request - Service: 0x1004 Method: 0x2 Payload size: 1

[SeatServiceClient] SeatPositionChanged event - Axis: FORWARD_BACKWARD New Position: 45%
[SeatServiceClient] MemorySaveConfirm event - Preset ID: 1 Save Result: SUCCESS
```

## 故障排除

### 1. 编译错误

**错误：找不到vsomeip头文件**
```bash
# 解决方案：安装vsomeip开发包
sudo apt install libvsomeip3-dev
# 或者设置包含路径
export CPLUS_INCLUDE_PATH=/usr/local/include:$CPLUS_INCLUDE_PATH
```

**错误：链接时找不到vsomeip库**
```bash
# 解决方案：更新库路径
sudo ldconfig
export LD_LIBRARY_PATH=/usr/local/lib:$LD_LIBRARY_PATH
```

### 2. 运行时错误

**错误：无法加载配置文件**
```bash
# 检查配置文件路径
echo $VSOMEIP_CONFIGURATION
ls -la $VSOMEIP_CONFIGURATION

# 检查配置文件格式
cat $VSOMEIP_CONFIGURATION | python -m json.tool
```

**错误：无法连接到服务**
```bash
# 检查网络连通性
ping 192.168.3.11

# 检查端口是否开放
telnet 192.168.3.11 30502

# 检查防火墙设置
sudo ufw status
```

**错误：权限不足**
```bash
# 设置网络权限
sudo setcap 'cap_net_raw,cap_net_admin=+ep' ./bin/test_door_client
```

### 3. 调试技巧

**启用详细日志：**
```bash
# 修改config/vsomeip.json中的日志级别
"logging": {
    "level": "debug",
    "console": true
}
```

**使用网络抓包：**
```bash
# 抓取SOME/IP消息
sudo tcpdump -i eth1 -n port 30502

# 使用Wireshark分析
wireshark -i eth1 -f "port 30502"
```

## 下一步计划

车门服务客户端测试成功后，可以继续实现：

1. **其他服务客户端**：
   - WindowServiceClient
   - LightServiceClient  
   - SeatServiceClient

2. **REST API服务器**：
   - HTTP服务器实现
   - WebSocket实时推送
   - JSON数据转换

3. **Web前端界面**：
   - HTML/CSS/JavaScript实现
   - 实时状态显示
   - 用户交互界面

## 验收标准

### DoorServiceClient验收标准：

- [ ] 程序能够成功编译和链接
- [ ] 能够连接到STM32H7的车门服务
- [ ] 能够发送SetLockState和GetLockState请求
- [ ] 能够接收和处理方法响应
- [ ] 能够订阅和接收事件通知
- [ ] 错误处理和日志记录工作正常
- [ ] 程序能够优雅地启动和关闭

### WindowServiceClient验收标准：

- [ ] 程序能够成功编译和链接
- [ ] 能够连接到STM32H7的车窗服务
- [ ] 能够发送SetWindowPosition、ControlWindow和GetWindowPosition请求
- [ ] 能够接收和处理所有方法响应
- [ ] 能够订阅和接收WindowPositionChanged事件通知
- [ ] 支持所有车窗控制命令（上升、下降、停止）
- [ ] 支持精确的位置设置（0-100%）
- [ ] 支持批量操作（所有4个车窗）
- [ ] 实时位置变化事件能够正确处理
- [ ] 错误处理和日志记录工作正常
- [ ] 程序能够优雅地启动和关闭

### LightServiceClient验收标准：

- [ ] 程序能够成功编译和链接
- [ ] 能够连接到STM32H7的灯光服务
- [ ] 能够发送SetHeadlightState、SetIndicatorState和SetPositionLightState请求
- [ ] 能够接收和处理所有方法响应
- [ ] 能够订阅和接收LightStateChanged事件通知
- [ ] 支持前大灯控制（关闭/近光/远光）
- [ ] 支持转向灯控制（关闭/左转/右转/双闪）
- [ ] 支持位置灯控制（开启/关闭）
- [ ] 支持组合灯光场景（夜间驾驶、紧急停车等）
- [ ] 快速切换响应性能良好
- [ ] 实时状态变化事件能够正确处理
- [ ] 错误处理和日志记录工作正常
- [ ] 程序能够优雅地启动和关闭

### SeatServiceClient验收标准： ⭐ **新增**

- [ ] 程序能够成功编译和链接
- [ ] 能够连接到STM32H7的座椅服务
- [ ] 能够发送AdjustSeat、RecallMemoryPosition和SaveMemoryPosition请求
- [ ] 能够接收和处理所有方法响应
- [ ] 能够订阅和接收SeatPositionChanged和MemorySaveConfirm事件通知
- [ ] 支持座椅前后调节（向前/向后/停止）
- [ ] 支持靠背角度调节（向后倾斜/向前倾斜/停止）
- [ ] 支持记忆位置管理（保存到1-3号位置）
- [ ] 支持记忆位置恢复（从1-3号位置恢复）
- [ ] 记忆位置ID验证正确（1-3范围检查）
- [ ] 连续调节和精确停止功能正常
- [ ] 快速调节响应性能良好
- [ ] 记忆位置循环测试稳定
- [ ] 错误处理（无效记忆位置ID）正确
- [ ] 实时位置变化和保存确认事件能够正确处理
- [ ] 错误处理和日志记录工作正常
- [ ] 程序能够优雅地启动和关闭

### 整体系统验收标准：

- [ ] 四个服务客户端可以同时运行而不冲突 ⭐ **更新**
- [ ] 网络资源使用合理，无内存泄漏
- [ ] 配置文件格式正确，服务发现工作正常
- [ ] 构建系统在不同平台上工作正常
- [ ] 测试脚本和文档完整准确
- [ ] 模块化代码结构清晰易维护
- [ ] 通信层100%完成，所有车身控制功能可用 ⭐ **新增**

完成这些验收标准后，车身域控制器系统的通信层将达到100%完成度，可以继续实现第二步：REST API服务器开发。
