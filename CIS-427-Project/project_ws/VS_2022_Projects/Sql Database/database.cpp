/*
* Author(s)    : Parashar Parikh,
*				 Jaishree Jaikumar,
*				 Deborah Sinani
* File         : Implementation file for the database class methods
* Date created : 09/29/2022
* Date modified: 10/07/2022
*/

#include "database.h"
static string invalid_command = "400 invalid command. \n";
static string invalid_arguments = "403 message format error. \n";
static string return_ok = "200  OK \n";

/*
* Description   :
* Pre-condition :
* Post-condition:
*/
static int c_callback(void* param, int argc, char** argv, char** azColName)
{
	database* db_object = reinterpret_cast<database*>(param);
	return db_object->callback(argc, argv, azColName);
}

/*
* Description   : Stores the values from the table into a map
* Pre-condition : Called inside the sqlite3_exec function
* Post-condition: Argument values from the SQL statements are pushed into the map
*/
int database::callback(int argc, char** argv, char** azColName)
{
	std::unordered_map<string, string> table;
	int i;
	for (i = 0; i < argc; i++)
	{
		string col_name = azColName[i];
		string col_value = argv[i] ? argv[i] : "NULL";
		table.insert(std::pair<string, string>(col_name, col_value));
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	this->entries.push_back(table);
	//printf("\n");
	return 0;
}

/*
* Description   : Constructor for the database class
* Pre-condition : -
* Post-condition: Creates and inserts into users and cryptos table
*/
database::database()
{
	if (!open())
	{
		return;					// Exits call if failed to open database
	}

	create_users_table();
	create_cryptos_table();
	insert_into_users_table();  // Add values to users table
	insert_into_crypto_table(); // Add values to crypto table

}

/*
* Description   : Opens the selected database
* Pre-condition : Database path set
* Post-condition: Throws an error if unable to open database
*/
bool database::open()
{
	try                                              // Exeption handling
	{                                                // for opening the
		rc = sqlite3_open(DATABASE_PATH, &db);       // database
		if (!rc)
		{
			//fprintf(stdout, 
			//"Opened database successfully\n");
			return(true);
		}
		else
		{
			throw(rc != SQLITE_OK);
		}
	}
	catch (int err)
	{
		fprintf(stderr, "Can't open database: %s\n", // Prints error message
			sqlite3_errmsg(db));                     // when unable to open
		return(false);                               // database
	}
}

/*
* Description   : Interprets user commands into function calls
* Pre-condition : Command from user is passed as parameter
* Post-condition: Displays an error message in case of invalid command
*/
string database::process(string in)
{
	string token = in.substr(0, in.find(" ")); // Separates commands with spacing
	string output;                             // to call buy and sell with their
	                                           // respective parameters
	if (token == "BUY")
	{
		output = buy(in);
	}
	else if (token == "SELL")
	{
		output = sell(in);
	}
	else if (token == "LIST")
	{
		output = list();
	}
	else if (token == "BALANCE")
	{
		output = balance("1");
	}
	else if (token == "SHUTDOWN")
	{
	}
	else
	{
		output = "400 invalid command. \n";   // Prints error message if
	}                                         // command is invalid

	entries.clear();                          // Clears the map to allow for the next call
	return output;                            // Returns the method's return value as a string
}

/*
* Description   : Handles users' purchase of cryptocurrency
* Pre-condition : Type and amount of cryptos is passed as parameter
* Post-condition: Missing or invalid commands trigger an error message
*/
string database::buy(string command)
{
	int pos = 0;
	vector<string> tokens;
	while ((pos = command.find(" ")) != string::npos)         // Get the commands into 
	{                                                         // sub strings for error checking
		tokens.push_back(command.substr(0, pos));
		command.erase(0, pos + 1);
	}

	if (tokens.size() != 5) {
		return invalid_arguments +                            // Handles missing paramaters 
			"Missing arguments for sell!\n";
	}


	string sql_command = "SELECT crypto_name, crypto_balance, user_id " \
		"FROM cryptos WHERE crypto_name = '" + tokens[1] + "'; "        \
		"SELECT usd_balance FROM users WHERE ID = '" + tokens.back() + "';";
	sql = &sql_command[0];                                    // Executes SQL statements
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);   // to fetch the crypto name  
	                                                          // and ID of buyer

	if (entries.empty())
	{
		return invalid_arguments + "The crypto "              // Error handling if no
			+ tokens[1] + " does not exist!\n";               // parameters passed
	}

	string crypto_name;
	string user_id;
	std::stringstream ss;

	for (int i = 0; i < entries.size(); i++) {
		for (auto itr = entries[i].begin(); itr != entries[i].end(); ++itr)
		{
			if (itr->first == "crypto_name") {
				crypto_name = itr->second;
			}
			else if (itr->first == "crypto_balance")
			{
				ss << itr->second << " " << tokens[2]    // Sorts the order of display
					<< " " << tokens[3];                 // for the user id, crypto name
				                                         // and crypto balance
			}
			else if (itr->first == "user_id")
			{
				user_id = itr->second;
			}
			else
			{
				ss << " " << itr->second << " ";
			}
		}
	}

	if (user_id != tokens.back())                         // Checks if the user_id is valid
		return invalid_command +
		"User id does not match!!\n";

	entries.clear();									  // Clears the table entries  
	                                                      // for getting the new balance
	double user_buy_amount;
	double user_buy_price;
	double old_crypto_balance;
	double old_user_balance;

	ss >> old_crypto_balance >> user_buy_amount
		>> user_buy_price >> old_user_balance;
	double new_user_balance = old_user_balance -          // Calculates the new crypto 
		(user_buy_amount * user_buy_price);               // and user balance
	double new_crypto_balance = old_crypto_balance
		- user_buy_amount;

	if (new_crypto_balance < 0 || new_user_balance < 0) {

		return invalid_command + "Not enough balance";
	}

	ss.str(std::string());                                // Clears the string stream

	ss << "UPDATE cryptos SET crypto_balance = "
		<< new_crypto_balance << " WHERE crypto_name = '"
		<< tokens[1] << "'; "\
		<< "UPDATE users SET usd_balance = "              // Updates the database
		<< new_user_balance << " WHERE ID = '"
		<< tokens.back() << "'; ";

	sql_command = ss.str();                               // Executes sql statements
	sql = &sql_command[0];                                // to update database
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);

	ss.str(std::string());
	ss << return_ok << "BOUGHT: New balance: "
		<< new_crypto_balance << " " << crypto_name       // Adds new crypto and user balance
		<< " USD $" << new_user_balance << endl;          // to the string stream
	return ss.str();                                      // Returns stream as a string object
}

