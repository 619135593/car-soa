#pragma once

#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>

namespace body_controller {
namespace web_api {

/**
 * @brief Web服务器配置
 */
struct WebServerConfig {
    // HTTP服务器配置
    int http_port = 8080;
    std::string http_host = "0.0.0.0";
    bool enable_cors = true;
    size_t max_request_size = 1024 * 1024; // 1MB
    
    // WebSocket服务器配置
    int websocket_port = 8081;
    std::string websocket_host = "0.0.0.0";
    size_t max_connections = 100;
    uint32_t ping_interval_ms = 30000; // 30秒
    
    // 静态文件配置
    std::string web_root = "./web";
    bool enable_directory_listing = false;
    
    // 日志配置
    std::string log_level = "info";
    bool enable_access_log = true;
    std::string log_file = "";
    
    // 安全配置
    bool enable_rate_limiting = true;
    uint32_t rate_limit_requests = 100;
    uint32_t rate_limit_window_ms = 60000; // 1分钟
    
    // 超时配置
    uint32_t request_timeout_ms = 5000;
    uint32_t response_timeout_ms = 10000;
    
    // SOME/IP配置
    std::string vsomeip_config_path = "./config/vsomeip.json";
    std::string vsomeip_app_name = "web_server";
};

/**
 * @brief 配置管理器
 */
class ConfigManager {
public:
    /**
     * @brief 从文件加载配置
     */
    static WebServerConfig LoadFromFile(const std::string& config_file);
    
    /**
     * @brief 从JSON加载配置
     */
    static WebServerConfig LoadFromJson(const nlohmann::json& config_json);
    
    /**
     * @brief 从环境变量加载配置
     */
    static WebServerConfig LoadFromEnvironment();
    
    /**
     * @brief 从命令行参数加载配置
     */
    static WebServerConfig LoadFromCommandLine(int argc, char* argv[]);
    
    /**
     * @brief 合并配置（优先级：命令行 > 环境变量 > 配置文件 > 默认值）
     */
    static WebServerConfig MergeConfigs(
        const WebServerConfig& base,
        const WebServerConfig& override
    );
    
    /**
     * @brief 验证配置
     */
    static bool ValidateConfig(const WebServerConfig& config, std::string& error_message);
    
    /**
     * @brief 保存配置到文件
     */
    static bool SaveToFile(const WebServerConfig& config, const std::string& config_file);
    
    /**
     * @brief 转换为JSON
     */
    static nlohmann::json ToJson(const WebServerConfig& config);
    
    /**
     * @brief 从JSON转换
     */
    static WebServerConfig FromJson(const nlohmann::json& json);
    
    /**
     * @brief 获取默认配置
     */
    static WebServerConfig GetDefaultConfig();
    
    /**
     * @brief 打印配置信息
     */
    static void PrintConfig(const WebServerConfig& config);

private:
    /**
     * @brief 获取环境变量值
     */
    static std::string GetEnvVar(const std::string& name, const std::string& default_value = "");
    
    /**
     * @brief 解析命令行参数
     */
    static std::unordered_map<std::string, std::string> ParseCommandLine(int argc, char* argv[]);
};

} // namespace web_api
} // namespace body_controller
