#include "communication/someip_client.h"
#include "communication/serialization.h"
#include "communication/someip_service_definitions.h"
#include <iostream>

namespace body_controller {
namespace communication {

// ============================================================================
// SeatServiceClient 实现
// ============================================================================

SeatServiceClient::SeatServiceClient(const std::string& app_name)
    : SomeipClient(app_name) {
    std::cout << "[SeatServiceClient] Created seat service client" << std::endl;
}

bool SeatServiceClient::Initialize() {
    if (!SomeipClient::Initialize()) {
        return false;
    }
    
    // 注册消息处理器（用于响应和事件）
    app_->register_message_handler(
        body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID, vsomeip::ANY_METHOD,
        std::bind(&SeatServiceClient::OnMessage, this, std::placeholders::_1)
    );

    // 注册可用性处理器
    app_->register_availability_handler(
        body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID,
        std::bind(&SeatServiceClient::OnAvailability, this,
                 std::placeholders::_1, std::placeholders::_2, std::placeholders::_3)
    );
    
    std::cout << "[SeatServiceClient] Seat service client initialized" << std::endl;

    // 请求座椅服务
    std::cout << "[SeatServiceClient] Requesting service 0x" << std::hex
              << body_controller::communication::SEAT_SERVICE_ID
              << ".0x" << body_controller::communication::SEAT_INSTANCE_ID << std::dec << std::endl;
    app_->request_service(body_controller::communication::SEAT_SERVICE_ID,
                          body_controller::communication::SEAT_INSTANCE_ID);
    return true;
}

void SeatServiceClient::AdjustSeat(const application::AdjustSeatReq& request) {
    std::cout << "[SeatServiceClient] Adjusting seat - Axis: ";
    switch (request.axis) {
        case application::SeatAxis::FORWARD_BACKWARD:
            std::cout << "FORWARD_BACKWARD";
            break;
        case application::SeatAxis::RECLINE:
            std::cout << "RECLINE";
            break;
        default:
            std::cout << "UNKNOWN(" << static_cast<int>(request.axis) << ")";
            break;
    }
    
    std::cout << " Direction: ";
    switch (request.direction) {
        case application::SeatDirection::POSITIVE:
            std::cout << "POSITIVE";
            break;
        case application::SeatDirection::NEGATIVE:
            std::cout << "NEGATIVE";
            break;
        case application::SeatDirection::STOP:
            std::cout << "STOP";
            break;
        default:
            std::cout << "UNKNOWN(" << static_cast<int>(request.direction) << ")";
            break;
    }
    std::cout << std::endl;
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID, seat_service::ADJUST_SEAT, payload_data);
}

void SeatServiceClient::RecallMemoryPosition(const application::RecallMemoryPositionReq& request) {
    std::cout << "[SeatServiceClient] Recalling memory position: " 
              << static_cast<int>(request.presetID) << std::endl;
    
    // 验证记忆位置ID范围
    if (request.presetID < 1 || request.presetID > 3) {
        std::cerr << "[SeatServiceClient] Invalid memory position ID: " 
                  << static_cast<int>(request.presetID) << " (valid range: 1-3)" << std::endl;
        return;
    }
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID, seat_service::RECALL_MEMORY_POSITION, payload_data);
}

void SeatServiceClient::SaveMemoryPosition(const application::SaveMemoryPositionReq& request) {
    std::cout << "[SeatServiceClient] Saving current position to memory slot: " 
              << static_cast<int>(request.presetID) << std::endl;
    
    // 验证记忆位置ID范围
    if (request.presetID < 1 || request.presetID > 3) {
        std::cerr << "[SeatServiceClient] Invalid memory position ID: " 
                  << static_cast<int>(request.presetID) << " (valid range: 1-3)" << std::endl;
        return;
    }
    
    // 序列化请求数据
    auto payload_data = Serializer::Serialize(request);
    
    // 发送请求
    SendRequest(body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID, seat_service::SAVE_MEMORY_POSITION, payload_data);
}

void SeatServiceClient::OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
    SomeipClient::OnAvailability(service, instance, is_available);
    
    if (service == body_controller::communication::SEAT_SERVICE_ID && instance == body_controller::communication::SEAT_INSTANCE_ID && is_available) {
        std::cout << "[SeatServiceClient] Seat service is available, subscribing to events..." << std::endl;

        // 订阅座椅位置变化事件
        SubscribeEvent(body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID,
                      seat_events::ON_SEAT_POSITION_CHANGED, body_controller::communication::SEAT_EVENTS_GROUP_ID);

        // 订阅记忆保存确认事件
        SubscribeEvent(body_controller::communication::SEAT_SERVICE_ID, body_controller::communication::SEAT_INSTANCE_ID,
                      seat_events::ON_MEMORY_SAVE_CONFIRM, body_controller::communication::SEAT_EVENTS_GROUP_ID);
    }
}

