#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;
    char *errMsg = 0;
    sqlite3_open("test.db", &db);

    // autocommit режим (каждый INSERT отдельно)
    printf("=== AUTOCOMMIT MODE ===\n");
    for (int i = 10; i < 13; i++) {
        char sql[100];
        sprintf(sql, "INSERT INTO COMPANY (ID, NAME, AGE) VALUES (%d, 'Temp', 0);", i);
        sqlite3_exec(db, sql, 0, 0, &errMsg);
    }

    // явная транзакция: BEGIN + COMMIT
    printf("=== TRANSACTION MODE ===\n");
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    for (int i = 20; i < 23; i++) {
        char sql[100];
        sprintf(sql, "INSERT INTO COMPANY (ID, NAME, AGE) VALUES (%d, 'Trans', 0);", i);
        sqlite3_exec(db, sql, 0, 0, &errMsg);
    }
    sqlite3_exec(db, "COMMIT;", 0, 0, 0);
    printf("Transaction committed.\n");

    // откат транзакции (ROLLBACK)
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    sqlite3_exec(db, "INSERT INTO COMPANY (ID, NAME, AGE) VALUES (30, 'Rollback', 0);", 0, 0, 0);
    sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
    printf("Rollback executed – row 30 not inserted.\n");

    sqlite3_close(db);
    return 0;
}
