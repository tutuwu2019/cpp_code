CREATE TABLE user (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    username TEXT NOT NULL,
    password_hash TEXT NOT NULL,
    email TEXT,
    phone TEXT,
    nickname TEXT,
    avatar_url TEXT,
    status INTEGER NOT NULL DEFAULT 1,
    is_deleted INTEGER NOT NULL DEFAULT 0,
    last_login_at TEXT,
    last_login_ip TEXT,
    created_at TEXT NOT NULL DEFAULT(datetime('now')),
    updated_at TEXT NOT NULL DEFAULT(datetime('now')),
    deleted_at TEXT
);

CREATE INDEX idx_user_created_at ON user (created_at);