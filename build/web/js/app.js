/**
 * Body Controller Web Application
 * 主应用程序，协调API通信、实时事件流连接和UI更新
 */

class BodyControllerApp {
    constructor() {
        this.api = new BodyControllerAPI();
        this.ws = new BodyControllerSSE();
        this.state = {
            connected: false,
            doors: {},
            windows: {},
            lights: {},
            seat: {},
            logs: [],
            theme: 'dark' // 默认深色主题
        };

        this.init();
    }

    /**
     * 初始化应用程序
     */
    async init() {
        console.log('Initializing Body Controller App...');
        
        // 显示加载指示器
        this.showLoading('正在连接系统...');
        
        try {
            // 初始化主题
            this.initializeTheme();

            // 设置实时事件流监听器
            this.setupWebSocketHandlers();

            // 连接实时事件流
            this.ws.connect();

            // 设置UI事件监听器
            this.setupUIHandlers();

            // 启动时钟
            this.startClock();

            // 初始化状态
            await this.initializeState();

            this.log('系统初始化完成', 'success');

        } catch (error) {
            console.error('Failed to initialize app:', error);
            this.showError('系统初始化失败: ' + error.message);
        } finally {
            this.hideLoading();
        }
    }

    /**
     * 设置实时事件流处理器
     */
    setupWebSocketHandlers() {
        // 连接状态事件
        this.ws.on('connected', () => {
            this.state.connected = true;
            this.updateConnectionStatus(true);
            this.log('实时事件流连接已建立', 'success');
        });

        this.ws.on('disconnected', () => {
            this.state.connected = false;
            this.updateConnectionStatus(false);
            this.log('实时事件流连接已断开', 'warning');
        });

        this.ws.on('reconnecting', (data) => {
            this.log(`正在重连... (第${data.attempt}次尝试)`, 'info');
        });

        this.ws.on('error', (error) => {
            this.log('事件流错误: ' + error.message, 'error');
        });

        // 车身控制事件
        this.ws.on('door_lock_changed', (data) => {
            this.handleDoorLockChanged(data);
        });

        this.ws.on('door_state_changed', (data) => {
            this.handleDoorStateChanged(data);
        });

        this.ws.on('window_position_changed', (data) => {
            this.handleWindowPositionChanged(data);
        });

        this.ws.on('light_state_changed', (data) => {
            this.handleLightStateChanged(data);
        });

        this.ws.on('seat_position_changed', (data) => {
            this.handleSeatPositionChanged(data);
        });

        this.ws.on('seat_memory_save_confirm', (data) => {
            this.handleSeatMemorySaveConfirm(data);
        });
    }

    /**
     * 设置UI事件处理器
     */
    setupUIHandlers() {
        // 页面卸载时断开连接
        window.addEventListener('beforeunload', () => {
            this.ws.disconnect();
        });

        // 页面可见性变化处理
        document.addEventListener('visibilitychange', () => {
            if (document.hidden) {
                // 页面隐藏时暂停某些操作
            } else {
                // 页面显示时恢复操作
                this.refreshAllStatus();
            }
        });
    }

    /**
     * 启动时钟显示
     */
    startClock() {
        const updateClock = () => {
            const now = new Date();
            const timeString = now.toLocaleTimeString('zh-CN', { hour12: false });
            const timeElement = document.getElementById('systemTime');
            if (timeElement) {
                timeElement.textContent = timeString;
            }
        };

        updateClock();
        setInterval(updateClock, 1000);
    }

    /**
     * 初始化系统状态
     */
    async initializeState() {
        try {
            // 获取系统信息
            const systemInfo = await this.api.getSystemInfo();
            this.log(`系统版本: ${systemInfo.data.version}`, 'info');

            // 获取健康状态
            const healthStatus = await this.api.getHealthStatus();
            this.updateSystemStatus(healthStatus.data.status === 'healthy');

            // 获取所有车门状态
            await this.refreshDoorStatus();

            // 获取所有车窗位置
            await this.refreshWindowStatus();

        } catch (error) {
            console.warn('Failed to initialize some states:', error);
            this.log('部分状态初始化失败', 'warning');
        }
    }

    // ==================== 车门控制方法 ====================

