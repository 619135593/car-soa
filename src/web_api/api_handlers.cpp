#include "web_api/api_handlers.h"
#include "web_api/http_server.h"
// WebSocket服务器已移除，使用SSE替代
#include "communication/someip_service_definitions.h"
#include <iostream>
#include <future>
#include <chrono>

namespace body_controller {
namespace web_api {

ApiHandlers::ApiHandlers() {
    std::cout << "[ApiHandlers] Created API handlers" << std::endl;
}

ApiHandlers::~ApiHandlers() {
    Stop();
}

bool ApiHandlers::Initialize() {
    try {
        // 创建所有服务客户端（不立即初始化）
        door_client_ = std::make_shared<communication::DoorServiceClient>("web_door_client");
        window_client_ = std::make_shared<communication::WindowServiceClient>("web_window_client");
        light_client_ = std::make_shared<communication::LightServiceClient>("web_light_client");
        seat_client_ = std::make_shared<communication::SeatServiceClient>("web_seat_client");

        // 设置响应处理器和事件处理器（即使没有SOME/IP连接也需要）
        SetupResponseHandlers();
        SetupEventHandlers();

        std::cout << "[ApiHandlers] Created SOME/IP service clients" << std::endl;
        std::cout << "[ApiHandlers] Note: SOME/IP clients will be initialized when services are available" << std::endl;
        std::cout << "[ApiHandlers] Web server will use mock data until SOME/IP services connect" << std::endl;

        return true;

    } catch (const std::exception& e) {
        std::cerr << "[ApiHandlers] Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

bool ApiHandlers::Start() {
    if (running_) {
        std::cout << "[ApiHandlers] API handlers already running" << std::endl;
        return true;
    }

    try {
        std::cout << "[ApiHandlers] Starting API handlers..." << std::endl;

        // 立即标记为运行状态
        running_ = true;

        // 初始化并启动各SOME/IP客户端
        if (door_client_) {
            if (door_client_->Initialize()) door_client_->Start();
        }
        if (window_client_) {
            if (window_client_->Initialize()) window_client_->Start();
        }
        if (light_client_) {
            if (light_client_->Initialize()) light_client_->Start();
        }
        if (seat_client_) {
            if (seat_client_->Initialize()) seat_client_->Start();
        }

        // 设置响应处理器和事件处理器
        SetupResponseHandlers();
        SetupEventHandlers();

        std::cout << "[ApiHandlers] ✅ API handlers started successfully" << std::endl;
        std::cout << "[ApiHandlers] 📝 Ready to serve requests (will use mock data until SOME/IP connects)" << std::endl;

        return true;

    } catch (const std::exception& e) {
        std::cerr << "[ApiHandlers] Failed to start: " << e.what() << std::endl;
        running_ = true; // 即使失败也标记为运行，使用模拟数据
        return true; // 返回true以不阻塞Web服务器启动
    }
}

void ApiHandlers::Stop() {
    if (!running_) {
        return;
    }
    
    std::cout << "[ApiHandlers] Stopping API handlers..." << std::endl;
    running_ = false;
    
    // 停止所有服务客户端
    if (door_client_) {
        door_client_->Stop();
    }
    if (window_client_) {
        window_client_->Stop();
    }
    if (light_client_) {
        light_client_->Stop();
    }
    if (seat_client_) {
        seat_client_->Stop();
    }
    
    std::cout << "[ApiHandlers] API handlers stopped" << std::endl;
}

void ApiHandlers::SetHttpServer(std::shared_ptr<HttpServer> http_server) {
    http_server_ = http_server;
    std::cout << "[ApiHandlers] HTTP server reference set for SSE event pushing" << std::endl;
}

// ============================================================================
// 车门服务处理
// ============================================================================

void ApiHandlers::HandleDoorLockRequest(const application::SetLockStateReq& request,
                                        std::function<void(const application::SetLockStateResp&)> callback) {
    std::cout << "[ApiHandlers] HandleDoorLockRequest called for door " << static_cast<int>(request.doorID) << std::endl;

    // 检查门服务是否可用
    if (!door_client_ || !running_ || !IsDoorServiceAvailable()) {
        std::cout << "[ApiHandlers] Door service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::SetLockStateResp mock_response;
            mock_response.doorID = request.doorID;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock door lock response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock door lock callback completed" << std::endl;

            // 推送SSE事件（模拟状态变化）
            if (auto http_server = http_server_.lock()) {
                bool new_lock_state = (request.command == application::LockCommand::LOCK);
                http_server->PushDoorLockEvent(static_cast<int>(request.doorID), new_lock_state);
            }
        } else {
            std::cout << "[ApiHandlers] ERROR: Door lock callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Door service is available, setting up real request..." << std::endl;

    // 使用shared_ptr确保回调函数的生命周期
    auto shared_callback = std::make_shared<std::function<void(const application::SetLockStateResp&)>>(callback);

    // 设置临时响应处理器
    door_client_->SetSetLockStateResponseHandler([shared_callback](const application::SetLockStateResp& response) {
        std::cout << "[ApiHandlers] Received door lock response from client" << std::endl;
        if (shared_callback && *shared_callback) {
            (*shared_callback)(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending door lock request to client..." << std::endl;
    door_client_->SetLockState(request);
    std::cout << "[ApiHandlers] Door lock request sent" << std::endl;
}

void ApiHandlers::HandleDoorStatusRequest(const application::GetLockStateReq& request,
                                         std::function<void(const application::GetLockStateResp&)> callback) {
    std::cout << "[ApiHandlers] HandleDoorStatusRequest called for door " << static_cast<int>(request.doorID) << std::endl;

    // 检查门服务是否可用
    if (!door_client_ || !running_ || !IsDoorServiceAvailable()) {
        std::cout << "[ApiHandlers] Door service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::GetLockStateResp mock_response;
            mock_response.doorID = request.doorID;
            mock_response.lockState = application::LockState::LOCKED; // 模拟锁定状态
            std::cout << "[ApiHandlers] Calling callback with mock response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock callback completed" << std::endl;
        } else {
            std::cout << "[ApiHandlers] ERROR: Callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Door service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    door_client_->SetGetLockStateResponseHandler([callback](const application::GetLockStateResp& response) {
        std::cout << "[ApiHandlers] Received response from door client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to door client..." << std::endl;
    door_client_->GetLockState(request);
    std::cout << "[ApiHandlers] Request sent" << std::endl;
}

// ============================================================================
// 车窗服务处理
// ============================================================================

void ApiHandlers::HandleWindowPositionRequest(const application::SetWindowPositionReq& request,
                                              std::function<void(const application::SetWindowPositionResp&)> callback) {
    if (!window_client_) {
        std::cerr << "[ApiHandlers] Window service client not available" << std::endl;
        return;
    }
    
    // 设置临时响应处理器
    window_client_->SetSetWindowPositionResponseHandler([callback](const application::SetWindowPositionResp& response) {
        if (callback) {
            callback(response);
        }
    });
    
    // 发送请求
    window_client_->SetWindowPosition(request);
}

void ApiHandlers::HandleWindowControlRequest(const application::ControlWindowReq& request,
                                             std::function<void(const application::ControlWindowResp&)> callback) {
    std::cout << "[ApiHandlers] HandleWindowControlRequest called for window " << static_cast<int>(request.windowID) << std::endl;

    // 检查窗口服务是否可用
    if (!window_client_ || !running_ || !IsWindowServiceAvailable()) {
        std::cout << "[ApiHandlers] Window service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::ControlWindowResp mock_response;
            mock_response.windowID = request.windowID;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock window control response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock window control callback completed" << std::endl;

            // 推送SSE事件（模拟状态变化）
            if (auto http_server = http_server_.lock()) {
                // 模拟窗口位置变化（根据控制命令）
                int new_position = 50; // 模拟位置
                if (request.command == application::WindowCommand::MOVE_UP) {
                    new_position = 0; // 完全关闭
                } else if (request.command == application::WindowCommand::MOVE_DOWN) {
                    new_position = 100; // 完全打开
                }
                http_server->PushWindowPositionEvent(static_cast<int>(request.windowID), new_position);
            }
        } else {
            std::cout << "[ApiHandlers] ERROR: Window control callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Window service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    window_client_->SetControlWindowResponseHandler([callback](const application::ControlWindowResp& response) {
        std::cout << "[ApiHandlers] Received window control response from client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending window control request to client..." << std::endl;
    window_client_->ControlWindow(request);
    std::cout << "[ApiHandlers] Window control request sent" << std::endl;
}

void ApiHandlers::HandleWindowPositionStatusRequest(const application::GetWindowPositionReq& request,
                                                    std::function<void(const application::GetWindowPositionResp&)> callback) {
    std::cout << "[ApiHandlers] HandleWindowPositionStatusRequest called for window " << static_cast<int>(request.windowID) << std::endl;

    // 检查窗口服务是否可用
    if (!window_client_ || !running_ || !IsWindowServiceAvailable()) {
        std::cout << "[ApiHandlers] Window service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::GetWindowPositionResp mock_response;
            mock_response.windowID = request.windowID;
            mock_response.position = 50; // 模拟50%位置
            std::cout << "[ApiHandlers] Calling callback with mock window response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock window callback completed" << std::endl;
        } else {
            std::cout << "[ApiHandlers] ERROR: Window callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Window service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    window_client_->SetGetWindowPositionResponseHandler([callback](const application::GetWindowPositionResp& response) {
        std::cout << "[ApiHandlers] Received response from window client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to window client..." << std::endl;
    window_client_->GetWindowPosition(request);
    std::cout << "[ApiHandlers] Window request sent" << std::endl;
}

// ============================================================================
// 灯光服务处理
// ============================================================================

void ApiHandlers::HandleHeadlightRequest(const application::SetHeadlightStateReq& request,
                                         std::function<void(const application::SetHeadlightStateResp&)> callback) {
    std::cout << "[ApiHandlers] HandleHeadlightRequest called" << std::endl;

    // 检查灯光服务是否可用
    if (!light_client_ || !running_ || !IsLightServiceAvailable()) {
        std::cout << "[ApiHandlers] Light service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::SetHeadlightStateResp mock_response;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock headlight response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock headlight callback completed" << std::endl;

            // 推送SSE事件（模拟状态变化）
            if (auto http_server = http_server_.lock()) {
                bool new_state = (request.command != application::HeadlightState::OFF);
                http_server->PushLightStateEvent("headlight", new_state);
            }
        } else {
            std::cout << "[ApiHandlers] ERROR: Headlight callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Light service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    light_client_->SetSetHeadlightStateResponseHandler([callback](const application::SetHeadlightStateResp& response) {
        std::cout << "[ApiHandlers] Received response from light client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to light client..." << std::endl;
    light_client_->SetHeadlightState(request);
    std::cout << "[ApiHandlers] Request sent" << std::endl;
}

void ApiHandlers::HandleIndicatorRequest(const application::SetIndicatorStateReq& request,
                                        std::function<void(const application::SetIndicatorStateResp&)> callback) {
    std::cout << "[ApiHandlers] HandleIndicatorRequest called" << std::endl;

    // 检查灯光服务是否可用
    if (!light_client_ || !running_ || !IsLightServiceAvailable()) {
        std::cout << "[ApiHandlers] Light service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::SetIndicatorStateResp mock_response;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock indicator response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock indicator callback completed" << std::endl;

            // 推送SSE事件（模拟状态变化）
            if (auto http_server = http_server_.lock()) {
                bool new_state = (request.command != application::IndicatorState::OFF);
                http_server->PushLightStateEvent("indicator", new_state);
            }
        } else {
            std::cout << "[ApiHandlers] ERROR: Indicator callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Light service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    light_client_->SetSetIndicatorStateResponseHandler([callback](const application::SetIndicatorStateResp& response) {
        std::cout << "[ApiHandlers] Received response from light client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to light client..." << std::endl;
    light_client_->SetIndicatorState(request);
    std::cout << "[ApiHandlers] Request sent" << std::endl;
}

void ApiHandlers::HandlePositionLightRequest(const application::SetPositionLightStateReq& request,
                                             std::function<void(const application::SetPositionLightStateResp&)> callback) {
    std::cout << "[ApiHandlers] HandlePositionLightRequest called" << std::endl;

    // 检查灯光服务是否可用
    if (!light_client_ || !running_ || !IsLightServiceAvailable()) {
        std::cout << "[ApiHandlers] Light service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::SetPositionLightStateResp mock_response;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock position light response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock position light callback completed" << std::endl;

            // 推送SSE事件（模拟状态变化）
            if (auto http_server = http_server_.lock()) {
                bool new_state = (request.command != application::PositionLightState::OFF);
                http_server->PushLightStateEvent("position", new_state);
            }
        } else {
            std::cout << "[ApiHandlers] ERROR: Position light callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Light service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    light_client_->SetSetPositionLightStateResponseHandler([callback](const application::SetPositionLightStateResp& response) {
        std::cout << "[ApiHandlers] Received response from light client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to light client..." << std::endl;
    light_client_->SetPositionLightState(request);
    std::cout << "[ApiHandlers] Request sent" << std::endl;
}

// ============================================================================
// 座椅服务处理
// ============================================================================

void ApiHandlers::HandleSeatAdjustRequest(const application::AdjustSeatReq& request,
                                          std::function<void(const application::AdjustSeatResp&)> callback) {
    std::cout << "[ApiHandlers] HandleSeatAdjustRequest called" << std::endl;

    // 检查座椅服务是否可用
    if (!seat_client_ || !running_ || !IsSeatServiceAvailable()) {
        std::cout << "[ApiHandlers] Seat service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::AdjustSeatResp mock_response;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock seat adjust response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock seat adjust callback completed" << std::endl;

            // 推送SSE事件（模拟状态变化）
            if (auto http_server = http_server_.lock()) {
                std::string position_info = "adjusted";
                http_server->PushSeatPositionEvent(0, position_info); // 假设座椅ID为0
            }
        } else {
            std::cout << "[ApiHandlers] ERROR: Seat adjust callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Seat service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    seat_client_->SetAdjustSeatResponseHandler([callback](const application::AdjustSeatResp& response) {
        std::cout << "[ApiHandlers] Received response from seat client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to seat client..." << std::endl;
    seat_client_->AdjustSeat(request);
    std::cout << "[ApiHandlers] Request sent" << std::endl;
}

void ApiHandlers::HandleSeatMemoryRecallRequest(const application::RecallMemoryPositionReq& request,
                                                std::function<void(const application::RecallMemoryPositionResp&)> callback) {
    std::cout << "[ApiHandlers] HandleSeatMemoryRecallRequest called" << std::endl;

    // 检查座椅服务是否可用
    if (!seat_client_ || !running_ || !IsSeatServiceAvailable()) {
        std::cout << "[ApiHandlers] Seat service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::RecallMemoryPositionResp mock_response;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock seat memory recall response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock seat memory recall callback completed" << std::endl;
        } else {
            std::cout << "[ApiHandlers] ERROR: Seat memory recall callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Seat service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    seat_client_->SetRecallMemoryPositionResponseHandler([callback](const application::RecallMemoryPositionResp& response) {
        std::cout << "[ApiHandlers] Received response from seat client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to seat client..." << std::endl;
    seat_client_->RecallMemoryPosition(request);
    std::cout << "[ApiHandlers] Request sent" << std::endl;
}

void ApiHandlers::HandleSeatMemorySaveRequest(const application::SaveMemoryPositionReq& request,
                                              std::function<void(const application::SaveMemoryPositionResp&)> callback) {
    std::cout << "[ApiHandlers] HandleSeatMemorySaveRequest called" << std::endl;

    // 检查座椅服务是否可用
    if (!seat_client_ || !running_ || !IsSeatServiceAvailable()) {
        std::cout << "[ApiHandlers] Seat service not available, returning mock response" << std::endl;
        // 返回模拟响应
        if (callback) {
            application::SaveMemoryPositionResp mock_response;
            mock_response.result = application::Result::SUCCESS; // 模拟成功
            std::cout << "[ApiHandlers] Calling callback with mock seat memory save response..." << std::endl;
            callback(mock_response);
            std::cout << "[ApiHandlers] Mock seat memory save callback completed" << std::endl;
        } else {
            std::cout << "[ApiHandlers] ERROR: Seat memory save callback is null!" << std::endl;
        }
        return;
    }

    std::cout << "[ApiHandlers] Seat service is available, setting up real request..." << std::endl;

    // 设置临时响应处理器
    seat_client_->SetSaveMemoryPositionResponseHandler([callback](const application::SaveMemoryPositionResp& response) {
        std::cout << "[ApiHandlers] Received response from seat client" << std::endl;
        if (callback) {
            callback(response);
        }
    });

    // 发送请求
    std::cout << "[ApiHandlers] Sending request to seat client..." << std::endl;
    seat_client_->SaveMemoryPosition(request);
    std::cout << "[ApiHandlers] Request sent" << std::endl;
}

// ============================================================================
// 服务状态检查
// ============================================================================

bool ApiHandlers::IsDoorServiceAvailable() const {
    return door_client_ && door_client_->IsServiceAvailable(body_controller::communication::DOOR_SERVICE_ID, body_controller::communication::DOOR_INSTANCE_ID);
}

bool ApiHandlers::IsWindowServiceAvailable() const {
    return window_client_ && window_client_->IsServiceAvailable(body_controller::communication::WINDOW_SERVICE_ID, body_controller::communication::WINDOW_INSTANCE_ID);
}

bool ApiHandlers::IsLightServiceAvailable() const {
    return light_client_ && light_client_->IsServiceAvailable(body_controller::communication::LIGHT_SERVICE_ID, body_controller::communication::LIGHT_INSTANCE_ID);
}

bool ApiHandlers::IsSeatServiceAvailable() const {
    return seat_client_ && seat_client_->IsServiceAvailable(body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID);
}

// ============================================================================
// WebSocket事件广播
// ============================================================================

// WebSocket服务器已移除，使用SSE替代事件广播

// ============================================================================
// 私有方法：设置处理器
// ============================================================================

void ApiHandlers::SetupResponseHandlers() {
    // 响应处理器在每次请求时动态设置，这里不需要设置全局处理器
    std::cout << "[ApiHandlers] Response handlers setup completed" << std::endl;
}

void ApiHandlers::SetupEventHandlers() {
    // 设置事件处理器，用于WebSocket广播
    if (door_client_) {
        door_client_->SetLockStateChangedHandler([this](const application::OnLockStateChangedData& data) {
            BroadcastEvent("door_lock_changed", JsonConverter::ToJson(data));
        });
        
        door_client_->SetDoorStateChangedHandler([this](const application::OnDoorStateChangedData& data) {
            BroadcastEvent("door_state_changed", JsonConverter::ToJson(data));
        });
    }
    
    if (window_client_) {
        window_client_->SetWindowPositionChangedHandler([this](const application::OnWindowPositionChangedData& data) {
            BroadcastEvent("window_position_changed", JsonConverter::ToJson(data));
        });
    }
    
    if (light_client_) {
        light_client_->SetLightStateChangedHandler([this](const application::OnLightStateChangedData& data) {
            BroadcastEvent("light_state_changed", JsonConverter::ToJson(data));
        });
    }
    
    if (seat_client_) {
        seat_client_->SetSeatPositionChangedHandler([this](const application::OnSeatPositionChangedData& data) {
            BroadcastEvent("seat_position_changed", JsonConverter::ToJson(data));
        });
        
        seat_client_->SetMemorySaveConfirmHandler([this](const application::OnMemorySaveConfirmData& data) {
            BroadcastEvent("seat_memory_save_confirm", JsonConverter::ToJson(data));
        });
    }
    
    std::cout << "[ApiHandlers] Event handlers setup completed" << std::endl;
}

void ApiHandlers::BroadcastEvent(const std::string& event_type, const nlohmann::json& data) {
    // 将SOME/IP事件映射并通过SSE推送到前端
    std::cout << "[ApiHandlers] Event broadcast via SSE: " << event_type << std::endl;
    if (auto http = http_server_.lock()) {
        try {
            if (event_type == "door_lock_changed") {
                const int door_id = data.value("doorID", 0);
                const bool lock_state = data.value("lockState", false);
                http->PushDoorLockEvent(door_id, lock_state);
                return;
            }
            if (event_type == "window_position_changed") {
                const int window_id = data.value("windowID", 0);
                const int position = data.value("position", 0);
                http->PushWindowPositionEvent(window_id, position);
                return;
            }
            if (event_type == "light_state_changed") {
                // 若数据包含细分类型，可读取；否则默认"headlight"
                const std::string light_type = data.value("lightType", std::string("headlight"));
                const bool state = data.contains("state") ? data["state"].get<bool>()
                                  : data.value("newState", 0) != 0;
                http->PushLightStateEvent(light_type, state);
                return;
            }
            if (event_type == "seat_position_changed") {
                const int seat_id = data.value("seatID", 0);
                const std::string position = data.contains("position") ? data["position"].dump() : data.dump();
                http->PushSeatPositionEvent(seat_id, position);
                return;
            }

            // 未识别事件：走通用事件发布
            http->PublishEvent(event_type, data);
        } catch (const std::exception& e) {
            std::cerr << "[ApiHandlers] BroadcastEvent error: " << e.what() << std::endl;
        }
    } else {
        std::cout << "[ApiHandlers] HTTP server not available for SSE push" << std::endl;
    }
}

} // namespace web_api
} // namespace body_controller
