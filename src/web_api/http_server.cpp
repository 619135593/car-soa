#include "web_api/http_server.h"
#include "web_api/json_converter.h"
#include "web_api/api_handlers.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <thread>
#include <chrono>
#include <future>
#include <mutex>

namespace body_controller {
namespace web_api {

HttpServer::HttpServer(int port) 
    : port_(port), running_(false) {
    std::cout << "[HttpServer] Created HTTP server on port " << port << std::endl;
}

HttpServer::~HttpServer() {
    Stop();
}

bool HttpServer::Initialize() {
    try {
        // 设置CORS支持
        server_.set_pre_routing_handler([](const httplib::Request& req, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            return httplib::Server::HandlerResponse::Unhandled;
        });
        
        // 处理OPTIONS请求（CORS预检）
        server_.Options(".*", [](const httplib::Request&, httplib::Response& res) {
            res.set_header("Access-Control-Allow-Origin", "*");
            res.set_header("Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS");
            res.set_header("Access-Control-Allow-Headers", "Content-Type, Authorization");
            res.status = 200;
        });
        
        // 设置错误处理器
        server_.set_error_handler([](const httplib::Request&, httplib::Response& res) {
            auto error_json = JsonConverter::CreateErrorResponse(
                "INTERNAL_ERROR", 
                "Internal server error occurred"
            );
            res.set_content(error_json.dump(), "application/json");
            res.status = 500;
        });
        
        // 设置异常处理器
        server_.set_exception_handler([](const httplib::Request&, httplib::Response& res, std::exception_ptr ep) {
            try {
                std::rethrow_exception(ep);
            } catch (const std::exception& e) {
                auto error_json = JsonConverter::CreateErrorResponse(
                    "EXCEPTION", 
                    std::string("Exception: ") + e.what()
                );
                res.set_content(error_json.dump(), "application/json");
                res.status = 500;
            }
        });
        
        SetupRoutes();
        
        std::cout << "[HttpServer] HTTP server initialized successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[HttpServer] Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

void HttpServer::SetupRoutes() {
    // ============================================================================
    // 静态文件服务
    // ============================================================================
    
    // 设置静态文件目录
    server_.set_mount_point("/", "./web");
    
    // 默认页面
    server_.Get("/", [](const httplib::Request&, httplib::Response& res) {
        std::ifstream file("./web/index.html");
        if (file.is_open()) {
            std::stringstream buffer;
            buffer << file.rdbuf();
            res.set_content(buffer.str(), "text/html");
        } else {
            res.status = 404;
            res.set_content("Web interface not found", "text/plain");
        }
    });
    
    // ============================================================================
    // API信息和健康检查
    // ============================================================================
    
    // 处理favicon.ico请求，避免404错误
    server_.Get("/favicon.ico", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204; // No Content
    });

    // 处理images目录下的图标请求
    server_.Get("/images/favicon.ico", [](const httplib::Request&, httplib::Response& res) {
        res.status = 204; // No Content
    });

    // API信息
    server_.Get("/api/info", [](const httplib::Request&, httplib::Response& res) {
        nlohmann::json info = {
            {"name", "Body Controller Web API"},
            {"version", "1.0.0"},
            {"description", "REST API for vehicle body control system"},
            {"services", {
                {"door", "Door lock/unlock control"},
                {"window", "Window position and movement control"},
                {"light", "Headlight, indicator, and position light control"},
                {"seat", "Seat adjustment and memory position control"}
            }},
            {"endpoints", {
                {"door", "/api/door/*"},
                {"window", "/api/window/*"},
                {"light", "/api/light/*"},
                {"seat", "/api/seat/*"},
                {"events", "/api/events"}
            }}
        };
        
        auto response = JsonConverter::CreateSuccessResponse(info);
        res.set_content(response.dump(2), "application/json");
    });
    
    // 健康检查
    server_.Get("/api/health", [this](const httplib::Request&, httplib::Response& res) {
        nlohmann::json health = {
            {"status", "healthy"},
            {"uptime", GetUptime()},
            {"services", {
                {"door_service", api_handlers_ ? api_handlers_->IsDoorServiceAvailable() : false},
                {"window_service", api_handlers_ ? api_handlers_->IsWindowServiceAvailable() : false},
                {"light_service", api_handlers_ ? api_handlers_->IsLightServiceAvailable() : false},
                {"seat_service", api_handlers_ ? api_handlers_->IsSeatServiceAvailable() : false}
            }}
        };

        auto response = JsonConverter::CreateSuccessResponse(health);
        res.set_content(response.dump(2), "application/json");
    });

    // Server-Sent Events (SSE) 端点用于实时推送
    server_.Get("/api/events", [this](const httplib::Request&, httplib::Response& res) {
        std::cout << "[HttpServer] SSE client connected" << std::endl;

        res.set_header("Content-Type", "text/event-stream");
        res.set_header("Cache-Control", "no-cache");
        res.set_header("Connection", "keep-alive");
        res.set_header("Access-Control-Allow-Origin", "*");

        // 为这个连接创建一个唯一ID
        auto connection_id = std::chrono::steady_clock::now().time_since_epoch().count();

        res.set_chunked_content_provider("text/event-stream",
            [this, connection_id](size_t /*offset*/, httplib::DataSink& sink) {
                // 注册SSE连接
                RegisterSSEConnection(connection_id, &sink);

                // 发送初始连接消息
                std::string welcome_msg = "data: {\"type\":\"welcome\",\"message\":\"Connected to Body Controller Events\",\"timestamp\":" +
                                        std::to_string(std::time(nullptr)) + "}\n\n";
                if (!sink.write(welcome_msg.c_str(), welcome_msg.length())) {
                    std::cout << "[HttpServer] SSE client disconnected during welcome" << std::endl;
                    UnregisterSSEConnection(connection_id);
                    return false;
                }

                // 心跳机制：每30秒发送一次心跳
                auto last_heartbeat = std::chrono::steady_clock::now();
                const auto heartbeat_interval = std::chrono::seconds(30);

                // 保持连接活跃，等待事件推送
                while (true) {
                    std::this_thread::sleep_for(std::chrono::seconds(1));

                    auto now = std::chrono::steady_clock::now();

                    // 发送心跳
                    if (now - last_heartbeat >= heartbeat_interval) {
                        nlohmann::json heartbeat = {
                            {"type", "heartbeat"},
                            {"timestamp", std::time(nullptr)},
                            {"uptime", GetUptime()}
                        };

                        std::string heartbeat_data = "data: " + heartbeat.dump() + "\n\n";
                        if (!sink.write(heartbeat_data.c_str(), heartbeat_data.length())) {
                            std::cout << "[HttpServer] SSE client disconnected during heartbeat" << std::endl;
                            break;
                        }

                        last_heartbeat = now;
                        std::cout << "[HttpServer] SSE heartbeat sent to connection " << connection_id << std::endl;
                    }

                    // 检查连接是否超时（10分钟）
                    if (std::chrono::duration_cast<std::chrono::minutes>(now - last_heartbeat).count() > 10) {
                        std::cout << "[HttpServer] SSE connection timeout, closing connection " << connection_id << std::endl;
                        break;
                    }
                }

                // 清理连接
                UnregisterSSEConnection(connection_id);
                std::cout << "[HttpServer] SSE client disconnected, connection " << connection_id << std::endl;
                return false; // 结束流
            });
    });
    
    // ============================================================================
    // 车门服务API路由
    // ============================================================================
    
    // 设置车门锁定状态
    server_.Post("/api/door/lock", [this](const httplib::Request& req, httplib::Response& res) {
        HandleDoorLockRequest(req, res);
    });
    
    // 获取车门锁定状态
    server_.Get("/api/door/([0-9]+)/status", [this](const httplib::Request& req, httplib::Response& res) {
        HandleDoorStatusRequest(req, res);
    });
    
    // ============================================================================
    // 车窗服务API路由
    // ============================================================================
    
    // 设置车窗位置
    server_.Post("/api/window/position", [this](const httplib::Request& req, httplib::Response& res) {
        HandleWindowPositionRequest(req, res);
    });
    
    // 控制车窗升降
    server_.Post("/api/window/control", [this](const httplib::Request& req, httplib::Response& res) {
        HandleWindowControlRequest(req, res);
    });
    
    // 获取车窗位置
    server_.Get("/api/window/([0-9]+)/position", [this](const httplib::Request& req, httplib::Response& res) {
        HandleWindowPositionStatusRequest(req, res);
    });
    
    // ============================================================================
    // 灯光服务API路由
    // ============================================================================
    
    // 设置前大灯状态
    server_.Post("/api/light/headlight", [this](const httplib::Request& req, httplib::Response& res) {
        HandleHeadlightRequest(req, res);
    });
    
    // 设置转向灯状态
    server_.Post("/api/light/indicator", [this](const httplib::Request& req, httplib::Response& res) {
        HandleIndicatorRequest(req, res);
    });
    
    // 设置位置灯状态
    server_.Post("/api/light/position", [this](const httplib::Request& req, httplib::Response& res) {
        HandlePositionLightRequest(req, res);
    });
    
    // ============================================================================
    // 座椅服务API路由
    // ============================================================================
    
    // 调节座椅
    server_.Post("/api/seat/adjust", [this](const httplib::Request& req, httplib::Response& res) {
        HandleSeatAdjustRequest(req, res);
    });
    
    // 恢复记忆位置
    server_.Post("/api/seat/memory/recall", [this](const httplib::Request& req, httplib::Response& res) {
        HandleSeatMemoryRecallRequest(req, res);
    });
    
    // 保存记忆位置
    server_.Post("/api/seat/memory/save", [this](const httplib::Request& req, httplib::Response& res) {
        HandleSeatMemorySaveRequest(req, res);
    });
    
    std::cout << "[HttpServer] API routes configured" << std::endl;
}

void HttpServer::SetApiHandlers(std::shared_ptr<ApiHandlers> handlers) {
    api_handlers_ = handlers;
    std::cout << "[HttpServer] API handlers set" << std::endl;
}

bool HttpServer::Start() {
    if (running_) {
        std::cout << "[HttpServer] Server is already running" << std::endl;
        return true;
    }
    
    start_time_ = std::chrono::steady_clock::now();
    
    // 在单独的线程中启动服务器
    server_thread_ = std::thread([this]() {
        std::cout << "[HttpServer] Starting HTTP server on port " << port_ << std::endl;
        running_ = true;
        
        if (!server_.listen("0.0.0.0", port_)) {
            std::cerr << "[HttpServer] Failed to start server on port " << port_ << std::endl;
            running_ = false;
        }
    });
    
    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    if (running_) {
        std::cout << "[HttpServer] HTTP server started successfully on port " << port_ << std::endl;
        std::cout << "[HttpServer] API documentation available at: http://localhost:" << port_ << "/api/info" << std::endl;
        return true;
    } else {
        if (server_thread_.joinable()) {
            server_thread_.join();
        }
        return false;
    }
}

void HttpServer::Stop() {
    if (!running_) {
        return;
    }
    
    std::cout << "[HttpServer] Stopping HTTP server..." << std::endl;
    running_ = false;
    server_.stop();
    
    if (server_thread_.joinable()) {
        server_thread_.join();
    }
    
    std::cout << "[HttpServer] HTTP server stopped" << std::endl;
}

bool HttpServer::IsRunning() const {
    return running_;
}

int HttpServer::GetPort() const {
    return port_;
}

double HttpServer::GetUptime() const {
    auto now = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::seconds>(now - start_time_);
    return duration.count();
}

// ============================================================================
// 私有方法：请求处理
// ============================================================================

void HttpServer::HandleDoorLockRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }
        
        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::SetLockStateReq>(json_data);
        
