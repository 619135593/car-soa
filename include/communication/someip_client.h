#pragma once

#include <memory>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <vsomeip/vsomeip.hpp>

#include "communication/someip_service_definitions.h"
#include "communication/serialization.h"
#include "application/data_structures.h"

namespace body_controller {
namespace communication {

/**
 * @brief SOME/IP客户端基类
 * 
 * 提供与STM32H7节点通信的基础功能
 * 基于vsomeip库实现标准的SOME/IP客户端
 */
class SomeipClient {
public:
    using MessageHandler = std::function<void(const std::shared_ptr<vsomeip::message>&)>;
    using AvailabilityHandler = std::function<void(vsomeip::service_t, vsomeip::instance_t, bool)>;
    using StateHandler = std::function<void(vsomeip::state_type_e)>;

protected:
    std::shared_ptr<vsomeip::runtime> runtime_;
    std::shared_ptr<vsomeip::application> app_;
    std::string application_name_;
    bool is_initialized_;
    bool is_running_;
    
    std::mutex mutex_;
    std::condition_variable condition_;

public:
    explicit SomeipClient(const std::string& app_name = "body_controller");
    virtual ~SomeipClient();

    /**
     * @brief 初始化客户端
     */
    virtual bool Initialize();

    /**
     * @brief 启动客户端
     */
    virtual void Start();

    /**
     * @brief 停止客户端
     */
    virtual void Stop();

    /**
     * @brief 检查服务是否可用
     */
    bool IsServiceAvailable(vsomeip::service_t service_id, vsomeip::instance_t instance_id) const;

protected:
    /**
     * @brief 状态变化处理器
     */
    virtual void OnState(vsomeip::state_type_e state);

    /**
     * @brief 服务可用性变化处理器
     */
    virtual void OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available);

    /**
     * @brief 消息处理器
     */
    virtual void OnMessage(const std::shared_ptr<vsomeip::message>& message);

    /**
     * @brief 发送请求消息
     */
    void SendRequest(vsomeip::service_t service_id, 
                    vsomeip::instance_t instance_id,
                    vsomeip::method_t method_id,
                    const std::vector<uint8_t>& payload_data);

    /**
     * @brief 订阅事件
     */
    void SubscribeEvent(vsomeip::service_t service_id,
                       vsomeip::instance_t instance_id,
                       vsomeip::event_t event_id,
                       vsomeip::eventgroup_t eventgroup_id);
};

/**
 * @brief 车窗服务客户端
 */
class WindowServiceClient : public SomeipClient {
public:
    using WindowPositionChangedHandler = std::function<void(const application::OnWindowPositionChangedData&)>;
    using SetWindowPositionResponseHandler = std::function<void(const application::SetWindowPositionResp&)>;
    using ControlWindowResponseHandler = std::function<void(const application::ControlWindowResp&)>;
    using GetWindowPositionResponseHandler = std::function<void(const application::GetWindowPositionResp&)>;

private:
    WindowPositionChangedHandler window_position_changed_handler_;
    SetWindowPositionResponseHandler set_position_response_handler_;
    ControlWindowResponseHandler control_response_handler_;
    GetWindowPositionResponseHandler get_position_response_handler_;

public:
    explicit WindowServiceClient(const std::string& app_name = "body_controller");

    bool Initialize() override;

    /**
     * @brief 设置车窗位置
     */
    void SetWindowPosition(const application::SetWindowPositionReq& request);

    /**
     * @brief 控制车窗
     */
    void ControlWindow(const application::ControlWindowReq& request);

    /**
     * @brief 获取车窗位置
     */
    void GetWindowPosition(const application::GetWindowPositionReq& request);

    /**
     * @brief 设置事件处理器
     */
    void SetWindowPositionChangedHandler(const WindowPositionChangedHandler& handler) {
        window_position_changed_handler_ = handler;
    }

    void SetSetWindowPositionResponseHandler(const SetWindowPositionResponseHandler& handler) {
        set_position_response_handler_ = handler;
    }

    void SetControlWindowResponseHandler(const ControlWindowResponseHandler& handler) {
        control_response_handler_ = handler;
    }

