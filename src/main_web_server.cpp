#include <iostream>
#include <memory>
#include <signal.h>
#include <thread>
#include <chrono>
#include "web_api/http_server.h"
// WebSocket服务器已移除，使用SSE替代
#include "web_api/api_handlers.h"

using namespace body_controller;

// 全局变量用于信号处理
std::shared_ptr<web_api::HttpServer> g_http_server;
std::shared_ptr<web_api::ApiHandlers> g_api_handlers;
bool g_running = true;

// 信号处理函数
void signal_handler(int signal) {
    std::cout << "\n[WebServer] Received signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    
    if (g_http_server) {
        g_http_server->Stop();
    }
    if (g_api_handlers) {
        g_api_handlers->Stop();
    }
}

void print_banner() {
    std::cout << R"(
╔══════════════════════════════════════════════════════════════╗
║                Body Controller Web Server                    ║
║                                                              ║
║  REST API + SSE Server for Vehicle Body Control       ║
║  Supports: Door, Window, Light, Seat Control                ║
║                                                              ║
║  HTTP API:     http://localhost:8080                        ║
║  SSE Events:  http://localhost:8080/api/events                        ║
║  Web UI:       http://localhost:8080                        ║
║                                                              ║
╚══════════════════════════════════════════════════════════════╝
)" << std::endl;
}

void print_usage() {
    std::cout << "Usage: body_controller_web_server [options]" << std::endl;
    std::cout << "Options:" << std::endl;
    std::cout << "  --http-port PORT     HTTP server port (default: 8080)" << std::endl;
    std::cout << "  --help               Show this help message" << std::endl;
    std::cout << std::endl;
    std::cout << "Environment Variables:" << std::endl;
    std::cout << "  VSOMEIP_CONFIGURATION    Path to vsomeip configuration file" << std::endl;
    std::cout << "  VSOMEIP_APPLICATION_NAME Application name for vsomeip" << std::endl;
    std::cout << std::endl;
    std::cout << "Example:" << std::endl;
    std::cout << "  export VSOMEIP_CONFIGURATION=./config/vsomeip.json" << std::endl;
    std::cout << "  export VSOMEIP_APPLICATION_NAME=web_server" << std::endl;
    std::cout << "  ./bin/body_controller_web_server --http-port 8080 --ws-port 8081" << std::endl;
}

bool check_environment() {
    const char* vsomeip_config = std::getenv("VSOMEIP_CONFIGURATION");
    const char* vsomeip_app_name = std::getenv("VSOMEIP_APPLICATION_NAME");
    
    if (!vsomeip_config) {
        std::cerr << "[WebServer] Error: VSOMEIP_CONFIGURATION environment variable not set" << std::endl;
        std::cerr << "[WebServer] Please set it to the path of your vsomeip configuration file" << std::endl;
        return false;
    }
    
    if (!vsomeip_app_name) {
        std::cerr << "[WebServer] Error: VSOMEIP_APPLICATION_NAME environment variable not set" << std::endl;
        std::cerr << "[WebServer] Please set it to a unique application name" << std::endl;
        return false;
    }
    
    std::cout << "[WebServer] Environment check passed:" << std::endl;
    std::cout << "[WebServer]   VSOMEIP_CONFIGURATION: " << vsomeip_config << std::endl;
    std::cout << "[WebServer]   VSOMEIP_APPLICATION_NAME: " << vsomeip_app_name << std::endl;
    
    return true;
}

void print_server_status() {
    std::cout << "\n[WebServer] Server Status:" << std::endl;
    
    if (g_http_server) {
        std::cout << "[WebServer]   HTTP Server: " 
                  << (g_http_server->IsRunning() ? "RUNNING" : "STOPPED")
                  << " on port " << g_http_server->GetPort() << std::endl;
    }
    
    // WebSocket服务器已移除，使用SSE替代
    std::cout << "[WebServer]   Real-time Events: SSE (Server-Sent Events) on HTTP port" << std::endl;
    
    if (g_api_handlers) {
        std::cout << "[WebServer]   Service Status:" << std::endl;
        std::cout << "[WebServer]     Door Service: " 
                  << (g_api_handlers->IsDoorServiceAvailable() ? "AVAILABLE" : "UNAVAILABLE") << std::endl;
        std::cout << "[WebServer]     Window Service: " 
                  << (g_api_handlers->IsWindowServiceAvailable() ? "AVAILABLE" : "UNAVAILABLE") << std::endl;
        std::cout << "[WebServer]     Light Service: " 
                  << (g_api_handlers->IsLightServiceAvailable() ? "AVAILABLE" : "UNAVAILABLE") << std::endl;
        std::cout << "[WebServer]     Seat Service: " 
                  << (g_api_handlers->IsSeatServiceAvailable() ? "AVAILABLE" : "UNAVAILABLE") << std::endl;
    }
}