        api_handlers_->HandleDoorLockRequest(request, [&res](const application::SetLockStateResp& response) {
            auto json_response = JsonConverter::ToJson(response);
            auto success_response = JsonConverter::CreateSuccessResponse(json_response);
            res.set_content(success_response.dump(), "application/json");
        });
        
    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleDoorStatusRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }
        
        int doorId = std::stoi(req.matches[1]);
        application::GetLockStateReq request(static_cast<application::Position>(doorId));
        
        api_handlers_->HandleDoorStatusRequest(request, [&res](const application::GetLockStateResp& response) {
            auto json_response = JsonConverter::ToJson(response);
            auto success_response = JsonConverter::CreateSuccessResponse(json_response);
            res.set_content(success_response.dump(), "application/json");
        });
        
    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleWindowPositionRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::SetWindowPositionReq>(json_data);

        api_handlers_->HandleWindowPositionRequest(request, [&res](const application::SetWindowPositionResp& response) {
            auto json_response = JsonConverter::ToJson(response);
            auto success_response = JsonConverter::CreateSuccessResponse(json_response);
            res.set_content(success_response.dump(), "application/json");
        });

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleWindowControlRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::ControlWindowReq>(json_data);

        api_handlers_->HandleWindowControlRequest(request, [&res](const application::ControlWindowResp& response) {
            auto json_response = JsonConverter::ToJson(response);
            auto success_response = JsonConverter::CreateSuccessResponse(json_response);
            res.set_content(success_response.dump(), "application/json");
        });

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleWindowPositionStatusRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        int windowId = std::stoi(req.matches[1]);
        application::GetWindowPositionReq request(static_cast<application::Position>(windowId));

