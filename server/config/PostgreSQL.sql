CREATE EXTENSION IF NOT EXISTS pgcrypto;

CREATE TABLE edge_devices (
    edge_id             VARCHAR(12) PRIMARY KEY, 
    edge_password_hash  TEXT    NOT NULL,
    version             VARCHAR(32) NOT NULL,
    registered_at       TIMESTAMPTZ NOT NULL DEFAULT NOW(),
	last_online_at    TIMESTAMPTZ,
    CHECK (
        edge_id ~ '^RED-[0-9A-F]{8}$'
    )
);

CREATE TABLE users (
    user_id            UUID        PRIMARY KEY DEFAULT gen_random_uuid(),
    email              TEXT        UNIQUE NOT NULL,
    user_name          TEXT,
    user_password_hash TEXT        NOT NULL,
    created_at         TIMESTAMPTZ NOT NULL DEFAULT NOW(),
    last_login_at      TIMESTAMPTZ  ,
    otp_secret         TEXT        ,
    otp_enabled        BOOLEAN DEFAULT FALSE,
    otp_backup_codes   TEXT[],
    CHECK (
        email ~ '^[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\.[A-Za-z]{2,}$'
    )
);

CREATE TABLE user_edge_bind (
    edge_id VARCHAR(12) NOT NULL,
    user_id UUID NOT NULL,
    display_name TEXT,
    bind_at TIMESTAMP WITH TIME ZONE DEFAULT now(),
    PRIMARY KEY (edge_id, user_id)
);

CREATE TABLE auth (
    refresh_token TEXT NOT NULL UNIQUE,
    user_id UUID NOT NULL REFERENCES users(user_id) ON DELETE CASCADE,
    created_at TIMESTAMP WITH TIME ZONE NOT NULL DEFAULT now(),
    expires_at TIMESTAMP WITH TIME ZONE NOT NULL,
    revoked BOOLEAN NOT NULL DEFAULT FALSE
);

// EDGE

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