#include "communication/someip_client.h"
#include "communication/serialization.h"
#include "communication/someip_service_definitions.h"
#include <iostream>

namespace body_controller {
namespace communication {

// ============================================================================
// WindowServiceClient 实现
// ============================================================================

WindowServiceClient::WindowServiceClient(const std::string& app_name)
    : SomeipClient(app_name) {
    std::cout << "[WindowServiceClient] Created window service client" << std::endl;
}

bool WindowServiceClient::Initialize() {
    if (!SomeipClient::Initialize()) {
        return false;
    }
    
    // 注册消息处理器（用于响应和事件）
    app_->register_message_handler(
        body_controller::communication::WINDOW_SERVICE_ID, body_controller::communication::WINDOW_INSTANCE_ID, vsomeip::ANY_METHOD,
        std::bind(&WindowServiceClient::OnMessage, this, std::placeholders::_1)
    );

    // 注册可用性处理器
    app_->register_availability_handler(
        body_controller::communication::WINDOW_SERVICE_ID, body_controller::communication::WINDOW_INSTANCE_ID,
        std::bind(&WindowServiceClient::OnAvailability, this,
                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    
    std::cout << "[WindowServiceClient] Window service client initialized" << std::endl;

    // 请求车窗服务
    std::cout << "[WindowServiceClient] Requesting service 0x" << std::hex
              << body_controller::communication::WINDOW_SERVICE_ID
              << ".0x" << body_controller::communication::WINDOW_INSTANCE_ID << std::dec << std::endl;
    app_->request_service(body_controller::communication::WINDOW_SERVICE_ID,
                          body_controller::communication::WINDOW_INSTANCE_ID);
    return true;
}

void WindowServiceClient::SetWindowPosition(const application::SetWindowPositionReq& request) {
    std::cout << "[WindowServiceClient] Setting window position for window: " 
              << static_cast<int>(request.windowID) 
              << " Position: " << static_cast<int>(request.position) << "%" << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::WINDOW_SERVICE_ID, body_controller::communication::WINDOW_INSTANCE_ID, window_service::SET_WINDOW_POSITION, payload_data);
}

void WindowServiceClient::ControlWindow(const application::ControlWindowReq& request) {
    std::cout << "[WindowServiceClient] Controlling window: " 
              << static_cast<int>(request.windowID) 
              << " Command: " << static_cast<int>(request.command) << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::WINDOW_SERVICE_ID, body_controller::communication::WINDOW_INSTANCE_ID, window_service::CONTROL_WINDOW, payload_data);
}

void WindowServiceClient::GetWindowPosition(const application::GetWindowPositionReq& request) {
    std::cout << "[WindowServiceClient] Getting window position for window: " 
              << static_cast<int>(request.windowID) << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::WINDOW_SERVICE_ID, body_controller::communication::WINDOW_INSTANCE_ID, window_service::GET_WINDOW_POSITION, payload_data);
}

void WindowServiceClient::OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
    SomeipClient::OnAvailability(service, instance, is_available);
    
    if (service == body_controller::communication::WINDOW_SERVICE_ID && instance == body_controller::communication::WINDOW_INSTANCE_ID && is_available) {
        std::cout << "[WindowServiceClient] Window service is available, subscribing to events..." << std::endl;
        
        // 订阅车窗位置变化事件
        SubscribeEvent(body_controller::communication::WINDOW_SERVICE_ID, body_controller::communication::WINDOW_INSTANCE_ID,
                      window_events::ON_WINDOW_POSITION_CHANGED, body_controller::communication::WINDOW_EVENTS_GROUP_ID);
    }
}

void WindowServiceClient::OnMessage(const std::shared_ptr<vsomeip::message>& message) {
    if (!message) {
        return;
    }
    
    // 检查是否是车窗服务的消息
    if (message->get_service() != body_controller::communication::WINDOW_SERVICE_ID || message->get_instance() != body_controller::communication::WINDOW_INSTANCE_ID) {
        return;
    }
    
    auto message_type = message->get_message_type();
    auto method_id = message->get_method();
    
    if (message_type == vsomeip::message_type_e::MT_RESPONSE) {
        // 处理方法响应
        if (method_id == window_service::SET_WINDOW_POSITION) {
            HandleSetWindowPositionResponse(message);
        } else if (method_id == window_service::CONTROL_WINDOW) {
            HandleControlWindowResponse(message);
        } else if (method_id == window_service::GET_WINDOW_POSITION) {
            HandleGetWindowPositionResponse(message);
        }
    } else if (message_type == vsomeip::message_type_e::MT_NOTIFICATION) {
        // 处理事件通知
        if (method_id == window_events::ON_WINDOW_POSITION_CHANGED) {
            HandleWindowPositionChangedEvent(message);
        }
    }
}

void WindowServiceClient::HandleSetWindowPositionResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[WindowServiceClient] SetWindowPosition response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::SetWindowPositionResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[WindowServiceClient] SetWindowPosition response - Window: " 
                  << static_cast<int>(response.windowID)
                  << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (set_position_response_handler_) {
            set_position_response_handler_(response);
        }
    } else {
        std::cerr << "[WindowServiceClient] Failed to deserialize SetWindowPosition response" << std::endl;
    }
}

void WindowServiceClient::HandleControlWindowResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[WindowServiceClient] ControlWindow response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::ControlWindowResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[WindowServiceClient] ControlWindow response - Window: " 
                  << static_cast<int>(response.windowID)
                  << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (control_response_handler_) {
            control_response_handler_(response);
        }
    } else {
        std::cerr << "[WindowServiceClient] Failed to deserialize ControlWindow response" << std::endl;
    }
}

void WindowServiceClient::HandleGetWindowPositionResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[WindowServiceClient] GetWindowPosition response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::GetWindowPositionResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[WindowServiceClient] GetWindowPosition response - Window: " 
                  << static_cast<int>(response.windowID)
                  << " Position: " << static_cast<int>(response.position) << "%" << std::endl;
        
        // 调用用户回调
        if (get_position_response_handler_) {
            get_position_response_handler_(response);
        }
    } else {
        std::cerr << "[WindowServiceClient] Failed to deserialize GetWindowPosition response" << std::endl;
    }
}

void WindowServiceClient::HandleWindowPositionChangedEvent(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[WindowServiceClient] WindowPositionChanged event has no payload" << std::endl;
        return;
    }
    
    // 反序列化事件数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::OnWindowPositionChangedData event_data;
    
    if (Serializer::Deserialize(payload_data, event_data)) {
        std::cout << "[WindowServiceClient] WindowPositionChanged event - Window: " 
                  << static_cast<int>(event_data.windowID)
                  << " New Position: " << static_cast<int>(event_data.newPosition) << "%" << std::endl;
        
        // 调用用户回调
        if (window_position_changed_handler_) {
            window_position_changed_handler_(event_data);
        }
    } else {
        std::cerr << "[WindowServiceClient] Failed to deserialize WindowPositionChanged event" << std::endl;
    }
}

} // namespace communication
} // namespace body_controller
