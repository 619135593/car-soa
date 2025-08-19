#pragma once

#include <cstdint>
#include <vsomeip/vsomeip.hpp>

namespace body_controller {
namespace communication {

/**
 * @brief SOME/IP服务定义
 * 基于project.md中的通信矩阵和服务信息定义
 * 使用vsomeip库的标准类型定义
 */

// ============================================================================
// 服务ID定义 (Service IDs)
// ============================================================================
constexpr vsomeip::service_t WINDOW_SERVICE_ID    = 0x1001;  // 车窗服务
constexpr vsomeip::service_t DOOR_SERVICE_ID      = 0x1002;  // 车门服务
constexpr vsomeip::service_t LIGHT_SERVICE_ID     = 0x1003;  // 灯光服务
constexpr vsomeip::service_t SEAT_SERVICE_ID      = 0x1004;  // 座椅服务

// ============================================================================
// 实例ID定义 (Instance IDs)
// ============================================================================
constexpr vsomeip::instance_t WINDOW_INSTANCE_ID   = 0x1001;
constexpr vsomeip::instance_t DOOR_INSTANCE_ID     = 0x1002;
constexpr vsomeip::instance_t LIGHT_INSTANCE_ID    = 0x1003;
constexpr vsomeip::instance_t SEAT_INSTANCE_ID     = 0x1004;

// ============================================================================
// 接口版本定义
// ============================================================================
constexpr uint8_t SERVICE_INTERFACE_VERSION_MAJOR = 1;
constexpr uint8_t SERVICE_INTERFACE_VERSION_MINOR = 0;

// ============================================================================
// 方法ID定义 (Method IDs)
// ============================================================================

// 车窗服务方法
namespace window_service {
    constexpr vsomeip::method_t SET_WINDOW_POSITION    = 0x0001;
    constexpr vsomeip::method_t CONTROL_WINDOW         = 0x0002;
    constexpr vsomeip::method_t GET_WINDOW_POSITION    = 0x0003;
}

// 车门服务方法
namespace door_service {
    constexpr vsomeip::method_t SET_LOCK_STATE         = 0x0001;
    constexpr vsomeip::method_t GET_LOCK_STATE         = 0x0002;
}

// 灯光服务方法
namespace light_service {
    constexpr vsomeip::method_t SET_HEADLIGHT_STATE    = 0x0001;
    constexpr vsomeip::method_t SET_INDICATOR_STATE    = 0x0002;
    constexpr vsomeip::method_t SET_POSITION_LIGHT_STATE = 0x0003;
}

// 座椅服务方法
namespace seat_service {
    constexpr vsomeip::method_t ADJUST_SEAT            = 0x0001;
    constexpr vsomeip::method_t RECALL_MEMORY_POSITION = 0x0002;
    constexpr vsomeip::method_t SAVE_MEMORY_POSITION   = 0x0003;
}

// ============================================================================
// 事件ID定义 (Event IDs)
// ============================================================================

// 车窗服务事件
namespace window_events {
    constexpr vsomeip::event_t ON_WINDOW_POSITION_CHANGED = 0x8001;
}

// 车门服务事件
namespace door_events {
    constexpr vsomeip::event_t ON_LOCK_STATE_CHANGED  = 0x8001;
    constexpr vsomeip::event_t ON_DOOR_STATE_CHANGED  = 0x8002;
}

// 灯光服务事件
namespace light_events {
    constexpr vsomeip::event_t ON_LIGHT_STATE_CHANGED = 0x8001;
}

// 座椅服务事件
namespace seat_events {
    constexpr vsomeip::event_t ON_SEAT_POSITION_CHANGED = 0x8001;
    constexpr vsomeip::event_t ON_MEMORY_SAVE_CONFIRM   = 0x8002;
}

// ============================================================================
// 事件组ID定义 (EventGroup IDs)
// ============================================================================
constexpr vsomeip::eventgroup_t WINDOW_EVENTS_GROUP_ID = 0x0001;
constexpr vsomeip::eventgroup_t DOOR_EVENTS_GROUP_ID   = 0x0001;
constexpr vsomeip::eventgroup_t LIGHT_EVENTS_GROUP_ID  = 0x0001;
constexpr vsomeip::eventgroup_t SEAT_EVENTS_GROUP_ID   = 0x0001;

// ============================================================================
// 网络配置定义
// ============================================================================

// STM32H7节点配置
namespace stm32_node {
    constexpr const char* IP_ADDRESS = "192.168.3.11";
    constexpr uint16_t WINDOW_SERVICE_PORT = 30501;
    constexpr uint16_t DOOR_SERVICE_PORT   = 30502;
    constexpr uint16_t LIGHT_SERVICE_PORT  = 30503;
    constexpr uint16_t SEAT_SERVICE_PORT   = 30504;
}

// PC节点配置
namespace pc_node {
    constexpr const char* IP_ADDRESS = "192.168.3.10";
    constexpr const char* MAC_ADDRESS = "08-97-98-C2-A8-B0";
    constexpr uint16_t WINDOW_CLIENT_PORT = 40001;
    constexpr uint16_t DOOR_CLIENT_PORT   = 40002;
    constexpr uint16_t LIGHT_CLIENT_PORT  = 40003;
    constexpr uint16_t SEAT_CLIENT_PORT   = 40004;
}

// ============================================================================
// 通用定义
// ============================================================================

// 协议类型
enum class ProtocolType : uint8_t {
    TCP = 0x06,
    UDP = 0x11
};

// 消息类型
enum class MessageType : uint8_t {
    REQUEST          = 0x00,
    REQUEST_NO_RETURN = 0x01,
    NOTIFICATION     = 0x02,
    RESPONSE         = 0x80,
    ERROR            = 0x81
};

// 返回码定义
enum class ReturnCode : uint8_t {
    E_OK                    = 0x00,  // 成功
    E_NOT_OK                = 0x01,  // 通用错误
    E_UNKNOWN_SERVICE       = 0x02,  // 未知服务
    E_UNKNOWN_METHOD        = 0x03,  // 未知方法
    E_NOT_READY             = 0x04,  // 服务未就绪
    E_NOT_REACHABLE         = 0x05,  // 服务不可达
    E_TIMEOUT               = 0x06,  // 超时
    E_WRONG_PROTOCOL_VERSION = 0x07,  // 协议版本错误
    E_WRONG_INTERFACE_VERSION = 0x08, // 接口版本错误
    E_MALFORMED_MESSAGE     = 0x09,  // 消息格式错误
    E_WRONG_MESSAGE_TYPE    = 0x0A   // 消息类型错误
};

// ============================================================================
// 服务状态定义
// ============================================================================
enum class ServiceState : uint8_t {
    DOWN = 0x00,        // 服务停止
    INITIALIZING = 0x01, // 初始化中
    UP = 0x02,          // 服务运行
    STOPPING = 0x03     // 停止中
};

// ============================================================================
// 超时配置
// ============================================================================
namespace timeouts {
    constexpr uint32_t METHOD_CALL_TIMEOUT_MS = 5000;    // 方法调用超时
    constexpr uint32_t SERVICE_DISCOVERY_TIMEOUT_MS = 10000; // 服务发现超时
    constexpr uint32_t CONNECTION_TIMEOUT_MS = 3000;     // 连接超时
}

// ============================================================================
// 服务发现配置
// ============================================================================
namespace service_discovery {
    constexpr uint16_t SD_PORT = 30490;                  // 服务发现端口
    constexpr uint32_t INITIAL_DELAY_MIN_MS = 10;        // 初始延迟最小值
    constexpr uint32_t INITIAL_DELAY_MAX_MS = 100;       // 初始延迟最大值
    constexpr uint32_t REPETITIONS_BASE_DELAY_MS = 200;  // 重复基础延迟
    constexpr uint8_t REPETITIONS_MAX = 3;               // 最大重复次数
    constexpr uint32_t TTL_MS = 3000;                    // 生存时间
    constexpr uint32_t CYCLIC_OFFER_DELAY_MS = 2000;     // 周期性提供延迟
    constexpr uint32_t REQUEST_RESPONSE_DELAY_MS = 1500; // 请求响应延迟
}

} // namespace communication
} // namespace body_controller
