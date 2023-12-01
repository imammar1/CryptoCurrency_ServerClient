/*
* Author(s)    : Parashar Parikh,
*				 Ibrahim Ammar,
*				 Zeineb Moalla
*				 
* File         : Implementation file for the database class methods
* Date created : 09/29/2022
* Date modified: 11/20/2022
*/

#include "database.hpp"
static string login_error = "403 Wrong UserID or Password. \n";
static string invalid_command = "400 invalid command. \n";
static string invalid_arguments = "403 message format error. \n";
static string return_ok = "200  OK \n";
static string crypto_invalid = "404 Your search did not match any records. \n";

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
	}
	this->entries.push_back(table);
	return 0;
}

/*
* Description   : Constructor for the database class
* Pre-condition : -
* Post-condition: Creates and inserts into users and cryptos table
*/
database::database()
{
	cout << "Initializaing SQLITE3 Database!! \n";
	if (!open())
	{
		return;					// Exits call if failed to open database
	}

	create_users_table();
	create_cryptos_table();
	insert_into_users_table();  // Add values to users table
	insert_into_crypto_table(); // Add values to crypto table

	cout << "-----------------\n";

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
			fprintf(stdout, 
			"Opened database successfully\n");
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
string database::process(string in, string IP, int* id)
{
	user_id = id;
	string token = in.substr(0, in.find(" ")); // Separates commands with spacing
	string output;                             // to call buy and sell with their
	std::transform(token.begin(), token.end(), token.begin(), toupper);
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
		output = balance();
	}
	else if (token == "WHO")
	{
		output = who();
	}
	else if (token == "DEPOSIT") 
	{
		output = deposit(in);
	}
	else if (token == "LOOKUP")
	{
		output = lookup(in);
	}
	else if (token == "LOGIN") 
	{
		in.append(IP + " ");
		output = login(in);
	}
	else if (token == "LOGOUT") 
	{
		output = logout();
	}
	else if (token == "SHUTDOWN")
	{
		output = shutdown();
	}
	else if (token == "QUIT") 
	{
		if (*user_id != -1) {
			logout();
		}
		output = "200  OK \nClient disconnected\n";
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
	// First need to check if the user is logged in or not.
	if (*user_id == -1) {
		return invalid_command +
			"User not logged in!!\n";
	}

	// We are logged in
	int pos = 0;
	vector<string> tokens;
	while ((pos = command.find(" ")) != string::npos)         // Get the commands into 
	{                                                         // sub strings for error checking
		tokens.push_back(command.substr(0, pos));
		command.erase(0, pos + 1);
	}

	if (tokens.size() != 4) {
		return invalid_arguments +                            // Handles missing paramaters 
			"Missing arguments for buy!\n";						//JUST CHANGED GRAMMER CHANGES
	}

	// Add the user_id to the back of tokens
	tokens.push_back(std::to_string(*user_id));

	string sql_command = \
		"SELECT crypto_name, crypto_balance FROM cryptos " \
		"WHERE crypto_name = '" + tokens[1] + "' AND user_id = '" + std::to_string(*user_id) + "';"        \
		"SELECT first_name, usd_balance FROM users WHERE ID = '" + tokens.back() + "';";
	sql = &sql_command[0];										// Executes SQL statements
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);		// to fetch the crypto name  
																// and ID of buyer
																// 
	// Can the current user have access to the crypto?
	if (entries.size() == 1)
	{
		return invalid_arguments + "The crypto " + tokens[1] +               // Error handling if crypro DNE
			" does not exist! for the current user\n";
	}

	string crypto_name;
	string first_name;
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
			else if (itr->first == "first_name")
			{
				first_name = itr->second;
			}
			else
			{
				ss << " " << itr->second << " ";
			}
		}
	}

	entries.clear();				// Clears the table entries  
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
	ss << return_ok << "BOUGHT: New balance : "			// Adds new crypto and user balance	
		<< new_crypto_balance << " " << crypto_name		// to the string stream
		<< first_name + "'s New balance : $" 
		<< new_user_balance << endl;
          
	return ss.str();                                      // Returns stream as a string object
}

