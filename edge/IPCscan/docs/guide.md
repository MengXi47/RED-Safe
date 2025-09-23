# IPCscan 服務說明

## 功能概述
- 掃描區域網路中的 ONVIF 相機，收集 IP、MAC、裝置名稱
- 透過 WS-Discovery 主動發出 Probe，並從回應解析 Scopes 資訊
- 若回應缺少 MAC，伺服器會利用本機 ARP 快取補齊
- 內建 gRPC 伺服器（預設埠號 20001），提供被動式掃描 API
- 回應結果以 JSON 字串（陣列格式）返回，無裝置時回傳空字串

## 系統架構與運用技術
- **C++26**：核心掃描與網路處理
- **WS-Discovery / ONVIF**：尋找相機所使用的通訊協定
- **UDP Socket**：廣播 Probe 封包並接收回應
- **ARP / sysctl**：在 Linux/macOS 上解析裝置 MAC 位址
- **gRPC + Protocol Buffers**：建立跨平台的掃描服務介面
  - 服務名稱：`ipcscan.grpc.IPCScanService`
  - 方法：`Scan(ScanRequest) -> ScanResponse`

## 使用方式
1. 啟動 IPCscan gRPC 伺服器：
   ```bash
   ./bin/IPCscan_bin
   ```
   終端機會顯示：`IPCscan gRPC伺服器啟動於埠 20001，等待掃描請求...`

2. 以 gRPC Client 發送請求（範例使用 grpcurl）：
   ```bash
   grpcurl -plaintext -d '{}' localhost:20001 ipcscan.grpc.IPCScanService/Scan
   ```

3. 回應格式：
   ```json
   [
     {"ip":"192.168.1.20","mac":"F4:2A:7D:40:A8:1F","name":"IPC"},
     {"ip":"192.168.1.21","mac":"60:3A:7C:8C:AE:FC","name":"TP-IPC"}
   ]
   ```
   若未找到任何相機則回傳空字串 `""`。

## gRPC API 詳細描述
- **請求**：`ScanRequest`（無欄位）
- **回應**：`ScanResponse`
  - `result`：字串型別，包含掃描結果 JSON 文字或空字串
- **流程**：
  1. 伺服器接收到 `Scan` 請求後會即時觸發掃描流程並輸出「收到gRPC掃描請求」
  2. 掃描完成後回傳結果，不保留緩存或背景任務


