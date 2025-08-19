#pragma once

#include <memory>
#include <vsomeip/vsomeip.hpp>
#include "application/data_structures.h"
#include "communication/someip_service_definitions.h"
#include "common/hardware_simulator.h"

namespace body_controller {
namespace services {

/**
 * @brief 车窗服务实现
 * 提供车窗位置控制和查询功能
 */
class WindowService {
public:
    /**
     * @brief 构造函数
     * @param app VSOMEIP应用程序实例
     * @param simulator 硬件模拟器实例
     */
    explicit WindowService(std::shared_ptr<vsomeip::application> app, 
                          std::shared_ptr<HardwareSimulator> simulator);
    
    /**
     * @brief 析构函数
     */
    ~WindowService();

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
     * @brief 处理车窗位置设置请求
     */
    void HandleSetWindowPositionRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 处理车窗控制请求
     */
    void HandleControlWindowRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 处理车窗位置查询请求
     */
    void HandleGetWindowPositionRequest(const std::shared_ptr<vsomeip::message>& request);
    
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
     * @brief 硬件事件处理器：车窗位置变化
     */
    void OnWindowPositionChanged(const application::OnWindowPositionChangedData& event_data);
    
    /**
     * @brief 发送车窗位置变化事件
     */
    void SendWindowPositionChangedEvent(const application::OnWindowPositionChangedData& event_data);

    /**
     * @brief 模拟车窗位置设置操作
     * @param window_id 车窗ID
     * @param target_position 目标位置
     * @return 操作结果
     */
    application::Result SimulateSetPositionOperation(application::Position window_id, 
                                                   uint8_t target_position);
    
    /**
     * @brief 模拟车窗控制操作
     * @param window_id 车窗ID
     * @param command 控制命令
     * @return 操作结果
     */
    application::Result SimulateControlOperation(application::Position window_id, 
                                               application::WindowCommand command);
    
    /**
     * @brief 获取当前车窗位置
     * @param window_id 车窗ID
     * @return 当前位置百分比
     */
    uint8_t GetCurrentPosition(application::Position window_id) const;

private:
    // VSOMEIP相关
    std::shared_ptr<vsomeip::application> app_;
    bool running_;
    
    // 硬件模拟器
    std::shared_ptr<HardwareSimulator> hardware_simulator_;
    
    // 当前状态存储
    std::array<uint8_t, 4> current_positions_;  // 0-100百分比
    
    // 服务配置
    static constexpr vsomeip::service_t SERVICE_ID = communication::WINDOW_SERVICE_ID;
    static constexpr vsomeip::instance_t INSTANCE_ID = communication::WINDOW_INSTANCE_ID;
    static constexpr vsomeip::major_version_t MAJOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MAJOR;
    static constexpr vsomeip::minor_version_t MINOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MINOR;
    
    // 方法ID
    static constexpr vsomeip::method_t SET_WINDOW_POSITION_METHOD = communication::window_service::SET_WINDOW_POSITION;
    static constexpr vsomeip::method_t CONTROL_WINDOW_METHOD = communication::window_service::CONTROL_WINDOW;
    static constexpr vsomeip::method_t GET_WINDOW_POSITION_METHOD = communication::window_service::GET_WINDOW_POSITION;
    
    // 事件ID
    static constexpr vsomeip::event_t WINDOW_POSITION_CHANGED_EVENT = communication::window_events::ON_WINDOW_POSITION_CHANGED;
    
    // 事件组ID
    static constexpr vsomeip::eventgroup_t EVENT_GROUP = 0x0001;
};

} // namespace services
} // namespace body_controller
