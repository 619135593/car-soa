#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <type_traits>
#include "application/data_structures.h"

namespace body_controller {
namespace communication {

/**
 * @brief SOME/IP消息序列化/反序列化工具
 * 
 * 提供车身控制器数据结构的序列化和反序列化功能
 * 遵循SOME/IP协议的数据格式规范
 */
class Serializer {
public:
    using ByteBuffer = std::vector<uint8_t>;

    // ========================================================================
    // 基础类型序列化
    // ========================================================================
    
    /**
     * @brief 序列化uint8_t
     */
    static void Serialize(ByteBuffer& buffer, uint8_t value) {
        buffer.push_back(value);
    }
    
    /**
     * @brief 反序列化uint8_t
     */
    static bool Deserialize(const ByteBuffer& buffer, size_t& offset, uint8_t& value) {
        if (offset >= buffer.size()) return false;
        value = buffer[offset++];
        return true;
    }
    
    /**
     * @brief 序列化枚举类型
     */
    template<typename EnumType>
    static void SerializeEnum(ByteBuffer& buffer, EnumType value) {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");
        Serialize(buffer, static_cast<uint8_t>(value));
    }
    
    /**
     * @brief 反序列化枚举类型
     */
    template<typename EnumType>
    static bool DeserializeEnum(const ByteBuffer& buffer, size_t& offset, EnumType& value) {
        static_assert(std::is_enum_v<EnumType>, "Type must be an enum");
        uint8_t raw_value;
        if (!Deserialize(buffer, offset, raw_value)) return false;
        value = static_cast<EnumType>(raw_value);
        return true;
    }

    // ========================================================================
    // 车窗服务数据结构序列化
    // ========================================================================
    
    static ByteBuffer Serialize(const application::SetWindowPositionReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.windowID);
        Serialize(buffer, req.position);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetWindowPositionReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.windowID) &&
               Deserialize(buffer, offset, req.position);
    }
    
