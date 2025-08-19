#pragma once

#include <functional>
#include <memory>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <atomic>

namespace body_controller {
namespace web_api {

/**
 * @brief 回调管理器
 * 
 * 管理异步回调函数的生命周期，防止内存泄漏和悬空指针
 */
template<typename ResponseType>
class CallbackManager {
public:
    using CallbackType = std::function<void(const ResponseType&)>;
    using CallbackId = uint64_t;
    
    /**
     * @brief 注册回调函数
     * @param callback 回调函数
     * @param timeout_ms 超时时间（毫秒）
     * @return 回调ID
     */
    CallbackId RegisterCallback(CallbackType callback, uint32_t timeout_ms = 5000) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        CallbackId id = next_id_++;
        auto expiry_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(timeout_ms);
        
        callbacks_[id] = {
            .callback = std::move(callback),
            .expiry_time = expiry_time,
            .is_active = true
        };
        
        return id;
    }
    
    /**
     * @brief 执行回调函数
     * @param id 回调ID
     * @param response 响应数据
     * @return 是否成功执行
     */
    bool ExecuteCallback(CallbackId id, const ResponseType& response) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = callbacks_.find(id);
        if (it != callbacks_.end() && it->second.is_active) {
            try {
                it->second.callback(response);
                callbacks_.erase(it);
                return true;
            } catch (const std::exception& e) {
                // 记录错误但不抛出异常
                callbacks_.erase(it);
                return false;
            }
        }
        
        return false;
    }
    
    /**
     * @brief 取消回调函数
     * @param id 回调ID
     */
    void CancelCallback(CallbackId id) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto it = callbacks_.find(id);
        if (it != callbacks_.end()) {
            it->second.is_active = false;
            callbacks_.erase(it);
        }
    }
    
    /**
     * @brief 清理过期的回调函数
     */
    void CleanupExpiredCallbacks() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        auto now = std::chrono::steady_clock::now();
        auto it = callbacks_.begin();
        
        while (it != callbacks_.end()) {
            if (now > it->second.expiry_time) {
                it = callbacks_.erase(it);
            } else {
                ++it;
            }
        }
    }
    
    /**
     * @brief 获取活跃回调数量
     */
    size_t GetActiveCallbackCount() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return callbacks_.size();
    }
    
    /**
     * @brief 清理所有回调函数
     */
    void Clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        callbacks_.clear();
    }

private:
    struct CallbackInfo {
        CallbackType callback;
        std::chrono::steady_clock::time_point expiry_time;
        bool is_active;
    };
    
    mutable std::mutex mutex_;
    std::unordered_map<CallbackId, CallbackInfo> callbacks_;
    std::atomic<CallbackId> next_id_{1};
};

/**
 * @brief 回调管理器工厂
 */
class CallbackManagerFactory {
public:
    template<typename ResponseType>
    static std::shared_ptr<CallbackManager<ResponseType>> Create() {
        return std::make_shared<CallbackManager<ResponseType>>();
    }
};

} // namespace web_api
} // namespace body_controller
