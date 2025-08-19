# VSOMEIP服务端模拟器

这个项目实现了车身控制系统的VSOMEIP服务端，用于模拟真实的硬件服务。

## 🎯 **项目目标**

- **完整的事件驱动测试**：验证从硬件事件到前端UI的完整通信链路
- **VSOMEIP通信验证**：测试客户端和服务端之间的VSOMEIP通信
- **SSE事件推送测试**：验证硬件状态变化能否正确推送到前端

## 🏗️ **架构设计**

```
硬件事件模拟器 → VSOMEIP服务端 → VSOMEIP客户端 → Web API → SSE → 前端UI
```

## 📋 **服务实现**

### 1. **车门服务 (Door Service)**
- **服务ID**: 0x1002
- **实例ID**: 0x1002
- **功能**: 车门锁定/解锁控制，状态查询，状态变化事件推送

### 2. **车窗服务 (Window Service)**
- **服务ID**: 0x1001
- **实例ID**: 0x1001
- **功能**: 车窗位置控制，位置查询，位置变化事件推送

### 3. **灯光服务 (Light Service)**
- **服务ID**: 0x1003
- **实例ID**: 0x1003
- **功能**: 前大灯、转向灯、位置灯控制和状态推送

### 4. **座椅服务 (Seat Service)**
- **服务ID**: 0x1004
- **实例ID**: 0x1004
- **功能**: 座椅调节，记忆位置保存/恢复，状态推送

## 🚀 **运行方式**

```bash
# 1. 编译服务端
cd vsomeip_services
mkdir build && cd build
cmake ..
make -j4

# 2. 启动服务端（在一个终端）
./bin/body_controller_services

# 3. 启动Web服务器（在另一个终端）
cd ../../
# 启动Web服务器...
```

## 📁 **项目结构**

```
vsomeip_services/
├── README.md                    # 项目说明
├── CMakeLists.txt              # 构建配置
├── config/
│   └── vsomeip_services.json   # VSOMEIP配置
├── include/
│   ├── services/               # 服务头文件
│   └── common/                 # 公共头文件
├── src/
│   ├── services/               # 服务实现
│   ├── common/                 # 公共实现
│   └── main.cpp               # 主程序
└── build/                      # 构建目录
```

## 🔧 **配置说明**

- 使用与客户端相同的VSOMEIP配置
- 不同的应用程序名称避免冲突
- 支持本地进程间通信
- 自动硬件事件模拟
