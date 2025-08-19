#include "services/door_service.h"
#include "common/serializer.h"
#include <iostream>
#include <thread>
#include <random>
#include <set>

namespace body_controller {
namespace services {

DoorService::DoorService(std::shared_ptr<vsomeip::application> app, 
                        std::shared_ptr<HardwareSimulator> simulator)
    : app_(app)
    , running_(false)
    , hardware_simulator_(simulator)
{
    // 初始化当前状态
    for (int i = 0; i < 4; ++i) {
        current_lock_states_[i] = application::LockState::UNLOCKED;
        current_door_states_[i] = application::DoorState::CLOSED;
    }

    // 初始化随机数生成器
    random_generator_.seed(std::chrono::steady_clock::now().time_since_epoch().count());
    
    std::cout << "[DoorService] Door service created" << std::endl;
}

DoorService::~DoorService() {
    Stop();
    std::cout << "[DoorService] Door service destroyed" << std::endl;
}

bool DoorService::Initialize() {
    if (!app_) {
        std::cerr << "[DoorService] VSOMEIP application not available" << std::endl;
        return false;
    }
    
    try {
        // 注册服务
        app_->offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 注册方法处理器
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, SET_LOCK_STATE_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleSetLockStateRequest(request);
            });
            
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, GET_LOCK_STATE_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleGetLockStateRequest(request);
            });
        
        // 注册事件
        std::set<vsomeip::eventgroup_t> event_groups;
        event_groups.insert(EVENT_GROUP);
        
        app_->offer_event(SERVICE_ID, INSTANCE_ID, LOCK_STATE_CHANGED_EVENT, 
                         event_groups, vsomeip::event_type_e::ET_EVENT, 
                         std::chrono::milliseconds::zero(),
                         false, true, nullptr, vsomeip::reliability_type_e::RT_RELIABLE);
                         
        app_->offer_event(SERVICE_ID, INSTANCE_ID, DOOR_STATE_CHANGED_EVENT, 
                         event_groups, vsomeip::event_type_e::ET_EVENT, 
                         std::chrono::milliseconds::zero(),
                         false, true, nullptr, vsomeip::reliability_type_e::RT_RELIABLE);
        
        // 设置硬件模拟器回调
        if (hardware_simulator_) {
            hardware_simulator_->SetDoorLockEventCallback(
                [this](const application::OnLockStateChangedData& event) {
                    OnDoorLockStateChanged(event);
                });
                
            hardware_simulator_->SetDoorStateEventCallback(
                [this](const application::OnDoorStateChangedData& event) {
                    OnDoorStateChanged(event);
                });
        }
        
        std::cout << "[DoorService] Door service initialized successfully" << std::endl;
        std::cout << "[DoorService] Service ID: 0x" << std::hex << SERVICE_ID << std::dec << std::endl;
        std::cout << "[DoorService] Instance ID: 0x" << std::hex << INSTANCE_ID << std::dec << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[DoorService] Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

bool DoorService::Start() {
    if (running_) {
        std::cout << "[DoorService] Already running" << std::endl;
        return true;
    }
    
    running_ = true;
    std::cout << "[DoorService] Door service started" << std::endl;
    return true;
}

void DoorService::Stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (app_) {
        // 停止提供服务
        app_->stop_offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 取消注册处理器
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, SET_LOCK_STATE_METHOD);
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, GET_LOCK_STATE_METHOD);
    }
    
    std::cout << "[DoorService] Door service stopped" << std::endl;
}

void DoorService::HandleSetLockStateRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[DoorService] Received SetLockState request" << std::endl;
    
    try {
        // 反序列化请求数据
        auto payload = request->get_payload();
        std::vector<uint8_t> data(payload->get_data(), payload->get_data() + payload->get_length());
        
        application::SetLockStateReq req;
        if (!Serializer::Deserialize(data, req)) {
            std::cerr << "[DoorService] Failed to deserialize SetLockState request" << std::endl;
            SendErrorResponse(request, 1); // 错误码1：反序列化失败
            return;
        }
        
        std::cout << "[DoorService] SetLockState - Door: " << static_cast<int>(req.doorID) 
                  << " Command: " << (req.command == application::LockCommand::LOCK ? "LOCK" : "UNLOCK") << std::endl;
        
        // 模拟锁定操作
        application::Result result = SimulateLockOperation(req.doorID, req.command);
        
        // 创建响应
        application::SetLockStateResp response;
        response.doorID = req.doorID;
        response.result = result;
        
        // 序列化并发送响应
        auto response_data = Serializer::Serialize(response);
        SendResponse(request, response_data);
        
        // 如果操作成功，触发硬件事件
        if (result == application::Result::SUCCESS && hardware_simulator_) {
            application::LockState new_state = (req.command == application::LockCommand::LOCK) 
                                             ? application::LockState::LOCKED 
                                             : application::LockState::UNLOCKED;
            
            // 延迟触发事件，模拟硬件响应时间
            std::thread([this, req, new_state]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                hardware_simulator_->TriggerDoorLockEvent(req.doorID, new_state);
            }).detach();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[DoorService] Error handling SetLockState request: " << e.what() << std::endl;
        SendErrorResponse(request, 2); // 错误码2：处理异常
    }
}

