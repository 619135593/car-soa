#include "communication/someip_client.h"
#include "communication/serialization.h"
#include "communication/someip_service_definitions.h"
#include <iostream>

namespace body_controller {
namespace communication {

// ============================================================================
// DoorServiceClient 实现
// ============================================================================

DoorServiceClient::DoorServiceClient(const std::string& app_name)
    : SomeipClient(app_name) {
    std::cout << "[DoorServiceClient] Created door service client" << std::endl;
}

bool DoorServiceClient::Initialize() {
    if (!SomeipClient::Initialize()) {
        return false;
    }
    
    // 注册消息处理器（用于响应和事件）
    app_->register_message_handler(
        body_controller::communication::DOOR_SERVICE_ID, body_controller::communication::DOOR_INSTANCE_ID, vsomeip::ANY_METHOD,
        std::bind(&DoorServiceClient::OnMessage, this, std::placeholders::_1)
    );

    // 注册可用性处理器
    app_->register_availability_handler(
        body_controller::communication::DOOR_SERVICE_ID, body_controller::communication::DOOR_INSTANCE_ID,
        std::bind(&DoorServiceClient::OnAvailability, this,
                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    
    std::cout << "[DoorServiceClient] Door service client initialized" << std::endl;

    // 请求车门服务
    std::cout << "[DoorServiceClient] Requesting service 0x" << std::hex
              << body_controller::communication::DOOR_SERVICE_ID
              << ".0x" << body_controller::communication::DOOR_INSTANCE_ID << std::dec << std::endl;
    app_->request_service(body_controller::communication::DOOR_SERVICE_ID,
                          body_controller::communication::DOOR_INSTANCE_ID);
    return true;
}

void DoorServiceClient::SetLockState(const application::SetLockStateReq& request) {
    std::cout << "[DoorServiceClient] Setting lock state for door: " 
              << static_cast<int>(request.doorID) 
              << " Command: " << static_cast<int>(request.command) << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::DOOR_SERVICE_ID, body_controller::communication::DOOR_INSTANCE_ID, door_service::SET_LOCK_STATE, payload_data);
}

void DoorServiceClient::GetLockState(const application::GetLockStateReq& request) {
    std::cout << "[DoorServiceClient] Getting lock state for door: " 
              << static_cast<int>(request.doorID) << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::DOOR_SERVICE_ID, body_controller::communication::DOOR_INSTANCE_ID, door_service::GET_LOCK_STATE, payload_data);
}

void DoorServiceClient::OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
    SomeipClient::OnAvailability(service, instance, is_available);
    
    if (service == body_controller::communication::DOOR_SERVICE_ID && instance == body_controller::communication::DOOR_INSTANCE_ID && is_available) {
        std::cout << "[DoorServiceClient] Door service is available, subscribing to events..." << std::endl;
        
        // 订阅锁定状态变化事件
        SubscribeEvent(body_controller::communication::DOOR_SERVICE_ID, body_controller::communication::DOOR_INSTANCE_ID,
                      door_events::ON_LOCK_STATE_CHANGED, body_controller::communication::DOOR_EVENTS_GROUP_ID);

        // 订阅车门状态变化事件
        SubscribeEvent(body_controller::communication::DOOR_SERVICE_ID, body_controller::communication::DOOR_INSTANCE_ID,
                      door_events::ON_DOOR_STATE_CHANGED, body_controller::communication::DOOR_EVENTS_GROUP_ID);
    }
}

void DoorServiceClient::OnMessage(const std::shared_ptr<vsomeip::message>& message) {
    if (!message) {
        return;
    }
    
    // 检查是否是车门服务的消息
    if (message->get_service() != body_controller::communication::DOOR_SERVICE_ID || message->get_instance() != body_controller::communication::DOOR_INSTANCE_ID) {
        return;
    }
    
    auto message_type = message->get_message_type();
    auto method_id = message->get_method();
    
    if (message_type == vsomeip::message_type_e::MT_RESPONSE) {
        // 处理方法响应
        if (method_id == door_service::SET_LOCK_STATE) {
            HandleSetLockStateResponse(message);
        } else if (method_id == door_service::GET_LOCK_STATE) {
            HandleGetLockStateResponse(message);
        }
    } else if (message_type == vsomeip::message_type_e::MT_NOTIFICATION) {
        // 处理事件通知
        if (method_id == door_events::ON_LOCK_STATE_CHANGED) {
            HandleLockStateChangedEvent(message);
        } else if (method_id == door_events::ON_DOOR_STATE_CHANGED) {
            HandleDoorStateChangedEvent(message);
        }
    }
}

void DoorServiceClient::HandleSetLockStateResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[DoorServiceClient] SetLockState response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::SetLockStateResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[DoorServiceClient] SetLockState response - Door: " 
                  << static_cast<int>(response.doorID)
                  << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (set_lock_response_handler_) {
            set_lock_response_handler_(response);
        }
    } else {
        std::cerr << "[DoorServiceClient] Failed to deserialize SetLockState response" << std::endl;
    }
}

void DoorServiceClient::HandleGetLockStateResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[DoorServiceClient] GetLockState response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::GetLockStateResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[DoorServiceClient] GetLockState response - Door: " 
                  << static_cast<int>(response.doorID)
                  << " State: " << (response.lockState == application::LockState::LOCKED ? "LOCKED" : "UNLOCKED") << std::endl;
        
        // 调用用户回调
        if (get_lock_response_handler_) {
            get_lock_response_handler_(response);
        }
    } else {
        std::cerr << "[DoorServiceClient] Failed to deserialize GetLockState response" << std::endl;
    }
}

void DoorServiceClient::HandleLockStateChangedEvent(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[DoorServiceClient] LockStateChanged event has no payload" << std::endl;
        return;
    }
    
    // 反序列化事件数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::OnLockStateChangedData event_data;
    
    if (Serializer::Deserialize(payload_data, event_data)) {
        std::cout << "[DoorServiceClient] LockStateChanged event - Door: " 
                  << static_cast<int>(event_data.doorID)
                  << " New State: " << (event_data.newLockState == application::LockState::LOCKED ? "LOCKED" : "UNLOCKED") << std::endl;
        
        // 调用用户回调
        if (lock_state_changed_handler_) {
            lock_state_changed_handler_(event_data);
        }
    } else {
        std::cerr << "[DoorServiceClient] Failed to deserialize LockStateChanged event" << std::endl;
    }
}

void DoorServiceClient::HandleDoorStateChangedEvent(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[DoorServiceClient] DoorStateChanged event has no payload" << std::endl;
        return;
    }
    
    // 反序列化事件数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::OnDoorStateChangedData event_data;
    
    if (Serializer::Deserialize(payload_data, event_data)) {
        std::cout << "[DoorServiceClient] DoorStateChanged event - Door: " 
                  << static_cast<int>(event_data.doorID)
                  << " New State: " << (event_data.newDoorState == application::DoorState::CLOSED ? "CLOSED" : "OPEN") << std::endl;
        
        // 调用用户回调
        if (door_state_changed_handler_) {
            door_state_changed_handler_(event_data);
        }
    } else {
        std::cerr << "[DoorServiceClient] Failed to deserialize DoorStateChanged event" << std::endl;
    }
}

} // namespace communication
} // namespace body_controller
