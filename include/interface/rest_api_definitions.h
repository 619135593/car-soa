#pragma once

#include <string>
#include <nlohmann/json.hpp>
#include "application/data_structures.h"

namespace body_controller {
namespace interface {

/**
 * @brief REST API接口定义
 * 
 * 定义Web界面与C++后端之间的HTTP接口规范
 * 提供JSON格式的请求/响应数据转换
 */

// ============================================================================
// API端点定义
// ============================================================================

namespace endpoints {
    // 基础路径
    constexpr const char* API_BASE_PATH = "/api/v1";
    
    // 车窗服务端点
    namespace window {
        constexpr const char* SET_POSITION = "/api/v1/window/position";      // POST
        constexpr const char* CONTROL = "/api/v1/window/control";            // POST
        constexpr const char* GET_POSITION = "/api/v1/window/position";      // GET
        constexpr const char* GET_ALL_POSITIONS = "/api/v1/window/positions"; // GET
    }
    
    // 车门服务端点
    namespace door {
        constexpr const char* SET_LOCK = "/api/v1/door/lock";                // POST
        constexpr const char* GET_LOCK_STATE = "/api/v1/door/lock";          // GET
        constexpr const char* GET_ALL_LOCK_STATES = "/api/v1/door/locks";    // GET
    }
    
    // 灯光服务端点
    namespace light {
        constexpr const char* SET_HEADLIGHT = "/api/v1/light/headlight";     // POST
        constexpr const char* SET_INDICATOR = "/api/v1/light/indicator";     // POST
        constexpr const char* SET_POSITION_LIGHT = "/api/v1/light/position"; // POST
        constexpr const char* GET_ALL_STATES = "/api/v1/light/states";       // GET
    }
    
    // 座椅服务端点
    namespace seat {
        constexpr const char* ADJUST = "/api/v1/seat/adjust";                // POST
        constexpr const char* RECALL_MEMORY = "/api/v1/seat/memory/recall";  // POST
        constexpr const char* SAVE_MEMORY = "/api/v1/seat/memory/save";      // POST
        constexpr const char* GET_POSITION = "/api/v1/seat/position";        // GET
    }
    
    // 系统管理端点
    namespace system {
        constexpr const char* STATUS = "/api/v1/system/status";              // GET
        constexpr const char* HEALTH = "/api/v1/system/health";              // GET
        constexpr const char* CONFIG = "/api/v1/system/config";              // GET/POST
    }
    
    // WebSocket端点
    constexpr const char* WEBSOCKET = "/ws";
}

// ============================================================================
// HTTP状态码定义
// ============================================================================
namespace http_status {
    constexpr int OK = 200;
    constexpr int CREATED = 201;
    constexpr int BAD_REQUEST = 400;
    constexpr int NOT_FOUND = 404;
    constexpr int INTERNAL_SERVER_ERROR = 500;
    constexpr int SERVICE_UNAVAILABLE = 503;
}

// ============================================================================
// JSON转换工具
// ============================================================================

/**
 * @brief JSON转换器
 * 提供数据结构与JSON之间的转换功能
 */
class JsonConverter {
public:
    // ========================================================================
    // 枚举类型转换
    // ========================================================================
    
    static std::string PositionToString(application::Position pos) {
        switch (pos) {
            case application::Position::FRONT_LEFT: return "front_left";
            case application::Position::FRONT_RIGHT: return "front_right";
            case application::Position::REAR_LEFT: return "rear_left";
            case application::Position::REAR_RIGHT: return "rear_right";
            default: return "unknown";
        }
    }
    
    static application::Position StringToPosition(const std::string& str) {
        if (str == "front_left") return application::Position::FRONT_LEFT;
        if (str == "front_right") return application::Position::FRONT_RIGHT;
        if (str == "rear_left") return application::Position::REAR_LEFT;
        if (str == "rear_right") return application::Position::REAR_RIGHT;
        return application::Position::FRONT_LEFT; // 默认值
    }
    
    static std::string ResultToString(application::Result result) {
        return (result == application::Result::SUCCESS) ? "success" : "fail";
    }
    
    static application::Result StringToResult(const std::string& str) {
        return (str == "success") ? application::Result::SUCCESS : application::Result::FAIL;
    }
    
    static std::string WindowCommandToString(application::WindowCommand cmd) {
        switch (cmd) {
            case application::WindowCommand::MOVE_UP: return "move_up";
            case application::WindowCommand::MOVE_DOWN: return "move_down";
            case application::WindowCommand::STOP: return "stop";
            default: return "stop";
        }
    }
    
