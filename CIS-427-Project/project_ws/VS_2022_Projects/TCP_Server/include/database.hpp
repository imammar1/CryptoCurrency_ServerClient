#pragma once
/*
* Author(s)    : Parashar Parikh,
*				 Ibrahim Ammar,
*				 Zeineb Moalla
* File         : Header file for the database class
* Date created : 09/29/2022
* Date modified: 11/20/2022
*/

#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <map>
#include <sstream>
#include <stdio.h>
#include <algorithm>
#include "sqlite3.h"
#include <string>
#include <stdexcept>
#define DATABASE_PATH "cis427_crypto.sqlite"

using std::string;
using std::vector;
using std::cout;
using std::endl;


class database
{
private:
	sqlite3* db;
	char* zErrMsg = 0;
	int rc;
	char* sql;
	int* user_id;
	vector<std::unordered_map<string, string>> entries;
	std::map<std::pair<int, string>, string > active_users;


public:
	database();
	~database();
	bool open();
	bool create_users_table();
	bool create_cryptos_table();
	bool insert_into_users_table();
	bool insert_into_crypto_table();

	string process(string in, string IP, int* id);
	string buy(string command);
	string sell(string command);
	string list();
	string balance();
	string shutdown();

	//Program 2: New functions : DEPOSIT, LOOKUP, WHO, LOGIN, LOGOUT
	string deposit(string command);
	string lookup(string command);
	string who();
	string login(string command);
	string logout();

	int callback(int argc, char** argv, char** azColName);

};