        api_handlers_->HandleWindowPositionStatusRequest(request, [&res](const application::GetWindowPositionResp& response) {
            auto json_response = JsonConverter::ToJson(response);
            auto success_response = JsonConverter::CreateSuccessResponse(json_response);
            res.set_content(success_response.dump(), "application/json");
        });

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleHeadlightRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::SetHeadlightStateReq>(json_data);

        // 使用同步等待来确保回调在函数返回前执行
        std::promise<application::SetHeadlightStateResp> promise;
        auto future = promise.get_future();

        api_handlers_->HandleHeadlightRequest(request, [&promise](const application::SetHeadlightStateResp& response) {
            promise.set_value(response);
        });

        // 等待回调完成（最多等待5秒）
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            SendErrorResponse(res, "REQUEST_TIMEOUT", "Request timed out", 408);
            return;
        }

        auto response = future.get();
        auto json_response = JsonConverter::ToJson(response);
        auto success_response = JsonConverter::CreateSuccessResponse(json_response);
        res.set_content(success_response.dump(), "application/json");

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleIndicatorRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::SetIndicatorStateReq>(json_data);

        // 使用同步等待来确保回调在函数返回前执行
        std::promise<application::SetIndicatorStateResp> promise;
        auto future = promise.get_future();

        api_handlers_->HandleIndicatorRequest(request, [&promise](const application::SetIndicatorStateResp& response) {
            promise.set_value(response);
        });

        // 等待回调完成（最多等待5秒）
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            SendErrorResponse(res, "REQUEST_TIMEOUT", "Request timed out", 408);
            return;
        }

        auto response = future.get();
        auto json_response = JsonConverter::ToJson(response);
        auto success_response = JsonConverter::CreateSuccessResponse(json_response);
        res.set_content(success_response.dump(), "application/json");

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandlePositionLightRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::SetPositionLightStateReq>(json_data);

        // 使用同步等待来确保回调在函数返回前执行
        std::promise<application::SetPositionLightStateResp> promise;
        auto future = promise.get_future();

        api_handlers_->HandlePositionLightRequest(request, [&promise](const application::SetPositionLightStateResp& response) {
            promise.set_value(response);
        });

        // 等待回调完成（最多等待5秒）
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            SendErrorResponse(res, "REQUEST_TIMEOUT", "Request timed out", 408);
            return;
        }

        auto response = future.get();
        auto json_response = JsonConverter::ToJson(response);
        auto success_response = JsonConverter::CreateSuccessResponse(json_response);
        res.set_content(success_response.dump(), "application/json");

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleSeatAdjustRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::AdjustSeatReq>(json_data);

        // 使用同步等待来确保回调在函数返回前执行
        std::promise<application::AdjustSeatResp> promise;
        auto future = promise.get_future();

        api_handlers_->HandleSeatAdjustRequest(request, [&promise](const application::AdjustSeatResp& response) {
            promise.set_value(response);
        });

        // 等待回调完成（最多等待5秒）
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            SendErrorResponse(res, "REQUEST_TIMEOUT", "Request timed out", 408);
            return;
        }

        auto response = future.get();
        auto json_response = JsonConverter::ToJson(response);
        auto success_response = JsonConverter::CreateSuccessResponse(json_response);
        res.set_content(success_response.dump(), "application/json");

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleSeatMemoryRecallRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::RecallMemoryPositionReq>(json_data);

        // 使用同步等待来确保回调在函数返回前执行
        std::promise<application::RecallMemoryPositionResp> promise;
        auto future = promise.get_future();

        api_handlers_->HandleSeatMemoryRecallRequest(request, [&promise](const application::RecallMemoryPositionResp& response) {
            promise.set_value(response);
        });

        // 等待回调完成（最多等待5秒）
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            SendErrorResponse(res, "REQUEST_TIMEOUT", "Request timed out", 408);
            return;
        }

        auto response = future.get();
        auto json_response = JsonConverter::ToJson(response);
        auto success_response = JsonConverter::CreateSuccessResponse(json_response);
        res.set_content(success_response.dump(), "application/json");

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::HandleSeatMemorySaveRequest(const httplib::Request& req, httplib::Response& res) {
    try {
        if (!api_handlers_) {
            SendErrorResponse(res, "SERVICE_UNAVAILABLE", "API handlers not available", 503);
            return;
        }

        auto json_data = nlohmann::json::parse(req.body);
        auto request = JsonConverter::FromJson<application::SaveMemoryPositionReq>(json_data);

        // 使用同步等待来确保回调在函数返回前执行
        std::promise<application::SaveMemoryPositionResp> promise;
        auto future = promise.get_future();

        api_handlers_->HandleSeatMemorySaveRequest(request, [&promise](const application::SaveMemoryPositionResp& response) {
            promise.set_value(response);
        });

        // 等待回调完成（最多等待5秒）
        if (future.wait_for(std::chrono::seconds(5)) == std::future_status::timeout) {
            SendErrorResponse(res, "REQUEST_TIMEOUT", "Request timed out", 408);
            return;
        }

        auto response = future.get();
        auto json_response = JsonConverter::ToJson(response);
        auto success_response = JsonConverter::CreateSuccessResponse(json_response);
        res.set_content(success_response.dump(), "application/json");

    } catch (const std::exception& e) {
        SendErrorResponse(res, "INVALID_REQUEST", e.what(), 400);
    }
}

