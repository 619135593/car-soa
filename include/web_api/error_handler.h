#pragma once

#include <string>
#include <exception>
#include <nlohmann/json.hpp>

namespace body_controller {
namespace web_api {

/**
 * @brief Web API异常类
 */
class WebApiException : public std::exception {
public:
    enum class ErrorCode {
        INVALID_REQUEST = 400,
        UNAUTHORIZED = 401,
        FORBIDDEN = 403,
        NOT_FOUND = 404,
        METHOD_NOT_ALLOWED = 405,
        INTERNAL_ERROR = 500,
        SERVICE_UNAVAILABLE = 503,
        TIMEOUT = 504
    };
    
    WebApiException(ErrorCode code, const std::string& message)
        : code_(code), message_(message) {}
    
    const char* what() const noexcept override {
        return message_.c_str();
    }
    
    ErrorCode getCode() const { return code_; }
    int getHttpCode() const { return static_cast<int>(code_); }
    
private:
    ErrorCode code_;
    std::string message_;
};

/**
 * @brief 错误处理器类
 */
class ErrorHandler {
public:
    /**
     * @brief 创建标准错误响应
     */
    static nlohmann::json CreateErrorResponse(
        WebApiException::ErrorCode code, 
        const std::string& message,
        const std::string& detail = ""
    );
    
    /**
     * @brief 处理异常并创建响应
     */
    static nlohmann::json HandleException(const std::exception& e);
    
    /**
     * @brief 验证请求参数
     */
    static void ValidateRequest(const nlohmann::json& request, const std::vector<std::string>& required_fields);
    
    /**
     * @brief 验证数值范围
     */
    static void ValidateRange(int value, int min, int max, const std::string& field_name);
};

/**
 * @brief RAII风格的错误上下文
 */
class ErrorContext {
public:
    ErrorContext(const std::string& operation) : operation_(operation) {}
    
    template<typename Func>
    auto Execute(Func&& func) -> decltype(func()) {
        try {
            return func();
        } catch (const WebApiException&) {
            throw; // 重新抛出Web API异常
        } catch (const std::exception& e) {
            throw WebApiException(
                WebApiException::ErrorCode::INTERNAL_ERROR,
                "Error in " + operation_ + ": " + e.what()
            );
        }
    }
    
private:
    std::string operation_;
};

} // namespace web_api
} // namespace body_controller