/*
* Description   : Handles users' selling of cryptocurrency
* Pre-condition : Type and amount of cryptos is passed as parameter
* Post-condition: Missing or invalid commands trigger an error message
*/
string database::sell(string command)
{
	// First need to check if the user is logged in or not.
	if (*user_id == -1) {
		return invalid_command +
			"User not logged in!!\n";
	}

	int pos = 0;
	vector<string> tokens;
	while ((pos = command.find(" ")) != string::npos)         // Get the commands into 
	{                                                         // sub strings for error checking
		tokens.push_back(command.substr(0, pos));
		command.erase(0, pos + 1);
	}

	if (tokens.size() != 4) {
		return invalid_arguments +                            // Handles missing paramaters 
			"Missing arguments for sell!\n";
	}
	
	// Add the user_id to the back
	tokens.push_back(std::to_string(*user_id));

	string sql_command = \
		"SELECT crypto_name, crypto_balance FROM cryptos " \
		"WHERE crypto_name = '" + tokens[1] + "' AND user_id = '" + std::to_string(*user_id) + "';"        \
		"SELECT first_name, usd_balance FROM users WHERE ID = '" + tokens.back() + "';";
	sql = &sql_command[0];										// Executes SQL statements
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);		// to fetch the crypto name  
																// and ID of seller

	// Can the current user have access to the crypto?
	if (entries.size() == 1)
	{
		return invalid_arguments + "The crypto " + tokens[1] +               // Error handling if crypro DNE
			" does not exist! for the current user\n";
	}

	string crypto_name;
	string first_name;
	std::stringstream ss;

	for (int i = 0; i < entries.size(); i++) {
		for (auto itr = entries[i].begin(); itr != entries[i].end(); ++itr)
		{
			if (itr->first == "crypto_name") {
				crypto_name = itr->second;
			}
			else if (itr->first == "crypto_balance")
			{
				ss << itr->second << " " << tokens[2]	// Sorts the order of display
					<< " " << tokens[3];				// for the user id, crypto name
														// and crypto balance
			}
			else if (itr->first == "first_name")
			{
				first_name = itr->second;
			}
			else
			{
				ss << " " << itr->second << " ";
			}
		}
	}

	entries.clear();							// Clears the table entries  
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
	ss << return_ok << "SOLD: New balance : "			// Adds new crypto and user balance	
		<< new_crypto_balance << " " << crypto_name		// to the string stream
		<< first_name + "'s New balance : $"
		<< new_user_balance << endl;

	return ss.str();                                      // Returns stream as a string object
}

/*
* Description   : Displays the data in cryptos tables as a list
* Pre-condition : A cryptos table is created
* Post-condition: All entries are stored and returned as a string
*/
string database::list()
{
	// First need to check if the user is logged in or not.
	if (*user_id == -1 && *user_id != 0) {
		return invalid_command +
			"User not logged in!!\n";
	}

	std::stringstream ss;
	ss << "The list of records in the Crypto "
		<< "database for user " << std::to_string(*user_id) << ": \n";
	std::string sql_command = "SELECT crypto_name, "\
							  "crypto_balance, user_id FROM cryptos";    // sql statement to list crypto name, crypto_balance

	// Ifira not the root user then get only that user's list
	if (*user_id != 0)
	{
		sql_command += " WHERE user_id = '" + std::to_string(*user_id) + "';";
	}
//	else // Root user so get names as well
//	{
//		sql_command += "; SELECT first_name FROM users;";
//	}

	sql = &sql_command[0];
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
string database::balance()
{
	// First need to check if the user is logged in or not.
	if (*user_id == -1) {
		return invalid_command +
			"User not logged in!!\n";
	}

	std::stringstream ss;
	ss << "Balance for user ";
	std::string user = "SELECT first_name, "\
		"last_name, usd_balance from Users WHERE ID = '" + std::to_string(*user_id) + "'";    // Executes sql statement
	sql = &user[0];                                             // to fetch user's first name, last 
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);     // name and balance 

	auto entry = entries.front();
	string bal;
	for (auto itr = entry.begin(); itr != entry.end(); ++itr)
	{
		if (itr->first != "usd_balance")
			bal.append(itr->second + " ");			// Adds user's USD balance into string stream
		else
			bal.append(": " + itr->second);			// Adds first and last name into string stream

	}                                                      
	ss << bal << " \n";
	return ss.str();                                           // Returns stream as a string object
}


