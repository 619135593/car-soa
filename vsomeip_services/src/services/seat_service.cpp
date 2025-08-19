#include "services/seat_service.h"
#include "common/serializer.h"
#include <iostream>
#include <thread>
#include <random>
#include <set>

namespace body_controller {
namespace services {

SeatService::SeatService(std::shared_ptr<vsomeip::application> app, 
                        std::shared_ptr<HardwareSimulator> simulator)
    : app_(app)
    , running_(false)
    , hardware_simulator_(simulator)
{
    // 初始化当前位置
    for (int i = 0; i < 4; ++i) {
        current_positions_[i] = {0, 45}; // 默认位置：前后0，靠背45度
        
        // 初始化记忆位置
        for (int j = 0; j < 3; ++j) {
            memory_positions_[i][j] = {0, 45}; // 默认记忆位置
        }
    }
    
    std::cout << "[SeatService] Seat service created" << std::endl;
}

SeatService::~SeatService() {
    Stop();
    std::cout << "[SeatService] Seat service destroyed" << std::endl;
}

bool SeatService::Initialize() {
    if (!app_) {
        std::cerr << "[SeatService] VSOMEIP application not available" << std::endl;
        return false;
    }
    
    try {
        // 注册服务
        app_->offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 注册方法处理器
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, ADJUST_SEAT_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleAdjustSeatRequest(request);
            });
            
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, RECALL_MEMORY_POSITION_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleRecallMemoryPositionRequest(request);
            });
            
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, SAVE_MEMORY_POSITION_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleSaveMemoryPositionRequest(request);
            });
        
        // 注册事件
        std::set<vsomeip::eventgroup_t> event_groups;
        event_groups.insert(EVENT_GROUP);
        
        app_->offer_event(SERVICE_ID, INSTANCE_ID, SEAT_POSITION_CHANGED_EVENT, 
                         event_groups, vsomeip::event_type_e::ET_EVENT, 
                         std::chrono::milliseconds::zero(),
                         false, true, nullptr, vsomeip::reliability_type_e::RT_RELIABLE);
                         
        app_->offer_event(SERVICE_ID, INSTANCE_ID, MEMORY_SAVE_CONFIRM_EVENT, 
                         event_groups, vsomeip::event_type_e::ET_EVENT, 
                         std::chrono::milliseconds::zero(),
                         false, true, nullptr, vsomeip::reliability_type_e::RT_RELIABLE);
        
        // 设置硬件模拟器回调
        if (hardware_simulator_) {
            hardware_simulator_->SetSeatPositionEventCallback(
                [this](const application::OnSeatPositionChangedData& event) {
                    OnSeatPositionChanged(event);
                });
        }
        
        std::cout << "[SeatService] Seat service initialized successfully" << std::endl;
        std::cout << "[SeatService] Service ID: 0x" << std::hex << SERVICE_ID << std::dec << std::endl;
        std::cout << "[SeatService] Instance ID: 0x" << std::hex << INSTANCE_ID << std::dec << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[SeatService] Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

bool SeatService::Start() {
    if (running_) {
        std::cout << "[SeatService] Already running" << std::endl;
        return true;
    }
    
    running_ = true;
    std::cout << "[SeatService] Seat service started" << std::endl;
    return true;
}

void SeatService::Stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (app_) {
        // 停止提供服务
        app_->stop_offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 取消注册处理器
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, ADJUST_SEAT_METHOD);
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, RECALL_MEMORY_POSITION_METHOD);
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, SAVE_MEMORY_POSITION_METHOD);
    }
    
    std::cout << "[SeatService] Seat service stopped" << std::endl;
}

