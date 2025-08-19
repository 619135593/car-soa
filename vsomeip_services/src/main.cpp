#include <iostream>
#include <memory>
#include <cstdlib>
#include "services/service_manager.h"

using namespace body_controller::services;

/**
 * @brief 打印程序启动横幅
 */
void PrintBanner() {
    std::cout << "\n";
    std::cout << "╔══════════════════════════════════════════════════════════════╗\n";
    std::cout << "║              Body Controller VSOMEIP Services                ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  Hardware Simulation Services for Vehicle Body Control      ║\n";
    std::cout << "║  Supports: Door, Window, Light, Seat Services               ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  🚗 Door Service:    Lock/Unlock Control & Status           ║\n";
    std::cout << "║  🪟 Window Service:  Position Control & Status              ║\n";
    std::cout << "║  💡 Light Service:   Headlight, Indicator, Position Light   ║\n";
    std::cout << "║  🪑 Seat Service:    Adjustment & Memory Positions          ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "║  📡 VSOMEIP Communication Protocol                          ║\n";
    std::cout << "║  🔄 Automatic Hardware Event Generation                     ║\n";
    std::cout << "║                                                              ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════════╝\n";
    std::cout << "\n";
}

/**
 * @brief 打印环境检查信息
 */
void PrintEnvironmentInfo() {
    std::cout << "[Main] Environment check:\n";
    
    // 检查VSOMEIP配置环境变量
    const char* config_path = std::getenv("VSOMEIP_CONFIGURATION");
    if (config_path) {
        std::cout << "[Main]   VSOMEIP_CONFIGURATION: " << config_path << "\n";
    } else {
        std::cout << "[Main]   VSOMEIP_CONFIGURATION: Not set (using default)\n";
    }
    
    const char* app_name = std::getenv("VSOMEIP_APPLICATION_NAME");
    if (app_name) {
        std::cout << "[Main]   VSOMEIP_APPLICATION_NAME: " << app_name << "\n";
    } else {
        std::cout << "[Main]   VSOMEIP_APPLICATION_NAME: Not set (using default)\n";
    }
    
    std::cout << "[Main] Environment check completed\n\n";
}

/**
 * @brief 打印服务信息
 */
void PrintServiceInfo() {
    std::cout << "[Main] Service Configuration:\n";
    std::cout << "[Main]   🚗 Door Service:    ID=0x1002, Instance=0x1002, Port=30502\n";
    std::cout << "[Main]   🪟 Window Service:  ID=0x1001, Instance=0x1001, Port=30501\n";
    std::cout << "[Main]   💡 Light Service:   ID=0x1003, Instance=0x1003, Port=30503\n";
    std::cout << "[Main]   🪑 Seat Service:    ID=0x1004, Instance=0x1004, Port=30504\n";
    std::cout << "[Main]\n";
    std::cout << "[Main] Hardware Simulation:\n";
    std::cout << "[Main]   🔄 Auto Events: Enabled (every 15 seconds)\n";
    std::cout << "[Main]   📊 Event Types: Door Lock/State, Window Position, Light State, Seat Position\n";
    std::cout << "[Main]   🎯 Success Rate: 95% (simulates real hardware reliability)\n";
    std::cout << "[Main]\n";
}

/**
 * @brief 主函数
 */
int main(int argc, char* argv[]) {
    // 打印启动横幅
    PrintBanner();
    
    // 打印环境信息
    PrintEnvironmentInfo();
    
    // 打印服务信息
    PrintServiceInfo();
    
    try {
        // 创建服务管理器
        std::cout << "[Main] Creating service manager...\n";
        auto service_manager = std::make_unique<ServiceManager>();
        
        // 运行服务管理器（阻塞调用）
        std::cout << "[Main] Starting services...\n";
        service_manager->Run();
        
        std::cout << "[Main] Services stopped normally\n";
        return 0;
        
    } catch (const std::exception& e) {
        std::cerr << "[Main] Fatal error: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "[Main] Unknown fatal error occurred" << std::endl;
        return 1;
    }
}

/**
 * @brief 程序使用说明
 */
void PrintUsage(const char* program_name) {
    std::cout << "\nUsage: " << program_name << " [options]\n";
    std::cout << "\nOptions:\n";
    std::cout << "  -h, --help     Show this help message\n";
    std::cout << "  -v, --version  Show version information\n";
    std::cout << "\nEnvironment Variables:\n";
    std::cout << "  VSOMEIP_CONFIGURATION      Path to VSOMEIP configuration file\n";
    std::cout << "  VSOMEIP_APPLICATION_NAME   Application name for VSOMEIP\n";
    std::cout << "\nExample:\n";
    std::cout << "  export VSOMEIP_CONFIGURATION=./config/vsomeip_services.json\n";
    std::cout << "  export VSOMEIP_APPLICATION_NAME=body_controller_services\n";
    std::cout << "  " << program_name << "\n";
    std::cout << "\nServices Provided:\n";
    std::cout << "  • Door Service (0x1002)    - Lock/unlock control and status\n";
    std::cout << "  • Window Service (0x1001)  - Position control and status\n";
    std::cout << "  • Light Service (0x1003)   - Headlight, indicator, position light control\n";
    std::cout << "  • Seat Service (0x1004)    - Adjustment and memory position control\n";
    std::cout << "\nFor more information, see README.md\n\n";
}

/**
 * @brief 打印版本信息
 */
void PrintVersion() {
    std::cout << "\nBody Controller VSOMEIP Services v1.0.0\n";
    std::cout << "Built with VSOMEIP library\n";
    std::cout << "Copyright (c) 2024 Body Controller Project\n\n";
}