/*
* Description	: Deposit funds into user balance
* Pre-condition	: Balance is needed to update user balance
* Post-condition: Missing or invalid commands trigger an error message
*/
string database::deposit(string command) 
{
	// First need to check if the user is logged in or not.
	if (*user_id == -1) {
		return invalid_command +
			"User not logged in!!\n";
	}

	std::stringstream ss;
    int pos = 0;
    vector<string> tokens;
    while ((pos = command.find(" ")) != string::npos)         // Get the commands into 
    {                                                         // sub strings for error checking
        tokens.push_back(command.substr(0, pos));
        command.erase(0, pos + 1);
    }

    if (tokens.size() != 2 ) {  // && userid == -1
        return invalid_arguments +                            // Handles missing paramaters
            "Missing arguments for Deposit!\n";                  // Only 2 arguments: Deposit and Amount
    }

    //This selects the USD balance from the user 
    string sql_command = "SELECT usd_balance FROM users WHERE ID = '" + std::to_string(*user_id) + "';";
    sql = &sql_command[0];
    rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);

	auto entry = entries[0].begin();
    //CALCULATIONS TO DEPOSIT MONEY INTO USERS USD BALANCE
	double user_deposit_amount = std::stod(tokens.back());
	double old_user_balance = std::stol(entry->second);

    if (user_deposit_amount < 0) {

        return invalid_command + "Invalid deposit amount!\n";
    }

	double new_user_balance = old_user_balance + user_deposit_amount;
    ss.str(std::string());                                // Clears the string stream

    ss << "UPDATE users SET usd_balance = "              // Updates the database to set the new balance for user
        << new_user_balance << " WHERE ID = '"
        << std::to_string(*user_id) << "'; ";

    sql_command = ss.str();                               // Executes sql statements to update database
    sql = &sql_command[0];
    rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);

    ss.str(std::string());
    ss << return_ok << "Deposit Successfully: New balance: "
        << " USD $" << new_user_balance << endl;          //// Adds deposit funds to new user balance to the string stream

    return ss.str();
}

/*
* Description	: Shows the current active users 
* Pre-condition	: None; The operation is only accessable by the root user
* Post-condition: Returns a string that will be sent to the user
*/
string database::who() 
{
	if (*user_id != 0) {
		return invalid_command +
			"DENIED: Non-root user dectected!!\n";
	}

	std::stringstream ss;
	ss << return_ok << "The list of the active users: \n";
	for (auto itr = active_users.begin(); itr != active_users.end(); ++itr) 
	{
		ss << itr->first.second << "  " << itr->second << endl;
	}

	return ss.str();
}

/*
* Description	: Finds a crypto entry based on the given user string
* Pre-condition	: User must be logged in.
* Post-condition: Returns the crypto entries that match the user search string
*/
string database::lookup(string command) 
{
	// First need to check if the user is logged in or not.
	if (*user_id == -1) {
		return invalid_command +
			"User not logged in!!\n";
	}

	std::stringstream ss;
	vector<string> tokens;
	int pos = 0;

	// Divides string into substrings: LOOKUP (name of crypto)
	while ((pos = command.find(" ")) != string::npos) {
		tokens.push_back(command.substr(0, pos));
		command.erase(0, pos + 1);
	}

	// Since there are 2 arguments: LOOKUP and crypto_name
	if (tokens.size() != 2) {
		return invalid_arguments + "Missing arguments for Lookup!\n";
	}

	// Used to find similar coins
	tokens[1].push_back('%');
	string sql_command = "SELECT crypto_name, crypto_balance " \
		"FROM cryptos WHERE crypto_name LIKE '" + tokens[1] + "' and user_id = '" + std::to_string(*user_id) + "';";
	sql = &sql_command[0];
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);

	if (entries.size() == 0) {
		return crypto_invalid;
	}

	ss << "Found " << entries.size() << " match! \n";

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
	return ss.str();
}

