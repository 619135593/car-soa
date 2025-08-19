#include "services/service_manager.h"
#include <iostream>
#include <signal.h>
#include <thread>
#include <chrono>

namespace body_controller {
namespace services {

// 全局变量用于信号处理
static ServiceManager* g_service_manager = nullptr;

// 信号处理函数
void signal_handler(int signal) {
    std::cout << "\n[ServiceManager] Received signal " << signal << ", shutting down..." << std::endl;
    if (g_service_manager) {
        g_service_manager->Stop();
    }
}

ServiceManager::ServiceManager()
    : running_(false)
    , vsomeip_ready_(false)
{
    std::cout << "[ServiceManager] Service manager created" << std::endl;
    
    // 设置全局指针用于信号处理
    g_service_manager = this;
    
    // 注册信号处理器
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
}

ServiceManager::~ServiceManager() {
    Stop();
    g_service_manager = nullptr;
    std::cout << "[ServiceManager] Service manager destroyed" << std::endl;
}

bool ServiceManager::Initialize() {
    std::cout << "[ServiceManager] Initializing service manager..." << std::endl;
    
    try {
        // 初始化VSOMEIP应用程序
        if (!InitializeVSomeIPApplication()) {
            std::cerr << "[ServiceManager] Failed to initialize VSOMEIP application" << std::endl;
            return false;
        }
        
        // 创建硬件模拟器
        hardware_simulator_ = std::make_shared<HardwareSimulator>();
        
        // 初始化所有服务
        if (!InitializeServices()) {
            std::cerr << "[ServiceManager] Failed to initialize services" << std::endl;
            return false;
        }
        
        std::cout << "[ServiceManager] Service manager initialized successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ServiceManager] Failed to initialize: " << e.what() << std::endl;
        return false;
    }
}

bool ServiceManager::Start() {
    if (running_) {
        std::cout << "[ServiceManager] Already running" << std::endl;
        return true;
    }
    
    std::cout << "[ServiceManager] Starting service manager..." << std::endl;
    
    try {
        // 启动VSOMEIP应用程序
        if (app_) {
            app_->start();
        }
        
        // 等待VSOMEIP就绪
        std::cout << "[ServiceManager] Waiting for VSOMEIP to be ready..." << std::endl;
        int wait_count = 0;
        while (!vsomeip_ready_ && wait_count < 50) { // 最多等待5秒
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            wait_count++;
        }
        
        if (!vsomeip_ready_) {
            std::cerr << "[ServiceManager] VSOMEIP failed to become ready" << std::endl;
            return false;
        }
        
        // 启动所有服务
        bool all_started = true;
        
        if (door_service_ && !door_service_->Start()) {
            std::cerr << "[ServiceManager] Failed to start door service" << std::endl;
            all_started = false;
        }
        
        if (window_service_ && !window_service_->Start()) {
            std::cerr << "[ServiceManager] Failed to start window service" << std::endl;
            all_started = false;
        }
        
        if (light_service_ && !light_service_->Start()) {
            std::cerr << "[ServiceManager] Failed to start light service" << std::endl;
            all_started = false;
        }
        
        if (seat_service_ && !seat_service_->Start()) {
            std::cerr << "[ServiceManager] Failed to start seat service" << std::endl;
            all_started = false;
        }
        
        if (!all_started) {
            std::cerr << "[ServiceManager] Some services failed to start" << std::endl;
            return false;
        }
        
        // 启动硬件模拟器
        StartHardwareSimulator();
        
        running_ = true;
        std::cout << "[ServiceManager] ✅ All services started successfully" << std::endl;
        
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ServiceManager] Failed to start: " << e.what() << std::endl;
        return false;
    }
}

void ServiceManager::Stop() {
    if (!running_) {
        return;
    }
    
    std::cout << "[ServiceManager] Stopping service manager..." << std::endl;
    
    running_ = false;
    
    // 停止硬件模拟器
    StopHardwareSimulator();
    
    // 停止所有服务
    if (door_service_) {
        door_service_->Stop();
    }
    
    if (window_service_) {
        window_service_->Stop();
    }
    
    if (light_service_) {
        light_service_->Stop();
    }
    
    if (seat_service_) {
        seat_service_->Stop();
    }
    
    // 停止VSOMEIP应用程序
    if (app_) {
        app_->stop();
    }
    
    std::cout << "[ServiceManager] Service manager stopped" << std::endl;
}

void ServiceManager::Run() {
    if (!Initialize()) {
        std::cerr << "[ServiceManager] Failed to initialize, exiting" << std::endl;
        return;
    }
    
    if (!Start()) {
        std::cerr << "[ServiceManager] Failed to start, exiting" << std::endl;
        return;
    }
    
    std::cout << "[ServiceManager] 🚀 Body Controller Services running..." << std::endl;
    std::cout << "[ServiceManager] 📡 VSOMEIP Services available for clients" << std::endl;
    std::cout << "[ServiceManager] 🔄 Hardware events will be generated automatically" << std::endl;
    std::cout << "[ServiceManager] Press Ctrl+C to stop" << std::endl;
    
    // 主循环：保持运行直到收到停止信号
    while (running_) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "[ServiceManager] Shutting down..." << std::endl;
}

void ServiceManager::OnStateChanged(vsomeip::state_type_e state) {
    std::cout << "[ServiceManager] VSOMEIP state changed: ";
    
    switch (state) {
        case vsomeip::state_type_e::ST_REGISTERED:
            std::cout << "REGISTERED" << std::endl;
            vsomeip_ready_ = true;
            break;
        case vsomeip::state_type_e::ST_DEREGISTERED:
            std::cout << "DEREGISTERED" << std::endl;
            vsomeip_ready_ = false;
            break;
        default:
            std::cout << "UNKNOWN(" << static_cast<int>(state) << ")" << std::endl;
            break;
    }
}

bool ServiceManager::InitializeVSomeIPApplication() {
    try {
        // 创建VSOMEIP应用程序
        app_ = vsomeip::runtime::get()->create_application(APPLICATION_NAME);
        if (!app_) {
            std::cerr << "[ServiceManager] Failed to create VSOMEIP application" << std::endl;
            return false;
        }
        
        // 设置状态变化回调
        app_->register_state_handler([this](vsomeip::state_type_e state) {
            OnStateChanged(state);
        });
        
        // 初始化应用程序
        if (!app_->init()) {
            std::cerr << "[ServiceManager] Failed to initialize VSOMEIP application" << std::endl;
            return false;
        }
        
        std::cout << "[ServiceManager] VSOMEIP application initialized: " << APPLICATION_NAME << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ServiceManager] Exception during VSOMEIP initialization: " << e.what() << std::endl;
        return false;
    }
}

