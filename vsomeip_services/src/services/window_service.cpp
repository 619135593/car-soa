#include "services/window_service.h"
#include "common/serializer.h"
#include <iostream>
#include <thread>
#include <random>
#include <set>

namespace body_controller {
namespace services {

WindowService::WindowService(std::shared_ptr<vsomeip::application> app, 
                            std::shared_ptr<HardwareSimulator> simulator)
    : app_(app)
    , running_(false)
    , hardware_simulator_(simulator)
{
    // 初始化当前位置（默认50%）
    for (int i = 0; i < 4; ++i) {
        current_positions_[i] = 50;
    }
    
    std::cout << "[WindowService] Window service created" << std::endl;
}

WindowService::~WindowService() {
    Stop();
    std::cout << "[WindowService] Window service destroyed" << std::endl;
}

bool WindowService::Initialize() {
    if (!app_) {
        std::cerr << "[WindowService] VSOMEIP application not available" << std::endl;
        return false;
    }
    
    try {
        // 注册服务
        app_->offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 注册方法处理器
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, SET_WINDOW_POSITION_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleSetWindowPositionRequest(request);
            });
            
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, CONTROL_WINDOW_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleControlWindowRequest(request);
            });
            
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, GET_WINDOW_POSITION_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleGetWindowPositionRequest(request);
            });
        
        // 注册事件
        std::set<vsomeip::eventgroup_t> event_groups;
        event_groups.insert(EVENT_GROUP);
        
        app_->offer_event(SERVICE_ID, INSTANCE_ID, WINDOW_POSITION_CHANGED_EVENT, 
                         event_groups, vsomeip::event_type_e::ET_EVENT, 
                         std::chrono::milliseconds::zero(),
                         false, true, nullptr, vsomeip::reliability_type_e::RT_RELIABLE);
        
        // 设置硬件模拟器回调
        if (hardware_simulator_) {
            hardware_simulator_->SetWindowPositionEventCallback(
                [this](const application::OnWindowPositionChangedData& event) {
                    OnWindowPositionChanged(event);
                });
        }
        
        std::cout << "[WindowService] Window service initialized successfully" << std::endl;
        std::cout << "[WindowService] Service ID: 0x" << std::hex << SERVICE_ID << std::dec << std::endl;
        std::cout << "[WindowService] Instance ID: 0x" << std::hex << INSTANCE_ID << std::dec << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[WindowService] Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

bool WindowService::Start() {
    if (running_) {
        std::cout << "[WindowService] Already running" << std::endl;
        return true;
    }
    
    running_ = true;
    std::cout << "[WindowService] Window service started" << std::endl;
    return true;
}

void WindowService::Stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (app_) {
        // 停止提供服务
        app_->stop_offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 取消注册处理器
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, SET_WINDOW_POSITION_METHOD);
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, CONTROL_WINDOW_METHOD);
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, GET_WINDOW_POSITION_METHOD);
    }
    
    std::cout << "[WindowService] Window service stopped" << std::endl;
}