    void SetGetWindowPositionResponseHandler(const GetWindowPositionResponseHandler& handler) {
        get_position_response_handler_ = handler;
    }

protected:
    void OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) override;
    void OnMessage(const std::shared_ptr<vsomeip::message>& message) override;

private:
    void HandleWindowPositionChangedEvent(const std::shared_ptr<vsomeip::message>& message);
    void HandleSetWindowPositionResponse(const std::shared_ptr<vsomeip::message>& message);
    void HandleControlWindowResponse(const std::shared_ptr<vsomeip::message>& message);
    void HandleGetWindowPositionResponse(const std::shared_ptr<vsomeip::message>& message);
};

/**
 * @brief 车门服务客户端
 */
class DoorServiceClient : public SomeipClient {
public:
    using LockStateChangedHandler = std::function<void(const application::OnLockStateChangedData&)>;
    using DoorStateChangedHandler = std::function<void(const application::OnDoorStateChangedData&)>;
    using SetLockStateResponseHandler = std::function<void(const application::SetLockStateResp&)>;
    using GetLockStateResponseHandler = std::function<void(const application::GetLockStateResp&)>;

private:
    LockStateChangedHandler lock_state_changed_handler_;
    DoorStateChangedHandler door_state_changed_handler_;
    SetLockStateResponseHandler set_lock_response_handler_;
    GetLockStateResponseHandler get_lock_response_handler_;

public:
    explicit DoorServiceClient(const std::string& app_name = "body_controller");

    bool Initialize() override;

    /**
     * @brief 设置车门锁定状态
     */
    void SetLockState(const application::SetLockStateReq& request);

    /**
     * @brief 获取车门锁定状态
     */
    void GetLockState(const application::GetLockStateReq& request);

    /**
     * @brief 设置事件处理器
     */
    void SetLockStateChangedHandler(const LockStateChangedHandler& handler) {
        lock_state_changed_handler_ = handler;
    }

    void SetDoorStateChangedHandler(const DoorStateChangedHandler& handler) {
        door_state_changed_handler_ = handler;
    }

    void SetSetLockStateResponseHandler(const SetLockStateResponseHandler& handler) {
        set_lock_response_handler_ = handler;
    }

    void SetGetLockStateResponseHandler(const GetLockStateResponseHandler& handler) {
        get_lock_response_handler_ = handler;
    }

protected:
    void OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) override;
    void OnMessage(const std::shared_ptr<vsomeip::message>& message) override;

private:
    void HandleLockStateChangedEvent(const std::shared_ptr<vsomeip::message>& message);
    void HandleDoorStateChangedEvent(const std::shared_ptr<vsomeip::message>& message);
    void HandleSetLockStateResponse(const std::shared_ptr<vsomeip::message>& message);
    void HandleGetLockStateResponse(const std::shared_ptr<vsomeip::message>& message);
};

/**
 * @brief 灯光服务客户端
 */
class LightServiceClient : public SomeipClient {
public:
    using LightStateChangedHandler = std::function<void(const application::OnLightStateChangedData&)>;
    using SetHeadlightStateResponseHandler = std::function<void(const application::SetHeadlightStateResp&)>;
    using SetIndicatorStateResponseHandler = std::function<void(const application::SetIndicatorStateResp&)>;
    using SetPositionLightStateResponseHandler = std::function<void(const application::SetPositionLightStateResp&)>;

public:
    explicit LightServiceClient(const std::string& app_name = "body_controller");

    bool Initialize() override;

    /**
     * @brief 设置前大灯状态
     */
    void SetHeadlightState(const application::SetHeadlightStateReq& request);

    /**
     * @brief 设置转向灯状态
     */
    void SetIndicatorState(const application::SetIndicatorStateReq& request);

    /**
     * @brief 设置位置灯状态
     */
    void SetPositionLightState(const application::SetPositionLightStateReq& request);

    /**
     * @brief 设置事件处理器
     */
    void SetLightStateChangedHandler(const LightStateChangedHandler& handler) {
        light_state_changed_handler_ = handler;
    }

    void SetSetHeadlightStateResponseHandler(const SetHeadlightStateResponseHandler& handler) {
        set_headlight_response_handler_ = handler;
    }