    static application::WindowCommand StringToWindowCommand(const std::string& str) {
        if (str == "move_up") return application::WindowCommand::MOVE_UP;
        if (str == "move_down") return application::WindowCommand::MOVE_DOWN;
        return application::WindowCommand::STOP;
    }
    
    static std::string LockCommandToString(application::LockCommand cmd) {
        return (cmd == application::LockCommand::LOCK) ? "lock" : "unlock";
    }
    
    static application::LockCommand StringToLockCommand(const std::string& str) {
        return (str == "lock") ? application::LockCommand::LOCK : application::LockCommand::UNLOCK;
    }
    
    static std::string LockStateToString(application::LockState state) {
        return (state == application::LockState::LOCKED) ? "locked" : "unlocked";
    }
    
    static std::string DoorStateToString(application::DoorState state) {
        return (state == application::DoorState::CLOSED) ? "closed" : "open";
    }
    
    static std::string HeadlightStateToString(application::HeadlightState state) {
        switch (state) {
            case application::HeadlightState::OFF: return "off";
            case application::HeadlightState::LOW_BEAM: return "low_beam";
            case application::HeadlightState::HIGH_BEAM: return "high_beam";
            default: return "off";
        }
    }
    
    static application::HeadlightState StringToHeadlightState(const std::string& str) {
        if (str == "low_beam") return application::HeadlightState::LOW_BEAM;
        if (str == "high_beam") return application::HeadlightState::HIGH_BEAM;
        return application::HeadlightState::OFF;
    }
    
    static std::string IndicatorStateToString(application::IndicatorState state) {
        switch (state) {
            case application::IndicatorState::OFF: return "off";
            case application::IndicatorState::LEFT: return "left";
            case application::IndicatorState::RIGHT: return "right";
            case application::IndicatorState::HAZARD: return "hazard";
            default: return "off";
        }
    }
    
    static application::IndicatorState StringToIndicatorState(const std::string& str) {
        if (str == "left") return application::IndicatorState::LEFT;
        if (str == "right") return application::IndicatorState::RIGHT;
        if (str == "hazard") return application::IndicatorState::HAZARD;
        return application::IndicatorState::OFF;
    }
    
    static std::string PositionLightStateToString(application::PositionLightState state) {
        return (state == application::PositionLightState::ON) ? "on" : "off";
    }
    
    static application::PositionLightState StringToPositionLightState(const std::string& str) {
        return (str == "on") ? application::PositionLightState::ON : application::PositionLightState::OFF;
    }
    
    static std::string SeatAxisToString(application::SeatAxis axis) {
        return (axis == application::SeatAxis::FORWARD_BACKWARD) ? "forward_backward" : "recline";
    }
    
    static application::SeatAxis StringToSeatAxis(const std::string& str) {
        return (str == "forward_backward") ? application::SeatAxis::FORWARD_BACKWARD : application::SeatAxis::RECLINE;
    }
    
    static std::string SeatDirectionToString(application::SeatDirection dir) {
        switch (dir) {
            case application::SeatDirection::POSITIVE: return "positive";
            case application::SeatDirection::NEGATIVE: return "negative";
            case application::SeatDirection::STOP: return "stop";
            default: return "stop";
        }
    }
    
    static application::SeatDirection StringToSeatDirection(const std::string& str) {
        if (str == "positive") return application::SeatDirection::POSITIVE;
        if (str == "negative") return application::SeatDirection::NEGATIVE;
        return application::SeatDirection::STOP;
    }

    // ========================================================================
    // JSON请求/响应格式定义
    // ========================================================================
    
    // 车窗位置设置请求
    static nlohmann::json ToJson(const application::SetWindowPositionReq& req) {
        return nlohmann::json{
            {"window_id", PositionToString(req.windowID)},
            {"position", req.position}
        };
    }
    
    static application::SetWindowPositionReq FromJson(const nlohmann::json& json) {
        application::SetWindowPositionReq req;
        req.windowID = StringToPosition(json["window_id"]);
        req.position = json["position"];
        return req;
    }
    
    // 车窗位置设置响应
    static nlohmann::json ToJson(const application::SetWindowPositionResp& resp) {
        return nlohmann::json{
            {"window_id", PositionToString(resp.windowID)},
            {"result", ResultToString(resp.result)}
        };
    }
    
    // 车窗控制请求
    static nlohmann::json ToJson(const application::ControlWindowReq& req) {
        return nlohmann::json{
            {"window_id", PositionToString(req.windowID)},
            {"command", WindowCommandToString(req.command)}
        };
    }
    