void SeatService::HandleAdjustSeatRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[SeatService] Received AdjustSeat request" << std::endl;
    
    try {
        // 反序列化请求数据
        auto payload = request->get_payload();
        std::vector<uint8_t> data(payload->get_data(), payload->get_data() + payload->get_length());
        
        application::AdjustSeatReq req;
        if (!Serializer::Deserialize(data, req)) {
            std::cerr << "[SeatService] Failed to deserialize AdjustSeat request" << std::endl;
            SendErrorResponse(request, 1);
            return;
        }
        
        std::string axis_str = (req.axis == application::SeatAxis::FORWARD_BACKWARD) ? "FORWARD_BACKWARD" : "RECLINE";
        std::string direction_str;
        switch (req.direction) {
            case application::SeatDirection::POSITIVE: direction_str = "POSITIVE"; break;
            case application::SeatDirection::NEGATIVE: direction_str = "NEGATIVE"; break;
            case application::SeatDirection::STOP: direction_str = "STOP"; break;
        }
        
        std::cout << "[SeatService] AdjustSeat - Axis: " << axis_str
                  << " Direction: " << direction_str << std::endl;

        // 模拟座椅调节操作（使用默认座椅ID）
        application::Result result = SimulateAdjustOperation(application::Position::FRONT_LEFT, req.axis, req.direction);
        
        // 创建响应
        auto response_data = (result == application::Result::SUCCESS)
                           ? Serializer::SerializeSuccessResponse()
                           : Serializer::SerializeFailResponse();
        SendResponse(request, response_data);
        
        // 如果操作成功，触发硬件事件
        if (result == application::Result::SUCCESS && hardware_simulator_ && 
            req.direction != application::SeatDirection::STOP) {
            
            std::thread([this, req]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(500)); // 座椅调节需要时间
                
                // 计算新位置（使用默认座椅索引）
                int seat_index = 0; // 简化实现，使用第一个座椅
                int new_position;
                
                if (req.axis == application::SeatAxis::FORWARD_BACKWARD) {
                    new_position = current_positions_[seat_index].forward_backward_position;
                    if (req.direction == application::SeatDirection::POSITIVE) {
                        new_position = std::min(100, new_position + 10);
                    } else {
                        new_position = std::max(-100, new_position - 10);
                    }
                    current_positions_[seat_index].forward_backward_position = new_position;
                } else {
                    new_position = current_positions_[seat_index].recline_position;
                    if (req.direction == application::SeatDirection::POSITIVE) {
                        new_position = std::min(90, new_position + 5);
                    } else {
                        new_position = std::max(0, new_position - 5);
                    }
                    current_positions_[seat_index].recline_position = new_position;
                }
                
                // 触发位置变化事件
                application::OnSeatPositionChangedData event_data;
                event_data.axis = req.axis;
                event_data.newPosition = new_position;
                
                // 直接调用硬件模拟器的回调
                OnSeatPositionChanged(event_data);
            }).detach();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[SeatService] Error handling AdjustSeat request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void SeatService::HandleRecallMemoryPositionRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[SeatService] Received RecallMemoryPosition request" << std::endl;
    
    try {
        // 创建成功响应（简化实现）
        auto response_data = Serializer::SerializeSuccessResponse();
        SendResponse(request, response_data);
        
        std::cout << "[SeatService] RecallMemoryPosition completed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[SeatService] Error handling RecallMemoryPosition request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void SeatService::HandleSaveMemoryPositionRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[SeatService] Received SaveMemoryPosition request" << std::endl;
    
    try {
        // 创建成功响应
        auto response_data = Serializer::SerializeSuccessResponse();
        SendResponse(request, response_data);
        
        // 触发记忆保存确认事件
        if (hardware_simulator_) {
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                
                application::OnMemorySaveConfirmData event_data;
                event_data.presetID = 1; // 简化实现
                event_data.saveResult = application::Result::SUCCESS;
                
                SendMemorySaveConfirmEvent(event_data);
            }).detach();
        }
        
        std::cout << "[SeatService] SaveMemoryPosition completed" << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[SeatService] Error handling SaveMemoryPosition request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void SeatService::SendResponse(const std::shared_ptr<vsomeip::message>& request,
                              const std::vector<uint8_t>& payload) {
    if (!app_) return;

    auto response = vsomeip::runtime::get()->create_response(request);
    auto response_payload = vsomeip::runtime::get()->create_payload();

    response_payload->set_data(payload.data(), static_cast<uint32_t>(payload.size()));
    response->set_payload(response_payload);

    app_->send(response);
}