void HttpServer::SendErrorResponse(httplib::Response& res, const std::string& error,
                                  const std::string& message, int status_code) {
    auto error_json = JsonConverter::CreateErrorResponse(error, message);
    res.set_content(error_json.dump(), "application/json");
    res.status = status_code;
}

// ============================================================================
// SSE连接管理实现
// ============================================================================

void HttpServer::RegisterSSEConnection(uint64_t connection_id, httplib::DataSink* sink) {
    std::lock_guard<std::mutex> lock(sse_connections_mutex_);
    sse_connections_[connection_id] = sink;
    std::cout << "[HttpServer] SSE connection registered: " << connection_id
              << " (total: " << sse_connections_.size() << ")" << std::endl;
}

void HttpServer::UnregisterSSEConnection(uint64_t connection_id) {
    std::lock_guard<std::mutex> lock(sse_connections_mutex_);
    auto it = sse_connections_.find(connection_id);
    if (it != sse_connections_.end()) {
        sse_connections_.erase(it);
        std::cout << "[HttpServer] SSE connection unregistered: " << connection_id
                  << " (remaining: " << sse_connections_.size() << ")" << std::endl;
    }
}

void HttpServer::BroadcastSSEEvent(const std::string& event_data) {
    std::lock_guard<std::mutex> lock(sse_connections_mutex_);

    if (sse_connections_.empty()) {
        std::cout << "[HttpServer] No SSE connections to broadcast to" << std::endl;
        return;
    }

    std::cout << "[HttpServer] Broadcasting SSE event to " << sse_connections_.size() << " connections" << std::endl;

    // 记录失效的连接
    std::vector<uint64_t> dead_connections;

    for (auto& [connection_id, sink] : sse_connections_) {
        if (!sink->write(event_data.c_str(), event_data.length())) {
            std::cout << "[HttpServer] SSE connection " << connection_id << " is dead, marking for removal" << std::endl;
            dead_connections.push_back(connection_id);
        }
    }

    // 清理失效连接
    for (uint64_t dead_id : dead_connections) {
        sse_connections_.erase(dead_id);
    }

    if (!dead_connections.empty()) {
        std::cout << "[HttpServer] Removed " << dead_connections.size() << " dead SSE connections" << std::endl;
    }
}

