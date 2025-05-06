## Client 向 Server 註冊

```json
{
    "client_version": "1.0.0",
    "message_type": "register",
    "message_id": "f47ac10b-58cc-4372-a567-0e02b2c3d479",
    "serial_number": "RED-9F1A2B3C-4D5E6F70-1A2B3C4D-5E6F7081",
    "timestamp": "2025-05-06T17:05:23+08:00"
}
```

---

## Client 向 Server 發送 event

```json
{
    "client_version": "1.0.0",
    "message_type": "event",
    "message_id": "f47ac10b-58cc-4372-a567-0e02b2c3d479",
    "serial_number": "RED-9F1A2B3C-4D5E6F70-1A2B3C4D-5E6F7081",
    "timestamp": "2025-05-06T17:05:23+08:00",
    "camera_id": "camera-01",
    "event": {
        "type": "fall_detected",    // 事件類型
        "severity": "critical",     // 嚴重度
        "confidence": 0.92,         // 偵測信心度 0-1
        "duration_ms": 450          // 動作持續時間
    }
}
```