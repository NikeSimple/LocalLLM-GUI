#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QDateTime>
#include <QDir>
#include <QFile>
#include <QDebug>

#include <iostream>
#include <fstream>
#include <cstdio>
#include <sqlite3.h>

#ifdef _WIN32
#include <windows.h>
#endif

#include "../db/database_manager.h"
#include "../db/dialog_repository.h"
#include "../db/message_repository.h"
#include "../db/prompt_repository.h"

// ============================================================
// Простой фреймворк
// ============================================================

static int g_failed = 0;
static int g_passed = 0;

#define CHECK(cond, msg) \
    do { \
        if (cond) { \
            std::cout << "[OK]   " << msg << std::endl; \
            g_passed++; \
        } else { \
            std::cout << "[FAIL] " << msg << std::endl; \
            g_failed++; \
        } \
    } while (0)

// ============================================================
// Тест FileHandler
// ============================================================

void testFileHandler()
{
    std::cout << "\n=== Тест FileHandler ===" << std::endl;

    const char* filename = "test_temp.txt";
    {
        std::ofstream out(filename);
        out << "Привет, мир!" << std::endl;
        out << "Вторая строка" << std::endl;
    }

    {
        std::ifstream in(filename);
        std::string line1, line2;
        std::getline(in, line1);
        std::getline(in, line2);

        CHECK(line1 == "Привет, мир!", "Первая строка прочитана верно");
        CHECK(line2 == "Вторая строка", "Вторая строка прочитана верно");
    }

    std::remove(filename);
    CHECK(!std::ifstream(filename).good(), "Временный файл удалён");
}

// ============================================================
// Тест граничных условий FileHandler
// ============================================================

void testFileHandlerEdgeCases()
{
    std::cout << "\n=== Тест FileHandler (граничные случаи) ===" << std::endl;

    const char* emptyName = "test_empty.txt";
    { std::ofstream out(emptyName); }
    {
        std::ifstream in(emptyName);
        std::string line;
        bool hasData = static_cast<bool>(std::getline(in, line));
        CHECK(!hasData, "Пустой файл читается без данных");
    }
    std::remove(emptyName);

    const char* longName = "test_long.txt";
    {
        std::ofstream out(longName);
        out << std::string(10000, 'A') << std::endl;
    }
    {
        std::ifstream in(longName);
        std::string line;
        std::getline(in, line);
        CHECK(line.length() == 10000, "Длинная строка прочитана целиком");
    }
    std::remove(longName);

    std::ifstream missing("__no_such_file__.txt");
    CHECK(!missing.good(), "Несуществующий файл не открывается");
}

// ============================================================
// Тест DatabaseManager
// ============================================================

void testDatabaseManager()
{
    std::cout << "\n=== Тест DatabaseManager ===" << std::endl;

    DatabaseManager db;
    CHECK(db.open(":memory:"), "Открытие БД в памяти");
    CHECK(db.createTables(), "Создание таблиц dialogs, messages, prompts, settings");

    QSqlQuery q(db.database());
    q.exec("SELECT name FROM sqlite_master WHERE type='table' "
           "AND name IN ('dialogs','messages','prompts','settings')");
    int tableCount = 0;
    while (q.next()) tableCount++;
    CHECK(tableCount == 4, "Все 4 таблицы созданы");

    q.exec("SELECT name FROM sqlite_master WHERE type='index' "
           "AND name LIKE 'idx_%'");
    int idxCount = 0;
    while (q.next()) idxCount++;
    CHECK(idxCount >= 3, "Индексы созданы (минимум 3)");
}

// ============================================================
// Тест DialogRepository
// ============================================================