// ============================================================================
// SSE事件推送实现
// ============================================================================

void HttpServer::PublishEvent(const std::string& event_type, const nlohmann::json& data) {
    // 生成符合SSE规范的事件帧：event: <type>\n data: <json>\n\n
    nlohmann::json envelope = {
        {"type", event_type},
        {"data", data},
        {"timestamp", std::time(nullptr)}
    };

    std::string frame;
    frame.reserve(64 + envelope.dump().size());
    frame += "event: ";
    frame += event_type;
    frame += "\n";
    frame += "data: ";
    frame += envelope.dump();
    frame += "\n\n";

    BroadcastSSEEvent(frame);
}

void HttpServer::PushDoorLockEvent(int door_id, bool lock_state) {
    nlohmann::json data = {
        {"doorID", door_id},
        {"lockState", lock_state}
    };
    PublishEvent("door_lock_changed", data);

    std::cout << "[HttpServer] Pushed door lock event: door " << door_id
              << " " << (lock_state ? "locked" : "unlocked") << std::endl;
}

void HttpServer::PushWindowPositionEvent(int window_id, int position) {
    nlohmann::json data = {
        {"windowID", window_id},
        {"position", position}
    };
    PublishEvent("window_position_changed", data);

    std::cout << "[HttpServer] Pushed window position event: window " << window_id
              << " position " << position << "%" << std::endl;
}

void HttpServer::PushLightStateEvent(const std::string& light_type, bool state) {
    nlohmann::json data = {
        {"lightType", light_type},
        {"state", state}
    };
    PublishEvent("light_state_changed", data);

    std::cout << "[HttpServer] Pushed light state event: " << light_type
              << " " << (state ? "on" : "off") << std::endl;
}

void HttpServer::PushSeatPositionEvent(int seat_id, const std::string& position) {
    nlohmann::json data = {
        {"seatID", seat_id},
        {"position", position}
    };
    PublishEvent("seat_position_changed", data);

    std::cout << "[HttpServer] Pushed seat position event: seat " << seat_id
              << " position " << position << std::endl;
}

} // namespace web_api
} // namespace body_controller
