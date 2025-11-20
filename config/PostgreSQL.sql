CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- Server

CREATE TABLE edge_devices (
    edge_id             VARCHAR(12) PRIMARY KEY, 
    edge_password_hash  TEXT        NOT NULL,
    version             VARCHAR(32) NOT NULL,
    registered_at       TIMESTAMPTZ NOT NULL DEFAULT NOW(),
	last_online_at      TIMESTAMPTZ,
    CHECK (
        edge_id ~ '^RED-[0-9A-F]{8}$'
    )
);

CREATE TABLE auth_users (
    user_id             UUID            PRIMARY KEY DEFAULT gen_random_uuid(),
    email               TEXT            NOT NULL UNIQUE,
    email_is_verified   BOOLEAN         NOT NULL DEFAULT FALSE,
    email_verified_at   TIMESTAMPTZ,
    password_hash       TEXT            NOT NULL,
    created_at          TIMESTAMPTZ     NOT NULL DEFAULT NOW(),
    updated_at          TIMESTAMPTZ     NOT NULL DEFAULT NOW(),
    last_login_at       TIMESTAMPTZ,
    last_password_at    TIMESTAMPTZ     NOT NULL DEFAULT NOW(),
    otp_secret          TEXT,
    otp_enabled         BOOLEAN         NOT NULL DEFAULT FALSE,
    otp_backup_codes    TEXT[],
    otp_enabled_at      TIMESTAMPTZ,
    revoked             BOOLEAN         NOT NULL DEFAULT FALSE
);

CREATE TABLE user_profiles (
    user_id      UUID           PRIMARY KEY DEFAULT gen_random_uuid(),
    email        TEXT           NOT NULL UNIQUE,
    user_name    TEXT,
    avatar_url   TEXT,
    locale       VARCHAR(16)    DEFAULT 'zh-TW',
    timezone     VARCHAR(64)    DEFAULT 'Asia/Taipei',
    created_at   TIMESTAMPTZ    NOT NULL DEFAULT NOW(),
    updated_at   TIMESTAMPTZ    NOT NULL DEFAULT NOW()
);

CREATE TABLE user_edge_bindings (
    edge_id         VARCHAR(12)     NOT NULL,
    user_id         UUID            NOT NULL,
    display_name    TEXT,
    bind_at         TIMESTAMP WITH TIME ZONE DEFAULT now(),
    PRIMARY KEY (edge_id, user_id)
);

CREATE TABLE user_notification_settings (
    user_id       UUID          PRIMARY KEY,
    email_enabled BOOLEAN       NOT NULL DEFAULT TRUE,
    sms_enabled   BOOLEAN       NOT NULL DEFAULT FALSE,
    apns_enabled  BOOLEAN       NOT NULL DEFAULT FALSE,
    updated_at    TIMESTAMPTZ   NOT NULL DEFAULT NOW()
);

CREATE TABLE auth_refresh_tokens (
    refresh_token   TEXT            PRIMARY KEY,
    user_id         UUID            NOT NULL,
    created_at      TIMESTAMPTZ     NOT NULL DEFAULT NOW(),
    expires_at      TIMESTAMPTZ     NOT NULL,
    revoked         BOOLEAN         NOT NULL DEFAULT FALSE
);

CREATE TABLE mail_log (
    id              BIGSERIAL       PRIMARY KEY,
    from_address    TEXT            NOT NULL,
    to_address      TEXT            NOT NULL,
    subject         TEXT            NOT NULL,
    body            TEXT            NOT NULL,
    status          BOOLEAN         NOT NULL,
    error_message   TEXT,
    created_at      TIMESTAMPTZ     NOT NULL DEFAULT NOW(),
    send_at         TIMESTAMPTZ
);

-- Edge Device Configuration Tables

CREATE TABLE config (
    edge_id VARCHAR(12) PRIMARY KEY, 
    edge_password TEXT
);

CREATE TABLE connected_ipc (
    ip_address INET NOT NULL PRIMARY KEY, 
    mac_address VARCHAR(17) NOT NULL,
    ipc_name TEXT, 
    custom_name TEXT,
    ipc_account TEXT,
    ipc_password TEXT
);

-- Fall detection settings per IPC
CREATE TABLE ipc_fall_detection_policy (
    ip_address INET PRIMARY KEY REFERENCES connected_ipc(ip_address) ON DELETE CASCADE,
    enabled BOOLEAN NOT NULL DEFAULT FALSE,
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Inactivity detection settings per IPC
CREATE TABLE ipc_inactivity_policy (
    ip_address INET PRIMARY KEY REFERENCES connected_ipc(ip_address) ON DELETE CASCADE,
    enabled BOOLEAN NOT NULL DEFAULT FALSE,
    idle_minutes INTEGER NOT NULL DEFAULT 5,    -- 持續靜止超過此分鐘數觸發
    quiet_start TIME,                           -- 每日免偵測開始時間（可為 NULL 表示不設）
    quiet_end TIME,                             -- 每日免偵測結束時間（可為 NULL 表示不設）
    quiet_enabled BOOLEAN NOT NULL DEFAULT FALSE, -- 是否啟用免偵測時段
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Bed ROI settings per IPC（僅供夜間離床判斷）
CREATE TABLE ipc_bed_roi_zones (
    ip_address INET PRIMARY KEY REFERENCES connected_ipc(ip_address) ON DELETE CASCADE,
    points JSONB NOT NULL,                       -- 正規化座標點列表（[{ "x":0.1,"y":0.2 }, ...]）
    enabled BOOLEAN NOT NULL DEFAULT TRUE,
    quiet_start TIME,                            -- 夜間離床偵測開始時間
    quiet_end TIME,                              -- 夜間離床偵測結束時間
    updated_at TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

-- Edge 偵測事件紀錄（僅留時間與事件類型，必要時可附加內容）
CREATE TABLE edge_events (
    id BIGSERIAL PRIMARY KEY,
    occurred_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    event_type TEXT NOT NULL,        -- 事件名稱，如 fall_detected / bed_exit
    payload JSONB                    -- 選填：附加屬性或原始資料
);
