Архитектура
===========

Многослойная архитектура
------------------------

.. list-table::
   :header-rows: 1

   * - Слой
     - Компоненты
   * - Представление
     - MainWindow, ChatWidget, DialogListWidget, SettingsPanel, SettingsOverlay
   * - Бизнес-логика
     - ApiClient, FileHandler
   * - Доступ к данным
     - DatabaseManager, DialogRepository, MessageRepository, PromptRepository
   * - Инфраструктура
     - SettingsManager, ThemeManager

Паттерны проектирования
-----------------------

- **MVC** — разделение gui/ и db/ через контроллер MainWindow;
- **Repository** — абстракция работы с SQLite;
- **Singleton** — ThemeManager, SettingsManager;
- **Observer** — сигналы/слоты Qt;
- **DTO** — ChatMessage, DialogInfo, MessageInfo, PromptInfo.

Схема базы данных
-----------------

.. list-table::
   :header-rows: 1

   * - Таблица
     - Поля
   * - dialogs
     - id (PK), title, created_at, updated_at, model_name, prompt_id (FK)
   * - messages
     - id (PK), dialog_id (FK), role, content, created_at, attachments
   * - prompts
     - id (PK), name, text, is_template
   * - settings
     - key (PK), value
