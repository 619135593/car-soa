# 车身域控制器系统架构总结

## 完整数据流向

```
Web界面 (JavaScript)
    ↓ HTTP/WebSocket
REST API服务器 (C++)
    ↓ 函数调用
SOME/IP适配层 (C++)
    ↓ vsomeip库调用
SOME/IP协议栈 (vsomeip)
    ↓ TCP/IP网络
STM32H7嵌入式节点
```

## 1. 已完成的核心组件

### ✅ SOME/IP服务定义 (`include/communication/someip_service_definitions.h`)
- **4个主要服务**：车窗(0x1001)、车门(0x1002)、灯光(0x1003)、座椅(0x1004)
- **12个方法ID**：涵盖所有控制操作
- **7个事件ID**：实时状态通知
- **网络配置**：PC端(192.168.3.10) ↔ STM32H7(192.168.3.11)

### ✅ 数据结构定义 (`include/application/data_structures.h`)
- **完整的请求/响应结构体**：基于project.md的DataElement规范
- **类型安全的枚举**：Position、Result、各种Command和State
- **24个数据结构**：覆盖所有服务的输入输出参数

### ✅ 序列化/反序列化 (`include/communication/serialization.h`)
- **SOME/IP兼容的序列化**：字节级精确控制
- **模板化枚举处理**：类型安全的枚举序列化
- **完整的转换函数**：支持所有定义的数据结构

### ✅ REST API接口规范 (`include/interface/rest_api_definitions.h`)
- **RESTful端点设计**：/api/v1/{service}/{action}
- **JSON转换工具**：数据结构 ↔ JSON格式
- **HTTP状态码定义**：标准化错误处理

### ✅ vsomeip配置 (`config/vsomeip.json`)
- **服务发现配置**：自动发现STM32H7服务
- **网络参数**：IP地址、端口、协议配置
- **事件订阅**：自动订阅状态变化事件

### ✅ 系统配置 (`config/system_config.json`)
- **完整的系统参数**：网络、服务、监控配置
- **车辆配置**：支持的功能和参数范围
- **开发调试选项**：便于开发和测试

## 2. 数据流向详解

### 2.1 控制命令流向 (Web → STM32H7)
```
1. Web界面发送HTTP POST请求
   POST /api/v1/window/position
   {"window_id": "front_left", "position": 80}

2. REST API服务器接收并解析
   JsonConverter::FromJson() → SetWindowPositionReq

3. 调用SOME/IP适配层
   WindowServiceClient::SetWindowPosition(req)

4. 序列化为SOME/IP消息
   Serializer::Serialize(req) → ByteBuffer

5. vsomeip发送到STM32H7
   app->call_method(service_id, instance_id, method_id, payload)

6. STM32H7执行控制逻辑并响应
   SetWindowPositionResp → 序列化 → SOME/IP响应

7. PC端接收响应并返回给Web
   响应解析 → JSON → HTTP 200 OK
```

### 2.2 状态通知流向 (STM32H7 → Web)
```
1. STM32H7状态变化
   车窗位置改变 → OnWindowPositionChangedData

2. 发送SOME/IP事件
   notify_event(service_id, instance_id, event_id, payload)

3. PC端接收事件通知
   on_event() → 反序列化 → OnWindowPositionChangedData

4. 通过WebSocket推送到Web界面
   WebSocket消息 → 实时更新界面状态
```

## 3. 关键技术特性

### 3.1 类型安全
- 强类型枚举避免魔数
- 编译时类型检查
- 自动序列化/反序列化

### 3.2 协议兼容性
- 严格遵循SOME/IP协议规范
- 与project.md中的通信矩阵完全一致
- 支持STM32H7的数据格式要求

### 3.3 实时性保证
- WebSocket实时状态推送
- 事件驱动架构
- 可配置的超时和重试机制

### 3.4 可扩展性
- 模块化设计
- 配置文件驱动
- 易于添加新服务和功能

## 4. 接下来的实现步骤

### 4.1 SOME/IP客户端实现
```cpp
class WindowServiceClient {
    void SetWindowPosition(const SetWindowPositionReq& req);
    void ControlWindow(const ControlWindowReq& req);
    GetWindowPositionResp GetWindowPosition(const GetWindowPositionReq& req);
    // 事件处理回调
    void OnWindowPositionChanged(const OnWindowPositionChangedData& data);
};
```

### 4.2 REST API服务器实现
```cpp
class RestApiServer {
    void HandleWindowPositionRequest(const httplib::Request& req, httplib::Response& res);
    void HandleWindowControlRequest(const httplib::Request& req, httplib::Response& res);
    // WebSocket处理
    void HandleWebSocketConnection(websocketpp::connection_hdl hdl);
};
```

### 4.3 应用层业务逻辑
```cpp
class BodyController {
    void Initialize();
    void StartServices();
    void HandleServiceEvents();
    void UpdateWebClients();
};
```

## 5. 配置使用说明

### 5.1 网络配置
- PC端IP: 192.168.3.10 (可在system_config.json中修改)
- STM32H7 IP: 192.168.3.11
- 服务端口: 30501-30504 (对应4个服务)

### 5.2 Web界面访问
- HTTP服务: http://192.168.3.10:8080
- WebSocket: ws://192.168.3.10:8080/ws
- API基础路径: /api/v1

### 5.3 开发调试
- 日志级别: 在system_config.json中配置
- Mock模式: 可模拟STM32H7进行开发
- 测试端点: 开发模式下提供额外的测试接口

## 6. 架构优势

1. **清晰的分层设计**：每层职责明确，便于维护
2. **标准化接口**：REST API + SOME/IP双重标准
3. **实时响应**：WebSocket + 事件驱动
4. **类型安全**：编译时错误检查
5. **配置驱动**：无需重编译即可调整参数
6. **易于测试**：模块化设计便于单元测试

这个架构设计完全基于您的project.md文件，确保了与STM32H7端的完美兼容性，同时提供了现代化的Web界面和强大的扩展能力。
