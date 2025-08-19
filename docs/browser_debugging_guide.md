# 浏览器开发者工具调试指南

## 🔧 开发者工具基础

### 打开开发者工具
- **Chrome/Edge**: `F12` 或 `Ctrl+Shift+I`
- **Firefox**: `F12` 或 `Ctrl+Shift+I`
- **Safari**: `Cmd+Option+I` (需先启用开发菜单)

## 📡 网络请求调试

### 1. Network (网络) 标签

#### **查看API请求**
1. 打开 Network 标签
2. 刷新页面或执行操作
3. 查看请求列表

#### **关键信息检查**
- **状态码**: 200(成功), 404(未找到), 500(服务器错误)
- **响应时间**: 正常应小于1秒
- **请求头**: Content-Type, Authorization等
- **响应内容**: JSON数据格式

#### **常见问题诊断**
```
状态码 | 含义 | 可能原因
------|------|----------
200   | 成功 | 请求正常
404   | 未找到 | API端点错误或后端未启动
500   | 服务器错误 | 后端代码异常
0     | 网络错误 | 连接失败或CORS问题
```

### 2. WebSocket 连接监控

#### **查看WebSocket连接**
1. 在 Network 标签中查找 `WS` 类型的连接
2. 点击连接查看详细信息
3. 切换到 `Messages` 查看消息流

#### **WebSocket状态**
- **Connecting**: 正在连接
- **Open**: 连接已建立
- **Closing**: 正在关闭
- **Closed**: 连接已关闭

## 🐛 JavaScript 调试

### 1. Console (控制台) 标签

#### **查看错误信息**
- **红色错误**: JavaScript运行时错误
- **黄色警告**: 非致命性问题
- **蓝色信息**: 调试信息

#### **手动测试API**
```javascript
// 测试健康检查
fetch('/api/health')
  .then(response => response.json())
  .then(data => console.log('健康检查:', data))
  .catch(error => console.error('错误:', error));

// 测试车门控制
fetch('/api/door/lock', {
  method: 'POST',
  headers: {
    'Content-Type': 'application/json'
  },
  body: JSON.stringify({
    doorID: 0,
    command: 1
  })
})
.then(response => response.json())
.then(data => console.log('车门控制:', data));

// 测试WebSocket连接
const ws = new WebSocket('ws://localhost:8081');
ws.onopen = () => console.log('WebSocket连接成功');
ws.onmessage = (event) => console.log('收到消息:', event.data);
ws.onerror = (error) => console.error('WebSocket错误:', error);
```

### 2. Sources (源代码) 标签

#### **设置断点调试**
1. 找到JavaScript文件 (如 `app.js`)
2. 点击行号设置断点
3. 执行操作触发断点
4. 使用调试控制按钮:
   - **Continue** (F8): 继续执行
   - **Step Over** (F10): 单步执行
   - **Step Into** (F11): 进入函数
   - **Step Out** (Shift+F11): 跳出函数

#### **查看变量值**
- **Scope**: 查看当前作用域变量
- **Watch**: 添加监视表达式
- **Call Stack**: 查看调用堆栈

## 🔍 具体调试场景

### 场景1: API请求失败

#### **症状**
- 界面显示"连接失败"
- 控制台有网络错误

#### **调试步骤**
1. 打开 Network 标签
2. 尝试执行操作
3. 查看失败的请求:
   ```
   如果状态码是 0 或 ERR_CONNECTION_REFUSED:
   - 检查后端服务是否启动
   - 确认端口号是否正确
   
   如果状态码是 404:
   - 检查API端点URL是否正确
   - 确认路由配置
   
   如果状态码是 500:
   - 查看后端服务器日志
   - 检查请求参数格式
   ```

### 场景2: WebSocket连接问题

#### **症状**
- 实时状态不更新
- 连接状态显示"未连接"

