#include "JsonValidator.h"

#define NUM_THREADS 6 //set the number of threads as the number of cores

int array_pointer = 0; //give the position of the char which will be validated in the string
char* input_string; //the char array (JSON string) which is waiting for validating
char current_char = '\0'; //the char which will be validated in the string
size_t input_end; //the end of the input char array

struct thread_data{
    vector<int> string_start_position; //the start positions of a string in step 2
};
struct thread_data td[NUM_THREADS]; //set the number of elements as the number of threads

int td_index = 0; //record which element to put in td

bool validateArray_parallel();

//the amount of bytes the file is initially
#ifndef _READFILE_BYTES
#define _READFILE_BYTES 256
#endif

//Read a file and transfer it to a char* by giving its file path
char* read_file_to_char(char* file_path){
    FILE* file = fopen(file_path, "r");
    if(!file){ //the file does not exist from the given path
        return NULL;
    }
    //allocate memory according to the initial bytes
    char* result = static_cast<char *>(malloc(sizeof(char) * _READFILE_BYTES + 1)); //+1 for trailing null byte
    if(result == NULL){ //memory allocation failed
        return NULL;
    }
    size_t pos = 0; //set the position in the string to write the character to
    size_t capacity = _READFILE_BYTES; //set the amount of memory allocated for the string
    char ch; //set a char to hold the currently-read char from the file
    while((ch = getc(file)) != EOF){ //until the char is the EOF char
        result[pos++] = ch; //update the character at position 'pos' to 'ch'
        if(pos >= capacity){ //if the next position would exceed bounds
            capacity += _READFILE_BYTES; //add the guess to the capacity
            //allocate memory accordingly
            result = static_cast<char *>(realloc(result, sizeof(char) * capacity + 1)); //+1 for trailing null byte
            if(result == NULL){ //memory allocation failed
                return NULL;
            }
        }
    }
    fclose(file); //close the file
    result = static_cast<char *>(realloc(result, sizeof(char) * pos)); //remove extra memory
    //memory allocation failed
    if(result == NULL)
        return NULL;
    result[pos] = '\0';  //'pos' now points to the index after the last character read
    return result;
}

//standard bit twiddling
#define haszero(v) \
  (((v)-UINT64_C(0x0101010101010101)) & ~(v)&UINT64_C(0x8080808080808080))

//Remove '\n' '\r' and ' ' before and after input string in parallel
inline void trim_parallel(char *bytes, size_t len){
    size_t pos = 0;
    size_t i = 0;
    uint64_t word = 0;
    uint64_t mask1 = ~UINT64_C(0) / 255 * (uint64_t)('\r');
    uint64_t mask2 = ~UINT64_C(0) / 255 * (uint64_t)('\n');
    uint64_t mask3 = ~UINT64_C(0) / 255 * (uint64_t)(' ');
    for (; i + 7 < len; i += 8) {
        memcpy(&word, bytes + i, sizeof(word));
        uint64_t xor1 = word ^ mask1;
        uint64_t xor2 = word ^ mask2;
        uint64_t xor3 = word ^ mask3;
        if (haszero(xor1) || haszero(xor2) || haszero(xor3)) {
            for (int k = 0; k < 8; ++k) {
                char c = bytes[i + k];
                if (c == '\r' || c == '\n' || c == ' ') {
                    continue;
                }
                bytes[pos++] = c;
            }
        } else {
            memmove(bytes + pos, bytes + i, sizeof(word));
            pos += 8;
        }
    }
    for (; i < len; ++i) { //deal with the remaining char
        char c = bytes[i];
        if (c == '\r' || c == '\n' || c == ' ') {
            continue;  //whitespaces found
        }
        bytes[pos++] = c;
    }
    input_end = pos - 1; //save the end of the new char array
}

//Return next char
char nextChar(){
	//if (array_pointer < 0 || (array_pointer >= input_string.length()))
	if (array_pointer < 0 || (array_pointer > input_end))
	{
		return 0;
	}
	current_char = input_string[array_pointer];
	array_pointer++;
	return current_char;
}

//Skip the space in the JSON file and return next real char
char nextRealChar(){
	do{
		nextChar();
	} while (current_char == '\0');
	if (current_char != 0 && (current_char < 32 || current_char == 127)){
        logic_error ex("Invalid char found");
        throw std::exception(ex);
	}
	return current_char;
}

