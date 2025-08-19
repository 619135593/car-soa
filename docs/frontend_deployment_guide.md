# 前端Web界面部署和构建指南

## 🚀 部署方案概述

车身域控制器系统前端采用**静态文件 + HTTP服务器**的部署方式，与后端REST API和WebSocket服务器集成。

## 📁 前端文件结构

```
web/
├── index.html              # 主页面（使用index_new.html）
├── css/
│   └── style.css          # 主样式文件（使用style_new.css）
├── js/
│   ├── api.js            # API通信模块
│   ├── websocket.js      # WebSocket通信模块
│   └── app.js            # 主应用程序（使用app_new.js）
├── images/               # 图标和图片资源
└── fonts/               # 字体文件（可选）
```

## 🔧 与CMake构建系统集成

### 1. 使用现有的web/CMakeLists.txt

前端文件已经集成到CMake构建系统中，构建时会自动处理：

```bash
# 构建项目
mkdir build && cd build
cmake ..
make -j$(nproc)

# 前端文件会被复制到 build/web/ 目录
```

### 2. 前端文件处理

CMake构建过程中会：
- 复制HTML、CSS、JavaScript文件到构建目录
- 处理模板文件中的变量替换
- 生成Web资源清单文件
- 创建开发服务器脚本

## 🌐 HTTP服务器配置

### 方案1：使用内置HTTP服务器（推荐）

后端已经包含HTTP服务器，可以直接提供静态文件：

```cpp
// 在body_controller_web_server中配置静态文件服务
server.set_mount_point("/", "./web");
```

### 方案2：使用Nginx反向代理

```nginx
server {
    listen 80;
    server_name localhost;
    
    # 静态文件服务
    location / {
        root /path/to/build/web;
        index index.html;
        try_files $uri $uri/ /index.html;
    }
    
    # API代理
    location /api/ {
        proxy_pass http://localhost:8080/api/;
        proxy_http_version 1.1;
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
    
    # WebSocket代理
    location /ws {
        proxy_pass http://localhost:8081;
        proxy_http_version 1.1;
        proxy_set_header Upgrade $http_upgrade;
        proxy_set_header Connection "Upgrade";
        proxy_set_header Host $host;
        proxy_set_header X-Real-IP $remote_addr;
        proxy_set_header X-Forwarded-For $proxy_add_x_forwarded_for;
        proxy_set_header X-Forwarded-Proto $scheme;
    }
    
    # 缓存策略
    location ~* \.(js|css|png|jpg|jpeg|gif|ico|svg)$ {
        expires 1y;
        add_header Cache-Control "public, immutable";
        add_header Access-Control-Allow-Origin "*";
    }
    
    # 安全头
    add_header X-Frame-Options "SAMEORIGIN" always;
    add_header X-Content-Type-Options "nosniff" always;
    add_header X-XSS-Protection "1; mode=block" always;
}
```

### 方案3：使用Apache HTTP服务器

```apache
<VirtualHost *:80>
    ServerName localhost
    DocumentRoot /path/to/build/web
    
    # 静态文件服务
    <Directory "/path/to/build/web">
        Options Indexes FollowSymLinks
        AllowOverride None
        Require all granted
        
        # SPA路由支持
        RewriteEngine On
        RewriteBase /
        RewriteRule ^index\.html$ - [L]
        RewriteCond %{REQUEST_FILENAME} !-f
        RewriteCond %{REQUEST_FILENAME} !-d
        RewriteRule . /index.html [L]
    </Directory>
    
    # API代理
    ProxyPreserveHost On
    ProxyPass /api/ http://localhost:8080/api/
    ProxyPassReverse /api/ http://localhost:8080/api/
    
    # WebSocket代理
    ProxyPass /ws ws://localhost:8081/
    ProxyPassReverse /ws ws://localhost:8081/
    
    # 缓存配置
    <LocationMatch "\.(css|js|png|jpg|jpeg|gif|ico|svg)$">
        ExpiresActive On
        ExpiresDefault "access plus 1 year"
        Header append Cache-Control "public, immutable"
    </LocationMatch>
</VirtualHost>
```

## 🔄 开发环境配置

### 1. 开发服务器

使用Python内置服务器进行开发：

```bash
# 进入Web目录
cd build/web

# 启动开发服务器
python3 -m http.server 8000

# 或使用Node.js
npx http-server -p 8000 -c-1
```

### 2. 实时重载（可选）

使用live-server进行开发：

```bash
# 安装live-server
npm install -g live-server

# 启动实时重载服务器
cd build/web
live-server --port=8000 --host=localhost
```

### 3. 开发环境API配置

开发时可能需要修改API端点：

```javascript
// 在js/api.js中配置开发环境
const API_BASE_URL = process.env.NODE_ENV === 'development' 
    ? 'http://localhost:8080/api'
    : '/api';

const WS_URL = process.env.NODE_ENV === 'development'
    ? 'ws://localhost:8081'
    : `ws://${window.location.host}/ws`;
```

## 🏭 生产环境部署

### 1. 文件优化

```bash
# 压缩JavaScript文件
uglifyjs js/app.js -o js/app.min.js -c -m

# 压缩CSS文件
cleancss css/style.css -o css/style.min.css

