/**
 * Body Controller API Client
 * 与后端REST API进行通信的客户端模块
 */

class BodyControllerAPI {
    constructor(baseURL = 'http://localhost:8080/api') {
        this.baseURL = baseURL;
        this.timeout = 5000; // 5秒超时
        this.retryCount = 3;
    }

    /**
     * 通用HTTP请求方法
     * @param {string} endpoint - API端点
     * @param {Object} options - 请求选项
     * @returns {Promise<Object>} 响应数据
     */
    async request(endpoint, options = {}) {
        const url = `${this.baseURL}${endpoint}`;
        const config = {
            headers: {
                'Content-Type': 'application/json',
                ...options.headers
            },
            timeout: this.timeout,
            ...options
        };

        for (let attempt = 1; attempt <= this.retryCount; attempt++) {
            try {
                const controller = new AbortController();
                const timeoutId = setTimeout(() => controller.abort(), this.timeout);

                const response = await fetch(url, {
                    ...config,
                    signal: controller.signal
                });

                clearTimeout(timeoutId);

                if (!response.ok) {
                    throw new APIError(
                        `HTTP ${response.status}: ${response.statusText}`,
                        response.status,
                        await response.text()
                    );
                }

                const data = await response.json();
                
                // 检查业务逻辑错误
                if (!data.success) {
                    throw new APIError(
                        data.message || 'API request failed',
                        response.status,
                        data
                    );
                }

                return data;

            } catch (error) {
                console.warn(`API request attempt ${attempt} failed:`, error);
                
                if (attempt === this.retryCount) {
                    throw error;
                }
                
                // 指数退避重试
                await this.delay(Math.pow(2, attempt) * 1000);
            }
        }
    }

    /**
     * 延迟函数
     * @param {number} ms - 延迟毫秒数
     */
    delay(ms) {
        return new Promise(resolve => setTimeout(resolve, ms));
    }

    // ==================== 车门服务API ====================

    /**
     * 设置车门锁定状态
     * @param {number} doorID - 车门ID (0=前左, 1=前右, 2=后左, 3=后右)
     * @param {number} command - 命令 (0=解锁, 1=锁定)
     * @returns {Promise<Object>} 响应数据
     */
    async lockDoor(doorID, command) {
        return this.request('/door/lock', {
            method: 'POST',
            body: JSON.stringify({ doorID, command })
        });
    }

    /**
     * 获取车门状态
     * @param {number} doorID - 车门ID
     * @returns {Promise<Object>} 车门状态数据
     */
    async getDoorStatus(doorID) {
        return this.request(`/door/${doorID}/status`);
    }

    // ==================== 车窗服务API ====================

    /**
     * 设置车窗位置
     * @param {number} windowID - 车窗ID (0=前左, 1=前右, 2=后左, 3=后右)
     * @param {number} position - 位置百分比 (0-100)
     * @returns {Promise<Object>} 响应数据
     */
    async setWindowPosition(windowID, position) {
        return this.request('/window/position', {
            method: 'POST',
            body: JSON.stringify({ windowID, position })
        });
    }

    /**
     * 控制车窗运动
     * @param {number} windowID - 车窗ID
     * @param {number} command - 命令 (0=上升, 1=下降, 2=停止)
     * @returns {Promise<Object>} 响应数据
     */
    async controlWindow(windowID, command) {
        return this.request('/window/control', {
            method: 'POST',
            body: JSON.stringify({ windowID, command })
        });
    }

    /**
     * 获取车窗位置
     * @param {number} windowID - 车窗ID
     * @returns {Promise<Object>} 车窗位置数据
     */
    async getWindowPosition(windowID) {
        return this.request(`/window/${windowID}/position`);
    }

    // ==================== 灯光服务API ====================

    /**
     * 设置前大灯状态
     * @param {number} command - 命令 (0=关闭, 1=近光, 2=远光)
     * @returns {Promise<Object>} 响应数据
     */
    async setHeadlight(command) {
        return this.request('/light/headlight', {
            method: 'POST',
            body: JSON.stringify({ command })
        });
    }

