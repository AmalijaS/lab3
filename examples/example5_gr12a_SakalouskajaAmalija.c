#include <stdio.h>
#include <sqlite3.h>

static int printTables(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++)
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    printf("\n");
    return 0;
}

int main() {
    sqlite3 *db;
    char *errMsg = 0;
    sqlite3_open("test.db", &db);

    // Список всех таблиц пользователя
    printf("=== TABLES ===\n");
    sqlite3_exec(db, "SELECT name FROM sqlite_master WHERE type='table' AND name NOT LIKE 'sqlite_%';", printTables, 0, &errMsg);

    // Информация о столбцах таблицы COMPANY
    printf("=== COLUMNS of COMPANY ===\n");
    sqlite3_exec(db, "PRAGMA table_info(COMPANY);", printTables, 0, &errMsg);

    // Альтернативный способ: запрос из sqlite_master
    printf("=== SQL CREATE statement ===\n");
    sqlite3_exec(db, "SELECT sql FROM sqlite_master WHERE type='table' AND name='COMPANY';", printTables, 0, &errMsg);

    sqlite3_close(db);
    return 0;
}
