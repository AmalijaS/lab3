#include <stdio.h>
#include <sqlite3.h>

static int callback(void *NotUsed, int argc, char **argv, char 
**azColName) {
    for (int i = 0; i < argc; i++)
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    printf("\n");
    return 0;
}

int main() {
    sqlite3 *db;
    char *errMsg = 0;
    int rc;

    rc = sqlite3_open("test.db", &db);
    if (rc) {
        fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("Opened database successfully\n");

    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS COMPANY("
                            "ID INT PRIMARY KEY NOT NULL,"
                            "NAME TEXT NOT NULL,"
                            "AGE INT NOT NULL);";
    rc = sqlite3_exec(db, sqlCreate, callback, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    const char *sqlInsert = "INSERT INTO COMPANY (ID, NAME, AGE) VALUES (1, 'Alice', 25);"
                            "INSERT INTO COMPANY (ID, NAME, AGE) VALUES (2, 'Bob', 32);";
    rc = sqlite3_exec(db, sqlInsert, callback, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    const char *sqlSelect = "SELECT * FROM COMPANY;";
    rc = sqlite3_exec(db, sqlSelect, callback, 0, &errMsg);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "SQL error: %s\n", errMsg);
        sqlite3_free(errMsg);
    }

    sqlite3_close(db);
    return 0;
}
