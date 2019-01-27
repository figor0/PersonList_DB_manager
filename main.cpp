#include <stdio.h>
#include <string>
#include <cstring>
extern "C"{
	#include "sqlite3.h"
	#include "users_sqlite3_functions.h"
}
#include <iostream>

#define SHELLSCRIPT "\
#/bin/bash \n\
if [ ! -f $test_db.db ] \n\
then\n\
touch test_db.db\n\
fi \n\
"

int main(int argc, char** argv) {

	system(SHELLSCRIPT);// Создание файла базы

	std::string table_name;
	int table_decision;

//////////////////////////////////////////////  Часть с обработкой входящих аргументов
	if(argc !=3){
		if(argc == 1){
			std::cout << "Не было введено аргументов.\n" \ 
			"Для работы программы необходимо ввести один из следующих пар аргументов:\n"\
			"Создать таблицу (Введите: \" create имя_таблицы\")\n" \
			"Удалить таблицу (Введите: \"delete имя_таблицы\" )\n" \
			"Изменить запись (Введите: \"change имя_таблицы\")\n"\
			"Показать таблицу (Введите: \"show имя_таблицы\" (вывод конкретной таблицы) )\n";
			return 0;
		}
		else{
			std::cout << "Введены неправильные аргументы.\n" \ 
			"Для работы программы необходимо ввести один из следующих пар аргументов:\n"\
			"Создать таблицу (Введите: \" create имя_таблицы\")\n" \
			"Удалить таблицу (Введите: \"delete имя_таблицы\" )\n" \
			"Изменить запись (Введите: \"change имя_таблицы\")\n"\
			"Показать таблицу (Введите: \"show имя_таблицы\" (вывод конкретной таблицы) )\n";
			return 0;
		}
	}
	if((std::string)argv[1] == "delete" || (std::string)argv[1] == "create" || (std::string)argv[1] == "change" || (std::string)argv[1] == "show" ){
		table_name = argv[2];
		if((std::string)argv[1] == "create")  table_decision = 1;
		if((std::string)argv[1] == "delete")  table_decision = 2;
		if((std::string)argv[1] == "change")  table_decision = 3;
		if((std::string)argv[1] == "show")  table_decision = 4;
	}
	else{
		std::cout << "Введены неправильные аргументы.\n" \ 
		"Для работы программы необходимо ввести один из следующих пар аргументов:\n"\
		"Создать таблицу (Введите: \" create имя_таблицы\")\n" \
		"Удалить таблицу (Введите: \"delete имя_таблицы\" )\n" \
		"Изменить запись (Введите: \"change имя_таблицы\")\n"\
		"Показать таблицу (Введите:  \"show имя_таблицы\" )\n";
		return 0;
	}
/////////////////////////////////////////////

	int n;
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;
	int entry_decision;
	int columns_amount;
	int ID;
	int rows_count;

	rc = sqlite3_open("test_db.db", &db);

	char* table_struct = "(ID int," \
	"Фамилия TEXT NOT NULL," \
	"Имя TEXT NOT NULL," \
	"Отчество TEXT);";

	sqlite3_stmt* selector = get_selector_stm(db,table_name.c_str());//Создание объекта выполняющего запрос SELECT table_name
	columns_amount = sqlite3_column_count(selector);// Подсчёт количества столбцов в таблице
	rows_count =  get_entry_amount(db, table_name.c_str());

	if( rc ) {
		std::cerr << "Can't open database:" << sqlite3_errmsg(db) << std::endl;
		return(0);
	} 

	switch(table_decision){
		case 1: // Создание таблицы
			table_create(db, table_name.c_str(), table_struct);
			break;

		case 2: // Удаление таблицы
			table_delete(db, table_name.c_str());
			break;

		case 3:{ // Изменения в таблице
			std::string buf_tb[columns_amount-1];
			if(columns_amount == 0){
				std::cout << "Error: Table not created" << std::endl;
				return 1;
			}
			if(rows_count==0){
				std::cout << "Возможно только добавить запись" << std::endl;
				entry_decision = 1;
			}
			else{
				std::cout << "Возможные изменения в таблице:\n"\
				"Добавить запись (Введите: \"1\")\n" \
				"Изменить запись (Введите: \"2\" )\n" \
				"Удаление записи (Введите: \"3\")" << std::endl;
				std::cin >> entry_decision;
			}
			switch(entry_decision){

				case 1:{// Entry creating

					sqlite3_stmt* inserter = get_inserter_stm(db, selector, table_name.c_str()); //Получаем переменную состояния с помощью которой
					std::cout << "Введите: \n";													//внесём вводимые значения в таблицу.
					///////////////// Привязка значений к столбцам
					if (sqlite3_bind_int(inserter, 1, rows_count + 1) != SQLITE_OK) {
						std::cerr << "Error: could't bind int";
						return 1;
					}
					for(int i = 1 ; i < columns_amount; i++ ){
						std::cout << sqlite3_column_name(selector, i) << "\t";
						std::cin >> buf_tb[i-1];
						sqlite3_bind_text(inserter, i + 1 , &buf_tb[i-1][0], buf_tb[i-1].size(), SQLITE_STATIC);
					}///////////////

					if (sqlite3_step(inserter) != SQLITE_DONE) { //Ввод этих элементов
    					std::cerr << "Could not step (execute) stmt." << std::endl;
    				}
    				break;
    				sqlite3_finalize(inserter);
    				rows_count++;
				}
				case 2:{ //entry changing

					std::cout << "Введите ID изменяемого элемента \t";
					std::cin >> ID;
					///////////////////Проверка на существование записи с таким ID
					if(rows_count < ID){
						std::cerr<< "\n Error: entry with input ID not exist" << std::endl;
						return 1;
					}
					//////////////////
					std::cin.get(); // Походу костыль, но я просто не знаю, как удобнее сбросить \n
					std::cout << "Введите новые значения для записи, состоящей из: ";
					for(int i=1; i< columns_amount; i++) {std::cout << sqlite3_column_name(selector, i) << "\t";}
					std::cout << "\nВ формате Имя_столбца1 = 'значение1', Имя_столбца2 = 'значение2' ...\n";
					std::cout << "Нельзя изменять ID записи.\n" << std::endl;
					std::string string_buff;
					std::getline(std::cin,string_buff);

					//////////////////////////// Проверка на изменение ID
					n = string_buff.find("ID ");
					int i;
					if(n >= 0){
						std::cout << "Нельзя изменять ID элемента\n";
						for(i = n; string_buff[i] !=  ',' && i < string_buff.length()-1; i++);
						if(i == string_buff.length()-1){
							while(n > 0 && string_buff[n] != ','){
								n--;
							}
						}
						string_buff.erase(string_buff.begin()+n,string_buff.begin()+i + 1);

					}
					n = string_buff.find("ID=");
					if(n >=0){
						std::cout << "Нельзя изменять ID элемента\n";
						for(i = n; string_buff[i] != ',' && i < string_buff.length()-1;i++);
						if(i == string_buff.length()-1){
							while(n > 0 && string_buff[n] != ','){
								n--;
							}
						}
						string_buff.erase(string_buff.begin()+n,string_buff.begin() + i + 1);
					}
					////////////////////////////

					std::string result_sql = "UPDATE " + table_name + " SET " + string_buff + " WHERE " + "ID =" + std::to_string(ID) + ';' + "\0";
					rc = sqlite3_exec(db, &result_sql[0], 0, 0, &zErrMsg);
					std::cout << result_sql << '\n';

					if( rc != SQLITE_OK ) {
						std::cerr<< "SQL error: " << zErrMsg << std::endl;
						sqlite3_free(zErrMsg);
						sqlite3_close(db);
						sqlite3_finalize(selector);
					}
					break;
				}
				case 3:{// delete entry
					std::cout << "Введите ID удаляемого элемента \t";
					std::cin >> ID;
					///////////////////Проверка на существование записи с таким ID
					if(rows_count < ID){
						std::cerr<< "\n Error: entry with input ID not exist" << std::endl;
						return 1;
					}
					//////////////////
					std::string string_buff = "DELETE from " + table_name +  " where " + "ID =" + std::to_string(ID) + ';' + "\0";

					rc = sqlite3_exec(db, &string_buff[0], 0, 0, &zErrMsg);

					if( rc != SQLITE_OK ) {
						std::cerr<< "SQL error: " << zErrMsg << std::endl;
						sqlite3_free(zErrMsg);
						sqlite3_close(db);
						sqlite3_finalize(selector);
						return 1;
					}

					string_buff = "UPDATE " + table_name + " SET ID = ID - 1 WHERE ID >" + std::to_string(ID) + ";" + "\0";
					rc = sqlite3_exec(db, &string_buff[0], 0, 0, &zErrMsg);

					rows_count--;

					if( rc != SQLITE_OK ) {
						std::cerr<< "SQL error: " << zErrMsg << std::endl;
						sqlite3_free(zErrMsg);
						sqlite3_close(db);
						sqlite3_finalize(selector);
						return 1;
					}
				}
			}
		}
		break;
		case 4:// Показать таблицу
				table_show(db, table_name.c_str());
			break;

	}
	sqlite3_finalize(selector);
	sqlite3_close(db);

	return 0;
}