    void SetSetIndicatorStateResponseHandler(const SetIndicatorStateResponseHandler& handler) {
        set_indicator_response_handler_ = handler;
    }

    void SetSetPositionLightStateResponseHandler(const SetPositionLightStateResponseHandler& handler) {
        set_position_light_response_handler_ = handler;
    }

protected:
    void OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) override;
    void OnMessage(const std::shared_ptr<vsomeip::message>& message) override;

private:
    void HandleLightStateChangedEvent(const std::shared_ptr<vsomeip::message>& message);
    void HandleSetHeadlightStateResponse(const std::shared_ptr<vsomeip::message>& message);
    void HandleSetIndicatorStateResponse(const std::shared_ptr<vsomeip::message>& message);
    void HandleSetPositionLightStateResponse(const std::shared_ptr<vsomeip::message>& message);

    // 私有成员变量
    LightStateChangedHandler light_state_changed_handler_;
    SetHeadlightStateResponseHandler set_headlight_response_handler_;
    SetIndicatorStateResponseHandler set_indicator_response_handler_;
    SetPositionLightStateResponseHandler set_position_light_response_handler_;
};

/**
 * @brief 座椅服务客户端
 */
class SeatServiceClient : public SomeipClient {
public:
    using SeatPositionChangedHandler = std::function<void(const application::OnSeatPositionChangedData&)>;
    using MemorySaveConfirmHandler = std::function<void(const application::OnMemorySaveConfirmData&)>;
    using AdjustSeatResponseHandler = std::function<void(const application::AdjustSeatResp&)>;
    using RecallMemoryPositionResponseHandler = std::function<void(const application::RecallMemoryPositionResp&)>;
    using SaveMemoryPositionResponseHandler = std::function<void(const application::SaveMemoryPositionResp&)>;

public:
    explicit SeatServiceClient(const std::string& app_name = "body_controller");

    bool Initialize() override;

    /**
     * @brief 调节座椅
     */
    void AdjustSeat(const application::AdjustSeatReq& request);

    /**
     * @brief 恢复记忆位置
     */
    void RecallMemoryPosition(const application::RecallMemoryPositionReq& request);

    /**
     * @brief 保存记忆位置
     */
    void SaveMemoryPosition(const application::SaveMemoryPositionReq& request);

    /**
     * @brief 设置事件处理器
     */
    void SetSeatPositionChangedHandler(const SeatPositionChangedHandler& handler) {
        seat_position_changed_handler_ = handler;
    }

    void SetMemorySaveConfirmHandler(const MemorySaveConfirmHandler& handler) {
        memory_save_confirm_handler_ = handler;
    }

    void SetAdjustSeatResponseHandler(const AdjustSeatResponseHandler& handler) {
        adjust_seat_response_handler_ = handler;
    }

    void SetRecallMemoryPositionResponseHandler(const RecallMemoryPositionResponseHandler& handler) {
        recall_memory_response_handler_ = handler;
    }

    void SetSaveMemoryPositionResponseHandler(const SaveMemoryPositionResponseHandler& handler) {
        save_memory_response_handler_ = handler;
    }

protected:
    void OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) override;
    void OnMessage(const std::shared_ptr<vsomeip::message>& message) override;

private:
    void HandleSeatPositionChangedEvent(const std::shared_ptr<vsomeip::message>& message);
    void HandleMemorySaveConfirmEvent(const std::shared_ptr<vsomeip::message>& message);
    void HandleAdjustSeatResponse(const std::shared_ptr<vsomeip::message>& message);
    void HandleRecallMemoryPositionResponse(const std::shared_ptr<vsomeip::message>& message);
    void HandleSaveMemoryPositionResponse(const std::shared_ptr<vsomeip::message>& message);

    // 私有成员变量
    SeatPositionChangedHandler seat_position_changed_handler_;
    MemorySaveConfirmHandler memory_save_confirm_handler_;
    AdjustSeatResponseHandler adjust_seat_response_handler_;
    RecallMemoryPositionResponseHandler recall_memory_response_handler_;
    SaveMemoryPositionResponseHandler save_memory_response_handler_;
};

} // namespace communication
} // namespace body_controller