    /**
     * 锁定/解锁车门
     */
    async lockDoor(doorID, command) {
        try {
            this.log(`${command ? '锁定' : '解锁'}车门 ${doorID}`, 'info');
            
            const response = await this.api.lockDoor(doorID, command);
            
            if (response.success) {
                this.showSuccess(`车门 ${doorID} ${command ? '锁定' : '解锁'}成功`);
                // 状态会通过WebSocket事件更新
            } else {
                throw new Error(response.message || '操作失败');
            }
            
        } catch (error) {
            this.showError(`车门操作失败: ${error.message}`);
            this.log(`车门操作失败: ${error.message}`, 'error');
        }
    }

    /**
     * 锁定所有车门
     */
    async lockAllDoors() {
        try {
            this.log('锁定所有车门', 'info');
            const results = await this.api.lockAllDoors();
            
            const successCount = results.filter(r => !r.error).length;
            const failCount = results.filter(r => r.error).length;
            
            if (failCount === 0) {
                this.showSuccess('所有车门锁定成功');
            } else {
                this.showError(`${failCount}个车门锁定失败`);
            }
            
        } catch (error) {
            this.showError(`批量锁定失败: ${error.message}`);
        }
    }

    /**
     * 解锁所有车门
     */
    async unlockAllDoors() {
        try {
            this.log('解锁所有车门', 'info');
            const results = await this.api.unlockAllDoors();
            
            const successCount = results.filter(r => !r.error).length;
            const failCount = results.filter(r => r.error).length;
            
            if (failCount === 0) {
                this.showSuccess('所有车门解锁成功');
            } else {
                this.showError(`${failCount}个车门解锁失败`);
            }
            
        } catch (error) {
            this.showError(`批量解锁失败: ${error.message}`);
        }
    }

    /**
     * 刷新车门状态
     */
    async refreshDoorStatus() {
        try {
            const results = await this.api.getAllDoorStatus();
            
            results.forEach(result => {
                if (!result.error && result.data) {
                    this.updateDoorStatus(result.data.doorID, result.data);
                }
            });
            
        } catch (error) {
            console.warn('Failed to refresh door status:', error);
        }
    }

    // ==================== 车窗控制方法 ====================

    /**
     * 设置车窗位置
     */
    async setWindowPosition(windowID, position) {
        try {
            this.log(`设置车窗 ${windowID} 位置为 ${position}%`, 'info');
            
            const response = await this.api.setWindowPosition(windowID, parseInt(position));
            
            if (response.success) {
                this.showSuccess(`车窗 ${windowID} 位置设置成功`);
                // 更新滑块显示
                this.updateWindowPosition(windowID, position);
            } else {
                throw new Error(response.message || '操作失败');
            }
            
        } catch (error) {
            this.showError(`车窗位置设置失败: ${error.message}`);
        }
    }

    /**
     * 控制车窗运动
     */
    async controlWindow(windowID, command) {
        try {
            const commandText = ['上升', '下降', '停止'][command];
            this.log(`车窗 ${windowID} ${commandText}`, 'info');
            
            const response = await this.api.controlWindow(windowID, command);
            
            if (response.success) {
                this.showSuccess(`车窗 ${windowID} ${commandText}成功`);
            } else {
                throw new Error(response.message || '操作失败');
            }
            
        } catch (error) {
            this.showError(`车窗控制失败: ${error.message}`);
        }
    }

    /**
     * 刷新车窗状态
     */
    async refreshWindowStatus() {
        try {
            const results = await this.api.getAllWindowPositions();
            
            results.forEach(result => {
                if (!result.error && result.data) {
                    this.updateWindowPosition(result.data.windowID, result.data.position);
                }
            });
            
        } catch (error) {
            console.warn('Failed to refresh window status:', error);
        }
    }

    // ==================== 灯光控制方法 ====================

    /**
     * 设置前大灯
     */
    async setHeadlight(command) {
        try {
            const commandText = ['关闭', '近光', '远光'][command];
            this.log(`设置前大灯: ${commandText}`, 'info');
            
            const response = await this.api.setHeadlight(command);
            
            if (response.success) {
                this.showSuccess(`前大灯${commandText}成功`);
                this.updateLightStatus('headlight', command);
            } else {
                throw new Error(response.message || '操作失败');
            }
            
        } catch (error) {
            this.showError(`前大灯控制失败: ${error.message}`);
        }
    }

