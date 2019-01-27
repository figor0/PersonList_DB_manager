#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "string.h"

int first_row; // глобальная переменная для select_callback

int select_callback(void *p_data, int num_fields, char **p_fields, char **p_col_names) {
//Взял готовую реализацию, поменял только коэффициенты при printf
int i;

   int* nof_records = (int*) p_data;
(*nof_records)++;

   if (first_row) {
      first_row = 0;
      printf("%s", p_col_names[0]);
    for (i=1; i < num_fields; i++) {
      printf("%30s", p_col_names[i]);
    }

    printf("\n");
    for (i=0; i< num_fields*25; i++) {
      printf("=");
    }
    printf("\n");
  }

  if (p_fields[0]) {
      printf("%s", p_fields[0]);
    }
    else {
      printf("%25s", " ");
    }

  for(i=1; i < num_fields; i++) {
    if (p_fields[i]) {
      printf("%25s", p_fields[i]);
    }
    else {
      printf("%25s", " ");
    }
  }

  printf("\n");
  return 0;
}


void select_stmt(sqlite3* db,const char* stmt) {
  char *errmsg;
  int   rc;
  int   nrecs = 0;

  first_row = 1;

  rc = sqlite3_exec(db, stmt, select_callback, &nrecs, &errmsg);

  if(rc!=SQLITE_OK) {
    printf("Error in select statement %s [%s].\n", stmt, errmsg);
  }
}

int table_create( sqlite3* db, const char* table_name, char *sql_param) {

   char *zErrMsg = 0;
   int rc;
   int table_nameSize = strlen(table_name); // Используется больше одного раза, не хочу каждый раз вызывать функцию strlen()
   int sql_paramSize = strlen(sql_param);
   int Begin_size = strlen("CREATE TABLE ");

   char sql[Begin_size + table_nameSize + sql_paramSize +1];
   memset(sql, '\0', sizeof(sql));

////////////////// сбор строки
   strncat (sql,"CREATE TABLE ", Begin_size);
   strncat (sql, table_name, table_nameSize);
   strncat (sql,sql_param, sql_paramSize);
/////////////////

   rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);
   
   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Таблица успешно создана\n");
   }

   return 0;
}

int table_delete(sqlite3* db, const char* table_name){

   char *zErrMsg = 0;
   int rc;

   /* Create SQL statement */
   char sql[strlen("DROP TABLE ") + strlen(table_name) + 1 ];
   memset(sql, '\0', sizeof(sql));
   strncat(sql,"DROP TABLE ",strlen("DROP TABLE "));
   strncat(sql,table_name,strlen(table_name));
   /* Execute SQL statement */
   rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

   if( rc != SQLITE_OK ){
      fprintf(stderr, "SQL error: %s\n", zErrMsg);
      sqlite3_free(zErrMsg);
   } else {
      fprintf(stdout, "Таблица успешно удалена\n");
   }
   return 0;
}

int table_show(sqlite3* db, const char* table_name){

   char *zErrMsg = 0;
   int rc;

   int begin_size = strlen("SELECT * from ");
   int table_name_size = strlen(table_name);

   char sql[begin_size + table_name_size + 1];
   memset(sql, '\0', sizeof(sql));
   first_row = 1;
   const char* data = "Callback function called";

   /* Create SQL statement */
   strncat (sql,"SELECT * from ", begin_size);
   strncat (sql,table_name, table_name_size);

   /* Execute SQL statement */
   select_stmt(db,sql);

   return 0;
}

sqlite3_stmt* get_selector_stm(sqlite3* db, const char* table_name){

   char* dst_name;

   int table_nameSize = strlen(table_name); // Используется больше одного раза, не хочу каждый раз вызывать функцию strlen()
   int Begin_size = strlen("SELECT * FROM ");

   char select_sql[Begin_size + table_nameSize];
   memset(select_sql, '\0', sizeof(select_sql));

   strncat (select_sql,"SELECT * FROM ", Begin_size);
   strncat (select_sql, table_name, table_nameSize);

   int countColumn;
   sqlite3_stmt *statement;

   sqlite3_prepare_v2(db, select_sql, -1, &statement, NULL);

   return statement;
}

//получение sqlite3_stmt для вставки данных
sqlite3_stmt* get_inserter_stm(sqlite3* db, sqlite3_stmt* selector_statement, const char* table_name){

   int column_amount = sqlite3_column_count(selector_statement);
   const char* column_names[column_amount];
   int columns_size;
   /////////////////////////// Сборка строки - запроса
   char* begin_part = "INSERT INTO ";
   int begin_part_size = strlen("INSERT INTO ");
   int table_name_size = strlen(table_name);

   for(int i = 0; i < column_amount; i++){
      column_names[i] = sqlite3_column_name(selector_statement, i);
      columns_size += strlen(column_names[i]);
   }

   char middle_part[strlen("()")+ columns_size + column_amount];

   memset(middle_part, '\0', sizeof(middle_part));
   int middle_part_size = sizeof(middle_part);
   strncat(middle_part,"(",1);
   strncat(middle_part,column_names[0], strlen(column_names[0]));

   for(int i = 1; i < column_amount; i++){
      strncat(middle_part,",",1);
      strncat(middle_part,column_names[i],strlen(column_names[i]));
   }
   strncat(middle_part,")",1);

   char end_part[strlen(" values();")+2*column_amount];
   memset(end_part, '\0', sizeof(end_part));
   int end_part_size = sizeof(end_part);

   strncat (end_part," values(?", strlen(" values(?"));
   for(int i = 1 ; i< column_amount; i++){
      strncat (end_part,",?", 2);
   }
   strncat (end_part,");", 2);

   char* sql_inserter[begin_part_size + middle_part_size +table_name_size + end_part_size + 1];
   memset(sql_inserter, '\0', sizeof(sql_inserter));

   strncat(sql_inserter,begin_part,begin_part_size);
   strncat(sql_inserter,table_name,table_name_size);
   strncat(sql_inserter,middle_part,middle_part_size);
   strncat(sql_inserter,end_part,end_part_size);
   ///////////////////////

   sqlite3_stmt *output_statement;

   sqlite3_prepare(db, sql_inserter, -1, &output_statement, NULL);

   return output_statement;
}

int get_entry_amount(sqlite3* db, const char* table_name){ //Получение количества записей в таблице
   int rowcount;
   int rc;
   sqlite3_stmt* stmt;
   char* begin_part = "SELECT COUNT(*) from ";

   char sqlQuery[strlen(begin_part) + strlen(table_name) + 2];
   memset(sqlQuery, '\0', sizeof(sqlQuery));

   strncat(sqlQuery, begin_part, strlen(begin_part));
   strncat(sqlQuery,table_name, strlen(table_name));
   strncat(sqlQuery,";",1);

   rc = sqlite3_prepare_v2(db, sqlQuery, -1, &stmt, NULL);
   if (rc != SQLITE_OK) {
      return 0;
  // error handling -> statement not prepared
   }
   rc = sqlite3_step(stmt);
   if (rc != SQLITE_ROW) {
      return 0;
  // error handling -> no rows returned, or an error occurred
   }
   rowcount = sqlite3_column_int(stmt, 0);
   sqlite3_finalize(stmt);
   return rowcount;
}