#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include "communication/someip_client.h"

using namespace body_controller;

// 全局变量用于信号处理
std::shared_ptr<communication::DoorServiceClient> g_door_client;
bool g_running = true;

// 信号处理函数
void signal_handler(int signal) {
    std::cout << "\n[TestDoorClient] Received signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    if (g_door_client) {
        g_door_client->Stop();
    }
}

// 事件处理器
void on_lock_state_changed(const application::OnLockStateChangedData& data) {
    std::cout << "[TestDoorClient] Lock state changed - Door: " 
              << static_cast<int>(data.doorID)
              << " New State: " << (data.newLockState == application::LockState::LOCKED ? "LOCKED" : "UNLOCKED") 
              << std::endl;
}

void on_door_state_changed(const application::OnDoorStateChangedData& data) {
    std::cout << "[TestDoorClient] Door state changed - Door: " 
              << static_cast<int>(data.doorID)
              << " New State: " << (data.newDoorState == application::DoorState::CLOSED ? "CLOSED" : "OPEN") 
              << std::endl;
}

void on_set_lock_response(const application::SetLockStateResp& response) {
    std::cout << "[TestDoorClient] Set lock response - Door: " 
              << static_cast<int>(response.doorID)
              << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") 
              << std::endl;
}

void on_get_lock_response(const application::GetLockStateResp& response) {
    std::cout << "[TestDoorClient] Get lock response - Door: " 
              << static_cast<int>(response.doorID)
              << " State: " << (response.lockState == application::LockState::LOCKED ? "LOCKED" : "UNLOCKED") 
              << std::endl;
}

// 测试命令发送函数
void send_test_commands(std::shared_ptr<communication::DoorServiceClient> client) {
    std::cout << "[TestDoorClient] Starting test command sequence..." << std::endl;
    
    // 等待服务可用
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试1：获取前左门锁定状态
    std::cout << "\n=== Test 1: Get front left door lock state ===" << std::endl;
    application::GetLockStateReq get_req(application::Position::FRONT_LEFT);
    client->GetLockState(get_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试2：锁定前左门
    std::cout << "\n=== Test 2: Lock front left door ===" << std::endl;
    application::SetLockStateReq lock_req(application::Position::FRONT_LEFT, application::LockCommand::LOCK);
    client->SetLockState(lock_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试3：解锁前左门
    std::cout << "\n=== Test 3: Unlock front left door ===" << std::endl;
    application::SetLockStateReq unlock_req(application::Position::FRONT_LEFT, application::LockCommand::UNLOCK);
    client->SetLockState(unlock_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试4：获取所有门的状态
    std::cout << "\n=== Test 4: Get all doors lock state ===" << std::endl;
    for (int i = 0; i < 4; ++i) {
        application::GetLockStateReq req(static_cast<application::Position>(i));
        client->GetLockState(req);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "[TestDoorClient] Test command sequence completed" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== Door Service Client Test ===" << std::endl;
    std::cout << "This test program demonstrates the DoorServiceClient functionality" << std::endl;
    std::cout << "Make sure STM32H7 is running and providing door services" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 注册信号处理器
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // 创建车门服务客户端
        g_door_client = std::make_shared<communication::DoorServiceClient>("door_test_client");
        
        // 设置事件处理器
        g_door_client->SetLockStateChangedHandler(on_lock_state_changed);
        g_door_client->SetDoorStateChangedHandler(on_door_state_changed);
        g_door_client->SetSetLockStateResponseHandler(on_set_lock_response);
        g_door_client->SetGetLockStateResponseHandler(on_get_lock_response);
        
        // 初始化客户端
        if (!g_door_client->Initialize()) {
            std::cerr << "[TestDoorClient] Failed to initialize door service client" << std::endl;
            return 1;
        }
        
        std::cout << "[TestDoorClient] Door service client initialized successfully" << std::endl;
        
        // 在单独的线程中发送测试命令
        std::thread test_thread([&]() {
            send_test_commands(g_door_client);
        });
        
        // 启动客户端（这是一个阻塞调用）
        std::cout << "[TestDoorClient] Starting door service client..." << std::endl;
        g_door_client->Start();
        
        // 等待测试线程完成
        if (test_thread.joinable()) {
            test_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[TestDoorClient] Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "[TestDoorClient] Test completed successfully" << std::endl;
    return 0;
}

/*
 * 使用说明：
 * 
 * 1. 编译程序：
 *    g++ -std=c++17 -I../include -lvsomeip3 -lpthread \
 *        src/communication/someip_client.cpp src/test_door_client.cpp \
 *        -o test_door_client
 * 
 * 2. 设置环境变量：
 *    export VSOMEIP_CONFIGURATION=./config/vsomeip.json
 *    export VSOMEIP_APPLICATION_NAME=door_test_client
 * 
 * 3. 运行测试：
 *    ./test_door_client
 * 
 * 4. 预期行为：
 *    - 程序启动后会尝试连接到STM32H7的车门服务
 *    - 如果服务可用，会自动订阅事件并发送测试命令
 *    - 会显示所有的请求、响应和事件信息
 *    - 按Ctrl+C可以优雅地退出程序
 * 
 * 5. 故障排除：
 *    - 如果连接失败，检查网络配置和STM32H7状态
 *    - 如果编译失败，确保vsomeip库已正确安装
 *    - 如果运行时错误，检查配置文件路径和格式
 */
