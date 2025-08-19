#include "communication/someip_client.h"
#include <iostream>
#include <thread>
#include <chrono>

namespace body_controller {
namespace communication {

// ============================================================================
// SomeipClient 基类实现
// ============================================================================

SomeipClient::SomeipClient(const std::string& app_name)
    : application_name_(app_name)
    , is_initialized_(false)
    , is_running_(false) {
    
    // 获取vsomeip运行时
    runtime_ = vsomeip::runtime::get();
    if (!runtime_) {
        std::cerr << "[SomeipClient] Failed to get vsomeip runtime" << std::endl;
        return;
    }
    
    // 创建应用程序实例
    app_ = runtime_->create_application(application_name_);
    if (!app_) {
        std::cerr << "[SomeipClient] Failed to create application: " << application_name_ << std::endl;
        return;
    }
    
    std::cout << "[SomeipClient] Created application: " << application_name_ << std::endl;
}

SomeipClient::~SomeipClient() {
    if (is_running_) {
        Stop();
    }
}

bool SomeipClient::Initialize() {
    if (is_initialized_) {
        std::cout << "[SomeipClient] Already initialized" << std::endl;
        return true;
    }
    
    if (!app_) {
        std::cerr << "[SomeipClient] Application not created" << std::endl;
        return false;
    }
    
    // 初始化应用程序
    if (!app_->init()) {
        std::cerr << "[SomeipClient] Failed to initialize application" << std::endl;
        return false;
    }
    
    // 注册状态处理器
    app_->register_state_handler(
        std::bind(&SomeipClient::OnState, this, std::placeholders::_1)
    );
    
    std::cout << "[SomeipClient] Application initialized successfully" << std::endl;
    is_initialized_ = true;
    return true;
}

void SomeipClient::Start() {
    if (!is_initialized_) {
        std::cerr << "[SomeipClient] Not initialized, cannot start" << std::endl;
        return;
    }

    if (is_running_) {
        std::cout << "[SomeipClient] Already running" << std::endl;
        return;
    }

    std::cout << "[SomeipClient] Starting application..." << std::endl;
    is_running_ = true;

    // 在单独的线程中启动应用程序，避免阻塞主线程
    std::thread app_thread([this]() {
        try {
            // 启动应用程序（这是一个阻塞调用）
            app_->start();
        } catch (const std::exception& e) {
            std::cerr << "[SomeipClient] Application start failed: " << e.what() << std::endl;
            is_running_ = false;
        }
    });

    // 分离线程，让它在后台运行
    app_thread.detach();

    // 给应用程序一点时间启动
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void SomeipClient::Stop() {
    if (!is_running_) {
        return;
    }
    
    std::cout << "[SomeipClient] Stopping application..." << std::endl;
    is_running_ = false;
    
    // 清理所有处理器
    app_->clear_all_handler();
    
    // 停止应用程序
    app_->stop();
    
    std::cout << "[SomeipClient] Application stopped" << std::endl;
}

bool SomeipClient::IsServiceAvailable(vsomeip::service_t service_id, vsomeip::instance_t instance_id) const {
    if (!app_) {
        return false;
    }
    return app_->is_available(service_id, instance_id);
}

void SomeipClient::OnState(vsomeip::state_type_e state) {
    std::cout << "[SomeipClient] State changed to: " << static_cast<int>(state) << std::endl;
    
    if (state == vsomeip::state_type_e::ST_REGISTERED) {
        std::cout << "[SomeipClient] Application registered successfully" << std::endl;
        // 子类可以重写此方法来请求特定服务
    }
}

void SomeipClient::OnAvailability(vsomeip::service_t service, vsomeip::instance_t instance, bool is_available) {
    std::cout << "[SomeipClient] Service 0x" << std::hex << service 
              << " Instance 0x" << instance 
              << " is " << (is_available ? "available" : "unavailable") << std::endl;
}

void SomeipClient::OnMessage(const std::shared_ptr<vsomeip::message>& message) {
    if (!message) {
        std::cerr << "[SomeipClient] Received null message" << std::endl;
        return;
    }
    
    std::cout << "[SomeipClient] Received message - Service: 0x" << std::hex << message->get_service()
              << " Method: 0x" << message->get_method()
              << " Type: " << static_cast<int>(message->get_message_type()) << std::endl;
}

void SomeipClient::SendRequest(vsomeip::service_t service_id, 
                              vsomeip::instance_t instance_id,
                              vsomeip::method_t method_id,
                              const std::vector<uint8_t>& payload_data) {
    if (!app_) {
        std::cerr << "[SomeipClient] Application not available" << std::endl;
        return;
    }
    
    // 检查服务是否可用
    if (!IsServiceAvailable(service_id, instance_id)) {
        std::cerr << "[SomeipClient] Service 0x" << std::hex << service_id 
                  << " Instance 0x" << instance_id << " is not available" << std::endl;
        return;
    }
    
    // 创建请求消息
    auto request = runtime_->create_request();
    if (!request) {
        std::cerr << "[SomeipClient] Failed to create request message" << std::endl;
        return;
    }
    
    // 设置消息头
    request->set_service(service_id);
    request->set_instance(instance_id);
    request->set_method(method_id);
    
    // 设置payload
    if (!payload_data.empty()) {
        auto payload = runtime_->create_payload(payload_data);
        if (payload) {
            request->set_payload(payload);
        } else {
            std::cerr << "[SomeipClient] Failed to create payload" << std::endl;
            return;
        }
    }
    
    // 发送请求
    app_->send(request);
    
    std::cout << "[SomeipClient] Sent request - Service: 0x" << std::hex << service_id
              << " Method: 0x" << method_id << " Payload size: " << payload_data.size() << std::endl;
}

void SomeipClient::SubscribeEvent(vsomeip::service_t service_id,
                                 vsomeip::instance_t instance_id,
                                 vsomeip::event_t event_id,
                                 vsomeip::eventgroup_t eventgroup_id) {
    if (!app_) {
        std::cerr << "[SomeipClient] Application not available" << std::endl;
        return;
    }
    
    // 第一步：请求事件
    app_->request_event(
        service_id, 
        instance_id, 
        event_id,
        {eventgroup_id},  // 事件组集合
        vsomeip::event_type_e::ET_EVENT
    );
    
    // 第二步：订阅事件组
    app_->subscribe(service_id, instance_id, eventgroup_id);
    
    std::cout << "[SomeipClient] Subscribed to event - Service: 0x" << std::hex << service_id
              << " Event: 0x" << event_id << " EventGroup: 0x" << eventgroup_id << std::endl;
}





} // namespace communication
} // namespace body_controller
