#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <array>

namespace body_controller {
namespace application {

// ============================================================================
// 基础枚举定义
// ============================================================================

// 车窗/车门位置枚举
enum class Position : uint8_t {
    FRONT_LEFT  = 0,
    FRONT_RIGHT = 1,
    REAR_LEFT   = 2,
    REAR_RIGHT  = 3
};

// 通用结果枚举
enum class Result : uint8_t {
    FAIL    = 0,
    SUCCESS = 1
};

// 车窗控制命令
enum class WindowCommand : uint8_t {
    MOVE_UP   = 0,
    MOVE_DOWN = 1,
    STOP      = 2
};

// 车门锁定命令
enum class LockCommand : uint8_t {
    LOCK   = 0,
    UNLOCK = 1
};

// 车门锁定状态
enum class LockState : uint8_t {
    LOCKED   = 0,
    UNLOCKED = 1
};

// 车门开关状态
enum class DoorState : uint8_t {
    CLOSED = 0,
    OPEN   = 1
};

// 前大灯状态
enum class HeadlightState : uint8_t {
    OFF       = 0,
    LOW_BEAM  = 1,
    HIGH_BEAM = 2
};

// 转向灯状态
enum class IndicatorState : uint8_t {
    OFF    = 0,
    LEFT   = 1,
    RIGHT  = 2,
    HAZARD = 3
};

// 位置灯状态
enum class PositionLightState : uint8_t {
    OFF = 0,
    ON  = 1
};

// 灯光类型
enum class LightType : uint8_t {
    HEADLIGHT      = 0,
    INDICATOR      = 1,
    POSITION_LIGHT = 2
};

// 座椅调节轴
enum class SeatAxis : uint8_t {
    FORWARD_BACKWARD = 0,
    RECLINE         = 1
};

// 座椅调节方向
enum class SeatDirection : uint8_t {
    POSITIVE = 0,
    NEGATIVE = 1,
    STOP     = 2
};

// ============================================================================
// 车窗服务数据结构
// ============================================================================

// 设置车窗位置请求
struct SetWindowPositionReq {
    Position windowID;      // 车窗ID
    uint8_t position;       // 目标位置 (0-100%)
    
    SetWindowPositionReq() : windowID(Position::FRONT_LEFT), position(0) {}
    SetWindowPositionReq(Position id, uint8_t pos) : windowID(id), position(pos) {}
};

// 设置车窗位置响应
struct SetWindowPositionResp {
    Position windowID;      // 车窗ID
    Result result;          // 执行结果
    
    SetWindowPositionResp() : windowID(Position::FRONT_LEFT), result(Result::FAIL) {}
    SetWindowPositionResp(Position id, Result res) : windowID(id), result(res) {}
};

// 控制车窗请求
struct ControlWindowReq {
    Position windowID;      // 车窗ID
    WindowCommand command;  // 控制命令
    
    ControlWindowReq() : windowID(Position::FRONT_LEFT), command(WindowCommand::STOP) {}
    ControlWindowReq(Position id, WindowCommand cmd) : windowID(id), command(cmd) {}
};

// 控制车窗响应
struct ControlWindowResp {
    Position windowID;      // 车窗ID
    Result result;          // 执行结果
    
    ControlWindowResp() : windowID(Position::FRONT_LEFT), result(Result::FAIL) {}
    ControlWindowResp(Position id, Result res) : windowID(id), result(res) {}
};

// 获取车窗位置请求
struct GetWindowPositionReq {
    Position windowID;      // 车窗ID
    
    GetWindowPositionReq() : windowID(Position::FRONT_LEFT) {}
    GetWindowPositionReq(Position id) : windowID(id) {}
};

// 获取车窗位置响应
struct GetWindowPositionResp {
    Position windowID;      // 车窗ID
    uint8_t position;       // 当前位置 (0-100%)
    