/*
* Description	: Allows the client to login given the username and the password
* Pre-condition	: Username and password given by the client
* Post-condition: Returns if the login was successfull and adds the user to the list of active users
*/
string database::login(string command) 
{
	std::stringstream ss;
	int pos = 0;
	vector<string> tokens;
	while ((pos = command.find(" ")) != string::npos)         // Get the commands into sub strings
	{                                                         // for error checking
		tokens.push_back(command.substr(0, pos));
		command.erase(0, pos + 1);
	}

	if (tokens.size() != 4) {  // && userid == -1
		return invalid_arguments +                            // Handles missing paramaters
			"Missing arguments for login!\n";                  // Only 2 arguments: Deposit and Amount
	}

	// Try to find the matching username and password
	string sql_command = "SELECT ID, first_name FROM users WHERE user_name='" + tokens[1] + "' and password='" + tokens[2] + "';";
	sql = &sql_command[0];
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);

	if (entries.empty())
	{
		return login_error;
	}

	// User found in database! Now check if the user is already logged in somewhere else.
	int id;
	string first_name;

	// Get the user id and first name
	auto entry = entries.front();
	for (auto itr = entry.begin(); itr != entry.end(); ++itr) 
	{
		if (itr->first == "ID")
		{
			id = std::stoi(itr->second);
		}
		else if (itr->first == "first_name")
		{
			first_name = itr->second;
		}
	}

	auto user = active_users.find({ id, first_name });

	// Check if the current user is already logged in!!
	if (*user_id != -1)
	{
		return invalid_command +
			"DENIDED: Please logout before logging in as another user.\n";
	}
	else if (user != active_users.end())
	{
		return invalid_command +
			"DENIDED: You are already logged in somewhere else!! \n";
	}

	// Add to the list of active users
	active_users.insert({ { id, first_name }, tokens.back()});
	//active_users.insert({ id, first_name });
	*user_id = id;
	return return_ok;
}

/*
* Description   : Logouts the user
* Pre-condition : -
* Post-condition: logouts the user and removes them from the list of active users
*/
string database::logout() 
{
	if (*user_id == -1)
	{
		return invalid_command + 
			"No user currently logged in!!\n";
	}

	// Get the first name
	string sql_command = "SELECT first_name FROM users WHERE ID ='" + std::to_string(*user_id) + "';";
	sql = &sql_command[0];
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);

	auto entry = entries.front().begin();
	string first_name = entry->second;

	// Remove from the active user's list
	auto user = active_users.find(std::make_pair(*user_id, first_name));
	if (user != active_users.end())
	{
		active_users.erase(user);
		*user_id = -1;
		return return_ok;
	}
	else
	{
		return invalid_command +
			"ERROR: logout failed!\n";
	}
}

/*
* Description   : Shutdowns the server
* Pre-condition : -
* Post-condition: Checks if the current user can shutdown the server
*/
string database::shutdown() 
{
	if (*user_id != -1)
		return return_ok + "Shutting down the server!\n";
	else
		return invalid_command + "DENIED: User not logged in!!\n";
}


/*
* Description   : Destructor for database class
* Pre-condition : -
* Post-condition: Closes and frees the database
*/
database::~database()
{
	// Closes the database
	sqlite3_close(db);
}