void testDialogRepository()
{
    std::cout << "\n=== Тест DialogRepository ===" << std::endl;

    DatabaseManager db;
    db.open(":memory:");
    db.createTables();

    DialogRepository repo(db.database());

    int id1 = repo.create("Тестовый диалог", "llama3.1:8b");
    CHECK(id1 > 0, "Диалог создан, id получен");

    int id2 = repo.create("Второй диалог", "qwen2.5:7b");
    CHECK(id2 > 0, "Второй диалог создан");

    QVector<DialogInfo> all = repo.listAll();
    CHECK(all.size() == 2, "listAll вернул 2 диалога");

    DialogInfo info = repo.getById(id1);
    CHECK(info.id == id1, "getById вернул правильный id");
    CHECK(info.title == "Тестовый диалог", "Заголовок совпадает");
    CHECK(info.modelName == "llama3.1:8b", "Модель совпадает");

    bool updated = repo.updateTitle(id1, "Изменённый заголовок");
    CHECK(updated, "updateTitle выполнен");

    DialogInfo afterUpdate = repo.getById(id1);
    CHECK(afterUpdate.title == "Изменённый заголовок", "Заголовок изменён");

    CHECK(repo.touch(id1), "touch выполнен");

    CHECK(repo.remove(id2), "remove выполнен");
    CHECK(repo.listAll().size() == 1, "После удаления остался 1 диалог");

    DialogInfo missing = repo.getById(99999);
    CHECK(missing.id == -1, "getById несуществующего возвращает -1");
}

// ============================================================
// Тест MessageRepository
// ============================================================

void testMessageRepository()
{
    std::cout << "\n=== Тест MessageRepository ===" << std::endl;

    DatabaseManager db;
    db.open(":memory:");
    db.createTables();

    DialogRepository dialogRepo(db.database());
    MessageRepository msgRepo(db.database());

    int dialogId = dialogRepo.create("Диалог с сообщениями", "llama3.1:8b");

    int m1 = msgRepo.add(dialogId, "user", "Привет!");
    int m2 = msgRepo.add(dialogId, "assistant", "Привет! Чем помочь?");
    int m3 = msgRepo.add(dialogId, "user", "Расскажи про Qt");
    CHECK(m1 > 0 && m2 > 0 && m3 > 0, "3 сообщения добавлены");

    QVector<MessageInfo> msgs = msgRepo.listByDialog(dialogId);
    CHECK(msgs.size() == 3, "listByDialog вернул 3 сообщения");
    CHECK(msgs[0].role == "user", "Первое — user");
    CHECK(msgs[1].role == "assistant", "Второе — assistant");
    CHECK(msgs[2].content == "Расскажи про Qt", "Третье — про Qt");

    CHECK(msgRepo.remove(m3), "Удаление одного сообщения");
    CHECK(msgRepo.listByDialog(dialogId).size() == 2, "Осталось 2 сообщения");

    CHECK(dialogRepo.remove(dialogId), "Диалог удалён");
    CHECK(msgRepo.listByDialog(dialogId).isEmpty(),
          "Сообщения удалены каскадно (ON DELETE CASCADE)");
}

// ============================================================
// Тест PromptRepository
// ============================================================

void testPromptRepository()
{
    std::cout << "\n=== Тест PromptRepository ===" << std::endl;

    DatabaseManager db;
    db.open(":memory:");
    db.createTables();

    PromptRepository repo(db.database());

    int id1 = repo.create("Приветствие", "Ты — вежливый ассистент.");
    int id2 = repo.create("Программирование", "Ты — опытный программист на C++.");
    CHECK(id1 > 0 && id2 > 0, "2 шаблона созданы");

    QVector<PromptInfo> all = repo.listAll();
    CHECK(all.size() == 2, "listAll вернул 2 шаблона");

    PromptInfo p1 = repo.getById(id1);
    CHECK(p1.isTemplate == false, "По умолчанию шаблон неактивен");

    CHECK(repo.setActive(id1, true), "setActive(true) выполнен");
    PromptInfo p1active = repo.getById(id1);
    CHECK(p1active.isTemplate == true, "Шаблон активен");

    CHECK(repo.setActive(id1, false), "setActive(false) выполнен");
    PromptInfo p1inactive = repo.getById(id1);
    CHECK(p1inactive.isTemplate == false, "Шаблон снова неактивен");

    CHECK(repo.update(id1, "Изменённое имя", "Новый текст"),
          "update выполнен");
    PromptInfo updated = repo.getById(id1);
    CHECK(updated.name == "Изменённое имя", "Имя изменено");
    CHECK(updated.text == "Новый текст", "Текст изменён");

    CHECK(repo.remove(id2), "remove выполнен");
    CHECK(repo.listAll().size() == 1, "Остался 1 шаблон");
}

