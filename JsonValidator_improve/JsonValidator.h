#include <string>
#include <cstring>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <pthread.h>
using namespace std;

class JsonValidator
{
private:
	int array_pointer = 0;
	string input_string;
	char current_char = '\0';

	//Read a file and transfer it to a String by giving its file path
	string readFileToString(string filename);

	//Remove '\n' '\r' and ' ' before and after input string
	string& trim(string& str);

public:
	//Return next char
	char nextChar();

	//Skip the space in the JSON file
	char nextRealChar();

	//validate the array structure
	bool validateArray();

	//validate the object structure
	bool validateObject();

	//validate the string structure
	void validateString();

	//validate the number structure
	void validateNumber();

	//validate the special value true/false/null
	void validateTFN();

	//Main function to validate the JSON file
	bool isJSON(const std::wstring &input);

	int main();
};
