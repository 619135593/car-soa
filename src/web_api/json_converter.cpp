#include "web_api/json_converter.h"
#include <nlohmann/json.hpp>

namespace body_controller {
namespace web_api {

using json = nlohmann::json;

// ============================================================================
// 车门服务JSON转换
// ============================================================================

json JsonConverter::ToJson(const application::SetLockStateReq& req) {
    return json{
        {"doorID", static_cast<int>(req.doorID)},
        {"command", static_cast<int>(req.command)}
    };
}

json JsonConverter::ToJson(const application::SetLockStateResp& resp) {
    return json{
        {"doorID", static_cast<int>(resp.doorID)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::GetLockStateReq& req) {
    return json{
        {"doorID", static_cast<int>(req.doorID)}
    };
}

json JsonConverter::ToJson(const application::GetLockStateResp& resp) {
    return json{
        {"doorID", static_cast<int>(resp.doorID)},
        {"lockState", static_cast<int>(resp.lockState)}
    };
}

json JsonConverter::ToJson(const application::OnLockStateChangedData& data) {
    return json{
        {"doorID", static_cast<int>(data.doorID)},
        {"newLockState", static_cast<int>(data.newLockState)}
    };
}

json JsonConverter::ToJson(const application::OnDoorStateChangedData& data) {
    return json{
        {"doorID", static_cast<int>(data.doorID)},
        {"newDoorState", static_cast<int>(data.newDoorState)}
    };
}

// ============================================================================
// 车窗服务JSON转换
// ============================================================================

json JsonConverter::ToJson(const application::SetWindowPositionReq& req) {
    return json{
        {"windowID", static_cast<int>(req.windowID)},
        {"position", static_cast<int>(req.position)}
    };
}

json JsonConverter::ToJson(const application::SetWindowPositionResp& resp) {
    return json{
        {"windowID", static_cast<int>(resp.windowID)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::ControlWindowReq& req) {
    return json{
        {"windowID", static_cast<int>(req.windowID)},
        {"command", static_cast<int>(req.command)}
    };
}

json JsonConverter::ToJson(const application::ControlWindowResp& resp) {
    return json{
        {"windowID", static_cast<int>(resp.windowID)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::GetWindowPositionReq& req) {
    return json{
        {"windowID", static_cast<int>(req.windowID)}
    };
}

json JsonConverter::ToJson(const application::GetWindowPositionResp& resp) {
    return json{
        {"windowID", static_cast<int>(resp.windowID)},
        {"position", static_cast<int>(resp.position)}
    };
}

json JsonConverter::ToJson(const application::OnWindowPositionChangedData& data) {
    return json{
        {"windowID", static_cast<int>(data.windowID)},
        {"newPosition", static_cast<int>(data.newPosition)}
    };
}

// ============================================================================
// 灯光服务JSON转换
// ============================================================================

json JsonConverter::ToJson(const application::SetHeadlightStateReq& req) {
    return json{
        {"command", static_cast<int>(req.command)}
    };
}

json JsonConverter::ToJson(const application::SetHeadlightStateResp& resp) {
    return json{
        {"newState", static_cast<int>(resp.newState)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::SetIndicatorStateReq& req) {
    return json{
        {"command", static_cast<int>(req.command)}
    };
}

json JsonConverter::ToJson(const application::SetIndicatorStateResp& resp) {
    return json{
        {"newState", static_cast<int>(resp.newState)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::SetPositionLightStateReq& req) {
    return json{
        {"command", static_cast<int>(req.command)}
    };
}

json JsonConverter::ToJson(const application::SetPositionLightStateResp& resp) {
    return json{
        {"newState", static_cast<int>(resp.newState)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::OnLightStateChangedData& data) {
    return json{
        {"lightType", static_cast<int>(data.lightType)},
        {"newState", static_cast<int>(data.newState)}
    };
}

// ============================================================================
// 座椅服务JSON转换
// ============================================================================

json JsonConverter::ToJson(const application::AdjustSeatReq& req) {
    return json{
        {"axis", static_cast<int>(req.axis)},
        {"direction", static_cast<int>(req.direction)}
    };
}

json JsonConverter::ToJson(const application::AdjustSeatResp& resp) {
    return json{
        {"axis", static_cast<int>(resp.axis)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::RecallMemoryPositionReq& req) {
    return json{
        {"presetID", static_cast<int>(req.presetID)}
    };
}

json JsonConverter::ToJson(const application::RecallMemoryPositionResp& resp) {
    return json{
        {"presetID", static_cast<int>(resp.presetID)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::SaveMemoryPositionReq& req) {
    return json{
        {"presetID", static_cast<int>(req.presetID)}
    };
}

json JsonConverter::ToJson(const application::SaveMemoryPositionResp& resp) {
    return json{
        {"presetID", static_cast<int>(resp.presetID)},
        {"result", static_cast<int>(resp.result)}
    };
}

json JsonConverter::ToJson(const application::OnSeatPositionChangedData& data) {
    return json{
        {"axis", static_cast<int>(data.axis)},
        {"newPosition", static_cast<int>(data.newPosition)}
    };
}

json JsonConverter::ToJson(const application::OnMemorySaveConfirmData& data) {
    return json{
        {"presetID", static_cast<int>(data.presetID)},
        {"saveResult", static_cast<int>(data.saveResult)}
    };
}

// ============================================================================
// JSON到C++结构体的转换
// ============================================================================

application::SetLockStateReq JsonConverter::FromJson(const json& j, application::SetLockStateReq*) {
    return application::SetLockStateReq(
        static_cast<application::Position>(j["doorID"].get<int>()),
        static_cast<application::LockCommand>(j["command"].get<int>())
    );
}

application::GetLockStateReq JsonConverter::FromJson(const json& j, application::GetLockStateReq*) {
    return application::GetLockStateReq(
        static_cast<application::Position>(j["doorID"].get<int>())
    );
}

application::SetWindowPositionReq JsonConverter::FromJson(const json& j, application::SetWindowPositionReq*) {
    return application::SetWindowPositionReq(
        static_cast<application::Position>(j["windowID"].get<int>()),
        static_cast<uint8_t>(j["position"].get<int>())
    );
}

application::ControlWindowReq JsonConverter::FromJson(const json& j, application::ControlWindowReq*) {
    return application::ControlWindowReq(
        static_cast<application::Position>(j["windowID"].get<int>()),
        static_cast<application::WindowCommand>(j["command"].get<int>())
    );
}

application::GetWindowPositionReq JsonConverter::FromJson(const json& j, application::GetWindowPositionReq*) {
    return application::GetWindowPositionReq(
        static_cast<application::Position>(j["windowID"].get<int>())
    );
}

application::SetHeadlightStateReq JsonConverter::FromJson(const json& j, application::SetHeadlightStateReq*) {
    return application::SetHeadlightStateReq(
        static_cast<application::HeadlightState>(j["command"].get<int>())
    );
}

application::SetIndicatorStateReq JsonConverter::FromJson(const json& j, application::SetIndicatorStateReq*) {
    return application::SetIndicatorStateReq(
        static_cast<application::IndicatorState>(j["command"].get<int>())
    );
}

application::SetPositionLightStateReq JsonConverter::FromJson(const json& j, application::SetPositionLightStateReq*) {
    return application::SetPositionLightStateReq(
        static_cast<application::PositionLightState>(j["command"].get<int>())
    );
}

application::AdjustSeatReq JsonConverter::FromJson(const json& j, application::AdjustSeatReq*) {
    return application::AdjustSeatReq(
        static_cast<application::SeatAxis>(j["axis"].get<int>()),
        static_cast<application::SeatDirection>(j["direction"].get<int>())
    );
}

application::RecallMemoryPositionReq JsonConverter::FromJson(const json& j, application::RecallMemoryPositionReq*) {
    return application::RecallMemoryPositionReq(
        static_cast<uint8_t>(j["presetID"].get<int>())
    );
}

application::SaveMemoryPositionReq JsonConverter::FromJson(const json& j, application::SaveMemoryPositionReq*) {
    return application::SaveMemoryPositionReq(
        static_cast<uint8_t>(j["presetID"].get<int>())
    );
}

// ============================================================================
// 通用错误响应
// ============================================================================

json JsonConverter::CreateErrorResponse(const std::string& error, const std::string& message) {
    return json{
        {"error", error},
        {"message", message},
        {"timestamp", std::time(nullptr)}
    };
}

json JsonConverter::CreateSuccessResponse(const json& data) {
    return json{
        {"success", true},
        {"data", data},
        {"timestamp", std::time(nullptr)}
    };
}

} // namespace web_api
} // namespace body_controller
