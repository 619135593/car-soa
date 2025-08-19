# 车身域控制器系统前端开发方案

## 🎯 技术栈选择

### 推荐方案：现代HTML/CSS/JavaScript + 轻量级框架

**选择理由**：
1. **轻量级**：无需复杂的构建工具，直接集成到CMake系统
2. **兼容性好**：支持所有现代浏览器
3. **开发效率高**：快速开发，易于维护
4. **部署简单**：静态文件直接通过HTTP服务器提供

### 技术组合
- **HTML5**：语义化结构
- **CSS3 + Flexbox/Grid**：响应式布局
- **Vanilla JavaScript (ES6+)**：核心逻辑
- **WebSocket API**：实时通信
- **Fetch API**：HTTP请求
- **CSS框架**：Bootstrap 5 或 Tailwind CSS（可选）

## 🎨 界面设计需求

### 主界面布局
```
┌─────────────────────────────────────────────────────────┐
│                    车身控制系统                          │
├─────────────────────────────────────────────────────────┤
│  连接状态: ● 已连接  |  系统状态: 正常  |  时间: 14:30:25  │
├─────────────────────────────────────────────────────────┤
│  ┌─────────────┐  ┌─────────────┐  ┌─────────────┐      │
│  │   车门控制   │  │   车窗控制   │  │   灯光控制   │      │
│  │             │  │             │  │             │      │
│  │  [前左] [前右] │  │  [前左] [前右] │  │  [前大灯]    │      │
│  │  [后左] [后右] │  │  [后左] [后右] │  │  [转向灯]    │      │
│  │             │  │             │  │  [位置灯]    │      │
│  └─────────────┘  └─────────────┘  └─────────────┘      │
│                                                         │
│  ┌─────────────┐  ┌─────────────────────────────────┐   │
│  │   座椅控制   │  │         活动日志                 │   │
│  │             │  │                                 │   │
│  │  [前后调节]  │  │  14:30:20 前左门已锁定           │   │
│  │  [靠背调节]  │  │  14:30:15 前左窗位置: 50%        │   │
│  │  [记忆位置]  │  │  14:30:10 前大灯已开启           │   │
│  └─────────────┘  └─────────────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

### 功能模块详细设计

#### 1. 车门控制模块
- **4个车门按钮**：前左、前右、后左、后右
- **状态显示**：锁定/解锁状态，开启/关闭状态
- **操作按钮**：锁定、解锁
- **状态指示器**：颜色编码（绿色=解锁，红色=锁定）

#### 2. 车窗控制模块
- **4个车窗控制**：前左、前右、后左、后右
- **位置滑块**：0-100%位置控制
- **快捷按钮**：上升、下降、停止
- **位置显示**：实时位置百分比

#### 3. 灯光控制模块
- **前大灯控制**：关闭、近光、远光
- **转向灯控制**：关闭、左转、右转、双闪
- **位置灯控制**：开启、关闭
- **状态指示**：图标和颜色显示

#### 4. 座椅控制模块
- **调节控制**：前后、靠背角度
- **记忆位置**：保存、恢复（1-3号位置）
- **调节按钮**：方向控制和停止

#### 5. 系统状态模块
- **连接状态**：WebSocket连接状态
- **服务状态**：各个服务的可用性
- **活动日志**：实时操作和事件日志
- **错误提示**：操作失败和系统错误

## 📱 响应式设计

### 桌面端 (≥1024px)
- 4列布局：车门、车窗、灯光、座椅
- 右侧活动日志面板
- 顶部状态栏

### 平板端 (768px-1023px)
- 2列布局：上排车门+车窗，下排灯光+座椅
- 底部活动日志
- 顶部状态栏

### 移动端 (≤767px)
- 单列布局：垂直排列所有模块
- 可折叠的活动日志
- 简化的状态栏

## 🎨 视觉设计规范

### 色彩方案
- **主色调**：深蓝色 (#1e3a8a)
- **辅助色**：浅蓝色 (#3b82f6)
- **成功色**：绿色 (#10b981)
- **警告色**：橙色 (#f59e0b)
- **错误色**：红色 (#ef4444)
- **中性色**：灰色系 (#6b7280, #f3f4f6)

### 字体规范
- **主字体**：'Segoe UI', 'Roboto', sans-serif
- **等宽字体**：'Consolas', 'Monaco', monospace
- **字体大小**：12px-24px，响应式调整

### 组件样式
- **圆角**：8px
- **阴影**：box-shadow: 0 2px 8px rgba(0,0,0,0.1)
- **间距**：8px, 16px, 24px, 32px
- **动画**：transition: all 0.3s ease

## 🔧 技术实现方案

### 项目结构
```
web/
├── index.html              # 主页面
├── css/
│   ├── style.css          # 主样式文件
│   ├── components.css     # 组件样式
│   └── responsive.css     # 响应式样式
├── js/
│   ├── app.js            # 主应用逻辑
│   ├── api.js            # API通信模块
│   ├── websocket.js      # WebSocket通信模块
│   ├── components/       # UI组件
│   │   ├── door-control.js
│   │   ├── window-control.js
│   │   ├── light-control.js
│   │   └── seat-control.js
│   └── utils/
│       ├── logger.js     # 日志工具
│       └── helpers.js    # 辅助函数
├── images/               # 图标和图片
└── fonts/               # 字体文件（如需要）
```

### 核心功能模块

#### 1. API通信模块 (api.js)
```javascript
class BodyControllerAPI {
    constructor(baseURL = 'http://localhost:8080/api') {
        this.baseURL = baseURL;
    }
    