    static ByteBuffer Serialize(const application::SetWindowPositionResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.windowID);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetWindowPositionResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.windowID) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::ControlWindowReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.windowID);
        SerializeEnum(buffer, req.command);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::ControlWindowReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.windowID) &&
               DeserializeEnum(buffer, offset, req.command);
    }
    
    static ByteBuffer Serialize(const application::ControlWindowResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.windowID);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::ControlWindowResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.windowID) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::GetWindowPositionReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.windowID);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::GetWindowPositionReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.windowID);
    }
    
    static ByteBuffer Serialize(const application::GetWindowPositionResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.windowID);
        Serialize(buffer, resp.position);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::GetWindowPositionResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.windowID) &&
               Deserialize(buffer, offset, resp.position);
    }
    
    static ByteBuffer Serialize(const application::OnWindowPositionChangedData& data) {
        ByteBuffer buffer;
        SerializeEnum(buffer, data.windowID);
        Serialize(buffer, data.newPosition);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::OnWindowPositionChangedData& data) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, data.windowID) &&
               Deserialize(buffer, offset, data.newPosition);
    }

    // ========================================================================
    // 车门服务数据结构序列化
    // ========================================================================
    
    static ByteBuffer Serialize(const application::SetLockStateReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.doorID);
        SerializeEnum(buffer, req.command);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetLockStateReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.doorID) &&
               DeserializeEnum(buffer, offset, req.command);
    }
    
    static ByteBuffer Serialize(const application::SetLockStateResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.doorID);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetLockStateResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.doorID) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::GetLockStateReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.doorID);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::GetLockStateReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.doorID);
    }
    
    static ByteBuffer Serialize(const application::GetLockStateResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.doorID);
        SerializeEnum(buffer, resp.lockState);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::GetLockStateResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.doorID) &&
               DeserializeEnum(buffer, offset, resp.lockState);
    }
    
    static ByteBuffer Serialize(const application::OnLockStateChangedData& data) {
        ByteBuffer buffer;
        SerializeEnum(buffer, data.doorID);
        SerializeEnum(buffer, data.newLockState);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::OnLockStateChangedData& data) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, data.doorID) &&
               DeserializeEnum(buffer, offset, data.newLockState);
    }
    
    static ByteBuffer Serialize(const application::OnDoorStateChangedData& data) {
        ByteBuffer buffer;
        SerializeEnum(buffer, data.doorID);
        SerializeEnum(buffer, data.newDoorState);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::OnDoorStateChangedData& data) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, data.doorID) &&
               DeserializeEnum(buffer, offset, data.newDoorState);
    }

    // ========================================================================
    // 灯光服务数据结构序列化
    // ========================================================================
    
    static ByteBuffer Serialize(const application::SetHeadlightStateReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.command);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetHeadlightStateReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.command);
    }
    
    static ByteBuffer Serialize(const application::SetHeadlightStateResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.newState);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetHeadlightStateResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.newState) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::SetIndicatorStateReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.command);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetIndicatorStateReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.command);
    }
    
    static ByteBuffer Serialize(const application::SetIndicatorStateResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.newState);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetIndicatorStateResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.newState) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::SetPositionLightStateReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.command);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetPositionLightStateReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.command);
    }
    
    static ByteBuffer Serialize(const application::SetPositionLightStateResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.newState);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SetPositionLightStateResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.newState) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::OnLightStateChangedData& data) {
        ByteBuffer buffer;
        SerializeEnum(buffer, data.lightType);
        Serialize(buffer, data.newState);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::OnLightStateChangedData& data) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, data.lightType) &&
               Deserialize(buffer, offset, data.newState);
    }

    // ========================================================================
    // 座椅服务数据结构序列化
    // ========================================================================
    
    static ByteBuffer Serialize(const application::AdjustSeatReq& req) {
        ByteBuffer buffer;
        SerializeEnum(buffer, req.axis);
        SerializeEnum(buffer, req.direction);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::AdjustSeatReq& req) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, req.axis) &&
               DeserializeEnum(buffer, offset, req.direction);
    }
    
    static ByteBuffer Serialize(const application::AdjustSeatResp& resp) {
        ByteBuffer buffer;
        SerializeEnum(buffer, resp.axis);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::AdjustSeatResp& resp) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, resp.axis) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::RecallMemoryPositionReq& req) {
        ByteBuffer buffer;
        Serialize(buffer, req.presetID);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::RecallMemoryPositionReq& req) {
        size_t offset = 0;
        return Deserialize(buffer, offset, req.presetID);
    }
    
    static ByteBuffer Serialize(const application::RecallMemoryPositionResp& resp) {
        ByteBuffer buffer;
        Serialize(buffer, resp.presetID);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::RecallMemoryPositionResp& resp) {
        size_t offset = 0;
        return Deserialize(buffer, offset, resp.presetID) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::SaveMemoryPositionReq& req) {
        ByteBuffer buffer;
        Serialize(buffer, req.presetID);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SaveMemoryPositionReq& req) {
        size_t offset = 0;
        return Deserialize(buffer, offset, req.presetID);
    }
    
    static ByteBuffer Serialize(const application::SaveMemoryPositionResp& resp) {
        ByteBuffer buffer;
        Serialize(buffer, resp.presetID);
        SerializeEnum(buffer, resp.result);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::SaveMemoryPositionResp& resp) {
        size_t offset = 0;
        return Deserialize(buffer, offset, resp.presetID) &&
               DeserializeEnum(buffer, offset, resp.result);
    }
    
    static ByteBuffer Serialize(const application::OnSeatPositionChangedData& data) {
        ByteBuffer buffer;
        SerializeEnum(buffer, data.axis);
        Serialize(buffer, data.newPosition);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::OnSeatPositionChangedData& data) {
        size_t offset = 0;
        return DeserializeEnum(buffer, offset, data.axis) &&
               Deserialize(buffer, offset, data.newPosition);
    }
    
    static ByteBuffer Serialize(const application::OnMemorySaveConfirmData& data) {
        ByteBuffer buffer;
        Serialize(buffer, data.presetID);
        SerializeEnum(buffer, data.saveResult);
        return buffer;
    }
    
    static bool Deserialize(const ByteBuffer& buffer, application::OnMemorySaveConfirmData& data) {
        size_t offset = 0;
        return Deserialize(buffer, offset, data.presetID) &&
               DeserializeEnum(buffer, offset, data.saveResult);
    }
};

} // namespace communication
} // namespace body_controller