# 优化图片
imagemin images/* --out-dir=images/optimized
```

### 2. 启动脚本

创建系统服务脚本：

```bash
#!/bin/bash
# /etc/systemd/system/body-controller-web.service

[Unit]
Description=Body Controller Web Server
After=network.target

[Service]
Type=simple
User=body-controller
WorkingDirectory=/opt/body-controller
ExecStart=/opt/body-controller/bin/body_controller_web_server
Restart=always
RestartSec=10

[Install]
WantedBy=multi-user.target
```

### 3. 部署检查清单

- [ ] 后端服务正常运行（端口8080、8081）
- [ ] 前端文件正确部署到Web目录
- [ ] HTTP服务器配置正确
- [ ] API代理配置正确
- [ ] WebSocket代理配置正确
- [ ] 防火墙规则配置
- [ ] SSL证书配置（生产环境）
- [ ] 日志配置
- [ ] 监控配置

## 🔒 安全配置

### 1. HTTPS配置

```nginx
server {
    listen 443 ssl http2;
    server_name your-domain.com;
    
    ssl_certificate /path/to/certificate.crt;
    ssl_certificate_key /path/to/private.key;
    ssl_protocols TLSv1.2 TLSv1.3;
    ssl_ciphers ECDHE-RSA-AES256-GCM-SHA512:DHE-RSA-AES256-GCM-SHA512;
    ssl_prefer_server_ciphers off;
    
    # 其他配置...
}

# HTTP重定向到HTTPS
server {
    listen 80;
    server_name your-domain.com;
    return 301 https://$server_name$request_uri;
}
```

### 2. 安全头配置

```nginx
# 安全头
add_header Strict-Transport-Security "max-age=31536000; includeSubDomains" always;
add_header X-Frame-Options "SAMEORIGIN" always;
add_header X-Content-Type-Options "nosniff" always;
add_header X-XSS-Protection "1; mode=block" always;
add_header Referrer-Policy "strict-origin-when-cross-origin" always;
add_header Content-Security-Policy "default-src 'self'; script-src 'self' 'unsafe-inline'; style-src 'self' 'unsafe-inline'; connect-src 'self' ws: wss:;" always;
```

## 📊 监控和日志

### 1. 访问日志

```nginx
# Nginx访问日志格式
log_format body_controller '$remote_addr - $remote_user [$time_local] '
                          '"$request" $status $body_bytes_sent '
                          '"$http_referer" "$http_user_agent" '
                          '$request_time $upstream_response_time';

access_log /var/log/nginx/body-controller-access.log body_controller;
error_log /var/log/nginx/body-controller-error.log;
```

### 2. 性能监控

```javascript
// 在前端添加性能监控
window.addEventListener('load', () => {
    const perfData = performance.getEntriesByType('navigation')[0];
    console.log('Page load time:', perfData.loadEventEnd - perfData.fetchStart);
    
    // 发送性能数据到监控系统
    if (window.navigator.sendBeacon) {
        const data = JSON.stringify({
            loadTime: perfData.loadEventEnd - perfData.fetchStart,
            domContentLoaded: perfData.domContentLoadedEventEnd - perfData.fetchStart,
            firstPaint: performance.getEntriesByType('paint')[0]?.startTime
        });
        navigator.sendBeacon('/api/metrics/performance', data);
    }
});
```

## 🧪 测试和验证

### 1. 功能测试

```bash
# 测试静态文件访问
curl -I http://localhost:8080/

# 测试API访问
curl http://localhost:8080/api/health

# 测试WebSocket连接
wscat -c ws://localhost:8081
```

### 2. 性能测试

```bash
# 使用Apache Bench测试
ab -n 1000 -c 10 http://localhost:8080/

# 使用wrk测试
wrk -t12 -c400 -d30s http://localhost:8080/
```

### 3. 浏览器兼容性测试

- Chrome 80+
- Firefox 75+
- Edge 80+
- Safari 13+（macOS）

## 🔄 更新和维护

### 1. 版本更新流程

```bash
# 1. 备份当前版本
cp -r /opt/body-controller/web /opt/body-controller/web.backup

# 2. 部署新版本
cp -r build/web/* /opt/body-controller/web/

# 3. 重启服务
systemctl restart body-controller-web

# 4. 验证部署
curl -I http://localhost:8080/
```

### 2. 回滚流程

```bash
# 如果新版本有问题，快速回滚
rm -rf /opt/body-controller/web
mv /opt/body-controller/web.backup /opt/body-controller/web
systemctl restart body-controller-web
```

## 📋 故障排除

### 常见问题

1. **静态文件404错误**
   - 检查文件路径和权限
   - 验证HTTP服务器配置

2. **API请求失败**
   - 检查后端服务状态
   - 验证代理配置

3. **WebSocket连接失败**
   - 检查WebSocket服务器状态
   - 验证防火墙规则

4. **页面加载缓慢**
   - 启用Gzip压缩
   - 配置静态文件缓存
   - 优化图片和资源

### 调试工具

- 浏览器开发者工具
- Nginx/Apache错误日志
- 网络抓包工具（Wireshark）
- 性能分析工具（Lighthouse）
