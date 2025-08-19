1.我们正在开发一个硕士研究生项目，与汽车公司合作实现demo级别的车身域控制器系统。该系统采用分布式架构，包含以下两个主要通信节点：

**系统架构：**
- 电脑端：作为主控通信节点，运行应用层服务
- STM32H7：作为嵌入式通信节点，处理底层控制逻辑

**技术栈：**
- 通信协议：SOME/IP (Scalable service-Oriented MiddlewarE over IP)，SOME/IP使用开源的vsomeip协议
- 服务架构：SOA（面向服务的架构）依靠vsomeip协议
- 网络协议：TCP/IP协议栈
- 物理层：以太网(Ethernet)

**团队分工：**
- 我：负责电脑端应用层开发和通信节点实现
- 同学A：负责SOME/IP协议栈移植到STM32和嵌入式通信节点开发
- 同学B：负责以太网通信和LwIP协议栈集成
- 同学C：负责硬件电路原理图设计

**通信流程设计：**
1. 电脑端发起服务请求，通过SOME/IP服务发现机制封装成SOME/IP消息
2. 消息经过TCP/IP协议栈逐层封装（应用层→传输层→网络层→数据链路层）
3. 通过以太网物理层传输到STM32H7节点
4. STM32H7接收数据后逐层解封装，还原出SOME/IP消息
5. STM32H7解析服务请求，通过SOME/IP反序列化获取具体服务参数
6. STM32H7执行相应的控制逻辑或将信号传递给下层硬件模块

**需要确认和补充的技术细节：**
- SOME/IP的具体实现方案（使用哪个开源库或自研）-vsomeip
- 服务发现机制的具体实现 -使用开源vsomeip的？
- 数据序列化/反序列化的格式定义 
- 错误处理和重传机制
- 实时性要求和QoS保证 

2.我的工作：请专注于电脑端的开发工作，具体包括：

**应用层服务开发**：
   - 设计和实现车身域控制器的应用层业务逻辑
   - 创建用户界面或API接口用于控制和监控
   - 实现服务的生命周期管理

**通信节点实现**：（基于vsomeip库的应用层集成，无需重新实现底层协议）
   - 基于vsomeip库实现SOME/IP通信客户端，调用库提供的API接口
   - 根据DataElement规范定义应用层数据结构，使用vsomeip的序列化功能
   - 配置vsomeip参数以实现与STM32H7节点的网络通信（IP地址、端口、服务ID等）
   - 使用vsomeip内置的服务发现、事件订阅和请求-响应机制，无需自行开发这些底层功能

3.我的工作重点应该是：
   - 应用层业务逻辑：
      - 根据DataElement规范定义数据结构
      - 实现4个服务的客户端调用逻辑
      - 处理业务层的状态管理
   - 配置和集成：
      - 配置vsomeip的服务参数（Service ID、Instance ID等）
      - 设置网络参数（IP地址、端口等）
      - 集成到Qt6应用程序中
   - 用户界面和控制逻辑：
      - 创建GUI界面
      - 将用户操作转换为SOME/IP服务调用
      - 处理服务响应和事件通知

请不要涉及STM32H7嵌入式开发、硬件电路设计或LwIP协议栈移植等其他团队成员负责的部分。重点关注电脑端的软件架构设计和SOME/IP通信实现。

4.以下是通信矩阵相关信息，不确保正确，请你自行判断，你需要根据这三个表格作为设计数据结构的依据：
Communication Matrix for BCM Demo Project																
                                                                