void WindowService::HandleSetWindowPositionRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[WindowService] Received SetWindowPosition request" << std::endl;
    
    try {
        // 反序列化请求数据
        auto payload = request->get_payload();
        std::vector<uint8_t> data(payload->get_data(), payload->get_data() + payload->get_length());
        
        application::SetWindowPositionReq req;
        if (!Serializer::Deserialize(data, req)) {
            std::cerr << "[WindowService] Failed to deserialize SetWindowPosition request" << std::endl;
            SendErrorResponse(request, 1);
            return;
        }
        
        std::cout << "[WindowService] SetWindowPosition - Window: " << static_cast<int>(req.windowID)
                  << " Target: " << static_cast<int>(req.position) << "%" << std::endl;
        
        // 模拟位置设置操作
        application::Result result = SimulateSetPositionOperation(req.windowID, req.position);

        // 创建响应
        auto response_data = (result == application::Result::SUCCESS)
                           ? Serializer::SerializeSuccessResponse()
                           : Serializer::SerializeFailResponse();
        SendResponse(request, response_data);
        
        // 如果操作成功，触发硬件事件
        if (result == application::Result::SUCCESS && hardware_simulator_) {
            // 延迟触发事件，模拟硬件响应时间
            std::thread([this, req]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(200)); // 车窗移动需要时间
                hardware_simulator_->TriggerWindowPositionEvent(req.windowID, req.position);
            }).detach();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[WindowService] Error handling SetWindowPosition request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void WindowService::HandleControlWindowRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[WindowService] Received ControlWindow request" << std::endl;
    
    try {
        // 反序列化请求数据
        auto payload = request->get_payload();
        std::vector<uint8_t> data(payload->get_data(), payload->get_data() + payload->get_length());
        
        application::ControlWindowReq req;
        if (!Serializer::Deserialize(data, req)) {
            std::cerr << "[WindowService] Failed to deserialize ControlWindow request" << std::endl;
            SendErrorResponse(request, 1);
            return;
        }
        
        std::string command_str;
        switch (req.command) {
            case application::WindowCommand::MOVE_UP: command_str = "MOVE_UP"; break;
            case application::WindowCommand::MOVE_DOWN: command_str = "MOVE_DOWN"; break;
            case application::WindowCommand::STOP: command_str = "STOP"; break;
        }
        
        std::cout << "[WindowService] ControlWindow - Window: " << static_cast<int>(req.windowID) 
                  << " Command: " << command_str << std::endl;
        
        // 模拟控制操作
        application::Result result = SimulateControlOperation(req.windowID, req.command);
        
        // 创建响应
        auto response_data = (result == application::Result::SUCCESS)
                           ? Serializer::SerializeSuccessResponse()
                           : Serializer::SerializeFailResponse();
        SendResponse(request, response_data);
        
        // 如果操作成功，触发硬件事件
        if (result == application::Result::SUCCESS && hardware_simulator_) {
            uint8_t new_position = GetCurrentPosition(req.windowID);
            
            // 根据命令计算新位置
            if (req.command == application::WindowCommand::MOVE_UP) {
                new_position = 0; // 完全关闭
            } else if (req.command == application::WindowCommand::MOVE_DOWN) {
                new_position = 100; // 完全打开
            }
            // STOP命令保持当前位置
            
            // 延迟触发事件
            std::thread([this, req, new_position]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                hardware_simulator_->TriggerWindowPositionEvent(req.windowID, new_position);
            }).detach();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[WindowService] Error handling ControlWindow request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void WindowService::HandleGetWindowPositionRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[WindowService] Received GetWindowPosition request" << std::endl;
    
    try {
        // 反序列化请求数据
        auto payload = request->get_payload();
        std::vector<uint8_t> data(payload->get_data(), payload->get_data() + payload->get_length());
        
        application::GetWindowPositionReq req;
        if (!Serializer::Deserialize(data, req)) {
            std::cerr << "[WindowService] Failed to deserialize GetWindowPosition request" << std::endl;
            SendErrorResponse(request, 1);
            return;
        }
        
        std::cout << "[WindowService] GetWindowPosition - Window: " << static_cast<int>(req.windowID) << std::endl;
        
        // 获取当前位置
        uint8_t current_position = GetCurrentPosition(req.windowID);
        
        // 创建响应
        application::GetWindowPositionResp response;
        response.windowID = req.windowID;
        response.position = current_position;
        
        // 序列化并发送响应
        auto response_data = Serializer::Serialize(response);
        SendResponse(request, response_data);
        
        std::cout << "[WindowService] GetWindowPosition response - Window: " << static_cast<int>(req.windowID)
                  << " Position: " << static_cast<int>(current_position) << "%" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[WindowService] Error handling GetWindowPosition request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void WindowService::SendResponse(const std::shared_ptr<vsomeip::message>& request,
                                const std::vector<uint8_t>& payload) {
    if (!app_) return;
    
    auto response = vsomeip::runtime::get()->create_response(request);
    auto response_payload = vsomeip::runtime::get()->create_payload();
    
    response_payload->set_data(payload.data(), static_cast<uint32_t>(payload.size()));
    response->set_payload(response_payload);
    
    app_->send(response);
}

void WindowService::SendErrorResponse(const std::shared_ptr<vsomeip::message>& request,
                                     uint8_t error_code) {
    auto error_data = Serializer::SerializeFailResponse();
    error_data.insert(error_data.begin(), error_code);
    
    SendResponse(request, error_data);
    
    std::cout << "[WindowService] Sent error response with code: " << static_cast<int>(error_code) << std::endl;
}

void WindowService::OnWindowPositionChanged(const application::OnWindowPositionChangedData& event_data) {
    std::cout << "[WindowService] Hardware event - Window position changed: Window " 
              << static_cast<int>(event_data.windowID) << " -> " 
              << static_cast<int>(event_data.newPosition) << "%" << std::endl;
    
    // 更新内部状态
    current_positions_[static_cast<int>(event_data.windowID)] = event_data.newPosition;
    
    // 发送事件到客户端
    SendWindowPositionChangedEvent(event_data);
}

void WindowService::SendWindowPositionChangedEvent(const application::OnWindowPositionChangedData& event_data) {
    if (!app_) return;
    
    auto event_payload = Serializer::Serialize(event_data);
    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(event_payload.data(), static_cast<uint32_t>(event_payload.size()));
    
    app_->notify(SERVICE_ID, INSTANCE_ID, WINDOW_POSITION_CHANGED_EVENT, payload);
    
    std::cout << "[WindowService] Sent window position changed event to clients" << std::endl;
}

application::Result WindowService::SimulateSetPositionOperation(application::Position window_id, 
                                                              uint8_t target_position) {
    // 模拟操作延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    int window_index = static_cast<int>(window_id);
    if (window_index < 0 || window_index >= 4 || target_position > 100) {
        return application::Result::FAIL;
    }
    
    // 模拟95%成功率
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    if ((gen() % 100) < 95) {
        return application::Result::SUCCESS;
    } else {
        std::cout << "[WindowService] Simulated position operation failure" << std::endl;
        return application::Result::FAIL;
    }
}

application::Result WindowService::SimulateControlOperation(application::Position window_id, 
                                                          application::WindowCommand command) {
    // 模拟操作延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    int window_index = static_cast<int>(window_id);
    if (window_index < 0 || window_index >= 4) {
        return application::Result::FAIL;
    }
    
    // 模拟95%成功率
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    if ((gen() % 100) < 95) {
        return application::Result::SUCCESS;
    } else {
        std::cout << "[WindowService] Simulated control operation failure" << std::endl;
        return application::Result::FAIL;
    }
}

uint8_t WindowService::GetCurrentPosition(application::Position window_id) const {
    int window_index = static_cast<int>(window_id);
    if (window_index >= 0 && window_index < 4) {
        return current_positions_[window_index];
    }
    return 50; // 默认位置
}

} // namespace services
} // namespace body_controller
