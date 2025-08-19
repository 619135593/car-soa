/**
 * Body Controller SSE Client (Server-Sent Events)
 * 使用浏览器原生 EventSource 订阅后端 /api/events 实时事件
 */

class BodyControllerSSE {
  /**
   * @param {string} url SSE端点，默认 /api/events
   */
  constructor(url = '/api/events') {
    this.url = url;
    this.source = null;
    this.eventHandlers = new Map();
    this.isConnected = false;

    this.onOpen = this.onOpen.bind(this);
    this.onMessage = this.onMessage.bind(this);
    this.onError = this.onError.bind(this);
  }

  /** 连接事件流 */
  connect() {
    if (!window.EventSource) {
      this.emit('error', new Error('当前浏览器不支持 EventSource')); return;
    }
    if (this.source && this.source.readyState === EventSource.OPEN) return;

    this.source = new EventSource(this.url);
    this.source.onopen = this.onOpen;
    this.source.onmessage = this.onMessage;
    this.source.onerror = this.onError;
  }

  /** 断开连接 */
  disconnect() {
    if (this.source) {
      this.source.onopen = null;
      this.source.onmessage = null;
      this.source.onerror = null;
      this.source.close();
      this.source = null;
    }
    this.isConnected = false;
    this.emit('disconnected');
  }

  onOpen(evt) {
    this.isConnected = true;
    this.emit('connected', evt);
  }

  onMessage(evt) {
    try {
      const msg = JSON.parse(evt.data);
      this.emit('message', msg);
      if (msg && msg.type) this.emit(msg.type, msg.data, msg.timestamp);
    } catch (e) {
      this.emit('error', e);
    }
  }

  onError(err) {
    this.isConnected = false;
    this.emit('error', err);
    // 由浏览器负责自动重连，无需手动处理
  }

  on(eventType, handler) {
    if (!this.eventHandlers.has(eventType)) this.eventHandlers.set(eventType, []);
    this.eventHandlers.get(eventType).push(handler);
  }

  off(eventType, handler) {
    if (!this.eventHandlers.has(eventType)) return;
    const arr = this.eventHandlers.get(eventType);
    const i = arr.indexOf(handler);
    if (i >= 0) arr.splice(i, 1);
  }

  emit(eventType, ...args) {
    const arr = this.eventHandlers.get(eventType);
    if (!arr) return;
    arr.forEach(fn => { try { fn(...args); } catch (e) { console.error(e); } });
  }
}

window.BodyControllerSSE = BodyControllerSSE;