    /**
     * 设置转向灯
     */
    async setIndicator(command) {
        try {
            const commandText = ['关闭', '左转', '右转', '双闪'][command];
            this.log(`设置转向灯: ${commandText}`, 'info');
            
            const response = await this.api.setIndicator(command);
            
            if (response.success) {
                this.showSuccess(`转向灯${commandText}成功`);
                this.updateLightStatus('indicator', command);
            } else {
                throw new Error(response.message || '操作失败');
            }
            
        } catch (error) {
            this.showError(`转向灯控制失败: ${error.message}`);
        }
    }

    /**
     * 设置位置灯
     */
    async setPositionLight(command) {
        try {
            const commandText = ['关闭', '开启'][command];
            this.log(`设置位置灯: ${commandText}`, 'info');
            
            const response = await this.api.setPositionLight(command);
            
            if (response.success) {
                this.showSuccess(`位置灯${commandText}成功`);
                this.updateLightStatus('position', command);
            } else {
                throw new Error(response.message || '操作失败');
            }
            
        } catch (error) {
            this.showError(`位置灯控制失败: ${error.message}`);
        }
    }

    // ==================== 座椅控制方法 ====================

    /**
     * 调节座椅
     */
    async adjustSeat(axis, direction) {
        try {
            const axisText = ['前后', '靠背'][axis];
            const directionText = ['正向', '负向', '停止'][direction];
            this.log(`座椅${axisText}调节: ${directionText}`, 'info');

            const response = await this.api.adjustSeat(axis, direction);

            if (response.success) {
                this.showSuccess(`座椅${axisText}${directionText}成功`);
            } else {
                throw new Error(response.message || '操作失败');
            }

        } catch (error) {
            this.showError(`座椅调节失败: ${error.message}`);
        }
    }

    /**
     * 恢复记忆位置
     */
    async recallMemoryPosition(presetID) {
        try {
            this.log(`恢复记忆位置 ${presetID}`, 'info');

            const response = await this.api.recallMemoryPosition(presetID);

            if (response.success) {
                this.showSuccess(`记忆位置 ${presetID} 恢复成功`);
            } else {
                throw new Error(response.message || '操作失败');
            }

        } catch (error) {
            this.showError(`记忆位置恢复失败: ${error.message}`);
        }
    }

    /**
     * 保存记忆位置
     */
    async saveMemoryPosition(presetID) {
        try {
            this.log(`保存记忆位置 ${presetID}`, 'info');

            const response = await this.api.saveMemoryPosition(presetID);

            if (response.success) {
                this.showSuccess(`记忆位置 ${presetID} 保存成功`);
            } else {
                throw new Error(response.message || '操作失败');
            }

        } catch (error) {
            this.showError(`记忆位置保存失败: ${error.message}`);
        }
    }

    // ==================== 主题控制方法 ====================

    /**
     * 初始化主题
     */
    initializeTheme() {
        // 从localStorage读取保存的主题
        const savedTheme = localStorage.getItem('body-controller-theme');
        if (savedTheme && (savedTheme === 'light' || savedTheme === 'dark')) {
            this.state.theme = savedTheme;
        }

        // 应用主题
        this.applyTheme(this.state.theme);
    }

    /**
     * 切换主题
     */
    toggleTheme() {
        const newTheme = this.state.theme === 'dark' ? 'light' : 'dark';
        this.state.theme = newTheme;

        // 保存到localStorage
        localStorage.setItem('body-controller-theme', newTheme);

        // 应用新主题
        this.applyTheme(newTheme);

        this.log(`切换到${newTheme === 'dark' ? '深色' : '浅色'}主题`, 'info');
    }

    /**
     * 应用主题
     */
    applyTheme(theme) {
        document.documentElement.setAttribute('data-theme', theme);

        // 更新主题切换按钮图标
        const themeIcon = document.querySelector('.theme-icon');
        if (themeIcon) {
            themeIcon.textContent = theme === 'dark' ? '🌙' : '☀️';
        }
    }

    // ==================== WebSocket事件处理方法 ====================

    /**
     * 处理车门锁定状态变化事件
     */
    handleDoorLockChanged(data) {
        this.log(`车门 ${data.doorID} 锁定状态变为: ${data.newLockState}`, 'info');
        this.updateDoorLockStatus(data.doorID, data.newLockState);
    }