Provider ECU	Provider ECU MAC Addr	PSI IP Addr	TCP/UDP	PSI Port	VID	Service Name	InstanceID(0x)	Service Type	EventGroup Name	Method/Event Name	Send Type	Cycle(ms)	Consumer ECU	Consumer ECU MAC Addr	CSI IP Addr	CSI Port（自动分配）
STM32_Node	待定	192.168.3.11	TCP	30501	1	WindowService	1001	Method	N/A	SetWindowPosition	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40001
STM32_Node	待定	192.168.3.11	TCP	30501	1	WindowService	1001	Method	N/A	ControlWindow	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40001
STM32_Node	待定	192.168.3.11	TCP	30501	1	WindowService	1001	Method	N/A	GetWindowPosition	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40001
STM32_Node	待定	192.168.3.11	TCP	30501	1	WindowService	1001	Event	WindowEvents	OnWindowPositionChanged	Notification	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40001
STM32_Node	待定	192.168.3.11	TCP	30502	1	DoorService	1002	Method	N/A	SetLockState	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40002
STM32_Node	待定	192.168.3.11	TCP	30502	1	DoorService	1002	Method	N/A	GetLockState	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40002
STM32_Node	待定	192.168.3.11	TCP	30502	1	DoorService	1002	Event	DoorEvents	OnLockStateChanged	Notification	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40002
STM32_Node	待定	192.168.3.11	TCP	30502	1	DoorService	1002	Event	DoorEvents	OnDoorStateChanged	Notification	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40002
STM32_Node	待定	192.168.3.11	TCP	30503	1	LightService	1003	Method	N/A	SetHeadlightState	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40003
STM32_Node	待定	192.168.3.11	TCP	30503	1	LightService	1003	Method	N/A	SetIndicatorState	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40003
STM32_Node	待定	192.168.3.11	TCP	30503	1	LightService	1003	Method	N/A	SetPositionLightState	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40003
STM32_Node	待定	192.168.3.11	TCP	30503	1	LightService	1003	Event	LightEvents	OnLightStateChanged	Notification	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40003
STM32_Node	待定	192.168.3.11	TCP	30504	1	SeatService	1004	Method	N/A	AdjustSeat	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40004
STM32_Node	待定	192.168.3.11	TCP	30504	1	SeatService	1004	Method	N/A	RecallMemoryPosition	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40004
STM32_Node	待定	192.168.3.11	TCP	30504	1	SeatService	1004	Method	N/A	SaveMemoryPosition	Request/Response	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40004
STM32_Node	待定	192.168.3.11	TCP	30504	1	SeatService	1004	Event	SeatEvents	OnSeatPositionChanged	Notification	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40004
STM32_Node	待定	192.168.3.11	TCP	30504	1	SeatService	1004	Event	SeatEvents	OnMemorySaveConfirm	Notification	N/A	PC_Node	08-97-98-C2-A8-B0	192.168.3.10	40004

ServiceInfo for BCM Demo Project											
                                            
Service Name	Service ID(0x)	ServiceInterfaceVersion	ServiceType	Field Name	Method/Event Name	Method/Event ID(0x)	EventGroup Name	EventGroup ID(0x)	Parameter name	Parameter Type	Parameter Data Type
WindowService	1001	1	Method	N/A	SetWindowPosition	1	N/A	N/A	SetWindowPositionReq	Input	Struct
WindowService	1001	1	Method	N/A	SetWindowPosition	1	N/A	N/A	SetWindowPositionResp	Output	Struct
WindowService	1001	1	Method	N/A	ControlWindow	2	N/A	N/A	ControlWindowReq	Input	Struct
WindowService	1001	1	Method	N/A	ControlWindow	2	N/A	N/A	ControlWindowResp	Output	Struct
WindowService	1001	1	Method	N/A	GetWindowPosition	3	N/A	N/A	GetWindowPositionReq	Input	Struct
WindowService	1001	1	Method	N/A	GetWindowPosition	3	N/A	N/A	GetWindowPositionResp	Output	Struct
WindowService	1001	1	Event	N/A	OnWindowPositionChanged	8001	WindowEvents	1	OnWindowPositionChangedData	Event	Struct
DoorService	1002	1	Method	N/A	SetLockState	1	N/A	N/A	SetLockStateReq	Input	Struct
DoorService	1002	1	Method	N/A	SetLockState	1	N/A	N/A	SetLockStateResp	Output	Struct
DoorService	1002	1	Method	N/A	GetLockState	2	N/A	N/A	GetLockStateReq	Input	Struct
DoorService	1002	1	Method	N/A	GetLockState	2	N/A	N/A	GetLockStateResp	Output	Struct
DoorService	1002	1	Event	N/A	OnLockStateChanged	8001	DoorEvents	1	OnLockStateChangedData	Event	Struct
DoorService	1002	1	Event	N/A	OnDoorStateChanged	8002	DoorEvents	1	OnDoorStateChangedData	Event	Struct
LightService	1003	1	Method	N/A	SetHeadlightState	1	N/A	N/A	SetHeadlightStateReq	Input	Struct
LightService	1003	1	Method	N/A	SetHeadlightState	1	N/A	N/A	SetHeadlightStateResp	Output	Struct
LightService	1003	1	Method	N/A	SetIndicatorState	2	N/A	N/A	SetIndicatorStateReq	Input	Struct
LightService	1003	1	Method	N/A	SetIndicatorState	2	N/A	N/A	SetIndicatorStateResp	Output	Struct
LightService	1003	1	Method	N/A	SetPositionLightState	3	N/A	N/A	SetPositionLightStateReq	Input	Struct
LightService	1003	1	Method	N/A	SetPositionLightState	3	N/A	N/A	SetPositionLightStateResp	Output	Struct
LightService	1003	1	Event	N/A	OnLightStateChanged	8001	LightEvents	1	OnLightStateChangedData	Event	Struct
SeatService	1004	1	Method	N/A	AdjustSeat	1	N/A	N/A	AdjustSeatReq	Input	Struct
SeatService	1004	1	Method	N/A	AdjustSeat	1	N/A	N/A	AdjustSeatResp	Output	Struct
SeatService	1004	1	Method	N/A	RecallMemoryPosition	2	N/A	N/A	RecallMemoryPositionReq	Input	Struct
SeatService	1004	1	Method	N/A	RecallMemoryPosition	2	N/A	N/A	RecallMemoryPositionResp	Output	Struct
SeatService	1004	1	Method	N/A	SaveMemoryPosition	3	N/A	N/A	SaveMemoryPositionReq	Input	Struct
SeatService	1004	1	Method	N/A	SaveMemoryPosition	3	N/A	N/A	SaveMemoryPositionResp	Output	Struct
SeatService	1004	1	Event	N/A	OnSeatPositionChanged	8001	SeatEvents	1	OnSeatPositionChangedData	Event	Struct
SeatService	1004	1	Event	N/A	OnMemorySaveConfirm	8002	SeatEvents	1	OnMemorySaveConfirmData	Event	Struct