bool ServiceManager::InitializeServices() {
    try {
        // 创建所有服务实例
        door_service_ = std::make_unique<DoorService>(app_, hardware_simulator_);
        window_service_ = std::make_unique<WindowService>(app_, hardware_simulator_);
        light_service_ = std::make_unique<LightService>(app_, hardware_simulator_);
        seat_service_ = std::make_unique<SeatService>(app_, hardware_simulator_);
        
        // 初始化所有服务
        bool all_initialized = true;
        
        if (!door_service_->Initialize()) {
            std::cerr << "[ServiceManager] Failed to initialize door service" << std::endl;
            all_initialized = false;
        }
        
        if (!window_service_->Initialize()) {
            std::cerr << "[ServiceManager] Failed to initialize window service" << std::endl;
            all_initialized = false;
        }
        
        if (!light_service_->Initialize()) {
            std::cerr << "[ServiceManager] Failed to initialize light service" << std::endl;
            all_initialized = false;
        }
        
        if (!seat_service_->Initialize()) {
            std::cerr << "[ServiceManager] Failed to initialize seat service" << std::endl;
            all_initialized = false;
        }
        
        if (!all_initialized) {
            std::cerr << "[ServiceManager] Some services failed to initialize" << std::endl;
            return false;
        }
        
        std::cout << "[ServiceManager] All services initialized successfully" << std::endl;
        return true;
        
    } catch (const std::exception& e) {
        std::cerr << "[ServiceManager] Exception during service initialization: " << e.what() << std::endl;
        return false;
    }
}

void ServiceManager::StartHardwareSimulator() {
    if (hardware_simulator_) {
        hardware_simulator_->SetEventInterval(15); // 15秒触发一次事件
        hardware_simulator_->SetAutoEventEnabled(true);
        hardware_simulator_->Start();
        
        std::cout << "[ServiceManager] Hardware simulator started" << std::endl;
    }
}

void ServiceManager::StopHardwareSimulator() {
    if (hardware_simulator_) {
        hardware_simulator_->Stop();
        std::cout << "[ServiceManager] Hardware simulator stopped" << std::endl;
    }
}

} // namespace services
} // namespace body_controller
