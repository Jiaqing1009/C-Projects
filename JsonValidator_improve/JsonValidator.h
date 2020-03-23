#ifndef JSONVALIDATOR_H_
#define JSONVALIDATOR_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdlib.h>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <thread>
using namespace std;

class JsonValidator
{
private:
	int array_pointer = 0;
	string input_string;
	char current_char = '\0';
	vector<string> validate_string;
	
	struct thread_data{
		string input_string;
	};

	//Read a file and transfer it to a String by giving its file path
	string readFileToString(string filename);

	//Remove '\n' '\r' and ' ' before and after input string
	string& trim(string& str);

public:

	//Return next char
	char nextChar();

	//Skip the space in the JSON file
	char nextRealChar();

	//validate the array structure in parallel
	bool validateArray_parallel();

	//validate the object structure in parallel
	bool validateObject_parallel();

	//validate the string structure in parallel
	void *validateString_parallel(void *thread_string);

	//validate the number structure
	void validateNumber();

	//validate the special value true/false/null
	void validateTFN();

	//Main function to validate the JSON file
	bool isJSON(const std::wstring &input);

	//Main function to validate the JSON file in parallel
	bool isJSON_parallel(string input);

	//Main function (Program entry)
	int main(int argc,char *argv[]);
};

#endif