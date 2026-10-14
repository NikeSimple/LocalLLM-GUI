-- ============================================================
-- LocalLLM-GUI — схема базы данных
-- Соответствует разделу 4.5 ТЗ
-- СУБД: SQLite
-- ============================================================

PRAGMA foreign_keys = ON;

-- ------------------------------------------------------------
-- Таблица dialogs — диалоги
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS dialogs (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    title       TEXT NOT NULL,
    created_at  TEXT NOT NULL,
    updated_at  TEXT NOT NULL,
    model_name  TEXT,
    prompt_id   INTEGER,
    FOREIGN KEY (prompt_id) REFERENCES prompts(id) ON DELETE SET NULL
);

-- ------------------------------------------------------------
-- Таблица messages — сообщения в диалоге
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS messages (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    dialog_id   INTEGER NOT NULL,
    role        TEXT NOT NULL,
    content     TEXT NOT NULL,
    created_at  TEXT NOT NULL,
    attachments TEXT,
    FOREIGN KEY (dialog_id) REFERENCES dialogs(id) ON DELETE CASCADE
);

-- ------------------------------------------------------------
-- Таблица prompts — шаблоны промптов
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS prompts (
    id          INTEGER PRIMARY KEY AUTOINCREMENT,
    name        TEXT NOT NULL,
    text        TEXT NOT NULL,
    is_template INTEGER DEFAULT 0
);

-- ------------------------------------------------------------
-- Таблица settings — настройки приложения
-- ------------------------------------------------------------
CREATE TABLE IF NOT EXISTS settings (
    key         TEXT PRIMARY KEY,
    value       TEXT
);

-- ------------------------------------------------------------
-- Индексы (раздел 4.5.3 ТЗ)
-- ------------------------------------------------------------
CREATE INDEX IF NOT EXISTS idx_messages_dialog_id ON messages(dialog_id);
CREATE INDEX IF NOT EXISTS idx_dialogs_updated_at ON dialogs(updated_at);
CREATE INDEX IF NOT EXISTS idx_dialogs_prompt_id  ON dialogs(prompt_id);