/*
* Author(s)    : Parashar Parikh,
*				 Jaishree Jaikumar,
*				 Deborah Sinani
* File         : Header file for the database class
* Date created : 09/29/2022
* Date modified: 10/07/2022
*/

#pragma once
#include <iostream>
#include <vector>
#include <unordered_map>
#include <sstream>
#include <stdio.h>
#include "sqlite3.h"
#include <string>
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
	vector<std::unordered_map<string, string>> entries;

public:
	database();
	~database();
	bool open();
	bool create_users_table();
	bool create_cryptos_table();
	bool insert_into_users_table();
	bool insert_into_crypto_table();

	string process(string in);
	string buy(string command);
	string sell(string command);
	string list();
	string balance(string para_id);

	int callback(int argc, char** argv, char** azColName);

};