    async request(endpoint, options = {}) {
        // HTTP请求封装
    }
    
    // 车门API
    async lockDoor(doorID, command) { }
    async getDoorStatus(doorID) { }
    
    // 车窗API
    async setWindowPosition(windowID, position) { }
    async controlWindow(windowID, command) { }
    async getWindowPosition(windowID) { }
    
    // 灯光API
    async setHeadlight(command) { }
    async setIndicator(command) { }
    async setPositionLight(command) { }
    
    // 座椅API
    async adjustSeat(axis, direction) { }
    async recallMemoryPosition(presetID) { }
    async saveMemoryPosition(presetID) { }
    
    // 系统API
    async getSystemInfo() { }
    async getHealthStatus() { }
}
```

#### 2. WebSocket通信模块 (websocket.js)
```javascript
class BodyControllerWebSocket {
    constructor(url = 'ws://localhost:8081') {
        this.url = url;
        this.ws = null;
        this.eventHandlers = new Map();
    }
    
    connect() {
        // WebSocket连接
    }
    
    subscribe(eventType, handler) {
        // 事件订阅
    }
    
    unsubscribe(eventType) {
        // 取消订阅
    }
    
    onMessage(event) {
        // 消息处理
    }
}
```

#### 3. UI组件模块
每个控制模块都有独立的JavaScript类：
- `DoorControl`：车门控制组件
- `WindowControl`：车窗控制组件
- `LightControl`：灯光控制组件
- `SeatControl`：座椅控制组件

### 状态管理
使用简单的观察者模式管理应用状态：
```javascript
class AppState {
    constructor() {
        this.state = {
            connected: false,
            doors: {},
            windows: {},
            lights: {},
            seat: {},
            logs: []
        };
        this.observers = [];
    }
    
    setState(newState) {
        this.state = { ...this.state, ...newState };
        this.notifyObservers();
    }
    
    subscribe(observer) {
        this.observers.push(observer);
    }
    
    notifyObservers() {
        this.observers.forEach(observer => observer(this.state));
    }
}
```

## 📊 开发优先级

### 第一阶段（核心功能）
1. 基础HTML结构和CSS样式
2. API通信模块
3. WebSocket连接模块
4. 车门控制功能

### 第二阶段（扩展功能）
1. 车窗控制功能
2. 灯光控制功能
3. 座椅控制功能
4. 活动日志显示

### 第三阶段（优化完善）
1. 响应式设计优化
2. 错误处理完善
3. 用户体验优化
4. 性能优化

## 🔍 用户体验设计

### 交互设计原则
1. **直观性**：图标和按钮含义清晰
2. **反馈性**：操作有即时视觉反馈
3. **一致性**：相同功能使用相同的交互方式
4. **容错性**：防止误操作，提供撤销机制

### 状态反馈
- **加载状态**：显示加载动画
- **成功状态**：绿色提示和图标变化
- **错误状态**：红色提示和错误信息
- **连接状态**：实时显示连接状态

### 无障碍设计
- **键盘导航**：支持Tab键导航
- **屏幕阅读器**：适当的ARIA标签
- **颜色对比**：确保足够的颜色对比度
- **字体大小**：支持浏览器字体缩放
