#pragma once

#include <vector>
#include <cstdint>
#include <cstring>
#include <vsomeip/vsomeip.hpp>
#include "application/data_structures.h"

namespace body_controller {
namespace services {

/**
 * @brief 序列化和反序列化工具类
 * 用于VSOMEIP消息的数据转换
 */
class Serializer {
public:
    // ============================================================================
    // 车门服务序列化
    // ============================================================================
    
    /**
     * @brief 序列化车门锁定状态设置请求
     */
    static std::vector<uint8_t> Serialize(const application::SetLockStateReq& request);
    
    /**
     * @brief 反序列化车门锁定状态设置请求
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::SetLockStateReq& request);
    
    /**
     * @brief 序列化车门锁定状态设置响应
     */
    static std::vector<uint8_t> Serialize(const application::SetLockStateResp& response);
    
    /**
     * @brief 反序列化车门锁定状态设置响应
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::SetLockStateResp& response);
    
    /**
     * @brief 序列化车门锁定状态查询请求
     */
    static std::vector<uint8_t> Serialize(const application::GetLockStateReq& request);
    
    /**
     * @brief 反序列化车门锁定状态查询请求
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::GetLockStateReq& request);
    
    /**
     * @brief 序列化车门锁定状态查询响应
     */
    static std::vector<uint8_t> Serialize(const application::GetLockStateResp& response);
    
    /**
     * @brief 反序列化车门锁定状态查询响应
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::GetLockStateResp& response);
    
    /**
     * @brief 序列化车门锁定状态变化事件
     */
    static std::vector<uint8_t> Serialize(const application::OnLockStateChangedData& event);
    
    /**
     * @brief 反序列化车门锁定状态变化事件
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::OnLockStateChangedData& event);
    
    /**
     * @brief 序列化车门开关状态变化事件
     */
    static std::vector<uint8_t> Serialize(const application::OnDoorStateChangedData& event);
    
    /**
     * @brief 反序列化车门开关状态变化事件
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::OnDoorStateChangedData& event);

    // ============================================================================
    // 车窗服务序列化
    // ============================================================================
    
    /**
     * @brief 序列化车窗位置设置请求
     */
    static std::vector<uint8_t> Serialize(const application::SetWindowPositionReq& request);
    
    /**
     * @brief 反序列化车窗位置设置请求
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::SetWindowPositionReq& request);
    
    /**
     * @brief 序列化车窗控制请求
     */
    static std::vector<uint8_t> Serialize(const application::ControlWindowReq& request);
    
    /**
     * @brief 反序列化车窗控制请求
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::ControlWindowReq& request);
    
    /**
     * @brief 序列化车窗位置查询请求
     */
    static std::vector<uint8_t> Serialize(const application::GetWindowPositionReq& request);
    
    /**
     * @brief 反序列化车窗位置查询请求
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::GetWindowPositionReq& request);
    
    /**
     * @brief 序列化车窗位置查询响应
     */
    static std::vector<uint8_t> Serialize(const application::GetWindowPositionResp& response);
    
    /**
     * @brief 反序列化车窗位置查询响应
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::GetWindowPositionResp& response);
    
    /**
     * @brief 序列化车窗位置变化事件
     */
    static std::vector<uint8_t> Serialize(const application::OnWindowPositionChangedData& event);
    
    /**
     * @brief 反序列化车窗位置变化事件
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::OnWindowPositionChangedData& event);

    // ============================================================================
    // 灯光服务序列化
    // ============================================================================
    
    /**
     * @brief 序列化前大灯状态设置请求
     */
    static std::vector<uint8_t> Serialize(const application::SetHeadlightStateReq& request);
    
    /**
     * @brief 反序列化前大灯状态设置请求
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::SetHeadlightStateReq& request);
    
    /**
     * @brief 序列化灯光状态变化事件
     */
    static std::vector<uint8_t> Serialize(const application::OnLightStateChangedData& event);
    
    /**
     * @brief 反序列化灯光状态变化事件
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::OnLightStateChangedData& event);

    // ============================================================================
    // 座椅服务序列化
    // ============================================================================
    
    /**
     * @brief 序列化座椅调节请求
     */
    static std::vector<uint8_t> Serialize(const application::AdjustSeatReq& request);
    
    /**
     * @brief 反序列化座椅调节请求
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::AdjustSeatReq& request);
    
    /**
     * @brief 序列化座椅位置变化事件
     */
    static std::vector<uint8_t> Serialize(const application::OnSeatPositionChangedData& event);
    
    /**
     * @brief 反序列化座椅位置变化事件
     */
    static bool Deserialize(const std::vector<uint8_t>& data, application::OnSeatPositionChangedData& event);

    // ============================================================================
    // 通用响应序列化
    // ============================================================================

    /**
     * @brief 序列化成功响应
     */
    static std::vector<uint8_t> SerializeSuccessResponse();

    /**
     * @brief 序列化失败响应
     */
    static std::vector<uint8_t> SerializeFailResponse();

private:
    /**
     * @brief 写入基本数据类型到缓冲区
     */
    template<typename T>
    static void WriteToBuffer(std::vector<uint8_t>& buffer, const T& value) {
        const uint8_t* ptr = reinterpret_cast<const uint8_t*>(&value);
        buffer.insert(buffer.end(), ptr, ptr + sizeof(T));
    }
    
    /**
     * @brief 从缓冲区读取基本数据类型
     */
    template<typename T>
    static bool ReadFromBuffer(const std::vector<uint8_t>& buffer, size_t& offset, T& value) {
        if (offset + sizeof(T) > buffer.size()) {
            return false;
        }
        std::memcpy(&value, buffer.data() + offset, sizeof(T));
        offset += sizeof(T);
        return true;
    }
};

} // namespace services
} // namespace body_controller
