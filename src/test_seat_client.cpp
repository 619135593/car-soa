#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include "communication/someip_client.h"

using namespace body_controller;

// 全局变量用于信号处理
std::shared_ptr<communication::SeatServiceClient> g_seat_client;
bool g_running = true;

// 信号处理函数
void signal_handler(int signal) {
    std::cout << "\n[TestSeatClient] Received signal " << signal << ", shutting down..." << std::endl;
    g_running = false;
    if (g_seat_client) {
        g_seat_client->Stop();
    }
}

// 事件处理器
void on_seat_position_changed(const application::OnSeatPositionChangedData& data) {
    std::cout << "[TestSeatClient] Seat position changed - Axis: ";
    switch (data.axis) {
        case application::SeatAxis::FORWARD_BACKWARD:
            std::cout << "FORWARD_BACKWARD";
            break;
        case application::SeatAxis::RECLINE:
            std::cout << "RECLINE";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << " New Position: " << static_cast<int>(data.newPosition) << "%" << std::endl;
}

void on_memory_save_confirm(const application::OnMemorySaveConfirmData& data) {
    std::cout << "[TestSeatClient] Memory save confirm - Preset ID: " 
              << static_cast<int>(data.presetID)
              << " Result: " << (data.saveResult == application::Result::SUCCESS ? "SUCCESS" : "FAIL") 
              << std::endl;
}

void on_adjust_seat_response(const application::AdjustSeatResp& response) {
    std::cout << "[TestSeatClient] Adjust seat response - Axis: ";
    switch (response.axis) {
        case application::SeatAxis::FORWARD_BACKWARD:
            std::cout << "FORWARD_BACKWARD";
            break;
        case application::SeatAxis::RECLINE:
            std::cout << "RECLINE";
            break;
        default:
            std::cout << "UNKNOWN";
            break;
    }
    std::cout << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") << std::endl;
}

void on_recall_memory_response(const application::RecallMemoryPositionResp& response) {
    std::cout << "[TestSeatClient] Recall memory response - Preset ID: " 
              << static_cast<int>(response.presetID)
              << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") 
              << std::endl;
}

void on_save_memory_response(const application::SaveMemoryPositionResp& response) {
    std::cout << "[TestSeatClient] Save memory response - Preset ID: " 
              << static_cast<int>(response.presetID)
              << " Result: " << (response.result == application::Result::SUCCESS ? "SUCCESS" : "FAIL") 
              << std::endl;
}

// 测试命令发送函数
void send_test_commands(std::shared_ptr<communication::SeatServiceClient> client) {
    std::cout << "[TestSeatClient] Starting test command sequence..." << std::endl;
    
    // 等待服务可用
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试1：前后调节 - 向前移动
    std::cout << "\n=== Test 1: Move seat forward ===" << std::endl;
    application::AdjustSeatReq forward_req(application::SeatAxis::FORWARD_BACKWARD, application::SeatDirection::POSITIVE);
    client->AdjustSeat(forward_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试2：停止前后调节
    std::cout << "\n=== Test 2: Stop forward/backward adjustment ===" << std::endl;
    application::AdjustSeatReq stop_fb_req(application::SeatAxis::FORWARD_BACKWARD, application::SeatDirection::STOP);
    client->AdjustSeat(stop_fb_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试3：前后调节 - 向后移动
    std::cout << "\n=== Test 3: Move seat backward ===" << std::endl;
    application::AdjustSeatReq backward_req(application::SeatAxis::FORWARD_BACKWARD, application::SeatDirection::NEGATIVE);
    client->AdjustSeat(backward_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试4：停止前后调节
    std::cout << "\n=== Test 4: Stop forward/backward adjustment ===" << std::endl;
    client->AdjustSeat(stop_fb_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试5：靠背调节 - 向后倾斜
    std::cout << "\n=== Test 5: Recline seat backward ===" << std::endl;
    application::AdjustSeatReq recline_back_req(application::SeatAxis::RECLINE, application::SeatDirection::POSITIVE);
    client->AdjustSeat(recline_back_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试6：停止靠背调节
    std::cout << "\n=== Test 6: Stop recline adjustment ===" << std::endl;
    application::AdjustSeatReq stop_recline_req(application::SeatAxis::RECLINE, application::SeatDirection::STOP);
    client->AdjustSeat(stop_recline_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试7：靠背调节 - 向前倾斜
    std::cout << "\n=== Test 7: Recline seat forward ===" << std::endl;
    application::AdjustSeatReq recline_forward_req(application::SeatAxis::RECLINE, application::SeatDirection::NEGATIVE);
    client->AdjustSeat(recline_forward_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试8：停止靠背调节
    std::cout << "\n=== Test 8: Stop recline adjustment ===" << std::endl;
    client->AdjustSeat(stop_recline_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试9：保存记忆位置1
    std::cout << "\n=== Test 9: Save current position to memory slot 1 ===" << std::endl;
    application::SaveMemoryPositionReq save1_req(1);
    client->SaveMemoryPosition(save1_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试10：调节到不同位置
    std::cout << "\n=== Test 10: Adjust to different position ===" << std::endl;
    client->AdjustSeat(forward_req);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client->AdjustSeat(stop_fb_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client->AdjustSeat(recline_back_req);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client->AdjustSeat(stop_recline_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试11：保存记忆位置2
    std::cout << "\n=== Test 11: Save current position to memory slot 2 ===" << std::endl;
    application::SaveMemoryPositionReq save2_req(2);
    client->SaveMemoryPosition(save2_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试12：再次调节到不同位置
    std::cout << "\n=== Test 12: Adjust to another different position ===" << std::endl;
    client->AdjustSeat(backward_req);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client->AdjustSeat(stop_fb_req);
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    client->AdjustSeat(recline_forward_req);
    std::this_thread::sleep_for(std::chrono::seconds(1));
    client->AdjustSeat(stop_recline_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试13：保存记忆位置3
    std::cout << "\n=== Test 13: Save current position to memory slot 3 ===" << std::endl;
    application::SaveMemoryPositionReq save3_req(3);
    client->SaveMemoryPosition(save3_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // 测试14：恢复记忆位置1
    std::cout << "\n=== Test 14: Recall memory position 1 ===" << std::endl;
    application::RecallMemoryPositionReq recall1_req(1);
    client->RecallMemoryPosition(recall1_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 测试15：恢复记忆位置2
    std::cout << "\n=== Test 15: Recall memory position 2 ===" << std::endl;
    application::RecallMemoryPositionReq recall2_req(2);
    client->RecallMemoryPosition(recall2_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 测试16：恢复记忆位置3
    std::cout << "\n=== Test 16: Recall memory position 3 ===" << std::endl;
    application::RecallMemoryPositionReq recall3_req(3);
    client->RecallMemoryPosition(recall3_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    // 测试17：快速调节测试
    std::cout << "\n=== Test 17: Rapid adjustment test ===" << std::endl;
    for (int i = 0; i < 3; ++i) {
        std::cout << "  Cycle " << (i + 1) << "/3" << std::endl;
        
        // 前后快速调节
        client->AdjustSeat(forward_req);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        client->AdjustSeat(stop_fb_req);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        
        client->AdjustSeat(backward_req);
        std::this_thread::sleep_for(std::chrono::milliseconds(800));
        client->AdjustSeat(stop_fb_req);
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    
    // 测试18：记忆位置循环测试
    std::cout << "\n=== Test 18: Memory position cycling test ===" << std::endl;
    for (int i = 1; i <= 3; ++i) {
        std::cout << "  Recalling position " << i << std::endl;
        application::RecallMemoryPositionReq recall_req(i);
        client->RecallMemoryPosition(recall_req);
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    
    // 测试19：错误处理测试（无效记忆位置ID）
    std::cout << "\n=== Test 19: Error handling test (invalid memory ID) ===" << std::endl;
    application::RecallMemoryPositionReq invalid_recall_req(5); // 无效ID
    client->RecallMemoryPosition(invalid_recall_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    application::SaveMemoryPositionReq invalid_save_req(0); // 无效ID
    client->SaveMemoryPosition(invalid_save_req);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // 测试20：最终位置设置
    std::cout << "\n=== Test 20: Final position setup ===" << std::endl;
    std::cout << "  Setting comfortable driving position..." << std::endl;
    client->RecallMemoryPosition(recall1_req); // 恢复到位置1
    
    std::cout << "[TestSeatClient] Test command sequence completed" << std::endl;
}

int main(int argc, char* argv[]) {
    std::cout << "=== Seat Service Client Test ===" << std::endl;
    std::cout << "This test program demonstrates the SeatServiceClient functionality" << std::endl;
    std::cout << "Make sure STM32H7 is running and providing seat services" << std::endl;
    std::cout << "Press Ctrl+C to exit" << std::endl;
    std::cout << "========================================" << std::endl;
    
    // 注册信号处理器
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);
    
    try {
        // 创建座椅服务客户端
        g_seat_client = std::make_shared<communication::SeatServiceClient>("seat_test_client");
        
        // 设置事件处理器
        g_seat_client->SetSeatPositionChangedHandler(on_seat_position_changed);
        g_seat_client->SetMemorySaveConfirmHandler(on_memory_save_confirm);
        g_seat_client->SetAdjustSeatResponseHandler(on_adjust_seat_response);
        g_seat_client->SetRecallMemoryPositionResponseHandler(on_recall_memory_response);
        g_seat_client->SetSaveMemoryPositionResponseHandler(on_save_memory_response);
        
        // 初始化客户端
        if (!g_seat_client->Initialize()) {
            std::cerr << "[TestSeatClient] Failed to initialize seat service client" << std::endl;
            return 1;
        }
        
        std::cout << "[TestSeatClient] Seat service client initialized successfully" << std::endl;
        
        // 在单独的线程中发送测试命令
        std::thread test_thread([&]() {
            send_test_commands(g_seat_client);
        });
        
        // 启动客户端（这是一个阻塞调用）
        std::cout << "[TestSeatClient] Starting seat service client..." << std::endl;
        g_seat_client->Start();
        
        // 等待测试线程完成
        if (test_thread.joinable()) {
            test_thread.join();
        }
        
    } catch (const std::exception& e) {
        std::cerr << "[TestSeatClient] Exception: " << e.what() << std::endl;
        return 1;
    }
    
    std::cout << "[TestSeatClient] Test completed successfully" << std::endl;
    return 0;
}

/*
 * 使用说明：
 * 
 * 1. 编译程序：
 *    在build目录中运行 make，会自动编译test_seat_client
 * 
 * 2. 设置环境变量：
 *    export VSOMEIP_CONFIGURATION=./config/vsomeip.json
 *    export VSOMEIP_APPLICATION_NAME=seat_test_client
 * 
 * 3. 运行测试：
 *    ./bin/test_seat_client
 * 
 * 4. 预期行为：
 *    - 程序启动后会尝试连接到STM32H7的座椅服务
 *    - 如果服务可用，会自动订阅事件并发送测试命令
 *    - 会显示所有的请求、响应和事件信息
 *    - 测试包括：座椅调节、记忆位置保存/恢复、错误处理等
 *    - 按Ctrl+C可以优雅地退出程序
 * 
 * 5. 测试内容：
 *    - 座椅前后调节：向前/向后/停止
 *    - 靠背角度调节：向后倾斜/向前倾斜/停止
 *    - 记忆位置管理：保存到1-3号位置
 *    - 记忆位置恢复：从1-3号位置恢复
 *    - 快速调节测试：测试系统响应性能
 *    - 记忆位置循环：测试记忆功能稳定性
 *    - 错误处理：无效记忆位置ID的处理
 *    - 实时事件：位置变化和保存确认的接收处理
 */