#### **调试步骤**
1. 在 Network 标签查找 WebSocket 连接
2. 检查连接状态和错误信息
3. 在 Console 中手动测试:
   ```javascript
   // 检查WebSocket支持
   if (typeof WebSocket !== 'undefined') {
     console.log('浏览器支持WebSocket');
   } else {
     console.error('浏览器不支持WebSocket');
   }
   
   // 测试连接
   const testWs = new WebSocket('ws://localhost:8081');
   testWs.onopen = () => console.log('连接成功');
   testWs.onerror = (e) => console.error('连接失败:', e);
   ```

### 场景3: 界面功能异常

#### **症状**
- 按钮点击无响应
- 状态显示不正确

#### **调试步骤**
1. 检查 Console 是否有JavaScript错误
2. 在 Sources 中设置断点:
   ```javascript
   // 在按钮点击处理函数设置断点
   async lockDoor(doorID, command) {
     debugger; // 手动断点
     // ... 函数代码
   }
   ```
3. 检查事件绑定:
   ```javascript
   // 在Console中检查元素事件
   $0.onclick // 查看点击事件
   getEventListeners($0) // 查看所有事件监听器
   ```

## 🛠️ 高级调试技巧

### 1. 网络限制测试

#### **模拟慢网络**
1. 打开 Network 标签
2. 点击 "No throttling" 下拉菜单
3. 选择 "Slow 3G" 或 "Fast 3G"
4. 测试应用在慢网络下的表现

### 2. 移动设备模拟

#### **响应式测试**
1. 按 `Ctrl+Shift+M` 开启设备模拟
2. 选择不同设备尺寸
3. 测试触摸交互和布局

### 3. 性能分析

#### **Performance 标签**
1. 点击录制按钮
2. 执行操作
3. 停止录制查看性能报告
4. 分析:
   - **FCP**: 首次内容绘制
   - **LCP**: 最大内容绘制
   - **JavaScript执行时间**
   - **网络请求时间**

## 📋 调试检查清单

### 连接测试
- [ ] 后端服务正在运行 (端口8080, 8081)
- [ ] 浏览器可以访问 http://localhost:8080
- [ ] Network标签显示成功的API请求
- [ ] WebSocket连接状态为 "Open"

### 功能测试
- [ ] 车门控制按钮响应正常
- [ ] 车窗滑块可以拖动
- [ ] 灯光控制状态正确更新
- [ ] 座椅调节功能正常
- [ ] 实时状态更新工作

### 错误排查
- [ ] Console无JavaScript错误
- [ ] Network无失败请求
- [ ] WebSocket消息正常收发
- [ ] 所有API端点返回正确状态码

## 🔧 常用调试命令

### Console 快捷命令
```javascript
// 查看应用状态
window.app.state

// 手动触发连接测试
window.app.testConnections()

// 查看WebSocket状态
window.app.ws.readyState

// 清空日志
window.app.clearLog()

// 切换主题
window.app.toggleTheme()

// 查看配置
window.AppConfig
```

### 网络请求监控
```javascript
// 监控所有fetch请求
const originalFetch = window.fetch;
window.fetch = function(...args) {
  console.log('Fetch请求:', args);
  return originalFetch.apply(this, args)
    .then(response => {
      console.log('Fetch响应:', response);
      return response;
    });
};
```

## 🚨 常见错误及解决方案

### CORS错误
```
Access to fetch at 'http://localhost:8080/api/health' from origin 'http://localhost:3000' has been blocked by CORS policy
```
**解决方案**: 确保后端设置了正确的CORS头，或使用同源访问

### WebSocket连接被拒绝
```
WebSocket connection to 'ws://localhost:8081/' failed: Error in connection establishment
```
**解决方案**: 检查WebSocket服务器是否启动，端口是否正确

### JavaScript模块错误
```
Uncaught SyntaxError: Cannot use import statement outside a module
```
**解决方案**: 检查script标签是否设置了 `type="module"`

通过这些调试技巧，您可以快速定位和解决前后端集成中的问题！
