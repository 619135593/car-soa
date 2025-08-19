#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include "communication/someip_client.h"

using namespace body_controller;

// 全局变量用于信号处理
std::shared_ptr<communication::WindowServiceClient> g_window_client;
bool g_running = true;

// 信号处理函数
void signal_handler(int signal) {
    std::cout << "\n[TestWindowClient] Received signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    if (g_window_client) {
        g_window_client->Stop();
    }
}

// 事件处理器
void on_window_position_changed(const application::OnWindowPositionChangedData& data) {
    std::cout << "[TestWindowClient] Window position changed - Window: " 
              << static_cast<int>(data.windowID)
              << " New Position: " << static_cast<int>(data.newPosition) << "%" 
              << std::endl;
}

void on_set_window_position_response(const application::SetWindowPositionResp& response) {
    std::cout << "[TestWindowClient] Set window position response - Window: " 
              << static_cast<int>(response.windowID)
              << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") 
              << std::endl;
}

void on_control_window_response(const application::ControlWindowResp& response) {
    std::cout << "[TestWindowClient] Control window response - Window: " 
              << static_cast<int>(response.windowID)
              << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") 
              << std::endl;
}

void on_get_window_position_response(const application::GetWindowPositionResp& response) {
    std::cout << "[TestWindowClient] Get window position response - Window: " 
              << static_cast<int>(response.windowID)
              << " Position: " << static_cast<int>(response.position) << "%" 
              << std::endl;
}

// 测试命令发送函数
void send_test_commands(std::shared_ptr<communication::WindowServiceClient> client) {
    std::cout << "[TestWindowClient] Starting test command sequence..." << std::endl;
    
    // 等待服务可用
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试1：获取前左窗位置
    std::cout << "\n=== Test 1: Get front left window position ===" << std::endl;
    application::GetWindowPositionReq get_req(application::Position::FRONT_LEFT);
    client->GetWindowPosition(get_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试2：设置前左窗位置为50%
    std::cout << "\n=== Test 2: Set front left window position to 50% ===" << std::endl;
    application::SetWindowPositionReq set_req(application::Position::FRONT_LEFT, 50);
    client->SetWindowPosition(set_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试3：控制前左窗向上移动
    std::cout << "\n=== Test 3: Move front left window up ===" << std::endl;
    application::ControlWindowReq up_req(application::Position::FRONT_LEFT, application::WindowCommand::MOVE_UP);
    client->ControlWindow(up_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试4：停止前左窗移动
    std::cout << "\n=== Test 4: Stop front left window ===" << std::endl;
    application::ControlWindowReq stop_req(application::Position::FRONT_LEFT, application::WindowCommand::STOP);
    client->ControlWindow(stop_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试5：控制前左窗向下移动
    std::cout << "\n=== Test 5: Move front left window down ===" << std::endl;
    application::ControlWindowReq down_req(application::Position::FRONT_LEFT, application::WindowCommand::MOVE_DOWN);
    client->ControlWindow(down_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试6：停止前左窗移动
    std::cout << "\n=== Test 6: Stop front left window ===" << std::endl;
    application::ControlWindowReq stop_req2(application::Position::FRONT_LEFT, application::WindowCommand::STOP);
    client->ControlWindow(stop_req2);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试7：设置前左窗位置为80%
    std::cout << "\n=== Test 7: Set front left window position to 80% ===" << std::endl;
    application::SetWindowPositionReq set_req2(application::Position::FRONT_LEFT, 80);
    client->SetWindowPosition(set_req2);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试8：获取所有窗户的位置
    std::cout << "\n=== Test 8: Get all windows position ===" << std::endl;
    for (int i = 0; i < 4; ++i) {
        application::GetWindowPositionReq req(static_cast<application::Position>(i));
        client->GetWindowPosition(req);
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    // 测试9：批量设置所有窗户位置
    std::cout << "\n=== Test 9: Set all windows to different positions ===" << std::endl;
    uint8_t positions[] = {100, 75, 50, 25}; // 前左、前右、后左、后右
    for (int i = 0; i < 4; ++i) {
        application::SetWindowPositionReq req(static_cast<application::Position>(i), positions[i]);
        client->SetWindowPosition(req);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试10：最终状态检查
    std::cout << "\n=== Test 10: Final position check ===" << std::endl;
    for (int i = 0; i < 4; ++i) {
        application::GetWindowPositionReq req(static_cast<application::Position>(i));
        client->GetWindowPosition(req);
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    std::cout << "[TestWindowClient] Test command sequence completed" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== Window Service Client Test ===" << std::endl;
    std::cout << "This test program demonstrates the WindowServiceClient functionality" << std::endl;
    std::cout << "Make sure STM32H7 is running and providing window services" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 注册信号处理器
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // 创建车窗服务客户端
        g_window_client = std::make_shared<communication::WindowServiceClient>("window_test_client");
        
        // 设置事件处理器
        g_window_client->SetWindowPositionChangedHandler(on_window_position_changed);
        g_window_client->SetSetWindowPositionResponseHandler(on_set_window_position_response);
        g_window_client->SetControlWindowResponseHandler(on_control_window_response);
        g_window_client->SetGetWindowPositionResponseHandler(on_get_window_position_response);
        
        // 初始化客户端
        if (!g_window_client->Initialize()) {
            std::cerr << "[TestWindowClient] Failed to initialize window service client" << std::endl;
            return 1;
        }
        
        std::cout << "[TestWindowClient] Window service client initialized successfully" << std::endl;
        
        // 在单独的线程中发送测试命令
        std::thread test_thread([&]() {
            send_test_commands(g_window_client);
        });
        
        // 启动客户端（这是一个阻塞调用）
        std::cout << "[TestWindowClient] Starting window service client..." << std::endl;
        g_window_client->Start();
        
        // 等待测试线程完成
        if (test_thread.joinable()) {
            test_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[TestWindowClient] Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "[TestWindowClient] Test completed successfully" << std::endl;
    return 0;
}

/*
 * 使用说明：
 * 
 * 1. 编译程序：
 *    在build目录中运行 make，会自动编译test_window_client
 * 
 * 2. 设置环境变量：
 *    export VSOMEIP_CONFIGURATION=./config/vsomeip.json
 *    export VSOMEIP_APPLICATION_NAME=window_test_client
 * 
 * 3. 运行测试：
 *    ./bin/test_window_client
 * 
 * 4. 预期行为：
 *    - 程序启动后会尝试连接到STM32H7的车窗服务
 *    - 如果服务可用，会自动订阅事件并发送测试命令
 *    - 会显示所有的请求、响应和事件信息
 *    - 测试包括：获取位置、设置位置、控制升降、批量操作等
 *    - 按Ctrl+C可以优雅地退出程序
 * 
 * 5. 测试内容：
 *    - 单个窗户的位置获取和设置
 *    - 窗户的升降控制（上升、下降、停止）
 *    - 所有窗户的批量操作
 *    - 实时位置变化事件的接收和处理
 *    - 错误处理和异常情况的测试
 */
