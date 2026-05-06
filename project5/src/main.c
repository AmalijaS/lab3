#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>

sqlite3 *db;
char *err_msg = 0;

// Callback для вывода SELECT-результатов
static int callback(void *NotUsed, int argc, char **argv, char **azColName) {
    for (int i = 0; i < argc; i++)
        printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
    printf("\n");
    return 0;
}

void print_menu() {
    printf("\n===== Управление зоопарком =====\n");
    printf("1. Показать всех животных (SELECT)\n");
    printf("2. Найти животное по ID\n");
    printf("3. Найти животное по фрагменту названия\n");
    printf("4. Найти животных по адресу зоопарка (город)\n");
    printf("5. Добавить новое животное (INSERT)\n");
    printf("6. Удалить животное (DELETE)\n");
    printf("7. Сохранить фото животного в файл\n");
    printf("8. Демонстрация autocommit vs транзакция\n");
    printf("0. Выйти\n");
    printf("Ваш выбор: ");
}

// 1. SELECT всех животных с информацией о зоопарке
void select_all_animals() {
    const char *sql = "SELECT animals.id, animals.name, animals.species_count, "
                      "zoo.address_city, zoo.total_animals, zoo.total_employees "
                      "FROM animals JOIN zoo ON animals.zoo_id = zoo.id;";
    if (sqlite3_exec(db, sql, callback, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Ошибка: %s\n", err_msg);
        sqlite3_free(err_msg);
    }
}

// 2. Параметризованный SELECT по ID животного
void select_animal_by_id() {
    int id;
    printf("Введите ID животного: ");
    scanf("%d", &id);
    getchar();

    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, name, species_count FROM animals WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID: %d\nНазвание: %s\nКоличество особей: %d\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_int(stmt, 2));
        // Проверка наличия фото
        int blob_size = sqlite3_column_bytes(stmt, 3);
        printf("Фото: %s\n", blob_size > 0 ? "есть (BLOB)" : "отсутствует");
    } else {
        printf("Животное с ID %d не найдено.\n", id);
    }
    sqlite3_finalize(stmt);
}

// 3. Поиск по фрагменту названия животного
void select_by_name_pattern() {
    char pattern[100];
    printf("Введите фрагмент названия: ");
    fgets(pattern, sizeof(pattern), stdin);
    pattern[strcspn(pattern, "\n")] = 0;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT id, name, species_count FROM animals WHERE name LIKE ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки: %s\n", sqlite3_errmsg(db));
        return;
    }
    char like_pattern[102];
    snprintf(like_pattern, sizeof(like_pattern), "%%%s%%", pattern);
    sqlite3_bind_text(stmt, 1, like_pattern, -1, SQLITE_STATIC);

    printf("Результаты поиска:\n");
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID: %d | Название: %s | Количество: %d\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_int(stmt, 2));
    }
    sqlite3_finalize(stmt);
}

// 4. Поиск по адресу (город) – поле, общее для многих животных
void select_by_city() {
    char city[100];
    printf("Введите город зоопарка: ");
    fgets(city, sizeof(city), stdin);
    city[strcspn(city, "\n")] = 0;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT animals.id, animals.name, animals.species_count, zoo.address_city "
                      "FROM animals JOIN zoo ON animals.zoo_id = zoo.id "
                      "WHERE zoo.address_city = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(stmt, 1, city, -1, SQLITE_STATIC);

    printf("Животные в зоопарке города %s:\n", city);
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        printf("ID: %d | Название: %s | Количество: %d\n",
               sqlite3_column_int(stmt, 0),
               sqlite3_column_text(stmt, 1),
               sqlite3_column_int(stmt, 2));
    }
    sqlite3_finalize(stmt);
}