/*
* Description   : Creates the Users table
* Pre-condition : Users table does not exist
* Post-condition: Displays an error message if unable to create table
*/
bool database::create_users_table()
{
	string temp = \
		"create table if not exists users("	\
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
		//fprintf(stderr, "SQL error: %s\n", zErrMsg);           // Error handling
		return false;
	}
	else {
		//fprintf(stdout, "Table created successfully\n");
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
	string temp = \
		"create table if not exists cryptos("	\
		"ID INTEGER PRIMARY KEY AUTOINCREMENT,"			\
		"crypto_name TEXT NOT NULL,"				\
		"crypto_balance DOUBLE,"						\
		"user_id TEXT,"									\
		"FOREIGN KEY(user_id) REFERENCES users(ID)); ";

	sql = &temp[0];                                              // Executes sql statement to
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);      // create cryptos table

	if (rc != SQLITE_OK)
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);             // Error handling
		return false;
	}
	else 
	{
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
	string sql_command;
	// First check if the table is empty
	sql_command = "select count(*) from users;";
	sql = &sql_command[0];
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);
	auto entry = entries[0].begin();
	if (entry->second != "0") { // If the entry is not zero then the user tables has values
		entries.clear();
		return true;
	}

	// User table is empty so lets add some values
	std::string temp = \
		"INSERT INTO users (ID, email, first_name ,last_name, user_name, password, usd_balance)"  \
		"VALUES (0,'root', 'root', 'N/A', 'root', 'Root_01', 69420.00 ); " \
		
		"INSERT INTO users (ID,email, first_name ,last_name, user_name, password, usd_balance)"  \
		"VALUES (1,'alex.park@hotmail.com', 'Alex', 'Park', 'alexp', 'Park-01', 100.00 ); " \

		"INSERT INTO Users (ID, email, first_name,last_name,user_name,password,usd_balance) "  \
		"VALUES (2, 'brain.smith@gmail.com','Brian', 'Smith', 'brians', 'Smith-02', 100.00 ); "     \

		"INSERT INTO Users (ID, email, first_name, last_name, user_name, password, usd_balance)" \
		"VALUES (3, 'carlosj@gmail.com','Carl', 'Johnson', 'carlj', 'Johnson-03', 100.00 );" \

		"INSERT INTO Users (ID, email, first_name, last_name, user_name, password, usd_balance)" \
		"VALUES (4,'ravendan@gmail.com', 'Danny', 'Raven', 'dannyr', 'Raven-04', 100.00 );"	\

		"INSERT INTO Users (ID, email, first_name, last_name, user_name, password, usd_balance)" \
		"VALUES (5, 'harry.pottah@gmail.com', 'Harry', 'Potter', 'harryp', 'Potter-08', 100.00 );"	\

		"INSERT INTO Users (ID, email, first_name, last_name, user_name, password, usd_balance)" \
		"VALUES (6, 'baba.yaga@gmail.com', 'John', 'Wick', 'johnw', 'Wick-10', 100.00 );";

	sql = &temp[0];                                                          // Executes sql statement to
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);                  // insert data into Users table

	if (rc != SQLITE_OK) 
	{
		fprintf(stderr, "SQL error: %s\n", zErrMsg);                         // Error handling
		return false;
	}
	else 
	{
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
	string sql_command;
	// First check if the table is empty
	sql_command = "select count(*) from cryptos;";
	sql = &sql_command[0];
	rc = sqlite3_exec(db, sql, c_callback, this, &zErrMsg);
	auto entry = entries[0].begin();
	if (entry->second != "0") { // If the entry is not zero then the crypto tables has values
		entries.clear();
		return true;
	}

	// Table is empty so lets add some values to it 
	sql_command = \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id)" \
		"VALUES (1, 'Bitcoin', 19483.23, 1);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (2, 'Ethereum', 1330.40, 1);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (3, 'Tether', 1.00, 1);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (4, 'USDC', 1.00, 3);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (5, 'Binance', 282.48, 2);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (6, 'XRP', 0.48, 4);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (7, 'BinanceUSD', 1.00, 2);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (8, 'Cardano', 0.44, 3);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (9, 'Solano', 33.86, 2);" \
		"INSERT INTO cryptos (ID,crypto_name,crypto_balance,user_id) " \
		"VALUES (10, 'Dogecoin', 0.06, 2);";

	sql = &sql_command[0];                                                          // Executes sql statement to
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