/*
* Description   : Handles users' selling of cryptocurrency
* Pre-condition : Type and amount of cryptos is passed as parameter
* Post-condition: Missing or invalid commands trigger an error message
*/
string database::sell(string command)
{
	int pos = 0;
	vector<string> tokens;
	while ((pos = command.find(" ")) != string::npos)         // Get the commands into 
	{                                                         // sub strings for error checking
		tokens.push_back(command.substr(0, pos));
		command.erase(0, pos + 1);
	}

	if (tokens.size() != 5) {
		return invalid_arguments +                            // Handles missing paramaters 
			"Missing arguments for sell!\n";
	}


	string sql_command = "SELECT crypto_name, crypto_balance, user_id " \
		"FROM cryptos WHERE crypto_name = '" + tokens[1] + "'; "        \
		"SELECT usd_balance FROM users WHERE ID = '" + tokens.back() + "';";
	sql = &sql_command[0];                                    // Executes SQL statements
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);   // to fetch the crypto name  
	                                                          // and ID of seller

	if (entries.empty())
	{
		return invalid_arguments + "The crypto "              // Error handling if no
			+ tokens[1] + " does not exist!\n";               // parameters passed
	}

	string crypto_name;
	string user_id;
	std::stringstream ss;

	for (int i = 0; i < entries.size(); i++) {
		for (auto itr = entries[i].begin(); itr != entries[i].end(); ++itr)
		{
			if (itr->first == "crypto_name") {
				crypto_name = itr->second;
			}
			else if (itr->first == "crypto_balance")
			{
				ss << itr->second << " " << tokens[2]    // Sorts the order of display
					<< " " << tokens[3];                 // for the user id, crypto name
				                                         // and crypto balance
			}
			else if (itr->first == "user_id")
			{
				user_id = itr->second;
			}
			else
			{
				ss << " " << itr->second << " ";
			}
		}
	}


	if (user_id != tokens.back())                         // Checks if the user_id is valid
		return invalid_command +
		"User id does not match!!\n";

	entries.clear();									  // Clears the table entries  
	                                                      // for getting the new balance
	double user_sell_amount;
	double user_sell_price;
	double old_crypto_balance;
	double old_user_balance;

	ss >> old_crypto_balance >> user_sell_amount
		>> user_sell_price >> old_user_balance;
	double new_user_balance = old_user_balance +          // Calculates the new crypto 
		(user_sell_amount * user_sell_price);             // and user balance
	double new_crypto_balance = old_crypto_balance
		+ user_sell_amount;


	ss.str(std::string());                                // Clears the string stream

	ss << "UPDATE cryptos SET crypto_balance = "
		<< new_crypto_balance << " WHERE crypto_name = '"
		<< tokens[1] << "'; "\
		<< "UPDATE users SET usd_balance = "              // Updates the database
		<< new_user_balance << " WHERE ID = '"
		<< tokens.back() << "'; ";

	sql_command = ss.str();                               // Executes sql statements
	sql = &sql_command[0];                                // to update database
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);

	ss.str(std::string());
	ss << return_ok << "SOLD: New balance: "
		<< new_crypto_balance << " " << crypto_name       // Adds new crypto and user balance
		<< " USD $" << new_user_balance << endl;          // to the string stream
	return ss.str();                                      // Returns stream as a string object
}

