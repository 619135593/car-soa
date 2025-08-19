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