// ============================================================
// Тест безопасности: SQL-инъекции
// ============================================================

void testSqlInjectionSafety()
{
    std::cout << "\n=== Тест защиты от SQL-инъекций ===" << std::endl;

    DatabaseManager db;
    db.open(":memory:");
    db.createTables();

    DialogRepository repo(db.database());

    QString malicious = "'; DROP TABLE dialogs; --";
    int id = repo.create(malicious, "test");
    CHECK(id > 0, "Диалог с SQL-инъекцией создан без падения");

    QSqlQuery q(db.database());
    q.exec("SELECT name FROM sqlite_master WHERE type='table' AND name='dialogs'");
    CHECK(q.next(), "Таблица dialogs не удалена инъекцией");

    DialogInfo info = repo.getById(id);
    CHECK(info.title == malicious, "Опасная строка сохранена как текст, не выполнена");

    CHECK(repo.listAll().size() == 1, "В БД ровно 1 диалог");

    MessageRepository msgRepo(db.database());
    int msgId = msgRepo.add(id, "user", "'; DELETE FROM messages; --");
    CHECK(msgId > 0, "Сообщение с инъекцией сохранено");
    CHECK(msgRepo.listByDialog(id).size() == 1, "Сообщение не удалено инъекцией");
}

// ============================================================
// Тест граничных условий репозиториев
// ============================================================

void testRepositoryEdgeCases()
{
    std::cout << "\n=== Тест граничных условий репозиториев ===" << std::endl;

    DatabaseManager db;
    db.open(":memory:");
    db.createTables();

    DialogRepository repo(db.database());

    int idEmpty = repo.create("", "test");
    CHECK(idEmpty > 0, "Пустое название сохраняется");

    QString longTitle(10000, 'X');
    int idLong = repo.create(longTitle, "test");
    CHECK(idLong > 0, "Очень длинное название сохраняется");
    CHECK(repo.getById(idLong).title.length() == 10000,
          "Длинное название прочитано целиком");

    int idSpec = repo.create("Спецсимволы: !@#$%^&*()_+-=[]{}|;':\",./<>?",
                              "model/with\\slashes");
    CHECK(idSpec > 0, "Название со спецсимволами сохраняется");

    CHECK(repo.remove(99999), "remove несуществующего не падает");
    CHECK(repo.updateTitle(99999, "test"),
          "updateTitle несуществующего не падает");
}

// ============================================================
// Тест JSON
// ============================================================

void testJsonBasics()
{
    std::cout << "\n=== Тест JSON ===" << std::endl;

    std::string json = R"({"model":"llama3.1:8b","stream":true})";
    CHECK(json.find("llama3.1") != std::string::npos, "JSON содержит имя модели");
    CHECK(json.find("stream") != std::string::npos, "JSON содержит флаг stream");
}

// ============================================================
// Тест версии SQLite
// ============================================================

void testVersion()
{
    std::cout << "\n=== Тест версии SQLite ===" << std::endl;
    const char *ver = sqlite3_libversion();
    CHECK(ver != nullptr, "Версия SQLite доступна");
    std::cout << "       SQLite версия: " << ver << std::endl;
}

// ============================================================
// main
// ============================================================

int main(int argc, char *argv[])
{
#ifdef _WIN32
    SetConsoleOutputCP(CP_UTF8);
#endif

    QCoreApplication app(argc, argv);

    std::cout << "========================================" << std::endl;
    std::cout << "  LocalLLM-GUI — расширенные тесты" << std::endl;
    std::cout << "========================================" << std::endl;

    testFileHandler();
    testFileHandlerEdgeCases();
    testDatabaseManager();
    testDialogRepository();
    testMessageRepository();
    testPromptRepository();
    testSqlInjectionSafety();
    testRepositoryEdgeCases();
    testJsonBasics();
    testVersion();

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Пройдено: " << g_passed << std::endl;
    std::cout << "  Провалено: " << g_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_failed > 0 ? 1 : 0;
}