    /**
     * 处理车门开关状态变化事件
     */
    handleDoorStateChanged(data) {
        this.log(`车门 ${data.doorID} 开关状态变为: ${data.newDoorState}`, 'info');
        this.updateDoorState(data.doorID, data.newDoorState);
    }

    /**
     * 处理车窗位置变化事件
     */
    handleWindowPositionChanged(data) {
        this.log(`车窗 ${data.windowID} 位置变为: ${data.position}%`, 'info');
        this.updateWindowPosition(data.windowID, data.position);
    }

    /**
     * 处理灯光状态变化事件
     */
    handleLightStateChanged(data) {
        this.log(`灯光状态变化: ${JSON.stringify(data)}`, 'info');
        // 根据具体的灯光类型更新状态
    }

    /**
     * 处理座椅位置变化事件
     */
    handleSeatPositionChanged(data) {
        this.log(`座椅位置变化: ${JSON.stringify(data)}`, 'info');
    }

    /**
     * 处理座椅记忆位置保存确认事件
     */
    handleSeatMemorySaveConfirm(data) {
        this.showSuccess(`记忆位置 ${data.presetID} 保存确认`);
        this.log(`记忆位置 ${data.presetID} 保存确认`, 'success');
    }

    // ==================== UI更新方法 ====================

    /**
     * 更新连接状态显示
     */
    updateConnectionStatus(connected) {
        const statusIndicator = document.getElementById('connectionStatus');
        const statusText = document.getElementById('connectionText');

        if (statusIndicator && statusText) {
            if (connected) {
                statusIndicator.className = 'status-indicator connected';
                statusText.textContent = '已连接';
            } else {
                statusIndicator.className = 'status-indicator disconnected';
                statusText.textContent = '未连接';
            }
        }
    }

    /**
     * 更新系统状态显示
     */
    updateSystemStatus(healthy) {
        const statusElement = document.getElementById('systemStatus');
        if (statusElement) {
            statusElement.textContent = healthy ? '系统正常' : '系统异常';
            statusElement.className = healthy ? 'system-status healthy' : 'system-status error';
        }
    }

    /**
     * 更新车门锁定状态
     */
    updateDoorLockStatus(doorID, lockState) {
        const element = document.getElementById(`doorLock${doorID}`);
        if (element) {
            const stateText = lockState ? '已锁定' : '已解锁';
            element.textContent = stateText;
            element.className = `status-badge lock-status ${lockState ? 'locked' : 'unlocked'}`;

            // 添加脉冲动画
            element.classList.add('pulse');
            setTimeout(() => element.classList.remove('pulse'), 2000);
        }

        this.state.doors[doorID] = { ...this.state.doors[doorID], lockState };
    }

    /**
     * 更新车门开关状态
     */
    updateDoorState(doorID, doorState) {
        const element = document.getElementById(`doorState${doorID}`);
        if (element) {
            const stateText = doorState ? '已打开' : '已关闭';
            element.textContent = stateText;
            element.className = `status-badge door-state ${doorState ? 'open' : 'closed'}`;

            // 添加脉冲动画
            element.classList.add('pulse');
            setTimeout(() => element.classList.remove('pulse'), 2000);
        }

        this.state.doors[doorID] = { ...this.state.doors[doorID], doorState };
    }

    /**
     * 更新车门状态（综合）
     */
    updateDoorStatus(doorID, data) {
        if (data.currentState !== undefined) {
            this.updateDoorLockStatus(doorID, data.currentState);
        }
        if (data.doorState !== undefined) {
            this.updateDoorState(doorID, data.doorState);
        }
    }

    /**
     * 更新车窗位置
     */
    updateWindowPosition(windowID, position) {
        // 更新位置文本
        const posElement = document.getElementById(`windowPos${windowID}`);
        if (posElement) {
            posElement.textContent = `位置: ${position}%`;
        }

        // 更新滑块位置
        const sliderElement = document.getElementById(`windowSlider${windowID}`);
        if (sliderElement) {
            sliderElement.value = position;
        }

        this.state.windows[windowID] = { position };
    }

