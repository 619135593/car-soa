#include "communication/someip_client.h"
#include "communication/serialization.h"
#include "communication/someip_service_definitions.h"
#include <iostream>

namespace body_controller {
namespace communication {

// ============================================================================
// LightServiceClient 实现
// ============================================================================

LightServiceClient::LightServiceClient(const std::string& app_name)
    : SomeipClient(app_name) {
    std::cout << "[LightServiceClient] Created light service client" << std::endl;
}

bool LightServiceClient::Initialize() {
    if (!SomeipClient::Initialize()) {
        return false;
    }
    
    // 注册消息处理器（用于响应和事件）
    app_->register_message_handler(
        body_controller::communication::LIGHT_SERVICE_ID, body_controller::communication::LIGHT_INSTANCE_ID, vsomeip::ANY_METHOD,
        std::bind(&LightServiceClient::OnMessage, this, std::placeholders::_1)
    );

    // 注册可用性处理器
    app_->register_availability_handler(
        body_controller::communication::LIGHT_SERVICE_ID, body_controller::communication::LIGHT_INSTANCE_ID,
        std::bind(&LightServiceClient::OnAvailability, this,
                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    
    std::cout << "[LightServiceClient] Light service client initialized" << std::endl;

    // 请求灯光服务
    std::cout << "[LightServiceClient] Requesting service 0x" << std::hex
              << body_controller::communication::LIGHT_SERVICE_ID
              << ".0x" << body_controller::communication::LIGHT_INSTANCE_ID << std::dec << std::endl;
    app_->request_service(body_controller::communication::LIGHT_SERVICE_ID,
                          body_controller::communication::LIGHT_INSTANCE_ID);
    return true;
}

void LightServiceClient::SetHeadlightState(const application::SetHeadlightStateReq& request) {
    std::cout << "[LightServiceClient] Setting headlight state: ";
    switch (request.command) {
        case application::HeadlightState::OFF:
            std::cout << "OFF";
            break;
        case application::HeadlightState::LOW_BEAM:
            std::cout << "LOW_BEAM";
            break;
        case application::HeadlightState::HIGH_BEAM:
            std::cout << "HIGH_BEAM";
            break;
        default:
            std::cout << "UNKNOWN(" << static_cast<int>(request.command) << ")";
            break;
    }
    std::cout << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::LIGHT_SERVICE_ID, body_controller::communication::LIGHT_INSTANCE_ID, light_service::SET_HEADLIGHT_STATE, payload_data);
}

void LightServiceClient::SetIndicatorState(const application::SetIndicatorStateReq& request) {
    std::cout << "[LightServiceClient] Setting indicator state: ";
    switch (request.command) {
        case application::IndicatorState::OFF:
            std::cout << "OFF";
            break;
        case application::IndicatorState::LEFT:
            std::cout << "LEFT";
            break;
        case application::IndicatorState::RIGHT:
            std::cout << "RIGHT";
            break;
        case application::IndicatorState::HAZARD:
            std::cout << "HAZARD";
            break;
        default:
            std::cout << "UNKNOWN(" << static_cast<int>(request.command) << ")";
            break;
    }
    std::cout << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::LIGHT_SERVICE_ID, body_controller::communication::LIGHT_INSTANCE_ID, light_service::SET_INDICATOR_STATE, payload_data);
}

void LightServiceClient::SetPositionLightState(const application::SetPositionLightStateReq& request) {
    std::cout << "[LightServiceClient] Setting position light state: " 
              << (request.command == application::PositionLightState::ON ? "ON" : "OFF") << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::LIGHT_SERVICE_ID, body_controller::communication::LIGHT_INSTANCE_ID, light_service::SET_POSITION_LIGHT_STATE, payload_data);
}

void LightServiceClient::OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
    SomeipClient::OnAvailability(service, instance, is_available);
    
    if (service == body_controller::communication::LIGHT_SERVICE_ID && instance == body_controller::communication::LIGHT_INSTANCE_ID && is_available) {
        std::cout << "[LightServiceClient] Light service is available, subscribing to events..." << std::endl;

        // 订阅灯光状态变化事件
        SubscribeEvent(body_controller::communication::LIGHT_SERVICE_ID, body_controller::communication::LIGHT_INSTANCE_ID,
                      light_events::ON_LIGHT_STATE_CHANGED, body_controller::communication::LIGHT_EVENTS_GROUP_ID);
    }
}

