#include <stdio.h>
#include <sqlite3.h>
#include <stdlib.h>

int main() {
    sqlite3 *db;
    sqlite3_stmt *stmt;
    FILE *fp;
    long file_size;
    char *image_data;
    size_t bytes;

    sqlite3_open("test.db", &db);
    const char *sqlCreate = "CREATE TABLE IF NOT EXISTS Images(Id INTEGER PRIMARY KEY, Data BLOB);";
    sqlite3_exec(db, sqlCreate, 0, 0, 0);

    // Вставка изображения из файла (укажите свой путь)
    fp = fopen("image.png", "rb");
    if (!fp) {
        fprintf(stderr, "Can't open image.jpg\n");
        return 1;
    }
    fseek(fp, 0, SEEK_END);
    file_size = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    image_data = malloc(file_size);
    bytes = fread(image_data, 1, file_size, fp);
    fclose(fp);

    const char *sqlInsert = "INSERT INTO Images(Data) VALUES(?);";
    sqlite3_prepare_v2(db, sqlInsert, -1, &stmt, 0);
    sqlite3_bind_blob(stmt, 1, image_data, bytes, SQLITE_TRANSIENT);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);
    free(image_data);
    printf("Image inserted.\n");

  
    const char *sqlSelect = "SELECT Data FROM Images WHERE Id = 1;";
    sqlite3_prepare_v2(db, sqlSelect, -1, &stmt, 0);
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const void *blob = sqlite3_column_blob(stmt, 0);
        int size = sqlite3_column_bytes(stmt, 0);
        fp = fopen("output.jpg", "wb");
        fwrite(blob, 1, size, fp);
        fclose(fp);
        printf("Image saved as output.jpg\n");
    }
    sqlite3_finalize(stmt);
    sqlite3_close(db);
    return 0;
}