void SeatServiceClient::OnMessage(const std::shared_ptr<vsomeip::message>& message) {
    if (!message) {
        return;
    }
    
    // 检查是否是座椅服务的消息
    if (message->get_service() != body_controller::communication::SEAT_SERVICE_ID || message->get_instance() != body_controller::communication::SEAT_INSTANCE_ID) {
        return;
    }
    
    auto message_type = message->get_message_type();
    auto method_id = message->get_method();
    
    if (message_type == vsomeip::message_type_e::MT_RESPONSE) {
        // 处理方法响应
        if (method_id == seat_service::ADJUST_SEAT) {
            HandleAdjustSeatResponse(message);
        } else if (method_id == seat_service::RECALL_MEMORY_POSITION) {
            HandleRecallMemoryPositionResponse(message);
        } else if (method_id == seat_service::SAVE_MEMORY_POSITION) {
            HandleSaveMemoryPositionResponse(message);
        }
    } else if (message_type == vsomeip::message_type_e::MT_NOTIFICATION) {
        // 处理事件通知
        if (method_id == seat_events::ON_SEAT_POSITION_CHANGED) {
            HandleSeatPositionChangedEvent(message);
        } else if (method_id == seat_events::ON_MEMORY_SAVE_CONFIRM) {
            HandleMemorySaveConfirmEvent(message);
        }
    }
}

void SeatServiceClient::HandleAdjustSeatResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[SeatServiceClient] AdjustSeat response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::AdjustSeatResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[SeatServiceClient] AdjustSeat response - Axis: ";
        switch (response.axis) {
            case application::SeatAxis::FORWARD_BACKWARD:
                std::cout << "FORWARD_BACKWARD";
                break;
            case application::SeatAxis::RECLINE:
                std::cout << "RECLINE";
                break;
            default:
                std::cout << "UNKNOWN(" << static_cast<int>(response.axis) << ")";
                break;
        }
        std::cout << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (adjust_seat_response_handler_) {
            adjust_seat_response_handler_(response);
        }
    } else {
        std::cerr << "[SeatServiceClient] Failed to deserialize AdjustSeat response" << std::endl;
    }
}

void SeatServiceClient::HandleRecallMemoryPositionResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[SeatServiceClient] RecallMemoryPosition response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::RecallMemoryPositionResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[SeatServiceClient] RecallMemoryPosition response - Preset ID: " 
                  << static_cast<int>(response.presetID)
                  << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (recall_memory_response_handler_) {
            recall_memory_response_handler_(response);
        }
    } else {
        std::cerr << "[SeatServiceClient] Failed to deserialize RecallMemoryPosition response" << std::endl;
    }
}

void SeatServiceClient::HandleSaveMemoryPositionResponse(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[SeatServiceClient] SaveMemoryPosition response has no payload" << std::endl;
        return;
    }
    
    // 反序列化响应数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::SaveMemoryPositionResp response;
    
    if (Serializer::Deserialize(payload_data, response)) {
        std::cout << "[SeatServiceClient] SaveMemoryPosition response - Preset ID: " 
                  << static_cast<int>(response.presetID)
                  << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (save_memory_response_handler_) {
            save_memory_response_handler_(response);
        }
    } else {
        std::cerr << "[SeatServiceClient] Failed to deserialize SaveMemoryPosition response" << std::endl;
    }
}

void SeatServiceClient::HandleSeatPositionChangedEvent(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[SeatServiceClient] SeatPositionChanged event has no payload" << std::endl;
        return;
    }
    
    // 反序列化事件数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::OnSeatPositionChangedData event_data;
    
    if (Serializer::Deserialize(payload_data, event_data)) {
        std::cout << "[SeatServiceClient] SeatPositionChanged event - Axis: ";
        switch (event_data.axis) {
            case application::SeatAxis::FORWARD_BACKWARD:
                std::cout << "FORWARD_BACKWARD";
                break;
            case application::SeatAxis::RECLINE:
                std::cout << "RECLINE";
                break;
            default:
                std::cout << "UNKNOWN(" << static_cast<int>(event_data.axis) << ")";
                break;
        }
        std::cout << " New Position: " << static_cast<int>(event_data.newPosition) << "%" << std::endl;
        
        // 调用用户回调
        if (seat_position_changed_handler_) {
            seat_position_changed_handler_(event_data);
        }
    } else {
        std::cerr << "[SeatServiceClient] Failed to deserialize SeatPositionChanged event" << std::endl;
    }
}

void SeatServiceClient::HandleMemorySaveConfirmEvent(const std::shared_ptr<vsomeip::message>& message) {
    auto payload = message->get_payload();
    if (!payload) {
        std::cerr << "[SeatServiceClient] MemorySaveConfirm event has no payload" << std::endl;
        return;
    }
    
    // 反序列化事件数据
    std::vector<uint8_t> payload_data(payload->get_data(), payload->get_data() + payload->get_length());
    application::OnMemorySaveConfirmData event_data;
    
    if (Serializer::Deserialize(payload_data, event_data)) {
        std::cout << "[SeatServiceClient] MemorySaveConfirm event - Preset ID: " 
                  << static_cast<int>(event_data.presetID)
                  << " Save Result: " << (event_data.saveResult == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
        
        // 调用用户回调
        if (memory_save_confirm_handler_) {
            memory_save_confirm_handler_(event_data);
        }
    } else {
        std::cerr << "[SeatServiceClient] Failed to deserialize MemorySaveConfirm event" << std::endl;
    }
}

} // namespace communication
} // namespace body_controller
