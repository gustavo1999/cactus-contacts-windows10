/**
Contacts List - A lightweight program for tracking a list of contacts.
Copyright(C) 2015  Gustavo Lopez

This program is free software : you can redistribute it and / or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.If not, see <http://www.gnu.org/licenses/>.
*/
#include "stdafx.h"


// TODO: reference any additional headers you need inside the file: STDAFX.H

using namespace std;

map<string, string> mapStore;

/* Reports results of Select statements. 
   Function will be called once for each statement executed by ssqlite3_exec()  */
static int callback(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	for (i = 0; i < argc; i++){
		printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
	}
	printf("\n");
	return 0;
}

int execSQLCmd(const char *fileName, const char *sql, int(*f)(void *NotUsed, int argc, char **argv, char **azColName)) {
	sqlite3 *db;
	char *zErrMsg = 0;
	int rc;

	//open the database 
	rc = sqlite3_open(fileName, &db);
	if (rc){
		fprintf(stderr, "\n\tCan't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		return(1);
	}
	const char* data = "Callback function called";
	//execute sql command on the database
	rc = sqlite3_exec(db, sql, f, (void*)data, &zErrMsg);

	//report any SQL errors
	if (rc != SQLITE_OK){
		fprintf(stderr, "\n\tSQL error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	//close database
	//To Do: determine if it would make sense to leave the database open while the program is running and make changes accordingly
	sqlite3_close(db);
	return 0;
}

/*utility function that splits a string by a delimiter into a vector of strings*/
void split(vector<string> &tokens, const string &text, char delim) {
	size_t start = 0, end = 0;
	while ((end = text.find(delim, start)) != string::npos) {
		tokens.push_back(text.substr(start, end - start));
		start = end + 1;
	}
	tokens.push_back(text.substr(start));
}

string get_env_var_win(string key) {
	LPCSTR lpName = key.c_str();
	char lpBuffer[33000];
	DWORD nSize = MAX_PATH;
	DWORD lpBufferSize = GetEnvironmentVariableA(lpName, lpBuffer, nSize);
	return lpBuffer == NULL ? std::string("") : std::string(lpBuffer);
}

/* Reports results of Select statements.
Function will be called once for each statement executed by ssqlite3_exec()  */
static int loadContactsIntoMapStore(void *NotUsed, int argc, char **argv, char **azColName){
	int i;
	string name, email;
	for (i = 0; i < argc; i++){
		//printf("%s = %s\n", azColName[i], argv[i] ? argv[i] : "NULL");
		name = argv[i];
		i++;
		email = argv[i];
		mapStore[name] = email;
	}
	printf("\n");
	return 0;
}

/*Loads contact list data into memory from a persistent datastore*/
void loadContactsDatafromDB(){
	//open existing cactus database or create a new one
	string userprofile = get_env_var_win("USERPROFILE");
	string filename = "\\.cactus.db";
	string fullpath = userprofile + filename;
	cout << "\n\tfullpath: " + fullpath + "\n";
	const char *fileName = fullpath.c_str();
	int rc;
	//rc = execSQLCmd(fileName, "create table if not exists contacts (name character varying(100),EMAIL character varying(150));");
	//rc = execSQLCmd(fileName, "insert into contacts (name,email) values ('AAA','A@A.COM')");
	rc = execSQLCmd(fileName, "select * from contacts", loadContactsIntoMapStore);
}

/*OBSOLETE - Loads contact list data into memory from a persistent datastore*/
void loadContactsDataFromFile(){
	//open existing cactus database or create a new one
	string userprofile = get_env_var_win("USERPROFILE");
	string filename = "\\.cactus.db";
	string fullpath = userprofile + filename;
	cout << "\n\tfullpath: " + fullpath + "\n";
	const char *fileName = fullpath.c_str();
	ifstream myfile("contactlist.txt");
	if (myfile.is_open())
	{
		const char delim = '\t';
		string line;
		vector<string> tokens;
		while (getline(myfile, line))
		{
			//DEBUG next line
			//cout << "loading line from file:" << line << '\n';
			split(tokens, line, delim);
			mapStore[tokens.at(0)] = tokens.at(1);
			tokens.erase(tokens.begin(), tokens.end());
		}
		myfile.close();
	}
	else cout << "An error occurred while loading your Contacts List from file: contactslist.txt";
}

/*saves contact list data to persistent datastore  */
void saveContactsData(){
	size_t length = mapStore.size();
	if (length > 0) {
		string userprofile = get_env_var_win("USERPROFILE");
		string filename = "\\.cactus.db";
		string fullpath = userprofile + filename;
		cout << "\n\tfullpath: " + fullpath + "\n";
		const char *fileName = fullpath.c_str();
		string name, email, sqlText;
		int rc;
		const char *sql;
		//clear contents of table
		sqlText = "delete * from contacts";
		sql = sqlText.c_str();
		rc = execSQLCmd(fileName, sql, NULL);
		//insert all records from memory into table
		for (std::map<string, string>::iterator i = mapStore.begin(); i != mapStore.end(); ++i) {
			name = i->first;
			email = i->second;
			sqlText = "insert into contacts (name,email) values ('" + name + "','" + email + "')";
			sql = sqlText.c_str();
			rc = execSQLCmd(fileName, sql,NULL);
		}
	}
}

/*Obsolete - saves contact list data to persistent datastore  */
void saveContactsDataToFile(){
	ofstream myfile;
	myfile.open("contactlist.txt");
	std::string theList;
	size_t length = mapStore.size();
	if (length > 0) {
		for (std::map<string, string>::iterator i = mapStore.begin(); i != mapStore.end(); ++i) {
			theList.append(i->first);
			theList.append("\t");
			theList.append(i->second);
			theList.append("\n");
			myfile << theList;
			theList = "";
		}
	}
	myfile.close();
}

/*ulity function that converts an integer primitive to a string object*/
string intToString(int Number){
	string Result;          // string which will contain the result
	ostringstream convert;   // stream used for the conversion
	convert << Number;      // insert the textual representation of 'Number' in the characters in the stream
	Result = convert.str(); // set 'Result' to the contents of the stream
	return Result;
}

/*ulity function that converts an integer primitive to a string object*/
string size_tToString(size_t number){
	string Result;          // string which will contain the result
	ostringstream convert;   // stream used for the conversion
	convert << number;      // insert the textual representation of 'Number' in the characters in the stream
	Result = convert.str(); // set 'Result' to the contents of the stream
	return Result;
}

/*captures a line of input from the command line and converts it to a string object*/
string readStringFromInput()
{
	string theString;
	getline(cin, theString);
	stringstream inputStream(theString);
	// if input is of type string return string
	inputStream >> theString;
	return theString;
}

/*utility function that causes the computer to emit a beeping sound*/
void errorNoise(){
	char d = (char)(7);
	printf("%c\n", d);
}

/*displays a set of commands that the user can invoke*/
void showMenu(){
	std::cout << "\n\t=======================================================\n";
	std::cout << "\n\tMain Menu\n\tEnter a number from 1 to 5 and press ENTER:\n";
	std::cout << "\t1. List All Contacts\n";
	std::cout << "\t2. Search by Name\n";
	std::cout << "\t3. Create New\n";
	std::cout << "\t4. Exit\n";
	std::cout << "\n\t> ";
}

/*displays a message to allow the user to display the Main Menu */
void showContinuePrompt(){
	cout << "\n\tPress Enter to Continue\n";
	cin.get();
}

/*creates a new contact record */
bool createNewContact(){
	//TODO: allow user to store additional fields beyond email address
	string key;
	string value;
	cout << "\n\t Full Name: ";
	key = readStringFromInput();
	cout << "\n\t Email Address: ";
	value = readStringFromInput();
	//TODO: Validate format of email address
	mapStore[key] = value;
	//TODO: save contacts list to disk
	cout << "\n\t 1 new record saved => \n\t\tName: " + key + "\n\t\tEmail Address: " + value + "\n";
	showContinuePrompt();
	saveContactsData();
	return true;
}

/*displays a list of the contacts in the database */
bool listAllContacts(){
	std::string theList;
	size_t length = mapStore.size();
	if (length > 0) {
		cout << "\n\t**********************\n";
		cout << "\n\t" + size_tToString(length) + " records found:\n\n";
		for (std::map<string, string>::iterator i = mapStore.begin(); i != mapStore.end(); ++i) {
			theList.append("\t");
			theList.append(i->first);
			theList.append(": ");
			theList.append(i->second);
			theList.append("\n");
			cout << theList;
			theList = "";
		}
		cout << "\n\t**********************\n";
	}
	else{
		cout << "\n\t**********************\n";
		cout << "0 records found:\n";
		cout << "\n\t**********************\n";
	}
	showContinuePrompt();
	return true;
}

/*displays a prompt allowing end user to search through the contact list by name */
bool lookupContact(){
	size_t length = mapStore.size();

	if (length > 0) {
		cout << "\tEnter the name to search: ";
		string searchTerm = readStringFromInput();
		std::size_t found;
		int countFound = 0;
		string contactName;
		string theList;

		for (std::map<string, string>::iterator i = mapStore.begin(); i != mapStore.end(); ++i) {
			contactName = i->first;
			std::transform(contactName.begin(), contactName.end(), contactName.begin(), ::tolower);
			found = contactName.find(searchTerm);
			if (found != std::string::npos){
				theList.append("\t");
				theList.append(contactName);
				theList.append(": ");
				theList.append(i->second);
				theList.append("\n");
				countFound++;
			}
		}
		cout << "\n\t**********************\n";
		cout << "\n\t" + intToString(countFound) + " records found:\n\n";
		cout << theList;
		cout << "\n\t**********************\n";
		theList = "";
	}
	else {
		cout << "\n\t**********************\n";
		cout << "0 records found:\n";
		cout << "\n\t**********************\n";
	}
	showContinuePrompt();
	return true;
}

/*saves edits to the contact list and exits the process */
bool exitThisProgram(){
	return false;
}

/*executes a command selected by the end user via the main menu*/
bool executeCommand(int command){
	switch (command){
	case 1:
		return listAllContacts(); break;
	case 2:
		return lookupContact(); break;
	case 3:
		return createNewContact(); break;
	case 4:
		return false;
	default:
		errorNoise();
		cout << "\n\tThat is not a valid selection.\n\tPlease enter a number from 1 to 5 and press ENTER.\n"; break;
	}
	return true;
}

/*loads a set of contacts into the contact list */
void loadTestData(){
	mapStore["Porky Pig"] = "porkypig@gmail.com";
	mapStore["Petunia Pig"] = "petuniapig@gmail.com";
	mapStore["Bugs Bunny"] = "bugs.bunny@aol.com";
	mapStore["Bugsy Seigel"] = "bugsy.l.siegel@hotmail.com";
	mapStore["Nelson Woodhouse"] = "nelson834@yahoo.com";
	mapStore["Woody Woodpecker"] = "wooooody@comcast.net";
}

/* */
int main(){
	//Main Menu
	cout << "\n\t@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";
	cout << "\tCactus Contacts [version 0.2] - Copyright (C) 2015 Gustavo Lopez\n";
	cout << "\tA lightweight program for tracking a list of contacts.\n";
	cout << "\tThis program comes with ABSOLUTELY NO WARRANTY\n";
	cout << "\tThis is free software, and you are welcome to redistribute it\n";
	cout <<   "\t@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n";

	//listen for commands and respond
	bool running = true;
	string userInput;
	int command = '0';

	//load contacts into memory so they can be displayed and modified
	loadContactsDatafromDB();

	//DEBUG purposes: Loads a set of dummy data
	//loadTestData();

	//displays the main menu and reacts to user commands
	//loop runs continously while program is running
	while (running){
		showMenu();
		userInput = "";
		getline(cin, userInput);
		stringstream inputStream(userInput);
		// if input is of type int execute the corresponding command
		if (inputStream >> command)
		{
			//cout << "You selected: " + intToString(command)+"endl";
			running = executeCommand(command);
		}
		else{
			errorNoise();
			cout << "\n\tThat is not a valid selection.\n\tPlease enter a number from 1 to 5 and press ENTER.\n" << endl;
		}
	}

	return 0;
}