    /**
     * 设置转向灯状态
     * @param {number} command - 命令 (0=关闭, 1=左转, 2=右转, 3=双闪)
     * @returns {Promise<Object>} 响应数据
     */
    async setIndicator(command) {
        return this.request('/light/indicator', {
            method: 'POST',
            body: JSON.stringify({ command })
        });
    }

    /**
     * 设置位置灯状态
     * @param {number} command - 命令 (0=关闭, 1=开启)
     * @returns {Promise<Object>} 响应数据
     */
    async setPositionLight(command) {
        return this.request('/light/position', {
            method: 'POST',
            body: JSON.stringify({ command })
        });
    }

    // ==================== 座椅服务API ====================

    /**
     * 调节座椅
     * @param {number} axis - 调节轴 (0=前后, 1=靠背)
     * @param {number} direction - 方向 (0=正向, 1=负向, 2=停止)
     * @returns {Promise<Object>} 响应数据
     */
    async adjustSeat(axis, direction) {
        return this.request('/seat/adjust', {
            method: 'POST',
            body: JSON.stringify({ axis, direction })
        });
    }

    /**
     * 恢复记忆位置
     * @param {number} presetID - 记忆位置ID (1-3)
     * @returns {Promise<Object>} 响应数据
     */
    async recallMemoryPosition(presetID) {
        return this.request('/seat/memory/recall', {
            method: 'POST',
            body: JSON.stringify({ presetID })
        });
    }

    /**
     * 保存记忆位置
     * @param {number} presetID - 记忆位置ID (1-3)
     * @returns {Promise<Object>} 响应数据
     */
    async saveMemoryPosition(presetID) {
        return this.request('/seat/memory/save', {
            method: 'POST',
            body: JSON.stringify({ presetID })
        });
    }

    // ==================== 系统信息API ====================

    /**
     * 获取系统信息
     * @returns {Promise<Object>} 系统信息
     */
    async getSystemInfo() {
        return this.request('/info');
    }

    /**
     * 获取健康状态
     * @returns {Promise<Object>} 健康状态
     */
    async getHealthStatus() {
        return this.request('/health');
    }

    // ==================== 批量操作API ====================

    /**
     * 获取所有车门状态
     * @returns {Promise<Array>} 所有车门状态
     */
    async getAllDoorStatus() {
        const promises = [0, 1, 2, 3].map(doorID => 
            this.getDoorStatus(doorID).catch(error => ({ doorID, error }))
        );
        return Promise.all(promises);
    }

    /**
     * 获取所有车窗位置
     * @returns {Promise<Array>} 所有车窗位置
     */
    async getAllWindowPositions() {
        const promises = [0, 1, 2, 3].map(windowID => 
            this.getWindowPosition(windowID).catch(error => ({ windowID, error }))
        );
        return Promise.all(promises);
    }

    /**
     * 锁定所有车门
     * @returns {Promise<Array>} 操作结果
     */
    async lockAllDoors() {
        const promises = [0, 1, 2, 3].map(doorID => 
            this.lockDoor(doorID, 1).catch(error => ({ doorID, error }))
        );
        return Promise.all(promises);
    }

    /**
     * 解锁所有车门
     * @returns {Promise<Array>} 操作结果
     */
    async unlockAllDoors() {
        const promises = [0, 1, 2, 3].map(doorID => 
            this.lockDoor(doorID, 0).catch(error => ({ doorID, error }))
        );
        return Promise.all(promises);
    }
}

/**
 * API错误类
 */
class APIError extends Error {
    constructor(message, status, response) {
        super(message);
        this.name = 'APIError';
        this.status = status;
        this.response = response;
    }
}

// 导出API客户端
window.BodyControllerAPI = BodyControllerAPI;
window.APIError = APIError;