    GetWindowPositionResp() : windowID(Position::FRONT_LEFT), position(0) {}
    GetWindowPositionResp(Position id, uint8_t pos) : windowID(id), position(pos) {}
};

// 车窗位置变化事件数据
struct OnWindowPositionChangedData {
    Position windowID;      // 车窗ID
    uint8_t newPosition;    // 新位置 (0-100%)
    
    OnWindowPositionChangedData() : windowID(Position::FRONT_LEFT), newPosition(0) {}
    OnWindowPositionChangedData(Position id, uint8_t pos) : windowID(id), newPosition(pos) {}
};

// ============================================================================
// 车门服务数据结构
// ============================================================================

// 设置锁定状态请求
struct SetLockStateReq {
    Position doorID;        // 车门ID
    LockCommand command;    // 锁定命令
    
    SetLockStateReq() : doorID(Position::FRONT_LEFT), command(LockCommand::LOCK) {}
    SetLockStateReq(Position id, LockCommand cmd) : doorID(id), command(cmd) {}
};

// 设置锁定状态响应
struct SetLockStateResp {
    Position doorID;        // 车门ID
    Result result;          // 执行结果
    
    SetLockStateResp() : doorID(Position::FRONT_LEFT), result(Result::FAIL) {}
    SetLockStateResp(Position id, Result res) : doorID(id), result(res) {}
};

// 获取锁定状态请求
struct GetLockStateReq {
    Position doorID;        // 车门ID
    
    GetLockStateReq() : doorID(Position::FRONT_LEFT) {}
    GetLockStateReq(Position id) : doorID(id) {}
};

// 获取锁定状态响应
struct GetLockStateResp {
    Position doorID;        // 车门ID
    LockState lockState;    // 锁定状态
    
    GetLockStateResp() : doorID(Position::FRONT_LEFT), lockState(LockState::LOCKED) {}
    GetLockStateResp(Position id, LockState state) : doorID(id), lockState(state) {}
};

// 锁定状态变化事件数据
struct OnLockStateChangedData {
    Position doorID;        // 车门ID
    LockState newLockState; // 新锁定状态
    
    OnLockStateChangedData() : doorID(Position::FRONT_LEFT), newLockState(LockState::LOCKED) {}
    OnLockStateChangedData(Position id, LockState state) : doorID(id), newLockState(state) {}
};

// 车门状态变化事件数据
struct OnDoorStateChangedData {
    Position doorID;        // 车门ID
    DoorState newDoorState; // 新车门状态
    
    OnDoorStateChangedData() : doorID(Position::FRONT_LEFT), newDoorState(DoorState::CLOSED) {}
    OnDoorStateChangedData(Position id, DoorState state) : doorID(id), newDoorState(state) {}
};

// ============================================================================
// 灯光服务数据结构
// ============================================================================

// 设置前大灯状态请求
struct SetHeadlightStateReq {
    HeadlightState command; // 前大灯命令
    
    SetHeadlightStateReq() : command(HeadlightState::OFF) {}
    SetHeadlightStateReq(HeadlightState cmd) : command(cmd) {}
};

// 设置前大灯状态响应
struct SetHeadlightStateResp {
    HeadlightState newState; // 新状态
    Result result;           // 执行结果
    
    SetHeadlightStateResp() : newState(HeadlightState::OFF), result(Result::FAIL) {}
    SetHeadlightStateResp(HeadlightState state, Result res) : newState(state), result(res) {}
};

// 设置转向灯状态请求
struct SetIndicatorStateReq {
    IndicatorState command; // 转向灯命令
    
    SetIndicatorStateReq() : command(IndicatorState::OFF) {}
    SetIndicatorStateReq(IndicatorState cmd) : command(cmd) {}
};

// 设置转向灯状态响应
struct SetIndicatorStateResp {
    IndicatorState newState; // 新状态
    Result result;           // 执行结果
    
    SetIndicatorStateResp() : newState(IndicatorState::OFF), result(Result::FAIL) {}
    SetIndicatorStateResp(IndicatorState state, Result res) : newState(state), result(res) {}
};

// 设置位置灯状态请求
struct SetPositionLightStateReq {
    PositionLightState command; // 位置灯命令
    
