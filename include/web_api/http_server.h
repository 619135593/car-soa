#pragma once

#include <httplib.h>
#include <nlohmann/json.hpp>
#include <memory>
#include <thread>
#include <atomic>
#include <chrono>
#include <unordered_map>
#include <mutex>
#include <string>
#include "application/data_structures.h"

namespace body_controller {
namespace web_api {

// 前向声明
class ApiHandlers;

/**
 * @brief HTTP服务器类
 * 
 * 提供RESTful API接口，处理Web前端的HTTP请求
 * 支持CORS、错误处理、静态文件服务等Web标准功能
 */
class HttpServer {
public:
    /**
     * @brief 构造函数
     * @param port HTTP服务器端口
     */
    explicit HttpServer(int port = 8080);
    
    /**
     * @brief 析构函数
     */
    ~HttpServer();
    
    /**
     * @brief 初始化HTTP服务器
     * @return 成功返回true，失败返回false
     */
    bool Initialize();
    
    /**
     * @brief 设置API处理器
     * @param handlers API处理器实例
     */
    void SetApiHandlers(std::shared_ptr<ApiHandlers> handlers);
    
    /**
     * @brief 启动HTTP服务器
     * @return 成功返回true，失败返回false
     */
    bool Start();
    
    /**
     * @brief 停止HTTP服务器
     */
    void Stop();
    
    /**
     * @brief 检查服务器是否正在运行
     * @return 运行中返回true，否则返回false
     */
    bool IsRunning() const;
    
    /**
     * @brief 获取服务器端口
     * @return 端口号
     */
    int GetPort() const;
    
    /**
     * @brief 获取服务器运行时间（秒）
     * @return 运行时间
     */
    double GetUptime() const;

    // ============================================================================
    // SSE事件推送方法
    // ============================================================================

    /**
     * @brief 推送车门状态变化事件
     * @param door_id 车门ID
     * @param lock_state 锁定状态
     */
    void PushDoorLockEvent(int door_id, bool lock_state);

    /**
     * @brief 推送车窗位置变化事件
     * @param window_id 车窗ID
     * @param position 位置百分比
     */
    void PushWindowPositionEvent(int window_id, int position);

    /**
     * @brief 推送车灯状态变化事件
     * @param light_type 灯光类型
     * @param state 开关状态
     */
    void PushLightStateEvent(const std::string& light_type, bool state);

    /**
     * @brief 推送座椅位置变化事件
     * @param seat_id 座椅ID
     * @param position 位置信息
     */
    void PushSeatPositionEvent(int seat_id, const std::string& position);

    /**
     * @brief 发布通用SSE事件
     * @param event_type 事件类型（将作为 event: 字段）
     * @param data 事件数据（将作为 data: 的JSON字符串）
     */
    void PublishEvent(const std::string& event_type, const nlohmann::json& data);

private:
    /**
     * @brief 设置API路由
     */
    void SetupRoutes();
    
    // ============================================================================
    // 车门服务请求处理
    // ============================================================================
    
    /**
     * @brief 处理车门锁定请求
     */
    void HandleDoorLockRequest(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理车门状态查询请求
     */
    void HandleDoorStatusRequest(const httplib::Request& req, httplib::Response& res);
    
    // ============================================================================
    // 车窗服务请求处理
    // ============================================================================
    
    /**
     * @brief 处理车窗位置设置请求
     */
    void HandleWindowPositionRequest(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理车窗控制请求
     */
    void HandleWindowControlRequest(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理车窗位置查询请求
     */
    void HandleWindowPositionStatusRequest(const httplib::Request& req, httplib::Response& res);
    
    // ============================================================================
    // 灯光服务请求处理
    // ============================================================================
    
    /**
     * @brief 处理前大灯控制请求
     */
    void HandleHeadlightRequest(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理转向灯控制请求
     */
    void HandleIndicatorRequest(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理位置灯控制请求
     */
    void HandlePositionLightRequest(const httplib::Request& req, httplib::Response& res);
    
    // ============================================================================
    // 座椅服务请求处理
    // ============================================================================
    
    /**
     * @brief 处理座椅调节请求
     */
    void HandleSeatAdjustRequest(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理座椅记忆位置恢复请求
     */
    void HandleSeatMemoryRecallRequest(const httplib::Request& req, httplib::Response& res);
    
    /**
     * @brief 处理座椅记忆位置保存请求
     */
    void HandleSeatMemorySaveRequest(const httplib::Request& req, httplib::Response& res);
    
    // ============================================================================
    // 工具方法
    // ============================================================================
    
    /**
     * @brief 发送错误响应
     * @param res HTTP响应对象
     * @param error 错误类型
     * @param message 错误消息
     * @param status_code HTTP状态码
     */
    void SendErrorResponse(httplib::Response& res, const std::string& error,
                          const std::string& message, int status_code = 400);

    // ============================================================================
    // SSE连接管理
    // ============================================================================

    /**
     * @brief 注册SSE连接
     * @param connection_id 连接ID
     * @param sink 数据流对象
     */
    void RegisterSSEConnection(uint64_t connection_id, httplib::DataSink* sink);

    /**
     * @brief 注销SSE连接
     * @param connection_id 连接ID
     */
    void UnregisterSSEConnection(uint64_t connection_id);

    /**
     * @brief 向所有SSE连接推送事件
     * @param event_data 事件数据
     */
    void BroadcastSSEEvent(const std::string& event_data);

private:
    httplib::Server server_;                    ///< HTTP服务器实例
    int port_;                                  ///< 服务器端口
    std::atomic<bool> running_;                 ///< 运行状态标志
    std::thread server_thread_;                 ///< 服务器线程
    std::shared_ptr<ApiHandlers> api_handlers_; ///< API处理器
    std::chrono::steady_clock::time_point start_time_; ///< 启动时间

    // SSE连接管理
    std::unordered_map<uint64_t, httplib::DataSink*> sse_connections_; ///< SSE连接映射
    std::mutex sse_connections_mutex_;          ///< SSE连接互斥锁
};

} // namespace web_api
} // namespace body_controller