/*
* Description   : Displays the data in cryptos tables as a list
* Pre-condition : A cryptos table is created
* Post-condition: All entries are stored and returned as a string
*/
string database::list()
{
	std::stringstream ss;
	ss << "The list of records in the Crypto "
		<< "database for user 1: \n";
	std::string list = "SELECT crypto_name, "\
		"crypto_balance, user_id FROM cryptos";                // Executes sql statements
	sql = &list[0];                                            // to list crypto name, crypto
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);    // balance and user id

	for (auto i = 0; i < entries.size(); i++)
	{
		string entry;
		for (auto itr = entries[i].begin();
			itr != entries[i].end(); ++itr)
		{
			entry.append(itr->second + " ");                   // Stores data fetched into a string
		}
		ss << entry << endl;
	}

	return ss.str();                                           // Returns stream as a string object
}

/*
* Description   : Displays the balance of a user in USD
* Pre-condition : User ID is passed as parameter
* Post-condition: User's balance is fetched and returned as a string
*/
string database::balance(string para_id)
{
	std::stringstream ss;
	ss << "Balance for user ";
	std::string user = "SELECT first_name, "\
		"last_name from Users WHERE ID = '" + para_id + "'";    // Executes sql statement
	sql = &user[0];                                             // to fetch user's first 
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);     // and last name 

	for (auto i = 0; i < entries.size(); i++)
	{
		string entry;
		for (auto itr = entries[i].begin();
			itr != entries[i].end(); ++itr)
		{
			entry.append(itr->second + " ");
		}                                                      // Adds first and last name
		ss << entry;                                           // into string stream
	}
	ss << ": ";
	std::string balance = "SELECT usd_balance from "\
		"Users WHERE ID = '" + para_id + "'";                  // Executes sql statement
	sql = &balance[0];                                         // to fetch user's USD 
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);    // balance
	//printf("%s", zErrMsg);

	for (auto i = 1; i < entries.size(); i++)
	{
		string entry;
		auto itr = entries[i].begin();
		entry.append(itr->second);                             // Adds user's USD balance
		ss << entry << endl;                                   // into string stream    
	}

	return ss.str();                                           // Returns stream as a string object
}

/*
* Description   : Destructor for database class
* Pre-condition : -
* Post-condition: Closes and frees the database
*/
database::~database()
{
	sqlite3_free(db);
	sqlite3_close(db);
}

/*
* Description   : Creates the Users table
* Pre-condition : Users table does not exist
* Post-condition: Displays an error message if unable to create table
*/
bool database::create_users_table()
{
	string temp = "create table if not exists users("	\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"			\
		"email TXT NOT NULL,"							\
		"first_name TEXT,"								\
		"last_name TEXT,"								\
		"user_name TEXT NOT NULL,"						\
		"password TEXT,"								\
		"usd_balance DOUBLE NOT NULL); ";

	sql = &temp[0];                                            // Executes sql statement to
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);    // create users table

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);           // Error handling
		return false;
	}
	else {
		fprintf(stdout, "Table created successfully\n");
		return true;
	}
}

