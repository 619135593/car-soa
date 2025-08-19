#pragma once

#include <memory>
#include <vsomeip/vsomeip.hpp>
#include "application/data_structures.h"
#include "communication/someip_service_definitions.h"
#include "common/hardware_simulator.h"

namespace body_controller {
namespace services {

/**
 * @brief 灯光服务实现
 * 提供前大灯、转向灯、位置灯控制功能
 */
class LightService {
public:
    /**
     * @brief 构造函数
     * @param app VSOMEIP应用程序实例
     * @param simulator 硬件模拟器实例
     */
    explicit LightService(std::shared_ptr<vsomeip::application> app, 
                         std::shared_ptr<HardwareSimulator> simulator);
    
    /**
     * @brief 析构函数
     */
    ~LightService();

    /**
     * @brief 初始化服务
     * @return 成功返回true，失败返回false
     */
    bool Initialize();
    
    /**
     * @brief 启动服务
     * @return 成功返回true，失败返回false
     */
    bool Start();
    
    /**
     * @brief 停止服务
     */
    void Stop();
    
    /**
     * @brief 检查服务是否正在运行
     */
    bool IsRunning() const { return running_; }

private:
    /**
     * @brief 处理前大灯状态设置请求
     */
    void HandleSetHeadlightStateRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 处理转向灯状态设置请求
     */
    void HandleSetIndicatorStateRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 处理位置灯状态设置请求
     */
    void HandleSetPositionLightStateRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 发送响应消息
     */
    void SendResponse(const std::shared_ptr<vsomeip::message>& request,
                     const std::vector<uint8_t>& payload);
    
    /**
     * @brief 发送错误响应
     */
    void SendErrorResponse(const std::shared_ptr<vsomeip::message>& request,
                          uint8_t error_code);

    /**
     * @brief 硬件事件处理器：灯光状态变化
     */
    void OnLightStateChanged(const application::OnLightStateChangedData& event_data);
    
    /**
     * @brief 发送灯光状态变化事件
     */
    void SendLightStateChangedEvent(const application::OnLightStateChangedData& event_data);

    /**
     * @brief 模拟前大灯状态设置操作
     * @param state 目标状态
     * @return 操作结果
     */
    application::Result SimulateHeadlightOperation(application::HeadlightState state);
    
    /**
     * @brief 模拟转向灯状态设置操作
     * @param state 目标状态
     * @return 操作结果
     */
    application::Result SimulateIndicatorOperation(application::IndicatorState state);
    
    /**
     * @brief 模拟位置灯状态设置操作
     * @param state 目标状态
     * @return 操作结果
     */
    application::Result SimulatePositionLightOperation(application::PositionLightState state);

private:
    // VSOMEIP相关
    std::shared_ptr<vsomeip::application> app_;
    bool running_;
    
    // 硬件模拟器
    std::shared_ptr<HardwareSimulator> hardware_simulator_;
    
    // 当前状态存储
    application::HeadlightState current_headlight_state_;
    application::IndicatorState current_indicator_state_;
    application::PositionLightState current_position_light_state_;
    
    // 服务配置
    static constexpr vsomeip::service_t SERVICE_ID = communication::LIGHT_SERVICE_ID;
    static constexpr vsomeip::instance_t INSTANCE_ID = communication::LIGHT_INSTANCE_ID;
    static constexpr vsomeip::major_version_t MAJOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MAJOR;
    static constexpr vsomeip::minor_version_t MINOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MINOR;
    
    // 方法ID
    static constexpr vsomeip::method_t SET_HEADLIGHT_STATE_METHOD = communication::light_service::SET_HEADLIGHT_STATE;
    static constexpr vsomeip::method_t SET_INDICATOR_STATE_METHOD = communication::light_service::SET_INDICATOR_STATE;
    static constexpr vsomeip::method_t SET_POSITION_LIGHT_STATE_METHOD = communication::light_service::SET_POSITION_LIGHT_STATE;
    
    // 事件ID
    static constexpr vsomeip::event_t LIGHT_STATE_CHANGED_EVENT = communication::light_events::ON_LIGHT_STATE_CHANGED;
    
    // 事件组ID
    static constexpr vsomeip::eventgroup_t EVENT_GROUP = 0x0001;
};

} // namespace services
} // namespace body_controller
