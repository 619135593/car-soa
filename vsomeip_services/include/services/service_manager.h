#pragma once

#include <memory>
#include <vector>
#include <atomic>
#include <vsomeip/vsomeip.hpp>
#include "services/door_service.h"
#include "services/window_service.h"
#include "services/light_service.h"
#include "services/seat_service.h"
#include "common/hardware_simulator.h"

namespace body_controller {
namespace services {

/**
 * @brief 服务管理器
 * 统一管理所有VSOMEIP服务的生命周期
 */
class ServiceManager {
public:
    /**
     * @brief 构造函数
     */
    ServiceManager();
    
    /**
     * @brief 析构函数
     */
    ~ServiceManager();

    /**
     * @brief 初始化服务管理器
     * @return 成功返回true，失败返回false
     */
    bool Initialize();
    
    /**
     * @brief 启动所有服务
     * @return 成功返回true，失败返回false
     */
    bool Start();
    
    /**
     * @brief 停止所有服务
     */
    void Stop();
    
    /**
     * @brief 检查是否正在运行
     */
    bool IsRunning() const { return running_; }
    
    /**
     * @brief 运行服务（阻塞调用）
     * 启动所有服务并等待停止信号
     */
    void Run();

    /**
     * @brief 获取硬件模拟器实例
     */
    std::shared_ptr<HardwareSimulator> GetHardwareSimulator() const {
        return hardware_simulator_;
    }

private:
    /**
     * @brief VSOMEIP应用程序状态回调
     */
    void OnStateChanged(vsomeip::state_type_e state);
    
    /**
     * @brief 初始化VSOMEIP应用程序
     */
    bool InitializeVSomeIPApplication();
    
    /**
     * @brief 初始化所有服务
     */
    bool InitializeServices();
    
    /**
     * @brief 启动硬件模拟器
     */
    void StartHardwareSimulator();
    
    /**
     * @brief 停止硬件模拟器
     */
    void StopHardwareSimulator();

private:
    // VSOMEIP应用程序
    std::shared_ptr<vsomeip::application> app_;
    
    // 运行状态
    std::atomic<bool> running_;
    std::atomic<bool> vsomeip_ready_;
    
    // 硬件模拟器
    std::shared_ptr<HardwareSimulator> hardware_simulator_;
    
    // 服务实例
    std::unique_ptr<DoorService> door_service_;
    std::unique_ptr<WindowService> window_service_;
    std::unique_ptr<LightService> light_service_;
    std::unique_ptr<SeatService> seat_service_;
    
    // 应用程序名称
    static constexpr const char* APPLICATION_NAME = "body_controller_services";
};

} // namespace services
} // namespace body_controller