/*
* Description   : Creates the Cryptos table
* Pre-condition : Cryptos table does not exist
* Post-condition: Displays an error message if unable to create table
*/
bool database::create_cryptos_table()
{
	string temp = "create table if not exists cryptos("	\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"			\
		"crypto_name varchar(10) NOT NULL,"				\
		"crypto_balance DOUBLE,"						\
		"user_id TEXT,"									\
		"FOREIGN KEY(user_id) REFERENCES users(ID)); ";

	sql = &temp[0];                                              // Executes sql statement to
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);      // create cryptos table

	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);             // Error handling
		return false;
	}
	else {
		fprintf(stdout, "Table created successfully\n");
		return true;
	}
}

/*
* Description   : Inserts data into Users table
* Pre-condition : Users table exists
* Post-condition: Displays an error message if unable to insert values
*/
bool database::insert_into_users_table()
{
	std::string temp = "INSERT INTO users (ID,first_name,last_name,user_name,password,usd_balance)"  \
		"VALUES (1, 'Alex', 'Park', 'alexp', 'Park-01', 100.00 ); " \
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance) "  \
		"VALUES (2, 'Brian', 'Smith', 'brians', 'Smith-02', 100.00 ); "     \
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (3, 'Carl', 'Johnson', 'carlj', 'Johnson-03', 100.00 );" \
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (4, 'Danny', 'Raven', 'dannyr', 'Raven-04', 100.00 );"
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (5, 'Eli', 'Hayes', 'elih', 'Hayes-05', 100.00 );"
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (6, 'Fabian', 'Rutter', 'fabianr', 'Rutter-06', 100.00 );"
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (7, 'Gabby', 'Evans', 'gabbye', 'Evans-07', 100.00 );"
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (8, 'Harry', 'Potter', 'harryp', 'Potter-08', 100.00 );"
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (9, 'Inez', 'Martin', 'inezm', 'Martin-09', 100.00 );"
		"INSERT INTO Users (ID,first_name,last_name,user_name,password,usd_balance)" \
		"VALUES (10, 'John', 'Wick', 'johnw', 'Wick-10', 100.00 );";

	sql = &temp[0];                                                          // Executes sql statement to
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);                  // insert data into Users table
	                                                                         
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);                         // Error handling
		return false;
	}
	else {
		fprintf(stdout, "Inserted user values successfully\n");
		return true;
	}
}

/*
* Description   : Inserts data into Cryptos table
* Pre-condition : Cryptos table exists
* Post-condition: Displays an error message if unable to insert values
*/
bool database::insert_into_crypto_table()
{
	string temp = "INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id)" \
		"VALUES (1, 'Bitcoin', 19483.23, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (2, 'Ethereum', 1330.40, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (3, 'Tether', 1.00, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (4, 'USDC', 1.00, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (5, 'Binance Coin', 282.48, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (6, 'XRP', 0.48, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (7, 'Binance USD', 1.00, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (8, 'Cardano', 0.44, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (9, 'Solano', 33.86, 1)" \
		"INSERT INTO Cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (10, 'Dogecoin', 0.06, 1);";

	sql = &temp[0];                                                          // Executes sql statement to
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);                  // insert data into Cryptos table         
	                                                                         
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL error: %s\n", zErrMsg);                         // Error handling
		return false;
	}
	else {
		fprintf(stdout, "Inserted crypto values successfully\n");
		return true;
	}
}



// Client code to test database class methods
int main(int argc, char* argv[]) 
{
	database db;
	cout << db.process("LIST ") << endl;
	cout << db.process("BALANCE ") << endl;
	cout << db.process("SELL Bitcoin 50 1.45 1 ") << endl;
	cout << db.process("LIST ") << endl;
	cout << db.process("BALANCE ") << endl;
	cout << db.process("BUY Tether 400 1.45 1 ") << endl;
	cout << db.process("LIST ") << endl;
	cout << db.process("BALANCE ") << endl;

	return 0;
}