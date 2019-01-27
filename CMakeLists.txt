cmake_minimum_required(VERSION 2.8)	 # Проверка версии CMake.
										# Если версия установленой программы
										# старее указаной, произайдёт аварийный выход.

project(TestProject)			# Название проекта

set(SOURCE_EXE main.cpp)		# Установка переменной со списком исходников для исполняемого файла

set(SOURCE_LIB sqlite3.c)			# Тоже самое, но для библиотеки

set(USERS_LIB users_sqlite3_functions.c)

add_library(sqlite3 SHARED ${SOURCE_LIB})	# Создание динамической библиотеки с именем sqlite3

add_library(sqlite3_users SHARED ${USERS_LIB})

add_executable(main ${SOURCE_EXE})	# Создает исполняемый файл с именем main

target_link_libraries(main sqlite3_users sqlite3 pthread dl)		# Линковка программы с библиотекой