    static application::ControlWindowReq ControlWindowFromJson(const nlohmann::json& json) {
        application::ControlWindowReq req;
        req.windowID = StringToPosition(json["window_id"]);
        req.command = StringToWindowCommand(json["command"]);
        return req;
    }
    
    // 车门锁定请求
    static nlohmann::json ToJson(const application::SetLockStateReq& req) {
        return nlohmann::json{
            {"door_id", PositionToString(req.doorID)},
            {"command", LockCommandToString(req.command)}
        };
    }
    
    static application::SetLockStateReq LockStateFromJson(const nlohmann::json& json) {
        application::SetLockStateReq req;
        req.doorID = StringToPosition(json["door_id"]);
        req.command = StringToLockCommand(json["command"]);
        return req;
    }
    
    // 前大灯设置请求
    static nlohmann::json ToJson(const application::SetHeadlightStateReq& req) {
        return nlohmann::json{
            {"command", HeadlightStateToString(req.command)}
        };
    }
    
    static application::SetHeadlightStateReq HeadlightFromJson(const nlohmann::json& json) {
        application::SetHeadlightStateReq req;
        req.command = StringToHeadlightState(json["command"]);
        return req;
    }
    
    // 转向灯设置请求
    static nlohmann::json ToJson(const application::SetIndicatorStateReq& req) {
        return nlohmann::json{
            {"command", IndicatorStateToString(req.command)}
        };
    }
    
    static application::SetIndicatorStateReq IndicatorFromJson(const nlohmann::json& json) {
        application::SetIndicatorStateReq req;
        req.command = StringToIndicatorState(json["command"]);
        return req;
    }
    
    // 位置灯设置请求
    static nlohmann::json ToJson(const application::SetPositionLightStateReq& req) {
        return nlohmann::json{
            {"command", PositionLightStateToString(req.command)}
        };
    }
    
    static application::SetPositionLightStateReq PositionLightFromJson(const nlohmann::json& json) {
        application::SetPositionLightStateReq req;
        req.command = StringToPositionLightState(json["command"]);
        return req;
    }
    
    // 座椅调节请求
    static nlohmann::json ToJson(const application::AdjustSeatReq& req) {
        return nlohmann::json{
            {"axis", SeatAxisToString(req.axis)},
            {"direction", SeatDirectionToString(req.direction)}
        };
    }
    
    static application::AdjustSeatReq SeatAdjustFromJson(const nlohmann::json& json) {
        application::AdjustSeatReq req;
        req.axis = StringToSeatAxis(json["axis"]);
        req.direction = StringToSeatDirection(json["direction"]);
        return req;
    }
    
    // 记忆位置恢复请求
    static nlohmann::json ToJson(const application::RecallMemoryPositionReq& req) {
        return nlohmann::json{
            {"preset_id", req.presetID}
        };
    }
    
    static application::RecallMemoryPositionReq RecallMemoryFromJson(const nlohmann::json& json) {
        application::RecallMemoryPositionReq req;
        req.presetID = json["preset_id"];
        return req;
    }
    
    // 记忆位置保存请求
    static nlohmann::json ToJson(const application::SaveMemoryPositionReq& req) {
        return nlohmann::json{
            {"preset_id", req.presetID}
        };
    }
    
    static application::SaveMemoryPositionReq SaveMemoryFromJson(const nlohmann::json& json) {
        application::SaveMemoryPositionReq req;
        req.presetID = json["preset_id"];
        return req;
    }
    
    // ========================================================================
    // 系统状态JSON格式
    // ========================================================================
    
    // 系统状态响应
    static nlohmann::json CreateSystemStatusResponse(bool connected, 
                                                     const std::string& stm32_ip,
                                                     uint32_t latency_ms,
                                                     uint32_t error_count) {
        return nlohmann::json{
            {"connected", connected},
            {"stm32_ip", stm32_ip},
            {"latency_ms", latency_ms},
            {"error_count", error_count},
            {"timestamp", std::time(nullptr)}
        };
    }
    
    // 健康检查响应
    static nlohmann::json CreateHealthResponse(const std::string& status,
                                              const std::vector<std::string>& services) {
        return nlohmann::json{
            {"status", status},
            {"services", services},
            {"timestamp", std::time(nullptr)}
        };
    }
    
    // 错误响应
    static nlohmann::json CreateErrorResponse(const std::string& error,
                                             const std::string& message,
                                             int code = 0) {
        return nlohmann::json{
            {"error", error},
            {"message", message},
            {"code", code},
            {"timestamp", std::time(nullptr)}
        };
    }
};

} // namespace interface
} // namespace body_controller
