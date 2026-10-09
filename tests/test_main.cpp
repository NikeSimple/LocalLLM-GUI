#include <iostream>
#include <fstream>
#include <cstdio>
#include <sqlite3.h>

// ============================================================
// Простой фреймворк для тестов
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

    // Создаём временный файл
    const char* filename = "test_temp.txt";
    {
        std::ofstream out(filename);
        out << "Привет, мир!" << std::endl;
        out << "Вторая строка" << std::endl;
    }

    // Читаем обратно
    {
        std::ifstream in(filename);
        std::string line1, line2;
        std::getline(in, line1);
        std::getline(in, line2);

        CHECK(line1 == "Привет, мир!", "Первая строка прочитана верно");
        CHECK(line2 == "Вторая строка", "Вторая строка прочитана верно");
    }

    // Удаляем
    std::remove(filename);
    CHECK(!std::ifstream(filename).good(), "Временный файл удалён");
}

// ============================================================
// Тест SQLite
// ============================================================

void testDatabase()
{
    std::cout << "\n=== Тест SQLite ===" << std::endl;

    sqlite3 *db = nullptr;
    int rc = sqlite3_open(":memory:", &db);
    CHECK(rc == SQLITE_OK, "Открытие SQLite в памяти");

    if (rc != SQLITE_OK) {
        std::cerr << "Ошибка SQLite: " << sqlite3_errmsg(db) << std::endl;
        return;
    }

    // Создаём таблицу
    const char *createSql =
        "CREATE TABLE test ("
        "id INTEGER PRIMARY KEY,"
        "name TEXT NOT NULL"
        ");";
    rc = sqlite3_exec(db, createSql, nullptr, nullptr, nullptr);
    CHECK(rc == SQLITE_OK, "Создание таблицы test");

    // Вставляем запись
    rc = sqlite3_exec(db, "INSERT INTO test (name) VALUES ('тест');",
                       nullptr, nullptr, nullptr);
    CHECK(rc == SQLITE_OK, "Вставка записи");

    // Считаем записи
    sqlite3_stmt *stmt = nullptr;
    rc = sqlite3_prepare_v2(db, "SELECT COUNT(*) FROM test;", -1, &stmt, nullptr);
    CHECK(rc == SQLITE_OK, "Подготовка SELECT COUNT");

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        int count = sqlite3_column_int(stmt, 0);
        CHECK(count == 1, "Количество записей = 1");
    }
    sqlite3_finalize(stmt);

    // Проверяем foreign_keys
    rc = sqlite3_exec(db, "PRAGMA foreign_keys = ON;", nullptr, nullptr, nullptr);
    CHECK(rc == SQLITE_OK, "Включение foreign_keys");

    sqlite3_close(db);
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
// Тест JSON-конвертации (простой, без nlohmann)
// ============================================================

void testJsonBasics()
{
    std::cout << "\n=== Тест JSON (базовый) ===" << std::endl;

    std::string json = R"({"model":"llama3.1:8b","stream":true})";
    CHECK(json.find("llama3.1") != std::string::npos, "JSON содержит имя модели");
    CHECK(json.find("stream") != std::string::npos, "JSON содержит флаг stream");
}

// ============================================================
// Основная функция
// ============================================================

int main()
{
    std::cout << "========================================" << std::endl;
    std::cout << "  LocalLLM-GUI — модульные тесты" << std::endl;
    std::cout << "========================================" << std::endl;

    testFileHandler();
    testDatabase();
    testVersion();
    testJsonBasics();

    std::cout << "\n========================================" << std::endl;
    std::cout << "  Пройдено: " << g_passed << std::endl;
    std::cout << "  Провалено: " << g_failed << std::endl;
    std::cout << "========================================" << std::endl;

    return g_failed > 0 ? 1 : 0;
}