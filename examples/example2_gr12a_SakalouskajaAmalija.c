#include <stdio.h>
#include <sqlite3.h>

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    int rc;

    rc = sqlite3_open("test.db", &db);
    if (rc) {
        fprintf(stderr, "Cannot open db: %s\n", sqlite3_errmsg(db));
        return 1;
    }

    const char *sql = "INSERT INTO COMPANY (ID, NAME, AGE) VALUES (?, ?, ?);";
    rc = sqlite3_prepare_v2(db, sql, -1, &stmt, 0);
    if (rc != SQLITE_OK) {
        fprintf(stderr, "Prepare error: %s\n", sqlite3_errmsg(db));
        sqlite3_close(db);
        return 1;
    }

    int ids[] = {3, 4};
    char *names[] = {"Charlie", "Diana"};
    int ages[] = {28, 23};

    for (int i = 0; i < 2; i++) {
        sqlite3_bind_int(stmt, 1, ids[i]);
        sqlite3_bind_text(stmt, 2, names[i], -1, SQLITE_STATIC);
        sqlite3_bind_int(stmt, 3, ages[i]);

        rc = sqlite3_step(stmt);
        if (rc != SQLITE_DONE)
            fprintf(stderr, "Execute error: %s\n", sqlite3_errmsg(db));

        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }

    sqlite3_finalize(stmt);
    sqlite3_close(db);
    printf("Parameterized insert completed.\n");
    return 0;
}

