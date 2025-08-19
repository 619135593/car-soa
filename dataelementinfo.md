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
