#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include "communication/someip_client.h"

using namespace body_controller;

// 全局变量用于信号处理
std::shared_ptr<communication::LightServiceClient> g_light_client;
bool g_running = true;

// 信号处理函数
void signal_handler(int signal) {
    std::cout << "\n[TestLightClient] Received signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    if (g_light_client) {
        g_light_client->Stop();
    }
}

// 事件处理器
void on_light_state_changed(const application::OnLightStateChangedData& data) {
    std::cout << "[TestLightClient] Light state changed - Type: ";
    switch (data.lightType) {
        case application::LightType::HEADLIGHT:
            std::cout << "HEADLIGHT";
            break;
        case application::LightType::INDICATOR:
            std::cout << "INDICATOR";
            break;
        case application::LightType::POSITION_LIGHT:
            std::cout << "POSITION_LIGHT";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << " New State: " << static_cast<int>(data.newState) << std::endl;
}

void on_set_headlight_response(const application::SetHeadlightStateResp& response) {
    std::cout << "[TestLightClient] Set headlight response - State: ";
    switch (response.newState) {
        case application::HeadlightState::OFF:
            std::cout << "OFF";
            break;
        case application::HeadlightState::LOW_BEAM:
            std::cout << "LOW_BEAM";
            break;
        case application::HeadlightState::HIGH_BEAM:
            std::cout << "HIGH_BEAM";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
}

void on_set_indicator_response(const application::SetIndicatorStateResp& response) {
    std::cout << "[TestLightClient] Set indicator response - State: ";
    switch (response.newState) {
        case application::IndicatorState::OFF:
            std::cout << "OFF";
            break;
        case application::IndicatorState::LEFT:
            std::cout << "LEFT";
            break;
        case application::IndicatorState::RIGHT:
            std::cout << "RIGHT";
            break;
        case application::IndicatorState::HAZARD:
            std::cout << "HAZARD";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
}

void on_set_position_light_response(const application::SetPositionLightStateResp& response) {
    std::cout << "[TestLightClient] Set position light response - State: " 
              << (response.newState == application::PositionLightState::ON ? "ON" : "OFF")
              << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") 
              << std::endl;
}

// 测试命令发送函数
void send_test_commands(std::shared_ptr<communication::LightServiceClient> client) {
    std::cout << "[TestLightClient] Starting test command sequence..." << std::endl;
    
    // 等待服务可用
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试1：开启位置灯
    std::cout << "\n=== Test 1: Turn on position lights ===" << std::endl;
    application::SetPositionLightStateReq pos_on_req(application::PositionLightState::ON);
    client->SetPositionLightState(pos_on_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试2：开启近光灯
    std::cout << "\n=== Test 2: Turn on low beam headlights ===" << std::endl;
    application::SetHeadlightStateReq low_beam_req(application::HeadlightState::LOW_BEAM);
    client->SetHeadlightState(low_beam_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试3：切换到远光灯
    std::cout << "\n=== Test 3: Switch to high beam headlights ===" << std::endl;
    application::SetHeadlightStateReq high_beam_req(application::HeadlightState::HIGH_BEAM);
    client->SetHeadlightState(high_beam_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试4：关闭前大灯
    std::cout << "\n=== Test 4: Turn off headlights ===" << std::endl;
    application::SetHeadlightStateReq headlight_off_req(application::HeadlightState::OFF);
    client->SetHeadlightState(headlight_off_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试5：左转向灯
    std::cout << "\n=== Test 5: Turn on left indicator ===" << std::endl;
    application::SetIndicatorStateReq left_indicator_req(application::IndicatorState::LEFT);
    client->SetIndicatorState(left_indicator_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试6：右转向灯
    std::cout << "\n=== Test 6: Turn on right indicator ===" << std::endl;
    application::SetIndicatorStateReq right_indicator_req(application::IndicatorState::RIGHT);
    client->SetIndicatorState(right_indicator_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试7：双闪灯
    std::cout << "\n=== Test 7: Turn on hazard lights ===" << std::endl;
    application::SetIndicatorStateReq hazard_req(application::IndicatorState::HAZARD);
    client->SetIndicatorState(hazard_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 测试8：关闭转向灯
    std::cout << "\n=== Test 8: Turn off indicators ===" << std::endl;
    application::SetIndicatorStateReq indicator_off_req(application::IndicatorState::OFF);
    client->SetIndicatorState(indicator_off_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试9：夜间模式（位置灯+近光灯）
    std::cout << "\n=== Test 9: Night mode (position lights + low beam) ===" << std::endl;
    client->SetPositionLightState(pos_on_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client->SetHeadlightState(low_beam_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试10：全部关闭
    std::cout << "\n=== Test 10: Turn off all lights ===" << std::endl;
    application::SetPositionLightStateReq pos_off_req(application::PositionLightState::OFF);
    client->SetPositionLightState(pos_off_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client->SetHeadlightState(headlight_off_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client->SetIndicatorState(indicator_off_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试11：快速切换测试
    std::cout << "\n=== Test 11: Rapid switching test ===" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "  Cycle " << (i + 1) << "/3" << std::endl;
        
        // 开启近光灯
        client->SetHeadlightState(low_beam_req);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        
        // 切换到远光灯
        client->SetHeadlightState(high_beam_req);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        
        // 关闭
        client->SetHeadlightState(headlight_off_req);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
    }
    
    // 测试12：组合灯光测试
    std::cout << "\n=== Test 12: Combined lighting test ===" << std::endl;
    std::cout << "  Setting up driving configuration..." << std::endl;
    client->SetPositionLightState(pos_on_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    client->SetHeadlightState(low_beam_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    client->SetIndicatorState(left_indicator_req);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "  Switching to right turn..." << std::endl;
    client->SetIndicatorState(right_indicator_req);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "  Emergency stop (hazard lights)..." << std::endl;
    client->SetIndicatorState(hazard_req);
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "  Clearing all lights..." << std::endl;
    client->SetIndicatorState(indicator_off_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    client->SetHeadlightState(headlight_off_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    client->SetPositionLightState(pos_off_req);
    
    std::cout << "[TestLightClient] Test command sequence completed" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== Light Service Client Test ===" << std::endl;
    std::cout << "This test program demonstrates the LightServiceClient functionality" << std::endl;
    std::cout << "Make sure STM32H7 is running and providing light services" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 注册信号处理器
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // 创建灯光服务客户端
        g_light_client = std::make_shared<communication::LightServiceClient>("light_test_client");
        
        // 设置事件处理器
        g_light_client->SetLightStateChangedHandler(on_light_state_changed);
        g_light_client->SetSetHeadlightStateResponseHandler(on_set_headlight_response);
        g_light_client->SetSetIndicatorStateResponseHandler(on_set_indicator_response);
        g_light_client->SetSetPositionLightStateResponseHandler(on_set_position_light_response);
        
        // 初始化客户端
        if (!g_light_client->Initialize()) {
            std::cerr << "[TestLightClient] Failed to initialize light service client" << std::endl;
            return 1;
        }
        
        std::cout << "[TestLightClient] Light service client initialized successfully" << std::endl;
        
        // 在单独的线程中发送测试命令
        std::thread test_thread([&]() {
            send_test_commands(g_light_client);
        });
        
        // 启动客户端（这是一个阻塞调用）
        std::cout << "[TestLightClient] Starting light service client..." << std::endl;
        g_light_client->Start();
        
        // 等待测试线程完成
        if (test_thread.joinable()) {
            test_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[TestLightClient] Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "[TestLightClient] Test completed successfully" << std::endl;
    return 0;
}

/*
 * 使用说明：
 * 
 * 1. 编译程序：
 *    在build目录中运行 make，会自动编译test_light_client
 * 
 * 2. 设置环境变量：
 *    export VSOMEIP_CONFIGURATION=./config/vsomeip.json
 *    export VSOMEIP_APPLICATION_NAME=light_test_client
 * 
 * 3. 运行测试：
 *    ./bin/test_light_client
 * 
 * 4. 预期行为：
 *    - 程序启动后会尝试连接到STM32H7的灯光服务
 *    - 如果服务可用，会自动订阅事件并发送测试命令
 *    - 会显示所有的请求、响应和事件信息
 *    - 测试包括：前大灯控制、转向灯控制、位置灯控制、组合灯光等
 *    - 按Ctrl+C可以优雅地退出程序
 * 
 * 5. 测试内容：
 *    - 前大灯：关闭/近光/远光切换
 *    - 转向灯：左转/右转/双闪/关闭
 *    - 位置灯：开启/关闭
 *    - 组合场景：夜间驾驶、紧急停车等
 *    - 快速切换：测试系统响应性能
 *    - 实时事件：灯光状态变化的接收和处理
 */