void DoorService::HandleGetLockStateRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[DoorService] Received GetLockState request" << std::endl;
    
    try {
        // 反序列化请求数据
        auto payload = request->get_payload();
        std::vector<uint8_t> data(payload->get_data(), payload->get_data() + payload->get_length());
        
        application::GetLockStateReq req;
        if (!Serializer::Deserialize(data, req)) {
            std::cerr << "[DoorService] Failed to deserialize GetLockState request" << std::endl;
            SendErrorResponse(request, 1);
            return;
        }
        
        std::cout << "[DoorService] GetLockState - Door: " << static_cast<int>(req.doorID) << std::endl;
        
        // 获取当前锁定状态
        application::LockState current_state = GetCurrentLockState(req.doorID);
        
        // 创建响应
        application::GetLockStateResp response;
        response.doorID = req.doorID;
        response.lockState = current_state;
        
        // 序列化并发送响应
        auto response_data = Serializer::Serialize(response);
        SendResponse(request, response_data);
        
        std::cout << "[DoorService] GetLockState response - Door: " << static_cast<int>(req.doorID) 
                  << " State: " << (current_state == application::LockState::LOCKED ? "LOCKED" : "UNLOCKED") << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "[DoorService] Error handling GetLockState request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void DoorService::SendResponse(const std::shared_ptr<vsomeip::message>& request,
                              const std::vector<uint8_t>& payload) {
    if (!app_) return;
    
    auto response = vsomeip::runtime::get()->create_response(request);
    auto response_payload = vsomeip::runtime::get()->create_payload();
    
    response_payload->set_data(payload.data(), static_cast<uint32_t>(payload.size()));
    response->set_payload(response_payload);
    
    app_->send(response);
}

void DoorService::SendErrorResponse(const std::shared_ptr<vsomeip::message>& request,
                                   uint8_t error_code) {
    auto error_data = Serializer::SerializeFailResponse();
    // 在错误数据前添加错误码
    error_data.insert(error_data.begin(), error_code);

    SendResponse(request, error_data);

    std::cout << "[DoorService] Sent error response with code: " << static_cast<int>(error_code) << std::endl;
}

void DoorService::OnDoorLockStateChanged(const application::OnLockStateChangedData& event_data) {
    std::cout << "[DoorService] Hardware event - Door lock state changed: Door " 
              << static_cast<int>(event_data.doorID) << " -> " 
              << (event_data.newLockState == application::LockState::LOCKED ? "LOCKED" : "UNLOCKED") << std::endl;
    
    // 更新内部状态
    current_lock_states_[static_cast<int>(event_data.doorID)] = event_data.newLockState;
    
    // 发送事件到客户端
    SendLockStateChangedEvent(event_data);
}

void DoorService::OnDoorStateChanged(const application::OnDoorStateChangedData& event_data) {
    std::cout << "[DoorService] Hardware event - Door state changed: Door " 
              << static_cast<int>(event_data.doorID) << " -> " 
              << (event_data.newDoorState == application::DoorState::OPEN ? "OPEN" : "CLOSED") << std::endl;
    
    // 更新内部状态
    current_door_states_[static_cast<int>(event_data.doorID)] = event_data.newDoorState;
    
    // 发送事件到客户端
    SendDoorStateChangedEvent(event_data);
}

void DoorService::SendLockStateChangedEvent(const application::OnLockStateChangedData& event_data) {
    if (!app_) return;
    
    auto event_payload = Serializer::Serialize(event_data);
    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(event_payload.data(), static_cast<uint32_t>(event_payload.size()));
    
    app_->notify(SERVICE_ID, INSTANCE_ID, LOCK_STATE_CHANGED_EVENT, payload);
    
    std::cout << "[DoorService] Sent lock state changed event to clients" << std::endl;
}

void DoorService::SendDoorStateChangedEvent(const application::OnDoorStateChangedData& event_data) {
    if (!app_) return;
    
    auto event_payload = Serializer::Serialize(event_data);
    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(event_payload.data(), static_cast<uint32_t>(event_payload.size()));
    
    app_->notify(SERVICE_ID, INSTANCE_ID, DOOR_STATE_CHANGED_EVENT, payload);
    
    std::cout << "[DoorService] Sent door state changed event to clients" << std::endl;
}

application::Result DoorService::SimulateLockOperation(application::Position door_id, 
                                                      application::LockCommand command) {
    // 模拟操作延迟
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // 简单的成功模拟（实际硬件可能会有失败情况）
    int door_index = static_cast<int>(door_id);
    if (door_index < 0 || door_index >= 4) {
        return application::Result::FAIL;
    }
    
    // 模拟操作成功率（95%成功率）
    if ((random_generator_() % 100) < 95) {
        return application::Result::SUCCESS;
    } else {
        std::cout << "[DoorService] Simulated lock operation failure" << std::endl;
        return application::Result::FAIL;
    }
}

application::LockState DoorService::GetCurrentLockState(application::Position door_id) const {
    int door_index = static_cast<int>(door_id);
    if (door_index >= 0 && door_index < 4) {
        return current_lock_states_[door_index];
    }
    return application::LockState::UNLOCKED;
}

} // namespace services
} // namespace body_controller
