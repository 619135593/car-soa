#include "common/serializer.h"
#include <iostream>
#include <cstring>

namespace body_controller {
namespace services {

// ============================================================================
// 车门服务序列化实现
// ============================================================================

std::vector<uint8_t> Serializer::Serialize(const application::SetLockStateReq& request) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(request.doorID));
    WriteToBuffer(buffer, static_cast<uint8_t>(request.command));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::SetLockStateReq& request) {
    if (data.size() < 2) return false;
    
    size_t offset = 0;
    uint8_t door_id, command;
    
    if (!ReadFromBuffer(data, offset, door_id) || 
        !ReadFromBuffer(data, offset, command)) {
        return false;
    }
    
    request.doorID = static_cast<application::Position>(door_id);
    request.command = static_cast<application::LockCommand>(command);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::SetLockStateResp& response) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(response.doorID));
    WriteToBuffer(buffer, static_cast<uint8_t>(response.result));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::SetLockStateResp& response) {
    if (data.size() < 2) return false;
    
    size_t offset = 0;
    uint8_t door_id, result;
    
    if (!ReadFromBuffer(data, offset, door_id) || 
        !ReadFromBuffer(data, offset, result)) {
        return false;
    }
    
    response.doorID = static_cast<application::Position>(door_id);
    response.result = static_cast<application::Result>(result);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::GetLockStateReq& request) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(request.doorID));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::GetLockStateReq& request) {
    if (data.size() < 1) return false;
    
    size_t offset = 0;
    uint8_t door_id;
    
    if (!ReadFromBuffer(data, offset, door_id)) {
        return false;
    }
    
    request.doorID = static_cast<application::Position>(door_id);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::GetLockStateResp& response) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(response.doorID));
    WriteToBuffer(buffer, static_cast<uint8_t>(response.lockState));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::GetLockStateResp& response) {
    if (data.size() < 2) return false;

    size_t offset = 0;
    uint8_t door_id, lock_state;

    if (!ReadFromBuffer(data, offset, door_id) ||
        !ReadFromBuffer(data, offset, lock_state)) {
        return false;
    }

    response.doorID = static_cast<application::Position>(door_id);
    response.lockState = static_cast<application::LockState>(lock_state);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::OnLockStateChangedData& event) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(event.doorID));
    WriteToBuffer(buffer, static_cast<uint8_t>(event.newLockState));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::OnLockStateChangedData& event) {
    if (data.size() < 2) return false;
    
    size_t offset = 0;
    uint8_t door_id, lock_state;
    
    if (!ReadFromBuffer(data, offset, door_id) || 
        !ReadFromBuffer(data, offset, lock_state)) {
        return false;
    }
    
    event.doorID = static_cast<application::Position>(door_id);
    event.newLockState = static_cast<application::LockState>(lock_state);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::OnDoorStateChangedData& event) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(event.doorID));
    WriteToBuffer(buffer, static_cast<uint8_t>(event.newDoorState));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::OnDoorStateChangedData& event) {
    if (data.size() < 2) return false;
    
    size_t offset = 0;
    uint8_t door_id, door_state;
    
    if (!ReadFromBuffer(data, offset, door_id) || 
        !ReadFromBuffer(data, offset, door_state)) {
        return false;
    }
    
    event.doorID = static_cast<application::Position>(door_id);
    event.newDoorState = static_cast<application::DoorState>(door_state);
    return true;
}

// ============================================================================
// 车窗服务序列化实现
// ============================================================================

