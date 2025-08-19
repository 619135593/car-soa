#pragma once

#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <functional>
#include <random>
#include "application/data_structures.h"

namespace body_controller {
namespace services {

/**
 * @brief 硬件事件模拟器
 * 模拟真实硬件的状态变化，定期触发事件
 */
class HardwareSimulator {
public:
    // 事件回调函数类型定义
    using DoorLockEventCallback = std::function<void(const application::OnLockStateChangedData&)>;
    using DoorStateEventCallback = std::function<void(const application::OnDoorStateChangedData&)>;
    using WindowPositionEventCallback = std::function<void(const application::OnWindowPositionChangedData&)>;
    using LightStateEventCallback = std::function<void(const application::OnLightStateChangedData&)>;
    using SeatPositionEventCallback = std::function<void(const application::OnSeatPositionChangedData&)>;

    /**
     * @brief 构造函数
     */
    HardwareSimulator();
    
    /**
     * @brief 析构函数
     */
    ~HardwareSimulator();

    /**
     * @brief 启动硬件模拟器
     */
    void Start();
    
    /**
     * @brief 停止硬件模拟器
     */
    void Stop();
    
    /**
     * @brief 检查是否正在运行
     */
    bool IsRunning() const { return running_; }

    // ============================================================================
    // 事件回调设置
    // ============================================================================
    
    /**
     * @brief 设置车门锁定状态变化事件回调
     */
    void SetDoorLockEventCallback(const DoorLockEventCallback& callback) {
        door_lock_callback_ = callback;
    }
    
    /**
     * @brief 设置车门开关状态变化事件回调
     */
    void SetDoorStateEventCallback(const DoorStateEventCallback& callback) {
        door_state_callback_ = callback;
    }
    
    /**
     * @brief 设置车窗位置变化事件回调
     */
    void SetWindowPositionEventCallback(const WindowPositionEventCallback& callback) {
        window_position_callback_ = callback;
    }
    
    /**
     * @brief 设置灯光状态变化事件回调
     */
    void SetLightStateEventCallback(const LightStateEventCallback& callback) {
        light_state_callback_ = callback;
    }
    
    /**
     * @brief 设置座椅位置变化事件回调
     */
    void SetSeatPositionEventCallback(const SeatPositionEventCallback& callback) {
        seat_position_callback_ = callback;
    }

    // ============================================================================
    // 手动触发事件（用于响应客户端请求）
    // ============================================================================
    
    /**
     * @brief 手动触发车门锁定状态变化事件
     */
    void TriggerDoorLockEvent(application::Position door_id, application::LockState new_state);
    
    /**
     * @brief 手动触发车窗位置变化事件
     */
    void TriggerWindowPositionEvent(application::Position window_id, uint8_t new_position);
    
    /**
     * @brief 手动触发灯光状态变化事件
     */
    void TriggerLightStateEvent(application::LightType light_type, uint8_t new_state);

    // ============================================================================
    // 配置方法
    // ============================================================================
    
    /**
     * @brief 设置事件触发间隔（秒）
     */
    void SetEventInterval(int seconds) { event_interval_seconds_ = seconds; }
    
    /**
     * @brief 启用/禁用自动事件生成
     */
    void SetAutoEventEnabled(bool enabled) { auto_events_enabled_ = enabled; }

private:
    /**
     * @brief 硬件模拟线程主函数
     */
    void SimulationThread();
    
    /**
     * @brief 生成随机车门锁定状态变化事件
     */
    void GenerateRandomDoorLockEvent();
    
    /**
     * @brief 生成随机车门开关状态变化事件
     */
    void GenerateRandomDoorStateEvent();
    
    /**
     * @brief 生成随机车窗位置变化事件
     */
    void GenerateRandomWindowPositionEvent();
    
    /**
     * @brief 生成随机灯光状态变化事件
     */
    void GenerateRandomLightStateEvent();
    
    /**
     * @brief 生成随机座椅位置变化事件
     */
    void GenerateRandomSeatPositionEvent();

private:
    // 线程控制
    std::atomic<bool> running_;
    std::unique_ptr<std::thread> simulation_thread_;
    
    // 配置参数
    int event_interval_seconds_;
    bool auto_events_enabled_;
    
    // 随机数生成器
    std::mt19937 random_generator_;
    std::uniform_int_distribution<int> door_distribution_;
    std::uniform_int_distribution<int> window_distribution_;
    std::uniform_int_distribution<int> position_distribution_;
    std::uniform_int_distribution<int> light_type_distribution_;
    std::uniform_int_distribution<int> light_state_distribution_;
    std::uniform_int_distribution<int> seat_axis_distribution_;
    
    // 事件回调函数
    DoorLockEventCallback door_lock_callback_;
    DoorStateEventCallback door_state_callback_;
    WindowPositionEventCallback window_position_callback_;
    LightStateEventCallback light_state_callback_;
    SeatPositionEventCallback seat_position_callback_;
    
    // 当前状态（用于生成合理的状态变化）
    std::array<application::LockState, 4> current_door_lock_states_;
    std::array<application::DoorState, 4> current_door_states_;
    std::array<uint8_t, 4> current_window_positions_;
    application::HeadlightState current_headlight_state_;
    application::IndicatorState current_indicator_state_;
    application::PositionLightState current_position_light_state_;
};

} // namespace services
} // namespace body_controller
