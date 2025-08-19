#pragma once

#include <memory>
#include <random>
#include <vsomeip/vsomeip.hpp>
#include "application/data_structures.h"
#include "communication/someip_service_definitions.h"
#include "common/hardware_simulator.h"

namespace body_controller {
namespace services {

/**
 * @brief 车门服务实现
 * 提供车门锁定/解锁控制和状态查询功能
 */
class DoorService {
public:
    /**
     * @brief 构造函数
     * @param app VSOMEIP应用程序实例
     * @param simulator 硬件模拟器实例
     */
    explicit DoorService(std::shared_ptr<vsomeip::application> app, 
                        std::shared_ptr<HardwareSimulator> simulator);
    
    /**
     * @brief 析构函数
     */
    ~DoorService();

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
     * @brief 处理车门锁定状态设置请求
     */
    void HandleSetLockStateRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 处理车门锁定状态查询请求
     */
    void HandleGetLockStateRequest(const std::shared_ptr<vsomeip::message>& request);
    
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
     * @brief 硬件事件处理器：车门锁定状态变化
     */
    void OnDoorLockStateChanged(const application::OnLockStateChangedData& event_data);
    
    /**
     * @brief 硬件事件处理器：车门开关状态变化
     */
    void OnDoorStateChanged(const application::OnDoorStateChangedData& event_data);
    
    /**
     * @brief 发送车门锁定状态变化事件
     */
    void SendLockStateChangedEvent(const application::OnLockStateChangedData& event_data);
    
    /**
     * @brief 发送车门开关状态变化事件
     */
    void SendDoorStateChangedEvent(const application::OnDoorStateChangedData& event_data);

    /**
     * @brief 模拟车门锁定操作
     * @param door_id 车门ID
     * @param command 锁定命令
     * @return 操作结果
     */
    application::Result SimulateLockOperation(application::Position door_id, 
                                            application::LockCommand command);
    
    /**
     * @brief 获取当前车门锁定状态
     * @param door_id 车门ID
     * @return 当前锁定状态
     */
    application::LockState GetCurrentLockState(application::Position door_id) const;

private:
    // VSOMEIP相关
    std::shared_ptr<vsomeip::application> app_;
    bool running_;
    
    // 硬件模拟器
    std::shared_ptr<HardwareSimulator> hardware_simulator_;
    
    // 当前状态存储
    std::array<application::LockState, 4> current_lock_states_;
    std::array<application::DoorState, 4> current_door_states_;

    // 随机数生成器（用于模拟操作成功率）
    mutable std::mt19937 random_generator_;
    
    // 服务配置
    static constexpr vsomeip::service_t SERVICE_ID = communication::DOOR_SERVICE_ID;
    static constexpr vsomeip::instance_t INSTANCE_ID = communication::DOOR_INSTANCE_ID;
    static constexpr vsomeip::major_version_t MAJOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MAJOR;
    static constexpr vsomeip::minor_version_t MINOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MINOR;
    
    // 方法ID
    static constexpr vsomeip::method_t SET_LOCK_STATE_METHOD = communication::door_service::SET_LOCK_STATE;
    static constexpr vsomeip::method_t GET_LOCK_STATE_METHOD = communication::door_service::GET_LOCK_STATE;
    
    // 事件ID
    static constexpr vsomeip::event_t LOCK_STATE_CHANGED_EVENT = communication::door_events::ON_LOCK_STATE_CHANGED;
    static constexpr vsomeip::event_t DOOR_STATE_CHANGED_EVENT = communication::door_events::ON_DOOR_STATE_CHANGED;
    
    // 事件组ID
    static constexpr vsomeip::eventgroup_t EVENT_GROUP = 0x0001;
};

} // namespace services
} // namespace body_controller