int main(int argc, char* argv[]) {
    // 解析命令行参数
    int http_port = 8080;
    
    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        
        if (arg == "--help") {
            print_usage();
            return 0;
        } else if (arg == "--http-port" && i + 1 < argc) {
            http_port = std::atoi(argv[++i]);
        } else {
            std::cerr << "[WebServer] Unknown argument: " << arg << std::endl;
            print_usage();
            return 1;
        }
    }
    
    print_banner();
    
    // 检查环境变量
    if (!check_environment()) {
        return 1;
    }
    
    // 注册信号处理器
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        std::cout << "[WebServer] Initializing Body Controller Web Server..." << std::endl;
        
        // 创建API处理器
        g_api_handlers = std::make_shared<web_api::ApiHandlers>();
        if (!g_api_handlers->Initialize()) {
            std::cerr << "[WebServer] Failed to initialize API handlers" << std::endl;
            return 1;
        }
        
        // WebSocket服务器已移除，使用SSE替代实时推送
        
        // 创建HTTP服务器
        g_http_server = std::make_shared<web_api::HttpServer>(http_port);
        if (!g_http_server->Initialize()) {
            std::cerr << "[WebServer] Failed to initialize HTTP server" << std::endl;
            return 1;
        }
        
        // 设置组件之间的关联
        g_http_server->SetApiHandlers(g_api_handlers);
        g_api_handlers->SetHttpServer(g_http_server);
        
        std::cout << "[WebServer] All components initialized successfully" << std::endl;
        
        // WebSocket服务器已移除，使用SSE替代

        // 启动HTTP服务器（先启动，不依赖SOME/IP）
        std::cout << "[WebServer] Starting HTTP server..." << std::endl;
        if (!g_http_server->Start()) {
            std::cerr << "[WebServer] Failed to start HTTP server" << std::endl;
            return 1;
        }

        std::cout << "\n[WebServer] ✅ Web Server started successfully!" << std::endl;
        std::cout << "[WebServer] 🌐 Web Interface: http://localhost:" << http_port << std::endl;
        std::cout << "[WebServer] 📡 SSE Events: http://localhost:" << http_port << "/api/events" << std::endl;
        std::cout << "[WebServer] 📚 API Documentation: http://localhost:" << http_port << "/api/info" << std::endl;
        std::cout << "[WebServer] 💚 Health Check: http://localhost:" << http_port << "/api/health" << std::endl;

        // 尝试启动SOME/IP服务客户端（异步，不阻塞Web服务器）
        std::cout << "[WebServer] Attempting to connect to SOME/IP services..." << std::endl;
        std::thread someip_thread([&]() {
            try {
                if (g_api_handlers->Start()) {
                    std::cout << "[WebServer] ✅ SOME/IP services connected successfully" << std::endl;
                } else {
                    std::cout << "[WebServer] ⚠️  SOME/IP services not available - running in standalone mode" << std::endl;
                    std::cout << "[WebServer] 📝 API calls will return mock data" << std::endl;
                }
            } catch (const std::exception& e) {
                std::cout << "[WebServer] ⚠️  SOME/IP connection failed: " << e.what() << std::endl;
                std::cout << "[WebServer] 📝 Running in standalone mode with mock data" << std::endl;
            }
        });
        someip_thread.detach(); // 让线程在后台运行

        std::cout << "[WebServer] Press Ctrl+C to stop the server" << std::endl;
        
        // 定期打印服务器状态
        std::thread status_thread([&]() {
            while (g_running) {
                std::this_thread::sleep_for(std::chrono::seconds(30));
                if (g_running) {
                    print_server_status();
                }
            }
        });
        
        // 主循环
        while (g_running) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        // 等待状态线程结束
        if (status_thread.joinable()) {
            status_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[WebServer] Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "[WebServer] Body Controller Web Server stopped successfully" << std::endl;
    return 0;
}

/*
 * Body Controller Web Server
 * 
 * 这是车身域控制器系统的Web服务器主程序，提供以下功能：
 * 
 * 1. RESTful API接口
 *    - 车门控制：POST /api/door/lock, GET /api/door/{id}/status
 *    - 车窗控制：POST /api/window/position, POST /api/window/control, GET /api/window/{id}/position
 *    - 灯光控制：POST /api/light/headlight, POST /api/light/indicator, POST /api/light/position
 *    - 座椅控制：POST /api/seat/adjust, POST /api/seat/memory/recall, POST /api/seat/memory/save
 * 
 * 2. SSE实时推送 (Server-Sent Events)
 *    - 车门状态变化事件
 *    - 车窗位置变化事件
 *    - 灯光状态变化事件
 *    - 座椅位置变化事件
 * 
 * 3. 静态文件服务
 *    - Web前端界面文件服务
 *    - 支持HTML、CSS、JavaScript等静态资源
 * 
 * 4. 系统监控
 *    - API信息：GET /api/info
 *    - 健康检查：GET /api/health
 *    - 服务状态监控
 * 
 * 使用方法：
 * 1. 设置环境变量：
 *    export VSOMEIP_CONFIGURATION=./config/vsomeip.json
 *    export VSOMEIP_APPLICATION_NAME=web_server
 * 
 * 2. 启动服务器：
 *    ./bin/body_controller_web_server --http-port 8080 --ws-port 8081
 * 
 * 3. 访问Web界面：
 *    http://localhost:8080
 * 
 * 4. 测试API：
 *    curl -X POST http://localhost:8080/api/door/lock \
 *         -H "Content-Type: application/json" \
 *         -d '{"doorID": 0, "command": 1}'
 */
