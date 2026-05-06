# Список примеров для работы с SQLite из C

| Файл | Описание |
|------|----------|
| `example1_grN_LastnameFirstName.c` | Создание БД, таблицы, вставка и 
выборка (по tutorialspoint). |
| `example2_grN_LastnameFirstName.c` | Параметризованные запросы с 
`sqlite3_prepare_v2` и `sqlite3_bind_*`. |
| `example3_grN_LastnameFirstName.c` | Вставка и чтение BLOB (изображений) 
в SQLite. |
| `example4_grN_LastnameFirstName.c` | Демонстрация autocommit, явных 
транзакций (BEGIN/COMMIT/ROLLBACK). |
| `example5_grN_LastnameFirstName.c` | Получение метаданных: список 
таблиц, структура колонок, PRAGMA. |

## Компиляция
```bash
gcc -o exampleX exampleX_grN_LastnameFirstName.c -lsqlite3
