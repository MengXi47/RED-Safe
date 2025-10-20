CREATE EXTENSION IF NOT EXISTS pgcrypto;

-- Server

CREATE TABLE edge_devices (
    edge_id             VARCHAR(12) PRIMARY KEY, 
    edge_password_hash  TEXT        NOT NULL,
    version             VARCHAR(32) NOT NULL,
    registered_at       TIMESTAMPTZ NOT NULL DEFAULT NOW(),
	last_online_at    TIMESTAMPTZ,
    CHECK (
        edge_id ~ '^RED-[0-9A-F]{8}$'
    )
);

CREATE TABLE auth_users (
    user_id          UUID        PRIMARY KEY DEFAULT gen_random_uuid(),
    email            TEXT        NOT NULL UNIQUE,
    email_is_verified  BOOLEAN        NOT NULL DEFAULT FALSE,
    email_verified_at  TIMESTAMPTZ,
    password_hash    TEXT        NOT NULL,
    created_at       TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at       TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    last_login_at    TIMESTAMPTZ,
    last_password_at TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    otp_secret       TEXT,
    otp_enabled      BOOLEAN     NOT NULL DEFAULT FALSE,
    otp_backup_codes TEXT[],
    otp_enabled_at    TIMESTAMPTZ,
    revoked BOOLEAN NOT NULL DEFAULT FALSE
);

CREATE TABLE user_profiles (
    user_id      UUID PRIMARY KEY DEFAULT gen_random_uuid(),
    email        TEXT NOT NULL UNIQUE,
    user_name    TEXT,
    avatar_url   TEXT,
    locale       VARCHAR(16) DEFAULT 'zh-TW',
    timezone     VARCHAR(64) DEFAULT 'Asia/Taipei',
    created_at   TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    updated_at   TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE user_edge_bindings (
    edge_id         VARCHAR(12)     NOT NULL,
    user_id         UUID            NOT NULL,
    display_name    TEXT,
    bind_at         TIMESTAMP WITH TIME ZONE DEFAULT now(),
    PRIMARY KEY (edge_id, user_id)
);

CREATE TABLE user_notification_settings (
    user_id       UUID      PRIMARY KEY,
    email_enabled BOOLEAN   NOT NULL DEFAULT TRUE,
    sms_enabled   BOOLEAN   NOT NULL DEFAULT FALSE,
    apns_enabled  BOOLEAN   NOT NULL DEFAULT FALSE,
    updated_at    TIMESTAMPTZ NOT NULL DEFAULT NOW()
);

CREATE TABLE auth_refresh_tokens (
    refresh_token   TEXT        PRIMARY KEY,
    user_id         UUID        NOT NULL,
    created_at      TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    expires_at      TIMESTAMPTZ NOT NULL,
    revoked         BOOLEAN     NOT NULL DEFAULT FALSE
);

-- Edge Device Configuration Tables

CREATE TABLE connected_ipc (
    ip_address INET NOT NULL PRIMARY KEY, 
    mac_address VARCHAR(17) NOT NULL,
    ipc_name TEXT, 
    custom_name TEXT,
    ipc_account TEXT,
    ipc_password TEXT
);

CREATE TABLE config (
    edge_id VARCHAR(12) PRIMARY KEY, 
    edge_password TEXT
);