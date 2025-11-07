# edge_web RTSP 預覽 WebRTC 邏輯分析

本文說明 Edge Web 管理介面中，RTSP 預覽透過 WebRTC 播放的完整流程，涵蓋後端 Django 服務與前端 Vue 元件協作方式，並整理關鍵模組與錯誤處理策略。

## 後端（Django / aiortc）

### 背景執行緒與資源管理
* `ui/views.py` 透過 `_ensure_preview_backend()` 檢查 `aiortc` 是否可用，並在首次呼叫時建立專用的 asyncio event loop 與背景執行緒（`webrtc-preview-loop`），所有 WebRTC 相關的 coroutine 都會透過 `_run_in_preview_loop()` 安排在該 loop 中執行。這可避免 Django 同步視圖直接操作非同步 API。【F:edge_web/ui/views.py†L1134-L1171】
* `_preview_sessions` 字典追蹤每個 WebRTC 會話的 `RTCPeerConnection` 與 `MediaPlayer` 實例，並透過 `_session_watchdog()` 在超過預設 180 秒 TTL 後自動清除，確保閒置連線不會佔用資源。【F:edge_web/ui/views.py†L1128-L1133】【F:edge_web/ui/views.py†L1193-L1206】

### 建立 RTSP 串流與 WebRTC Answer
* `_create_webrtc_session()` 會為每次預覽生成 session ID 與 `RTCPeerConnection`，並使用 `MediaPlayer` 從指定的 RTSP URL 拉流。若無視訊 track 或播放器初始化失敗，會拋出 `PreviewStreamError` 以回傳對應訊息。成功時會將攝影機影像透過 `MediaRelay` 訂閱後加入 PeerConnection，並監聽 `connectionstatechange` 以在連線關閉時釋放資源。【F:edge_web/ui/views.py†L1173-L1217】
* `_handle_preview_offer()` 接收前端送出的 SDP offer，設定遠端描述後產生 answer。函式同時等待 ICE Gathering 完成（最長 3 秒），確保回答中包含必要的候選。任何錯誤都會中止 session 並釋放資源。【F:edge_web/ui/views.py†L1219-L1253】

### API 端點
* `/api/cameras/preview/probe` 使用 `ffmpeg` 嘗試抓取單張影像，藉此檢測 RTSP 是否可連線，以及帳密是否正確。錯誤訊息會轉換為標準代碼，例如認證失敗（`AUTH_REQUIRED`）或逾時（`TIMEOUT`）。【F:edge_web/ui/views.py†L1258-L1326】
* `/api/cameras/preview/webrtc/offer` 會解析前端傳來的 IP 與帳密，組成 RTSP URL 後進行 `_handle_preview_offer()`，成功時回傳 session ID 與 WebRTC answer，失敗則根據 `PreviewStreamError` 映射狀態碼並紀錄行為日誌。【F:edge_web/ui/views.py†L1328-L1450】
* `/api/cameras/preview/webrtc/hangup` 接收 session ID，透過背景 loop 執行 `_close_preview_session()`。若 session 已不存在會回傳 `SESSION_NOT_FOUND`，否則確認釋放播放器與 PeerConnection。【F:edge_web/ui/views.py†L1452-L1506】

## 前端（Vue 3 + TypeScript）

### 預覽流程 Hook
* `useCameraPreview` composable 管理整個預覽生命周期，內含 session ID、`RTCPeerConnection` 與帳密暫存。呼叫 `openPreview()` 時會顯示預覽視窗並記錄選取攝影機。【F:edge_web/frontend/src/composables/useCameraManagement.ts†L116-L160】【F:edge_web/frontend/src/composables/useCameraManagement.ts†L202-L216】
* `startPreview()` 首先執行 `ensureProbe()`：先以空帳密呼叫 `/preview/probe` 測試，若後端回覆需要認證便透過 `window.prompt` 取得帳密並再次測試。若探測通過，建立 `RTCPeerConnection`（預設加入 `recvonly` 視訊 transceiver 並設定 Google STUN 伺服器），產生本地 offer、送往 `/preview/webrtc/offer`，最後將 answer 設為遠端描述並保存 session ID。【F:edge_web/frontend/src/composables/useCameraManagement.ts†L162-L238】【F:edge_web/frontend/src/composables/useCameraManagement.ts†L218-L261】
* PeerConnection 的 `track` 事件會把串流交給 `options.onStream`，而 `iceconnectionstatechange` 失敗時會觸發錯誤提示並停止預覽。`stopPreview()` 則會停止所有 track、關閉 PeerConnection、通知後端 hangup，並清除暫存資訊。組件作用域結束或使用者關閉視窗時同樣會呼叫 `stopPreview()`。【F:edge_web/frontend/src/composables/useCameraManagement.ts†L219-L247】【F:edge_web/frontend/src/composables/useCameraManagement.ts†L263-L304】

### 預覽對話框元件
* `CameraPreviewModal` 透過 `BaseModal` 顯示預覽，內建 Loading 與錯誤覆蓋層。當 `open` 變為 `true` 時發出 `start` 事件啟動連線；關閉時會發出 `stop` 與 `close` 以確保資源釋放。`attachStream()` 會把收到的 `MediaStream` 指派給 `<video>`，而 `handleError()` 則呈現錯誤訊息。【F:edge_web/frontend/src/components/cameras/CameraPreviewModal.vue†L1-L102】

### API 封裝
* `cameraService.ts` 提供 `previewProbe`、`previewOffer`、`previewHangup` 等函式，分別對應上述後端端點。Vue 組件與 composable 皆透過此服務層存取 HTTP API，維持邏輯集中。【F:edge_web/frontend/src/lib/services/cameraService.ts†L39-L84】

## 錯誤處理與日誌
* 後端所有關鍵 API 都會呼叫 `_log_web_action()` 寫入資料庫日誌，記錄行為、錯誤碼與攝影機資訊，方便追蹤部署狀況。`PreviewStreamError` 包含 `stderr` 與自訂代碼，可將外部程式（例如 `ffmpeg`）的訊息翻譯成使用者友善的提示。【F:edge_web/ui/views.py†L1328-L1450】
* 前端在接收到錯誤時會呼叫 `options.onError()`，由 `CameraPreviewModal` 顯示提示，同時於控制台輸出詳細資訊。`ensureProbe()` 對 `AUTH_REQUIRED` 特別處理，避免無帳密重複嘗試造成攝影機鎖定。【F:edge_web/frontend/src/composables/useCameraManagement.ts†L182-L238】

## 流程摘要
1. 使用者在攝影機列表點選「預覽」，`CameraPreviewModal` 開啟並觸發 `startPreview()`。
2. 前端呼叫 `/preview/probe` 測試 RTSP；若需帳密則提示使用者輸入，再次測試。
3. 通過探測後，前端建立 WebRTC offer，傳給 `/preview/webrtc/offer`。
4. 後端在背景 loop 中拉取 RTSP 並建立 PeerConnection，回傳 answer 與 session ID。
5. 前端設置遠端描述後開始播放，`track` 事件將串流掛載到 `<video>`。
6. 使用者關閉預覽時，前端停止 PeerConnection 並呼叫 `/preview/webrtc/hangup`，後端清理 MediaPlayer 與 session。

以上即為 edge_web RTSP 預覽的 WebRTC 主要邏輯，涵蓋資源管理、API 互動與前端控制流程。