//validate the string structure in parallel
void *validateString_parallel(void *thread_string){
    struct thread_data *my_data;
    my_data = (struct thread_data *) thread_string;
    string special = "\"\\/bfnrtu";
    int input_string_index;
    for(int my_data_index = 0 ; my_data_index < my_data->string_start_position.size() ; ++my_data_index) {
        input_string_index = my_data->string_start_position[my_data_index] + 1; //skip the first char (")
        do {
            if (input_string[input_string_index] == '\\') {
                if (special.find(input_string[++input_string_index]) == string::npos) {
                    logic_error ex("Invalid escape char found"); //fixed char after '\' check
                    throw std::exception(ex);
                }
                if (input_string[input_string_index] == 'u') { //check unicode format 0-9 a-f A-F
                    for (int i = 0; i < 4; ++i) {
                        char validate_unicode = input_string[input_string_index++]; // check unicode format
                        if (validate_unicode < 48 || (validate_unicode > 57 && validate_unicode < 65) ||
                            (validate_unicode > 70 && validate_unicode < 97) || validate_unicode > 102) {
                            logic_error ex("Invalid hex found");
                            throw std::exception(ex);
                        }
                    }
                } else if (input_string[input_string_index] == '"') {
                    input_string_index++; //make the '\"' correct while it comes with the '"'
                }
            }
        } while (input_string[input_string_index] >= 32 && input_string[input_string_index] != 34 &&
                input_string[input_string_index] != 127);
        if (input_string[input_string_index] == 0) {
            logic_error ex("Unclosed quote found");
            throw std::exception(ex);
        } else if (input_string[input_string_index] != 34) {
            logic_error ex("Invalid string found");
            throw std::exception(ex);
        }
    }
    pthread_exit(nullptr);
}

//validate the number structure
void validateNumber(){
	if (current_char == '-'){
		current_char = nextChar(); //it can begin with '-'
	}
	if (current_char > 48 && current_char <= 57){
		while (current_char >= 48 && current_char <= 57){
			current_char = nextChar(); //integer part cannot start with 0 and follow with other numbers
		}
	}
	else if (current_char == 48){
		current_char = nextChar(); //integer part start with 0
	}
	else{
        logic_error ex("Invalid number found");
        throw std::exception(ex);
	}
	if (current_char == '.'){ //fraction
		current_char = nextChar();
		if (current_char >= 48 && current_char <= 57){
			while (current_char >= 48 && current_char <= 57){
				current_char = nextChar(); //after '.', it can be 0-9
			}
		}
		else{
            logic_error ex("Invalid number found");
            throw std::exception(ex);
		}
	}
	if (current_char == 'e' || current_char == 'E'){ //exponent
		current_char = nextChar(); //scientific notation first has 'e'/'E'
		if (current_char == '+' || current_char == '-'){
			current_char = nextChar(); //scientific notation then has '-'/'+'
		}
		if (current_char < 48 || current_char > 57){
            logic_error ex("Invalid number found");
            throw std::exception(ex);
		}
		while (current_char >= 48 && current_char <= 57){
			current_char = nextChar(); //it can have any digits next
		}
	}
	array_pointer--;
}

//validate the special value true/false/null
void validateTFN(){
	string special = ",]#/";
	string sb = "";
	do{
		sb.append(1,current_char);
		current_char = nextChar();
	} while (current_char >= ' ' && special.find(current_char) == string::npos && current_char != 127);
	if (sb.compare("true")==1 && sb.compare("false")==1 && sb.compare("null")==1){
		sb.erase(0);
        logic_error ex("Invalid true/false/null");
        throw std::exception(ex);
	}
	array_pointer--;
	sb.erase(0);
}

//skip and store the string in the first step
void skipString(){
    td[td_index%NUM_THREADS].string_start_position.push_back(array_pointer);//store the start position (") of a string
    bool check_special;
    do{
        check_special = false;
        current_char = nextChar();
        if(current_char == '\\'){
            check_special = true ;
            current_char = nextChar();
        }
    }while (current_char >= 32 && current_char != 34 && current_char != 127 && check_special == false);
    if (current_char == 0){
        logic_error ex("Unclosed quote found");
        throw std::exception(ex);
        
    }else if (current_char != 34){
        logic_error ex("Invalid string found");
        throw std::exception(ex);
    }
    td_index++; //finish putting one start position of a string
}

//validate the object structure in parallel
bool validateObject_parallel(){
    nextRealChar();
    if (current_char == '}'){
        return true; //empty object, return true
    }
    else if (current_char == ','){
        logic_error ex("Extra comma found"); //extra comma after '{'
        throw std::exception(ex);
    }
    while (true){
        if (current_char == '}'){
            logic_error ex("Extra comma found"); //extra comma, this is testing while it has iterations
            throw std::exception(ex);
        }
        else if (current_char == '"'){
            skipString(); //skip the string key
        }
        else{
            return false;
        }
        if (nextRealChar() != ':'){
            return false;
        }
        nextRealChar(); //go to the value
        if (current_char == ','){
            logic_error ex("No values in key-value pair"); //No values in the pair
            throw std::exception(ex);
        }
        else if (current_char == '"'){
            skipString(); //skip the string value
        }
        else if (current_char == '-' || (current_char >= 48 && current_char <= 57)){
            validateNumber(); //number
        }
        else if (current_char == '{'){
            if (!validateObject_parallel()){ //object
                return false;
            }
        }
        else if (current_char == '['){
            if (!validateArray_parallel()){ //array
                return false;
            }
        }
        else if (current_char == 't' || current_char == 'f' || current_char == 'n'){
            validateTFN(); //test the special value true/false/null
        }
        else{
            return false;
        }
        switch (nextRealChar()){
        case ',':
            nextRealChar(); //it still has other elements
            continue;
        case '}':
            return true; //no other elements
        default:
            return false; //error char
        }
    }
}