void LightServiceClient::OnMessage(const std::shared_ptr<vsomeip::message>& message) {
    if (!message) {
        return;
    }
    
    // 检查是否是灯光服务的消息
    if (message->get_service() != body_controller::communication::LIGHT_SERVICE_ID || message->get_instance() != body_controller::communication::LIGHT_INSTANCE_ID) {
        return;
    }
    
    auto message_type = message->get_message_type();
    auto method_id = message->get_method();
    
    if (message_type == vsomeip::message_type_e::MT_RESPONSE) {
        // 处理方法响应
        if (method_id == light_service::SET_HEADLIGHT_STATE) {
            HandleSetHeadlightStateResponse(message);
        } else if (method_id == light_service::SET_INDICATOR_STATE) {
            HandleSetIndicatorStateResponse(message);
        } else if (method_id == light_service::SET_POSITION_LIGHT_STATE) {
            HandleSetPositionLightStateResponse(message);
        }
    } else if (message_type == vsomeip::message_type_e::MT_NOTIFICATION) {
        // 处理事件通知
        if (method_id == light_events::ON_LIGHT_STATE_CHANGED) {
            HandleLightStateChangedEvent(message);
        }
    }
}

void LightServiceClient::HandleSetHeadlightStateResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[LightServiceClient] SetHeadlightState response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::SetHeadlightStateResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[LightServiceClient] SetHeadlightState response - New State: ";
        switch (response.newState) {
            case application::HeadlightState::OFF:
                std::cout << "OFF";
                break;
            case application::HeadlightState::LOW_BEAM:
                std::cout << "LOW_BEAM";
                break;
            case application::HeadlightState::HIGH_BEAM:
                std::cout << "HIGH_BEAM";
                break;
            default:
                std::cout << "UNKNOWN(" << static_cast<int>(response.newState) << ")";
                break;
        }
        std::cout << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (set_headlight_response_handler_) {
            set_headlight_response_handler_(response);
        }
    } else {
        std::cerr << "[LightServiceClient] Failed to deserialize SetHeadlightState response" << std::endl;
    }
}

void LightServiceClient::HandleSetIndicatorStateResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[LightServiceClient] SetIndicatorState response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::SetIndicatorStateResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[LightServiceClient] SetIndicatorState response - New State: ";
        switch (response.newState) {
            case application::IndicatorState::OFF:
                std::cout << "OFF";
                break;
            case application::IndicatorState::LEFT:
                std::cout << "LEFT";
                break;
            case application::IndicatorState::RIGHT:
                std::cout << "RIGHT";
                break;
            case application::IndicatorState::HAZARD:
                std::cout << "HAZARD";
                break;
            default:
                std::cout << "UNKNOWN(" << static_cast<int>(response.newState) << ")";
                break;
        }
        std::cout << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (set_indicator_response_handler_) {
            set_indicator_response_handler_(response);
        }
    } else {
        std::cerr << "[LightServiceClient] Failed to deserialize SetIndicatorState response" << std::endl;
    }
}

void LightServiceClient::HandleSetPositionLightStateResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[LightServiceClient] SetPositionLightState response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::SetPositionLightStateResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[LightServiceClient] SetPositionLightState response - New State: " 
                  << (response.newState == application::PositionLightState::ON ? "ON" : "OFF")
                  << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (set_position_light_response_handler_) {
            set_position_light_response_handler_(response);
        }
    } else {
        std::cerr << "[LightServiceClient] Failed to deserialize SetPositionLightState response" << std::endl;
    }
}

void LightServiceClient::HandleLightStateChangedEvent(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[LightServiceClient] LightStateChanged event has no payload" << std::endl;
        return;
    }
    
    // 反序列化事件数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::OnLightStateChangedData event_data;
    
    if (Serializer::Deserialize(payload_data, event_data)) {
        std::cout << "[LightServiceClient] LightStateChanged event - Light Type: ";
        switch (event_data.lightType) {
            case application::LightType::HEADLIGHT:
                std::cout << "HEADLIGHT";
                break;
            case application::LightType::INDICATOR:
                std::cout << "INDICATOR";
                break;
            case application::LightType::POSITION_LIGHT:
                std::cout << "POSITION_LIGHT";
                break;
            default:
                std::cout << "UNKNOWN(" << static_cast<int>(event_data.lightType) << ")";
                break;
        }
        std::cout << " New State: " << static_cast<int>(event_data.newState) << std::endl;
        
        // 调用用户回调
        if (light_state_changed_handler_) {
            light_state_changed_handler_(event_data);
        }
    } else {
        std::cerr << "[LightServiceClient] Failed to deserialize LightStateChanged event" << std::endl;
    }
}

} // namespace communication
} // namespace body_controller