DataElementInfo for BCM Demo Project (Final Corrected Version)								
                                
ElementName	Member	SubMember	Length(Byte)	DataType	Initial Value (hex)	Range(DEC)	Signal Conversion	Value Description
SetWindowPositionReq				Struct				SetWindowPosition方法的输入参数结构体
    windowID		1	Enum	0	0~3	E=N*1	0: FRONT_LEFT, 1: FRONT_RIGHT, 2: REAR_LEFT, 3: REAR_RIGHT
    position		1	Uint8	0	0~100	E=N*1%	目标位置, 0%为全关, 100%为全开
SetWindowPositionResp				Struct				SetWindowPosition方法的输出参数结构体 
    windowID		1	Enum	0	0~3	E=N*1	响应所对应的windowID，与请求中一致
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
ControlWindowReq				Struct				ControlWindow方法的输入参数结构体
    windowID		1	Enum	0	0~3	E=N*1	见上文 windowID 定义
    command		1	Enum	2	0~2	E=N*1	0: MOVE_UP, 1: MOVE_DOWN, 2: STOP
ControlWindowResp				Struct				ControlWindow方法的输出参数结构体 
    windowID		1	Enum	0	0~3	E=N*1	 响应所对应的windowID，与请求中一致
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
GetWindowPositionReq				Struct				GetWindowPosition方法的输入参数结构体
    windowID		1	Enum	0	0~3	E=N*1	见上文 windowID 定义
GetWindowPositionResp				Struct				GetWindowPosition方法的输出参数结构体
    windowID		1	Enum	0	0~3	E=N*1	响应所对应的windowID，与请求中一致
    position		1	Uint8	0	0~100	E=N*1%	当前位置, 0%为全关, 100%为全开
OnWindowPositionChangedData				Struct				OnWindowPositionChanged事件的数据结构体
    windowID		1	Enum	0	0~3	E=N*1	见上文 windowID 定义
    newPosition		1	Uint8	0	0~100	E=N*1%	变化后的新位置
SetLockStateReq				Struct				SetLockState方法的输入参数结构体
    doorID		1	Enum	0	0~3	E=N*1	0: FRONT_LEFT, 1: FRONT_RIGHT, ...
    command		1	Enum	0	0~1	E=N*1	0: LOCK, 1: UNLOCK
SetLockStateResp				Struct				SetLockState方法的输出参数结构体
    doorID		1	Enum	0	0~3	E=N*1	响应所对应的doorID，与请求中一致
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
GetLockStateReq				Struct				GetLockState方法的输入参数结构体
    doorID		1	Enum	0	0~3	E=N*1	见上文 doorID 定义