    SetPositionLightStateReq() : command(PositionLightState::OFF) {}
    SetPositionLightStateReq(PositionLightState cmd) : command(cmd) {}
};

// 设置位置灯状态响应
struct SetPositionLightStateResp {
    PositionLightState newState; // 新状态
    Result result;               // 执行结果
    
    SetPositionLightStateResp() : newState(PositionLightState::OFF), result(Result::FAIL) {}
    SetPositionLightStateResp(PositionLightState state, Result res) : newState(state), result(res) {}
};

// 灯光状态变化事件数据
struct OnLightStateChangedData {
    LightType lightType;    // 灯光类型
    uint8_t newState;       // 新状态值
    
    OnLightStateChangedData() : lightType(LightType::HEADLIGHT), newState(0) {}
    OnLightStateChangedData(LightType type, uint8_t state) : lightType(type), newState(state) {}
};

// ============================================================================
// 座椅服务数据结构
// ============================================================================

// 调节座椅请求
struct AdjustSeatReq {
    SeatAxis axis;          // 调节轴
    SeatDirection direction; // 调节方向
    
    AdjustSeatReq() : axis(SeatAxis::FORWARD_BACKWARD), direction(SeatDirection::STOP) {}
    AdjustSeatReq(SeatAxis ax, SeatDirection dir) : axis(ax), direction(dir) {}
};

// 调节座椅响应
struct AdjustSeatResp {
    SeatAxis axis;          // 调节轴
    Result result;          // 执行结果
    
    AdjustSeatResp() : axis(SeatAxis::FORWARD_BACKWARD), result(Result::FAIL) {}
    AdjustSeatResp(SeatAxis ax, Result res) : axis(ax), result(res) {}
};

// 恢复记忆位置请求
struct RecallMemoryPositionReq {
    uint8_t presetID;       // 记忆位置ID (1-3)
    
    RecallMemoryPositionReq() : presetID(1) {}
    RecallMemoryPositionReq(uint8_t id) : presetID(id) {}
};

// 恢复记忆位置响应
struct RecallMemoryPositionResp {
    uint8_t presetID;       // 记忆位置ID
    Result result;          // 执行结果
    
    RecallMemoryPositionResp() : presetID(1), result(Result::FAIL) {}
    RecallMemoryPositionResp(uint8_t id, Result res) : presetID(id), result(res) {}
};

// 保存记忆位置请求
struct SaveMemoryPositionReq {
    uint8_t presetID;       // 记忆位置ID (1-3)
    
    SaveMemoryPositionReq() : presetID(1) {}
    SaveMemoryPositionReq(uint8_t id) : presetID(id) {}
};

// 保存记忆位置响应
struct SaveMemoryPositionResp {
    uint8_t presetID;       // 记忆位置ID
    Result result;          // 执行结果
    
    SaveMemoryPositionResp() : presetID(1), result(Result::FAIL) {}
    SaveMemoryPositionResp(uint8_t id, Result res) : presetID(id), result(res) {}
};

// 座椅位置变化事件数据
struct OnSeatPositionChangedData {
    SeatAxis axis;          // 调节轴
    uint8_t newPosition;    // 新位置 (0-100%)
    
    OnSeatPositionChangedData() : axis(SeatAxis::FORWARD_BACKWARD), newPosition(0) {}
    OnSeatPositionChangedData(SeatAxis ax, uint8_t pos) : axis(ax), newPosition(pos) {}
};

// 记忆保存确认事件数据
struct OnMemorySaveConfirmData {
    uint8_t presetID;       // 记忆位置ID
    Result saveResult;      // 保存结果
    
    OnMemorySaveConfirmData() : presetID(1), saveResult(Result::FAIL) {}
    OnMemorySaveConfirmData(uint8_t id, Result res) : presetID(id), saveResult(res) {}
};

} // namespace application
} // namespace body_controller
