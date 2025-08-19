#pragma once

#include <memory>
#include <vsomeip/vsomeip.hpp>
#include "application/data_structures.h"
#include "communication/someip_service_definitions.h"
#include "common/hardware_simulator.h"

namespace body_controller {
namespace services {

/**
 * @brief 座椅服务实现
 * 提供座椅调节和记忆位置功能
 */
class SeatService {
public:
    /**
     * @brief 构造函数
     * @param app VSOMEIP应用程序实例
     * @param simulator 硬件模拟器实例
     */
    explicit SeatService(std::shared_ptr<vsomeip::application> app, 
                        std::shared_ptr<HardwareSimulator> simulator);
    
    /**
     * @brief 析构函数
     */
    ~SeatService();

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
     * @brief 处理座椅调节请求
     */
    void HandleAdjustSeatRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 处理记忆位置恢复请求
     */
    void HandleRecallMemoryPositionRequest(const std::shared_ptr<vsomeip::message>& request);
    
    /**
     * @brief 处理记忆位置保存请求
     */
    void HandleSaveMemoryPositionRequest(const std::shared_ptr<vsomeip::message>& request);
    
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
     * @brief 硬件事件处理器：座椅位置变化
     */
    void OnSeatPositionChanged(const application::OnSeatPositionChangedData& event_data);
    
    /**
     * @brief 发送座椅位置变化事件
     */
    void SendSeatPositionChangedEvent(const application::OnSeatPositionChangedData& event_data);
    
    /**
     * @brief 发送记忆保存确认事件
     */
    void SendMemorySaveConfirmEvent(const application::OnMemorySaveConfirmData& event_data);

    /**
     * @brief 模拟座椅调节操作
     * @param seat_id 座椅ID
     * @param axis 调节轴
     * @param direction 调节方向
     * @return 操作结果
     */
    application::Result SimulateAdjustOperation(application::Position seat_id,
                                              application::SeatAxis axis,
                                              application::SeatDirection direction);
    
    /**
     * @brief 模拟记忆位置恢复操作
     * @param seat_id 座椅ID
     * @param memory_id 记忆位置ID
     * @return 操作结果
     */
    application::Result SimulateRecallMemoryOperation(application::Position seat_id,
                                                    uint8_t memory_id);
    
    /**
     * @brief 模拟记忆位置保存操作
     * @param seat_id 座椅ID
     * @param memory_id 记忆位置ID
     * @return 操作结果
     */
    application::Result SimulateSaveMemoryOperation(application::Position seat_id,
                                                  uint8_t memory_id);

private:
    // VSOMEIP相关
    std::shared_ptr<vsomeip::application> app_;
    bool running_;
    
    // 硬件模拟器
    std::shared_ptr<HardwareSimulator> hardware_simulator_;
    
    // 当前状态存储（简化的座椅位置表示）
    struct SeatPosition {
        int forward_backward_position;  // 前后位置 (-100 到 100)
        int recline_position;          // 靠背角度 (0 到 90)
    };
    std::array<SeatPosition, 4> current_positions_;
    
    // 记忆位置存储
    std::array<std::array<SeatPosition, 3>, 4> memory_positions_;  // 每个座椅3个记忆位置
    
    // 服务配置
    static constexpr vsomeip::service_t SERVICE_ID = communication::SEAT_SERVICE_ID;
    static constexpr vsomeip::instance_t INSTANCE_ID = communication::SEAT_INSTANCE_ID;
    static constexpr vsomeip::major_version_t MAJOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MAJOR;
    static constexpr vsomeip::minor_version_t MINOR_VERSION = communication::SERVICE_INTERFACE_VERSION_MINOR;
    
    // 方法ID
    static constexpr vsomeip::method_t ADJUST_SEAT_METHOD = communication::seat_service::ADJUST_SEAT;
    static constexpr vsomeip::method_t RECALL_MEMORY_POSITION_METHOD = communication::seat_service::RECALL_MEMORY_POSITION;
    static constexpr vsomeip::method_t SAVE_MEMORY_POSITION_METHOD = communication::seat_service::SAVE_MEMORY_POSITION;
    
    // 事件ID
    static constexpr vsomeip::event_t SEAT_POSITION_CHANGED_EVENT = communication::seat_events::ON_SEAT_POSITION_CHANGED;
    static constexpr vsomeip::event_t MEMORY_SAVE_CONFIRM_EVENT = communication::seat_events::ON_MEMORY_SAVE_CONFIRM;
    
    // 事件组ID
    static constexpr vsomeip::eventgroup_t EVENT_GROUP = 0x0001;
};

} // namespace services
} // namespace body_controller