//validate the array structure in parallel
bool validateArray_parallel(){
	nextRealChar();
	if (current_char == ']'){
		return true; //empty array, return true
	}
	else if (current_char == ','){
        logic_error ex("Extra comma found"); //extra comma after '['
        throw std::exception(ex);
	}
	while (true){
		if (current_char == ']'){
            logic_error ex("Extra comma found"); //extra comma, this is testing while it has iterations
            throw std::exception(ex);
		}
		else if (current_char == '"'){
			skipString(); //skip the string
		}
		else if (current_char == '-' || (current_char >= 48 && current_char <= 57)){
			validateNumber(); //number
		}
		else if (current_char == '{'){
			if (!validateObject_parallel()){ //object
				return false;
			}
		}
		else if (current_char == '['){
			if (!validateArray_parallel()){ //array
				return false;
			}
		}
		else if (current_char == 't' || current_char == 'f' || current_char == 'n'){
			validateTFN(); //test the special value true/false/null
		}
		else{
			return false;
		}
		switch (nextRealChar()){
		case ',':
			nextRealChar(); //it still has other elements
			continue;
		case ']':
			return true; //no other elements
		default:
			return false; //error char
		}
	}
}

//Main function to validate the JSON file
static bool isJSON(){
	try{
		array_pointer = 0;
		//find the next real char as the entry of the validator
		switch (nextRealChar()){
		case '[': //an array at the outset
			if (nextRealChar() == ']'){
                if (array_pointer <= input_end && nextRealChar() != input_string[array_pointer]){
                    cout << "end" <<endl ;
					return false; //there are other chars after the outset array
				}
				return true; //there is nothing after the outset array
			}
			array_pointer--; //Go back one char and then go to the array validator
			if (validateArray_parallel() == true){
                if (array_pointer <= input_end && nextRealChar() != input_string[array_pointer]){
                    cout << "end" <<endl ;
					return false; //there are other chars after the outset array
				}
				return true; //there is nothing after the outset array
			}
			return false;
		case '{': //an object at the outset
			if (nextRealChar() == '}'){
                if (array_pointer <= input_end && nextRealChar() != input_string[array_pointer]){
					return false; //there are other chars after the outset object
				}
				return true; //there is nothing after the outset object
			}
			array_pointer--; //Go back one char and then go to the object validator part
			if (validateObject_parallel() == true){
                if (array_pointer <= input_end && nextRealChar() != input_string[array_pointer]){
					return false; //there are other chars after the outset object
				}
				return true; //there is nothing after the outset object
			}
			return false;
		default:
			return false;
		}
	}
	catch (exception e){
		return false;
	}
}

//Main function to validate the JSON file in parallel
static bool isJSON_parallel(){
    if(isJSON() == false){
        return false; //validate result is false without validating strings
    }else{ //validate result is true before validating strings
        try { //validate strings
            pthread_t tids[NUM_THREADS]; //define the ID of the thread
            int i;
            int ret;
            for (i = 0; i < NUM_THREADS; ++i) {
                //ID of created thread, thread parameter, called function, passed in function parameter
                ret = pthread_create(&tids[i], nullptr, validateString_parallel, (void *) &td[i]);
                if (ret) {
                    cout << "Error:unable to create thread " << ret << endl;
                    exit(-1);
                }
            }
            return true; //all the strings passed
        }catch (exception e){
            return false; //exceptions while validating strings
        }
    }
}

//Main function (Program entry)
int main(int argc,char *argv[]){
    clock_t begin, end;
    double cost;
    input_string = read_file_to_char(argv[1]);
    begin = clock();
    size_t file_len = strlen(input_string);
    trim_parallel(input_string,file_len);
    try {
        if (isJSON_parallel() == true){
            cout << "pass" << endl;
        }
        else {
            cout << "fail" << endl;
        }
    }
    catch (exception e){
        cout << "fail" << endl;
    }
    end = clock();
    cost = (double)(end - begin) / CLOCKS_PER_SEC;
    cout << "constant CLOCKS_PER_SEC is: " << CLOCKS_PER_SEC << ", time cost is: " << cost << "secs" << endl;
    return 0;
}