#ifndef JSONVALIDATOR_H_
#define JSONVALIDATOR_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <time.h>
#include <pthread.h>
#include <vector>
#include <thread>
#include <wmmintrin.h>
#include <x86intrin.h>
#include <nmmintrin.h>
#include <chrono>
#include <zconf.h>
#include <cstring>
using namespace std;

class JsonValidator
{
private:

    int array_pointer = 0; //give the position of the char which will be validated in the string
    char* input_string; //the string which is waiting for validating
    char current_char = '\0'; //the char which will be validated in the string
    size_t input_end; //the end of the input char array

    struct thread_data{
        vector<int> string_start_position; //the start positions of a string in step 2
    };

    int td_index = 0; //record which element to put in td

    //Read a file and transfer it to a char* by giving its file path
    char* read_file_to_char(char* file_path);

    //Remove '\n' '\r' and ' ' before and after input string in parallel
    inline void trim_parallel(char *bytes, size_t howmany);

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
	static bool isJSON(const std::wstring &input);

	//Main function to validate the JSON file in parallel
	static bool isJSON_parallel(string input);

	//Main function (Program entry)
	int main(int argc,char *argv[]);
};

#endif