void SeatService::SendErrorResponse(const std::shared_ptr<vsomeip::message>& request,
                                   uint8_t error_code) {
    auto error_data = Serializer::SerializeFailResponse();
    error_data.insert(error_data.begin(), error_code);

    SendResponse(request, error_data);

    std::cout << "[SeatService] Sent error response with code: " << static_cast<int>(error_code) << std::endl;
}

void SeatService::OnSeatPositionChanged(const application::OnSeatPositionChangedData& event_data) {
    std::cout << "[SeatService] Hardware event - Seat position changed: Axis "
              << static_cast<int>(event_data.axis) << " -> "
              << event_data.newPosition << std::endl;

    // 更新内部状态（使用默认座椅索引）
    int seat_index = 0;
    if (seat_index >= 0 && seat_index < 4) {
        if (event_data.axis == application::SeatAxis::FORWARD_BACKWARD) {
            current_positions_[seat_index].forward_backward_position = event_data.newPosition;
        } else {
            current_positions_[seat_index].recline_position = event_data.newPosition;
        }
    }

    // 发送事件到客户端
    SendSeatPositionChangedEvent(event_data);
}

void SeatService::SendSeatPositionChangedEvent(const application::OnSeatPositionChangedData& event_data) {
    if (!app_) return;

    auto event_payload = Serializer::Serialize(event_data);
    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(event_payload.data(), static_cast<uint32_t>(event_payload.size()));

    app_->notify(SERVICE_ID, INSTANCE_ID, SEAT_POSITION_CHANGED_EVENT, payload);

    std::cout << "[SeatService] Sent seat position changed event to clients" << std::endl;
}

void SeatService::SendMemorySaveConfirmEvent(const application::OnMemorySaveConfirmData& event_data) {
    if (!app_) return;

    // 简化的记忆保存确认事件序列化
    std::vector<uint8_t> event_payload;
    event_payload.push_back(event_data.presetID);
    event_payload.push_back(static_cast<uint8_t>(event_data.saveResult));

    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(event_payload.data(), static_cast<uint32_t>(event_payload.size()));

    app_->notify(SERVICE_ID, INSTANCE_ID, MEMORY_SAVE_CONFIRM_EVENT, payload);

    std::cout << "[SeatService] Sent memory save confirm event to clients" << std::endl;
}

application::Result SeatService::SimulateAdjustOperation(application::Position seat_id,
                                                        application::SeatAxis axis,
                                                        application::SeatDirection direction) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int seat_index = static_cast<int>(seat_id);
    if (seat_index < 0 || seat_index >= 4) {
        return application::Result::FAIL;
    }

    // 模拟95%成功率
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    if ((gen() % 100) < 95) {
        return application::Result::SUCCESS;
    } else {
        std::cout << "[SeatService] Simulated adjust operation failure" << std::endl;
        return application::Result::FAIL;
    }
}

application::Result SeatService::SimulateRecallMemoryOperation(application::Position seat_id,
                                                             uint8_t memory_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    int seat_index = static_cast<int>(seat_id);
    if (seat_index < 0 || seat_index >= 4 || memory_id < 1 || memory_id > 3) {
        return application::Result::FAIL;
    }

    // 模拟95%成功率
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    return ((gen() % 100) < 95) ? application::Result::SUCCESS : application::Result::FAIL;
}

application::Result SeatService::SimulateSaveMemoryOperation(application::Position seat_id,
                                                           uint8_t memory_id) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    int seat_index = static_cast<int>(seat_id);
    if (seat_index < 0 || seat_index >= 4 || memory_id < 1 || memory_id > 3) {
        return application::Result::FAIL;
    }

    // 保存当前位置到记忆位置
    memory_positions_[seat_index][memory_id - 1] = current_positions_[seat_index];

    return application::Result::SUCCESS;
}

} // namespace services
} // namespace body_controller