    /**
     * 更新灯光状态
     */
    updateLightStatus(lightType, state) {
        let element, stateText;

        switch (lightType) {
            case 'headlight':
                element = document.getElementById('headlightStatus');
                stateText = ['关闭', '近光', '远光'][state];
                break;
            case 'indicator':
                element = document.getElementById('indicatorStatus');
                stateText = ['关闭', '左转', '右转', '双闪'][state];
                break;
            case 'position':
                element = document.getElementById('positionLightStatus');
                stateText = ['关闭', '开启'][state];
                break;
        }

        if (element) {
            element.textContent = stateText;
            element.className = `status-badge ${state ? 'active' : 'inactive'}`;

            // 添加脉冲动画
            element.classList.add('pulse');
            setTimeout(() => element.classList.remove('pulse'), 2000);
        }

        this.state.lights[lightType] = state;
    }

    // ==================== 工具方法 ====================

    /**
     * 刷新所有状态
     */
    async refreshAllStatus() {
        try {
            await Promise.all([
                this.refreshDoorStatus(),
                this.refreshWindowStatus()
            ]);
        } catch (error) {
            console.warn('Failed to refresh all status:', error);
        }
    }

    /**
     * 显示加载指示器
     */
    showLoading(message = '加载中...') {
        const overlay = document.getElementById('loadingOverlay');
        const text = overlay?.querySelector('.loading-text');

        if (overlay) {
            overlay.classList.add('show');
        }
        if (text) {
            text.textContent = message;
        }
    }

    /**
     * 隐藏加载指示器
     */
    hideLoading() {
        const overlay = document.getElementById('loadingOverlay');
        if (overlay) {
            overlay.classList.remove('show');
        }
    }

    /**
     * 显示错误提示
     */
    showError(message) {
        const toast = document.getElementById('errorToast');
        const messageElement = document.getElementById('errorMessage');

        if (toast && messageElement) {
            messageElement.textContent = message;
            toast.classList.add('show');

            // 3秒后自动隐藏
            setTimeout(() => this.hideError(), 5000);
        }
    }

    /**
     * 隐藏错误提示
     */
    hideError() {
        const toast = document.getElementById('errorToast');
        if (toast) {
            toast.classList.remove('show');
        }
    }

    /**
     * 显示成功提示
     */
    showSuccess(message) {
        const toast = document.getElementById('successToast');
        const messageElement = document.getElementById('successMessage');

        if (toast && messageElement) {
            messageElement.textContent = message;
            toast.classList.add('show');

            // 3秒后自动隐藏
            setTimeout(() => this.hideSuccess(), 3000);
        }
    }

    /**
     * 隐藏成功提示
     */
    hideSuccess() {
        const toast = document.getElementById('successToast');
        if (toast) {
            toast.classList.remove('show');
        }
    }

    /**
     * 记录日志
     */
    log(message, type = 'info') {
        const timestamp = new Date().toLocaleTimeString('zh-CN', { hour12: false });
        const logEntry = {
            timestamp,
            message,
            type
        };

        this.state.logs.unshift(logEntry);

        // 限制日志数量
        if (this.state.logs.length > 100) {
            this.state.logs = this.state.logs.slice(0, 100);
        }

        this.updateLogDisplay();
    }

    /**
     * 更新日志显示
     */
    updateLogDisplay() {
        const logContent = document.getElementById('logContent');
        if (!logContent) return;

        const logHTML = this.state.logs.map(entry => `
            <div class="log-item log-${entry.type}">
                <span class="log-time">${entry.timestamp}</span>
                <span class="log-message">${entry.message}</span>
            </div>
        `).join('');

        logContent.innerHTML = logHTML;
    }

    /**
     * 清空日志
     */
    clearLog() {
        this.state.logs = [];
        this.updateLogDisplay();
        this.log('日志已清空', 'info');
    }

    /**
     * 导出日志
     */
    exportLog() {
        const logData = this.state.logs.map(entry =>
            `${entry.timestamp} [${entry.type.toUpperCase()}] ${entry.message}`
        ).join('\n');

        const blob = new Blob([logData], { type: 'text/plain;charset=utf-8' });
        const url = URL.createObjectURL(blob);

        const a = document.createElement('a');
        a.href = url;
        a.download = `body_controller_log_${new Date().toISOString().slice(0, 10)}.txt`;
        document.body.appendChild(a);
        a.click();
        document.body.removeChild(a);

        URL.revokeObjectURL(url);
        this.log('日志已导出', 'success');
    }
}

// 创建全局应用实例
let app;

// 页面加载完成后初始化应用
document.addEventListener('DOMContentLoaded', () => {
    app = new BodyControllerApp();
});

// 导出应用实例供全局使用
window.app = app;