GetLockStateResp				Struct				GetLockState方法的输出参数结构体
    doorID		1	Enum	0	0~3	E=N*1	 响应所对应的doorID，与请求中一致
    lockState		1	Enum	0	0~1	E=N*1	0: LOCKED, 1: UNLOCKED
OnLockStateChangedData				Struct				OnLockStateChanged事件的数据结构体
    doorID		1	Enum	0	0~3	E=N*1	见上文 doorID 定义
    newLockState		1	Enum	0	0~1	E=N*1	0: LOCKED, 1: UNLOCKED
OnDoorStateChangedData				Struct				OnDoorStateChanged事件的数据结构体
    doorID		1	Enum	0	0~3	E=N*1	见上文 doorID 定义
    newDoorState		1	Enum	0	0~1	E=N*1	0: CLOSED, 1: OPEN
SetHeadlightStateReq				Struct				SetHeadlightState方法的输入参数结构体
    command		1	Enum	0	0~2	E=N*1	0: OFF, 1: LOW_BEAM, 2: HIGH_BEAM
SetHeadlightStateResp				Struct				SetHeadlightState方法的输出参数结构体 
    newState		1	Enum	0	0~2	E=N*1	 命令执行后的新状态，与command对应
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
SetIndicatorStateReq				Struct				SetIndicatorState方法的输入参数结构体
    command		1	Enum	0	0~3	E=N*1	0: OFF, 1: LEFT, 2: RIGHT, 3: HAZARD
SetIndicatorStateResp				Struct				SetIndicatorState方法的输出参数结构体
    newState		1	Enum	0	0~3	E=N*1	命令执行后的新状态，与command对应
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
SetPositionLightStateReq				Struct				SetPositionLightState方法的输入参数结构体
    command		1	Enum	0	0~1	E=N*1	0: OFF, 1: ON
SetPositionLightStateResp				Struct				SetPositionLightState方法的输出参数结构体 
    newState		1	Enum	0	0~1	E=N*1	命令执行后的新状态，与command对应
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
OnLightStateChangedData				Struct				OnLightStateChanged事件的数据结构体
    lightType		1	Enum	0	0~2	E=N*1	0: HEADLIGHT, 1: INDICATOR, 2: POSITION_LIGHT
    newState		1	Uint8	0	0~3	E=N*1	状态值，其含义需根据lightType进行解释
AdjustSeatReq				Struct				AdjustSeat方法的输入参数结构体
    axis		1	Enum	0	0~1	E=N*1	0: FORWARD_BACKWARD, 1: RECLINE
    direction		1	Enum	2	0~2	E=N*1	0: POSITIVE, 1: NEGATIVE, 2: STOP
AdjustSeatResp				Struct				AdjustSeat方法的输出参数结构体 
    axis		1	Enum	0	0~1	E=N*1	 响应所对应的axis，与请求中一致
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
RecallMemoryPositionReq				Struct				RecallMemoryPosition方法的输入参数结构体
    presetID		1	Uint8	1	1~3	E=N*1	要恢复的记忆位置ID
RecallMemoryPositionResp				Struct				RecallMemoryPosition方法的输出参数结构体
    presetID		1	Uint8	1	1~3	E=N*1	响应所对应的presetID，与请求中一致
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
SaveMemoryPositionReq				Struct				SaveMemoryPosition方法的输入参数结构体
    presetID		1	Uint8	1	1~3	E=N*1	要保存的记忆位置ID
SaveMemoryPositionResp				Struct				SaveMemoryPosition方法的输出参数结构体
    presetID		1	Uint8	1	1~3	E=N*1	响应所对应的presetID，与请求中一致
    result		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS
OnSeatPositionChangedData				Struct				OnSeatPositionChanged事件的数据结构体
    axis		1	Enum	0	0~1	E=N*1	见上文axis定义
    newPosition		1	Uint8	0	0~100	E=N*1%	调节停止后的新位置
OnMemorySaveConfirmData				Struct				OnMemorySaveConfirm事件的数据结构体
    presetID		1	Uint8	1	1~3	E=N*1	刚刚保存的记忆位置ID
    saveResult		1	Enum	0	0~1	E=N*1	0: FAIL, 1: SUCCESS

5.技术选型：
    语言：C++
    通信库：vsomeip (SOME/IP实现)
    构建系统：CMake
    配置管理：JSON配置文件
    用户界面：Web界面：
                后端：C++ REST API
                前端：HTML/CSS/JavaScript + WebSocket实时通信