std::vector<uint8_t> Serializer::Serialize(const application::SetWindowPositionReq& request) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(request.windowID));
    WriteToBuffer(buffer, request.position);
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::SetWindowPositionReq& request) {
    if (data.size() < 2) return false;

    size_t offset = 0;
    uint8_t window_id;

    if (!ReadFromBuffer(data, offset, window_id) ||
        !ReadFromBuffer(data, offset, request.position)) {
        return false;
    }

    request.windowID = static_cast<application::Position>(window_id);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::ControlWindowReq& request) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(request.windowID));
    WriteToBuffer(buffer, static_cast<uint8_t>(request.command));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::ControlWindowReq& request) {
    if (data.size() < 2) return false;
    
    size_t offset = 0;
    uint8_t window_id, command;
    
    if (!ReadFromBuffer(data, offset, window_id) || 
        !ReadFromBuffer(data, offset, command)) {
        return false;
    }
    
    request.windowID = static_cast<application::Position>(window_id);
    request.command = static_cast<application::WindowCommand>(command);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::GetWindowPositionReq& request) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(request.windowID));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::GetWindowPositionReq& request) {
    if (data.size() < 1) return false;
    
    size_t offset = 0;
    uint8_t window_id;
    
    if (!ReadFromBuffer(data, offset, window_id)) {
        return false;
    }
    
    request.windowID = static_cast<application::Position>(window_id);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::GetWindowPositionResp& response) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(response.windowID));
    WriteToBuffer(buffer, response.position);
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::GetWindowPositionResp& response) {
    if (data.size() < 2) return false;

    size_t offset = 0;
    uint8_t window_id;

    if (!ReadFromBuffer(data, offset, window_id) ||
        !ReadFromBuffer(data, offset, response.position)) {
        return false;
    }

    response.windowID = static_cast<application::Position>(window_id);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::OnWindowPositionChangedData& event) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(event.windowID));
    WriteToBuffer(buffer, event.newPosition);
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::OnWindowPositionChangedData& event) {
    if (data.size() < 2) return false;
    
    size_t offset = 0;
    uint8_t window_id;
    
    if (!ReadFromBuffer(data, offset, window_id) || 
        !ReadFromBuffer(data, offset, event.newPosition)) {
        return false;
    }
    
    event.windowID = static_cast<application::Position>(window_id);
    return true;
}

// ============================================================================
// 灯光服务序列化实现
// ============================================================================

std::vector<uint8_t> Serializer::Serialize(const application::SetHeadlightStateReq& request) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(request.command));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::SetHeadlightStateReq& request) {
    if (data.size() < 1) return false;

    size_t offset = 0;
    uint8_t command;

    if (!ReadFromBuffer(data, offset, command)) {
        return false;
    }

    request.command = static_cast<application::HeadlightState>(command);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::OnLightStateChangedData& event) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(event.lightType));
    WriteToBuffer(buffer, event.newState);
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::OnLightStateChangedData& event) {
    if (data.size() < 2) return false;

    size_t offset = 0;
    uint8_t light_type;

    if (!ReadFromBuffer(data, offset, light_type) ||
        !ReadFromBuffer(data, offset, event.newState)) {
        return false;
    }

    event.lightType = static_cast<application::LightType>(light_type);
    return true;
}

// ============================================================================
// 座椅服务序列化实现
// ============================================================================

std::vector<uint8_t> Serializer::Serialize(const application::AdjustSeatReq& request) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(request.axis));
    WriteToBuffer(buffer, static_cast<uint8_t>(request.direction));
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::AdjustSeatReq& request) {
    if (data.size() < 2) return false;

    size_t offset = 0;
    uint8_t axis, direction;

    if (!ReadFromBuffer(data, offset, axis) ||
        !ReadFromBuffer(data, offset, direction)) {
        return false;
    }

    request.axis = static_cast<application::SeatAxis>(axis);
    request.direction = static_cast<application::SeatDirection>(direction);
    return true;
}

std::vector<uint8_t> Serializer::Serialize(const application::OnSeatPositionChangedData& event) {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(event.axis));
    WriteToBuffer(buffer, event.newPosition);
    return buffer;
}

bool Serializer::Deserialize(const std::vector<uint8_t>& data, application::OnSeatPositionChangedData& event) {
    if (data.size() < 2) return false;

    size_t offset = 0;
    uint8_t axis;

    if (!ReadFromBuffer(data, offset, axis) ||
        !ReadFromBuffer(data, offset, event.newPosition)) {
        return false;
    }

    event.axis = static_cast<application::SeatAxis>(axis);
    return true;
}

// ============================================================================
// 通用响应序列化实现
// ============================================================================

// 简单的成功响应序列化
std::vector<uint8_t> Serializer::SerializeSuccessResponse() {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(application::Result::SUCCESS));
    return buffer;
}

// 简单的失败响应序列化
std::vector<uint8_t> Serializer::SerializeFailResponse() {
    std::vector<uint8_t> buffer;
    WriteToBuffer(buffer, static_cast<uint8_t>(application::Result::FAIL));
    return buffer;
}

} // namespace services
} // namespace body_controller