// 5. Добавление нового животного (INSERT)
void insert_animal() {
    char name[100];
    int count;
    printf("Название животного: ");
    fgets(name, sizeof(name), stdin);
    name[strcspn(name, "\n")] = 0;
    printf("Количество особей: ");
    scanf("%d", &count);
    getchar();

    // Предполагаем, что zoo_id = 1 (единственный зоопарк)
    sqlite3_stmt *stmt;
    const char *sql = "INSERT INTO animals (zoo_id, name, species_count) VALUES (1, ?, ?);";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_text(stmt, 1, name, -1, SQLITE_STATIC);
    sqlite3_bind_int(stmt, 2, count);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Животное добавлено.\n");
        // Обновляем общее количество животных в таблице zoo
        sqlite3_exec(db, "UPDATE zoo SET total_animals = total_animals + ? WHERE id = 1;", 0, 0, 0);
    } else {
        fprintf(stderr, "Ошибка вставки: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// 6. Удаление животного (DELETE)
void delete_animal() {
    int id;
    printf("Введите ID животного для удаления: ");
    scanf("%d", &id);
    getchar();

    sqlite3_stmt *stmt;
    const char *sql = "DELETE FROM animals WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_DONE) {
        printf("Животное удалено. Затронуто строк: %d\n", sqlite3_changes(db));
        sqlite3_exec(db, "UPDATE zoo SET total_animals = total_animals - ? WHERE id = 1;", 0, 0, 0);
    } else {
        fprintf(stderr, "Ошибка удаления: %s\n", sqlite3_errmsg(db));
    }
    sqlite3_finalize(stmt);
}

// 7. Сохранение фото животного в файл
void save_photo() {
    int id;
    char filename[256];
    printf("Введите ID животного: ");
    scanf("%d", &id);
    getchar();
    printf("Имя файла для сохранения (например, lion.jpg): ");
    fgets(filename, sizeof(filename), stdin);
    filename[strcspn(filename, "\n")] = 0;

    sqlite3_stmt *stmt;
    const char *sql = "SELECT photo FROM animals WHERE id = ?;";
    if (sqlite3_prepare_v2(db, sql, -1, &stmt, 0) != SQLITE_OK) {
        fprintf(stderr, "Ошибка подготовки: %s\n", sqlite3_errmsg(db));
        return;
    }
    sqlite3_bind_int(stmt, 1, id);

    if (sqlite3_step(stmt) == SQLITE_ROW) {
        const void *data = sqlite3_column_blob(stmt, 0);
        int size = sqlite3_column_bytes(stmt, 0);
        if (size > 0) {
            FILE *f = fopen(filename, "wb");
            if (f) {
                fwrite(data, 1, size, f);
                fclose(f);
                printf("Фото сохранено в %s\n", filename);
            } else {
                printf("Не удалось создать файл.\n");
            }
        } else {
            printf("У этого животного нет фото.\n");
        }
    } else {
        printf("Животное с ID %d не найдено.\n", id);
    }
    sqlite3_finalize(stmt);
}

// 8. Демонстрация autocommit (построчное выполнение) и транзакции
void transaction_demo() {
    printf("\n--- Режим autocommit (каждый INSERT фиксируется сразу) ---\n");
    const char *sql1 = "INSERT INTO animals (zoo_id, name, species_count) VALUES (1, 'Зебра', 7);";
    const char *sql2 = "INSERT INTO animals (zoo_id, name, species_count) VALUES (1, 'Кенгуру', 4);";
    const char *sql3 = "INSERT INTO animals (zoo_id, name, species_count) VALUES (1, 'ОченьДлинноеНазваниеКоторогоНеДолжноБытьВБазеПотомуЧтоМыСделаемОшибку', 0);";
    if (sqlite3_exec(db, sql1, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Ошибка: %s\n", err_msg); sqlite3_free(err_msg);
    } else printf("Зебра добавлена.\n");
    if (sqlite3_exec(db, sql2, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Ошибка: %s\n", err_msg); sqlite3_free(err_msg);
    } else printf("Кенгуру добавлена.\n");
    if (sqlite3_exec(db, sql3, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Ошибка при вставке: %s (строка не добавлена)\n", err_msg); sqlite3_free(err_msg);
    } // эта вставка не выполнится, но первые две останутся в БД

    printf("\n--- Режим транзакции (откат при ошибке) ---\n");
    sqlite3_exec(db, "BEGIN TRANSACTION;", 0, 0, 0);
    const char *sql_t1 = "INSERT INTO animals (zoo_id, name, species_count) VALUES (1, 'Носорог', 2);";
    const char *sql_t2 = "INSERT INTO animals (zoo_id, name, species_count) VALUES (1, 'Бегемот', 3);";
    const char *sql_t3 = "INSERT INTO animals (zoo_id, name, species_count) VALUES (1, 'Ошибка', -100);"; // вызовет ошибку
    sqlite3_exec(db, sql_t1, 0, 0, 0);
    sqlite3_exec(db, sql_t2, 0, 0, 0);
    if (sqlite3_exec(db, sql_t3, 0, 0, &err_msg) != SQLITE_OK) {
        fprintf(stderr, "Ошибка в транзакции: %s\nВыполняется ROLLBACK\n", err_msg);
        sqlite3_free(err_msg);
        sqlite3_exec(db, "ROLLBACK;", 0, 0, 0);
    } else {
        sqlite3_exec(db, "COMMIT;", 0, 0, 0);
        printf("Транзакция зафиксирована.\n");
    }
}

int main() {
    if (sqlite3_open("db/zoo.db", &db) != SQLITE_OK) {
        fprintf(stderr, "Не удалось открыть БД: %s\n", sqlite3_errmsg(db));
        return 1;
    }
    printf("Подключение к зоопарку установлено.\n");

    int choice;
    do {
        print_menu();
        scanf("%d", &choice);
        getchar();
        switch (choice) {
            case 1: select_all_animals(); break;
            case 2: select_animal_by_id(); break;
            case 3: select_by_name_pattern(); break;
            case 4: select_by_city(); break;
            case 5: insert_animal(); break;
            case 6: delete_animal(); break;
            case 7: save_photo(); break;
            case 8: transaction_demo(); break;
            case 0: printf("До свидания!\n"); break;
            default: printf("Неверный выбор.\n");
        }
    } while (choice != 0);

    sqlite3_close(db);
    return 0;
}
