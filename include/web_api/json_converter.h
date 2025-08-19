#pragma once

#include <nlohmann/json.hpp>
#include <string>
#include <ctime>
#include "application/data_structures.h"

namespace body_controller {
namespace web_api {

/**
 * @brief JSON转换器类
 * 
 * 负责在C++数据结构和JSON之间进行转换
 * 支持所有车身域控制器的数据结构
 */
class JsonConverter {
public:
    // ============================================================================
    // 车门服务JSON转换
    // ============================================================================
    
    static nlohmann::json ToJson(const application::SetLockStateReq& req);
    static nlohmann::json ToJson(const application::SetLockStateResp& resp);
    static nlohmann::json ToJson(const application::GetLockStateReq& req);
    static nlohmann::json ToJson(const application::GetLockStateResp& resp);
    static nlohmann::json ToJson(const application::OnLockStateChangedData& data);
    static nlohmann::json ToJson(const application::OnDoorStateChangedData& data);
    
    // ============================================================================
    // 车窗服务JSON转换
    // ============================================================================
    
    static nlohmann::json ToJson(const application::SetWindowPositionReq& req);
    static nlohmann::json ToJson(const application::SetWindowPositionResp& resp);
    static nlohmann::json ToJson(const application::ControlWindowReq& req);
    static nlohmann::json ToJson(const application::ControlWindowResp& resp);
    static nlohmann::json ToJson(const application::GetWindowPositionReq& req);
    static nlohmann::json ToJson(const application::GetWindowPositionResp& resp);
    static nlohmann::json ToJson(const application::OnWindowPositionChangedData& data);
    
    // ============================================================================
    // 灯光服务JSON转换
    // ============================================================================
    
    static nlohmann::json ToJson(const application::SetHeadlightStateReq& req);
    static nlohmann::json ToJson(const application::SetHeadlightStateResp& resp);
    static nlohmann::json ToJson(const application::SetIndicatorStateReq& req);
    static nlohmann::json ToJson(const application::SetIndicatorStateResp& resp);
    static nlohmann::json ToJson(const application::SetPositionLightStateReq& req);
    static nlohmann::json ToJson(const application::SetPositionLightStateResp& resp);
    static nlohmann::json ToJson(const application::OnLightStateChangedData& data);
    
    // ============================================================================
    // 座椅服务JSON转换
    // ============================================================================
    
    static nlohmann::json ToJson(const application::AdjustSeatReq& req);
    static nlohmann::json ToJson(const application::AdjustSeatResp& resp);
    static nlohmann::json ToJson(const application::RecallMemoryPositionReq& req);
    static nlohmann::json ToJson(const application::RecallMemoryPositionResp& resp);
    static nlohmann::json ToJson(const application::SaveMemoryPositionReq& req);
    static nlohmann::json ToJson(const application::SaveMemoryPositionResp& resp);
    static nlohmann::json ToJson(const application::OnSeatPositionChangedData& data);
    static nlohmann::json ToJson(const application::OnMemorySaveConfirmData& data);
    
    // ============================================================================
    // JSON到C++结构体的转换
    // ============================================================================
    
    // 车门服务
    static application::SetLockStateReq FromJson(const nlohmann::json& j, application::SetLockStateReq*);
    static application::GetLockStateReq FromJson(const nlohmann::json& j, application::GetLockStateReq*);
    
    // 车窗服务
    static application::SetWindowPositionReq FromJson(const nlohmann::json& j, application::SetWindowPositionReq*);
    static application::ControlWindowReq FromJson(const nlohmann::json& j, application::ControlWindowReq*);
    static application::GetWindowPositionReq FromJson(const nlohmann::json& j, application::GetWindowPositionReq*);
    
    // 灯光服务
    static application::SetHeadlightStateReq FromJson(const nlohmann::json& j, application::SetHeadlightStateReq*);
    static application::SetIndicatorStateReq FromJson(const nlohmann::json& j, application::SetIndicatorStateReq*);
    static application::SetPositionLightStateReq FromJson(const nlohmann::json& j, application::SetPositionLightStateReq*);
    
    // 座椅服务
    static application::AdjustSeatReq FromJson(const nlohmann::json& j, application::AdjustSeatReq*);
    static application::RecallMemoryPositionReq FromJson(const nlohmann::json& j, application::RecallMemoryPositionReq*);
    static application::SaveMemoryPositionReq FromJson(const nlohmann::json& j, application::SaveMemoryPositionReq*);
    
    // ============================================================================
    // 通用响应创建
    // ============================================================================
    
    /**
     * @brief 创建错误响应
     * @param error 错误类型
     * @param message 错误消息
     * @return JSON格式的错误响应
     */
    static nlohmann::json CreateErrorResponse(const std::string& error, const std::string& message);
    
    /**
     * @brief 创建成功响应
     * @param data 响应数据
     * @return JSON格式的成功响应
     */
    static nlohmann::json CreateSuccessResponse(const nlohmann::json& data);
    
    // ============================================================================
    // 模板方法用于类型安全的转换
    // ============================================================================
    
    /**
     * @brief 模板方法：从JSON转换为指定类型
     * @tparam T 目标类型
     * @param j JSON对象
     * @return 转换后的C++对象
     */
    template<typename T>
    static T FromJson(const nlohmann::json& j) {
        return FromJson(j, static_cast<T*>(nullptr));
    }
    
    /**
     * @brief 模板方法：将C++对象转换为JSON
     * @tparam T 源类型
     * @param obj C++对象
     * @return JSON对象
     */
    template<typename T>
    static nlohmann::json ToJson(const T& obj) {
        return ToJson(obj);
    }
};

// ============================================================================
// 枚举值到字符串的转换（用于调试和日志）
// ============================================================================

/**
 * @brief 枚举值转换器
 * 提供枚举值到可读字符串的转换
 */
class EnumConverter {
public:
    // 车门相关枚举
    static std::string ToString(application::Position position);
    static std::string ToString(application::LockState lockState);
    static std::string ToString(application::DoorState doorState);

    // 车窗相关枚举
    static std::string ToString(application::WindowCommand command);
    
    // 灯光相关枚举
    static std::string ToString(application::HeadlightState state);
    static std::string ToString(application::IndicatorState state);
    static std::string ToString(application::PositionLightState state);
    static std::string ToString(application::LightType type);
    
    // 座椅相关枚举
    static std::string ToString(application::SeatAxis axis);
    static std::string ToString(application::SeatDirection direction);
    
    // 通用枚举
    static std::string ToString(application::Result result);
};

} // namespace web_api
} // namespace body_controller
