#pragma once

#include <memory>
#include <functional>
#include <atomic>
#include <nlohmann/json.hpp>
#include "communication/someip_client.h"
#include "application/data_structures.h"
#include "web_api/json_converter.h"

namespace body_controller {
namespace web_api {

// 前向声明
class WebSocketServer;
class HttpServer;

/**
 * @brief API处理器类
 * 
 * 连接HTTP服务器和SOME/IP客户端的桥梁
 * 负责处理Web API请求并调用相应的SOME/IP服务
 */
class ApiHandlers {
public:
    /**
     * @brief 构造函数
     */
    ApiHandlers();
    
    /**
     * @brief 析构函数
     */
    ~ApiHandlers();
    
    /**
     * @brief 初始化API处理器
     * @return 成功返回true，失败返回false
     */
    bool Initialize();
    
    /**
     * @brief 启动API处理器
     * @return 成功返回true，失败返回false
     */
    bool Start();
    
    /**
     * @brief 停止API处理器
     */
    void Stop();

    /**
     * @brief 设置HTTP服务器引用（用于SSE事件推送）
     * @param http_server HTTP服务器实例
     */
    void SetHttpServer(std::shared_ptr<HttpServer> http_server);

    // WebSocket服务器已移除，使用SSE替代事件广播
    
    // ============================================================================
    // 车门服务处理
    // ============================================================================
    
    /**
     * @brief 处理车门锁定请求
     * @param request 锁定请求
     * @param callback 响应回调函数
     */
    void HandleDoorLockRequest(const application::SetLockStateReq& request,
                              std::function<void(const application::SetLockStateResp&)> callback);
    
    /**
     * @brief 处理车门状态查询请求
     * @param request 状态查询请求
     * @param callback 响应回调函数
     */
    void HandleDoorStatusRequest(const application::GetLockStateReq& request,
                                std::function<void(const application::GetLockStateResp&)> callback);
    
    // ============================================================================
    // 车窗服务处理
    // ============================================================================
    
    /**
     * @brief 处理车窗位置设置请求
     * @param request 位置设置请求
     * @param callback 响应回调函数
     */
    void HandleWindowPositionRequest(const application::SetWindowPositionReq& request,
                                    std::function<void(const application::SetWindowPositionResp&)> callback);
    
    /**
     * @brief 处理车窗控制请求
     * @param request 控制请求
     * @param callback 响应回调函数
     */
    void HandleWindowControlRequest(const application::ControlWindowReq& request,
                                   std::function<void(const application::ControlWindowResp&)> callback);
    
    /**
     * @brief 处理车窗位置查询请求
     * @param request 位置查询请求
     * @param callback 响应回调函数
     */
    void HandleWindowPositionStatusRequest(const application::GetWindowPositionReq& request,
                                          std::function<void(const application::GetWindowPositionResp&)> callback);
    
    // ============================================================================
    // 灯光服务处理
    // ============================================================================
    
    /**
     * @brief 处理前大灯控制请求
     * @param request 前大灯控制请求
     * @param callback 响应回调函数
     */
    void HandleHeadlightRequest(const application::SetHeadlightStateReq& request,
                               std::function<void(const application::SetHeadlightStateResp&)> callback);
    
    /**
     * @brief 处理转向灯控制请求
     * @param request 转向灯控制请求
     * @param callback 响应回调函数
     */
    void HandleIndicatorRequest(const application::SetIndicatorStateReq& request,
                               std::function<void(const application::SetIndicatorStateResp&)> callback);
    
    /**
     * @brief 处理位置灯控制请求
     * @param request 位置灯控制请求
     * @param callback 响应回调函数
     */
    void HandlePositionLightRequest(const application::SetPositionLightStateReq& request,
                                   std::function<void(const application::SetPositionLightStateResp&)> callback);
    
    // ============================================================================
    // 座椅服务处理
    // ============================================================================
    
    /**
     * @brief 处理座椅调节请求
     * @param request 座椅调节请求
     * @param callback 响应回调函数
     */
    void HandleSeatAdjustRequest(const application::AdjustSeatReq& request,
                                std::function<void(const application::AdjustSeatResp&)> callback);
    
    /**
     * @brief 处理座椅记忆位置恢复请求
     * @param request 记忆位置恢复请求
     * @param callback 响应回调函数
     */
    void HandleSeatMemoryRecallRequest(const application::RecallMemoryPositionReq& request,
                                      std::function<void(const application::RecallMemoryPositionResp&)> callback);
    
    /**
     * @brief 处理座椅记忆位置保存请求
     * @param request 记忆位置保存请求
     * @param callback 响应回调函数
     */
    void HandleSeatMemorySaveRequest(const application::SaveMemoryPositionReq& request,
                                    std::function<void(const application::SaveMemoryPositionResp&)> callback);
    
    // ============================================================================
    // 服务状态检查
    // ============================================================================
    
    /**
     * @brief 检查车门服务是否可用
     * @return 可用返回true，否则返回false
     */
    bool IsDoorServiceAvailable() const;
    
    /**
     * @brief 检查车窗服务是否可用
     * @return 可用返回true，否则返回false
     */
    bool IsWindowServiceAvailable() const;
    
    /**
     * @brief 检查灯光服务是否可用
     * @return 可用返回true，否则返回false
     */
    bool IsLightServiceAvailable() const;
    
    /**
     * @brief 检查座椅服务是否可用
     * @return 可用返回true，否则返回false
     */
    bool IsSeatServiceAvailable() const;

private:
    /**
     * @brief 设置响应处理器
     */
    void SetupResponseHandlers();
    
    /**
     * @brief 设置事件处理器
     */
    void SetupEventHandlers();
    
    /**
     * @brief 广播事件到WebSocket客户端
     * @param event_type 事件类型
     * @param data 事件数据
     */
    void BroadcastEvent(const std::string& event_type, const nlohmann::json& data);

private:
    // SOME/IP服务客户端
    std::shared_ptr<communication::DoorServiceClient> door_client_;
    std::shared_ptr<communication::WindowServiceClient> window_client_;
    std::shared_ptr<communication::LightServiceClient> light_client_;
    std::shared_ptr<communication::SeatServiceClient> seat_client_;
    
    // WebSocket服务器已移除，使用SSE替代事件广播

    // HTTP服务器引用（用于SSE事件推送）
    std::weak_ptr<HttpServer> http_server_;

    // 运行状态
    std::atomic<bool> running_{false};
};

} // namespace web_api
} // namespace body_controller
