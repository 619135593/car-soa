#include "services/light_service.h"
#include "common/serializer.h"
#include <iostream>
#include <thread>
#include <random>
#include <set>

namespace body_controller {
namespace services {

LightService::LightService(std::shared_ptr<vsomeip::application> app, 
                          std::shared_ptr<HardwareSimulator> simulator)
    : app_(app)
    , running_(false)
    , hardware_simulator_(simulator)
    , current_headlight_state_(application::HeadlightState::OFF)
    , current_indicator_state_(application::IndicatorState::OFF)
    , current_position_light_state_(application::PositionLightState::OFF)
{
    std::cout << "[LightService] Light service created" << std::endl;
}

LightService::~LightService() {
    Stop();
    std::cout << "[LightService] Light service destroyed" << std::endl;
}

bool LightService::Initialize() {
    if (!app_) {
        std::cerr << "[LightService] VSOMEIP application not available" << std::endl;
        return false;
    }
    
    try {
        // 注册服务
        app_->offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 注册方法处理器
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, SET_HEADLIGHT_STATE_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleSetHeadlightStateRequest(request);
            });
            
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, SET_INDICATOR_STATE_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleSetIndicatorStateRequest(request);
            });
            
        app_->register_message_handler(SERVICE_ID, INSTANCE_ID, SET_POSITION_LIGHT_STATE_METHOD,
            [this](const std::shared_ptr<vsomeip::message>& request) {
                HandleSetPositionLightStateRequest(request);
            });
        
        // 注册事件
        std::set<vsomeip::eventgroup_t> event_groups;
        event_groups.insert(EVENT_GROUP);
        
        app_->offer_event(SERVICE_ID, INSTANCE_ID, LIGHT_STATE_CHANGED_EVENT, 
                         event_groups, vsomeip::event_type_e::ET_EVENT, 
                         std::chrono::milliseconds::zero(),
                         false, true, nullptr, vsomeip::reliability_type_e::RT_RELIABLE);
        
        // 设置硬件模拟器回调
        if (hardware_simulator_) {
            hardware_simulator_->SetLightStateEventCallback(
                [this](const application::OnLightStateChangedData& event) {
                    OnLightStateChanged(event);
                });
        }
        
        std::cout << "[LightService] Light service initialized successfully" << std::endl;
        std::cout << "[LightService] Service ID: 0x" << std::hex << SERVICE_ID << std::dec << std::endl;
        std::cout << "[LightService] Instance ID: 0x" << std::hex << INSTANCE_ID << std::dec << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[LightService] Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

bool LightService::Start() {
    if (running_) {
        std::cout << "[LightService] Already running" << std::endl;
        return true;
    }
    
    running_ = true;
    std::cout << "[LightService] Light service started" << std::endl;
    return true;
}

void LightService::Stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (app_) {
        // 停止提供服务
        app_->stop_offer_service(SERVICE_ID, INSTANCE_ID, MAJOR_VERSION, MINOR_VERSION);
        
        // 取消注册处理器
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, SET_HEADLIGHT_STATE_METHOD);
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, SET_INDICATOR_STATE_METHOD);
        app_->unregister_message_handler(SERVICE_ID, INSTANCE_ID, SET_POSITION_LIGHT_STATE_METHOD);
    }
    
    std::cout << "[LightService] Light service stopped" << std::endl;
}

void LightService::HandleSetHeadlightStateRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[LightService] Received SetHeadlightState request" << std::endl;
    
    try {
        // 反序列化请求数据
        auto payload = request->get_payload();
        std::vector<uint8_t> data(payload->get_data(), payload->get_data() + payload->get_length());
        
        application::SetHeadlightStateReq req;
        if (!Serializer::Deserialize(data, req)) {
            std::cerr << "[LightService] Failed to deserialize SetHeadlightState request" << std::endl;
            SendErrorResponse(request, 1);
            return;
        }
        
        std::string state_str;
        switch (req.command) {
            case application::HeadlightState::OFF: state_str = "OFF"; break;
            case application::HeadlightState::LOW_BEAM: state_str = "LOW_BEAM"; break;
            case application::HeadlightState::HIGH_BEAM: state_str = "HIGH_BEAM"; break;
        }

        std::cout << "[LightService] SetHeadlightState - State: " << state_str << std::endl;

        // 模拟前大灯操作
        application::Result result = SimulateHeadlightOperation(req.command);
        
        // 创建响应
        auto response_data = (result == application::Result::SUCCESS)
                           ? Serializer::SerializeSuccessResponse()
                           : Serializer::SerializeFailResponse();
        SendResponse(request, response_data);
        
        // 如果操作成功，触发硬件事件
        if (result == application::Result::SUCCESS && hardware_simulator_) {
            std::thread([this, req]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                hardware_simulator_->TriggerLightStateEvent(application::LightType::HEADLIGHT,
                                                          static_cast<uint8_t>(req.command));
            }).detach();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[LightService] Error handling SetHeadlightState request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void LightService::HandleSetIndicatorStateRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[LightService] Received SetIndicatorState request" << std::endl;
    
    try {
        // 这里需要实现转向灯状态设置的反序列化
        // 由于数据结构定义可能不完整，先创建一个简单的响应
        
        std::cout << "[LightService] SetIndicatorState - Processing..." << std::endl;
        
        // 创建成功响应
        auto response_data = Serializer::SerializeSuccessResponse();
        SendResponse(request, response_data);
        
        // 触发硬件事件（模拟转向灯状态变化）
        if (hardware_simulator_) {
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                hardware_simulator_->TriggerLightStateEvent(application::LightType::INDICATOR, 1);
            }).detach();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[LightService] Error handling SetIndicatorState request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void LightService::HandleSetPositionLightStateRequest(const std::shared_ptr<vsomeip::message>& request) {
    std::cout << "[LightService] Received SetPositionLightState request" << std::endl;
    
    try {
        // 创建成功响应
        auto response_data = Serializer::SerializeSuccessResponse();
        SendResponse(request, response_data);
        
        // 触发硬件事件
        if (hardware_simulator_) {
            std::thread([this]() {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                hardware_simulator_->TriggerLightStateEvent(application::LightType::POSITION_LIGHT, 1);
            }).detach();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[LightService] Error handling SetPositionLightState request: " << e.what() << std::endl;
        SendErrorResponse(request, 2);
    }
}

void LightService::SendResponse(const std::shared_ptr<vsomeip::message>& request,
                               const std::vector<uint8_t>& payload) {
    if (!app_) return;

    auto response = vsomeip::runtime::get()->create_response(request);
    auto response_payload = vsomeip::runtime::get()->create_payload();

    response_payload->set_data(payload.data(), static_cast<uint32_t>(payload.size()));
    response->set_payload(response_payload);

    app_->send(response);
}

void LightService::SendErrorResponse(const std::shared_ptr<vsomeip::message>& request,
                                    uint8_t error_code) {
    auto error_data = Serializer::SerializeFailResponse();
    error_data.insert(error_data.begin(), error_code);

    SendResponse(request, error_data);

    std::cout << "[LightService] Sent error response with code: " << static_cast<int>(error_code) << std::endl;
}

void LightService::OnLightStateChanged(const application::OnLightStateChangedData& event_data) {
    std::cout << "[LightService] Hardware event - Light state changed: Type "
              << static_cast<int>(event_data.lightType) << " -> "
              << static_cast<int>(event_data.newState) << std::endl;

    // 更新内部状态
    switch (event_data.lightType) {
        case application::LightType::HEADLIGHT:
            current_headlight_state_ = static_cast<application::HeadlightState>(event_data.newState);
            break;
        case application::LightType::INDICATOR:
            current_indicator_state_ = static_cast<application::IndicatorState>(event_data.newState);
            break;
        case application::LightType::POSITION_LIGHT:
            current_position_light_state_ = static_cast<application::PositionLightState>(event_data.newState);
            break;
    }

    // 发送事件到客户端
    SendLightStateChangedEvent(event_data);
}

void LightService::SendLightStateChangedEvent(const application::OnLightStateChangedData& event_data) {
    if (!app_) return;

    auto event_payload = Serializer::Serialize(event_data);
    auto payload = vsomeip::runtime::get()->create_payload();
    payload->set_data(event_payload.data(), static_cast<uint32_t>(event_payload.size()));

    app_->notify(SERVICE_ID, INSTANCE_ID, LIGHT_STATE_CHANGED_EVENT, payload);

    std::cout << "[LightService] Sent light state changed event to clients" << std::endl;
}

application::Result LightService::SimulateHeadlightOperation(application::HeadlightState state) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 模拟95%成功率
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    if ((gen() % 100) < 95) {
        return application::Result::SUCCESS;
    } else {
        std::cout << "[LightService] Simulated headlight operation failure" << std::endl;
        return application::Result::FAIL;
    }
}

application::Result LightService::SimulateIndicatorOperation(application::IndicatorState state) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 模拟95%成功率
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    if ((gen() % 100) < 95) {
        return application::Result::SUCCESS;
    } else {
        std::cout << "[LightService] Simulated indicator operation failure" << std::endl;
        return application::Result::FAIL;
    }
}

application::Result LightService::SimulatePositionLightOperation(application::PositionLightState state) {
    std::this_thread::sleep_for(std::chrono::milliseconds(50));

    // 模拟95%成功率
    static std::mt19937 gen(std::chrono::steady_clock::now().time_since_epoch().count());
    if ((gen() % 100) < 95) {
        return application::Result::SUCCESS;
    } else {
        std::cout << "[LightService] Simulated position light operation failure" << std::endl;
        return application::Result::FAIL;
    }
}

} // namespace services
} // namespace body_controller
