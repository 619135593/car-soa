#include "common/hardware_simulator.h"
#include <iostream>
#include <chrono>

namespace body_controller {
namespace services {

HardwareSimulator::HardwareSimulator()
    : running_(false)
    , event_interval_seconds_(10)  // 默认10秒触发一次事件
    , auto_events_enabled_(true)
    , random_generator_(std::chrono::steady_clock::now().time_since_epoch().count())
    , door_distribution_(0, 3)      // 4个车门
    , window_distribution_(0, 3)    // 4个车窗
    , position_distribution_(0, 100) // 位置百分比
    , light_type_distribution_(0, 2) // 3种灯光类型
    , light_state_distribution_(0, 2) // 灯光状态
    , seat_axis_distribution_(0, 1)  // 2个座椅调节轴
{
    // 初始化当前状态
    for (int i = 0; i < 4; ++i) {
        current_door_lock_states_[i] = application::LockState::UNLOCKED;
        current_door_states_[i] = application::DoorState::CLOSED;
        current_window_positions_[i] = 50; // 默认50%位置
    }
    
    current_headlight_state_ = application::HeadlightState::OFF;
    current_indicator_state_ = application::IndicatorState::OFF;
    current_position_light_state_ = application::PositionLightState::OFF;
    
    std::cout << "[HardwareSimulator] Hardware simulator created" << std::endl;
}

HardwareSimulator::~HardwareSimulator() {
    Stop();
    std::cout << "[HardwareSimulator] Hardware simulator destroyed" << std::endl;
}

void HardwareSimulator::Start() {
    if (running_) {
        std::cout << "[HardwareSimulator] Already running" << std::endl;
        return;
    }
    
    running_ = true;
    simulation_thread_ = std::make_unique<std::thread>(&HardwareSimulator::SimulationThread, this);
    
    std::cout << "[HardwareSimulator] Hardware simulator started" << std::endl;
    std::cout << "[HardwareSimulator] Auto events: " << (auto_events_enabled_ ? "enabled" : "disabled") << std::endl;
    std::cout << "[HardwareSimulator] Event interval: " << event_interval_seconds_ << " seconds" << std::endl;
}

void HardwareSimulator::Stop() {
    if (!running_) {
        return;
    }
    
    running_ = false;
    
    if (simulation_thread_ && simulation_thread_->joinable()) {
        simulation_thread_->join();
    }
    
    std::cout << "[HardwareSimulator] Hardware simulator stopped" << std::endl;
}

void HardwareSimulator::TriggerDoorLockEvent(application::Position door_id, application::LockState new_state) {
    if (door_lock_callback_) {
        application::OnLockStateChangedData event_data;
        event_data.doorID = door_id;
        event_data.newLockState = new_state;
        
        // 更新内部状态
        current_door_lock_states_[static_cast<int>(door_id)] = new_state;
        
        std::cout << "[HardwareSimulator] Triggering door lock event: Door " 
                  << static_cast<int>(door_id) << " -> " 
                  << (new_state == application::LockState::LOCKED ? "LOCKED" : "UNLOCKED") << std::endl;
        
        door_lock_callback_(event_data);
    }
}

void HardwareSimulator::TriggerWindowPositionEvent(application::Position window_id, uint8_t new_position) {
    if (window_position_callback_) {
        application::OnWindowPositionChangedData event_data;
        event_data.windowID = window_id;
        event_data.newPosition = new_position;
        
        // 更新内部状态
        current_window_positions_[static_cast<int>(window_id)] = new_position;
        
        std::cout << "[HardwareSimulator] Triggering window position event: Window " 
                  << static_cast<int>(window_id) << " -> " << static_cast<int>(new_position) << "%" << std::endl;
        
        window_position_callback_(event_data);
    }
}

void HardwareSimulator::TriggerLightStateEvent(application::LightType light_type, uint8_t new_state) {
    if (light_state_callback_) {
        application::OnLightStateChangedData event_data;
        event_data.lightType = light_type;
        event_data.newState = new_state;
        
        // 更新内部状态
        switch (light_type) {
            case application::LightType::HEADLIGHT:
                current_headlight_state_ = static_cast<application::HeadlightState>(new_state);
                break;
            case application::LightType::INDICATOR:
                current_indicator_state_ = static_cast<application::IndicatorState>(new_state);
                break;
            case application::LightType::POSITION_LIGHT:
                current_position_light_state_ = static_cast<application::PositionLightState>(new_state);
                break;
        }
        
        std::cout << "[HardwareSimulator] Triggering light state event: Type " 
                  << static_cast<int>(light_type) << " -> " << static_cast<int>(new_state) << std::endl;
        
        light_state_callback_(event_data);
    }
}

void HardwareSimulator::SimulationThread() {
    std::cout << "[HardwareSimulator] Simulation thread started" << std::endl;
    
    while (running_) {
        // 等待指定的时间间隔
        for (int i = 0; i < event_interval_seconds_ && running_; ++i) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
        
        if (!running_) break;
        
        // 如果启用了自动事件生成，随机生成一个事件
        if (auto_events_enabled_) {
            int event_type = random_generator_() % 5; // 5种事件类型
            
            switch (event_type) {
                case 0:
                    GenerateRandomDoorLockEvent();
                    break;
                case 1:
                    GenerateRandomDoorStateEvent();
                    break;
                case 2:
                    GenerateRandomWindowPositionEvent();
                    break;
                case 3:
                    GenerateRandomLightStateEvent();
                    break;
                case 4:
                    GenerateRandomSeatPositionEvent();
                    break;
            }
        }
    }
    
    std::cout << "[HardwareSimulator] Simulation thread stopped" << std::endl;
}

void HardwareSimulator::GenerateRandomDoorLockEvent() {
    if (!door_lock_callback_) return;
    
    int door_index = door_distribution_(random_generator_);
    application::Position door_id = static_cast<application::Position>(door_index);
    
    // 切换当前状态
    application::LockState current_state = current_door_lock_states_[door_index];
    application::LockState new_state = (current_state == application::LockState::LOCKED) 
                                     ? application::LockState::UNLOCKED 
                                     : application::LockState::LOCKED;
    
    TriggerDoorLockEvent(door_id, new_state);
}

void HardwareSimulator::GenerateRandomDoorStateEvent() {
    if (!door_state_callback_) return;
    
    int door_index = door_distribution_(random_generator_);
    application::Position door_id = static_cast<application::Position>(door_index);
    
    // 切换当前状态
    application::DoorState current_state = current_door_states_[door_index];
    application::DoorState new_state = (current_state == application::DoorState::CLOSED) 
                                     ? application::DoorState::OPEN 
                                     : application::DoorState::CLOSED;
    
    current_door_states_[door_index] = new_state;
    
    application::OnDoorStateChangedData event_data;
    event_data.doorID = door_id;
    event_data.newDoorState = new_state;
    
    std::cout << "[HardwareSimulator] Generated door state event: Door " 
              << door_index << " -> " 
              << (new_state == application::DoorState::OPEN ? "OPEN" : "CLOSED") << std::endl;
    
    door_state_callback_(event_data);
}

void HardwareSimulator::GenerateRandomWindowPositionEvent() {
    if (!window_position_callback_) return;
    
    int window_index = window_distribution_(random_generator_);
    application::Position window_id = static_cast<application::Position>(window_index);
    
    // 生成新的随机位置
    uint8_t new_position = static_cast<uint8_t>(position_distribution_(random_generator_));
    
    TriggerWindowPositionEvent(window_id, new_position);
}

void HardwareSimulator::GenerateRandomLightStateEvent() {
    if (!light_state_callback_) return;
    
    int light_type_index = light_type_distribution_(random_generator_);
    application::LightType light_type = static_cast<application::LightType>(light_type_index);
    
    uint8_t new_state;
    
    // 根据灯光类型生成合适的状态
    switch (light_type) {
        case application::LightType::HEADLIGHT:
            new_state = static_cast<uint8_t>(random_generator_() % 3); // 0-2: OFF, LOW_BEAM, HIGH_BEAM
            break;
        case application::LightType::INDICATOR:
            new_state = static_cast<uint8_t>(random_generator_() % 4); // 0-3: OFF, LEFT, RIGHT, HAZARD
            break;
        case application::LightType::POSITION_LIGHT:
            new_state = static_cast<uint8_t>(random_generator_() % 2); // 0-1: OFF, ON
            break;
        default:
            new_state = 0;
            break;
    }
    
    TriggerLightStateEvent(light_type, new_state);
}

void HardwareSimulator::GenerateRandomSeatPositionEvent() {
    if (!seat_position_callback_) return;
    
    int seat_index = random_generator_() % 4; // 4个座椅
    int axis_index = seat_axis_distribution_(random_generator_);

    application::SeatAxis axis = static_cast<application::SeatAxis>(axis_index);
    
    // 生成新的位置值
    int new_position;
    if (axis == application::SeatAxis::FORWARD_BACKWARD) {
        new_position = (random_generator_() % 201) - 100; // -100 到 100
    } else { // RECLINE
        new_position = random_generator_() % 91; // 0 到 90
    }
    
    application::OnSeatPositionChangedData event_data;
    event_data.axis = axis;
    event_data.newPosition = new_position;
    
    std::cout << "[HardwareSimulator] Generated seat position event: Seat " 
              << seat_index << " Axis " << axis_index << " -> " << new_position << std::endl;
    
    seat_position_callback_(event_data);
}

} // namespace services
} // namespace body_controller
