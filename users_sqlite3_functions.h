

int callback(void *NotUsed, int argc, char **argv, char **azColName);
int table_create(sqlite3* db, const char* table_name, char *sql_param);
int table_delete(sqlite3* db, const char* table_name);
int table_change(const char* file_name,char* sql);
int table_show(sqlite3* db, const char* table_name);
int entry_insert(sqlite3* db, const char* table_name);
inline char* get_column_name(   sqlite3* db, const char* table_name, int column_number);
sqlite3_stmt* get_selector_stm(sqlite3* db, const char* table_name);
sqlite3_stmt* get_inserter_stm(sqlite3* db, sqlite3_stmt* selector_statement, const char* table_name);
int get_entry_amount(sqlite3* db,const char* table_name);
int select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names);
void select_stmt(sqlite3 db, const char* stmt);