#include"JsonValidator.h"

int array_pointer = 0;
string input_string;
char current_char = '\0';

bool validateArray();

//Read a file and transfer it to a String by giving its file path
string readFileToString(string filename)
{
	ifstream ifile(filename);
	ostringstream buf;
	char ch;
	while (buf && ifile.get(ch))
		buf.put(ch);
	return buf.str();
}

//Remove '\n' '\r' and ' ' before and after input string
string& trim(string& str)
{
	string::size_type pos = str.find_last_not_of(' ');
	pos = str.find_last_not_of('\r');
	pos = str.find_last_not_of('\n');
	if (pos != string::npos)
	{
		str.erase(pos + 1);
		pos = str.find_first_not_of(' ');
		pos = str.find_first_not_of('\r');
		pos = str.find_first_not_of('\n');
		if (pos != string::npos)
		{
			str.erase(0, pos);
		}
	}
	else
	{
		str.erase(str.begin(), str.end());
	}
	return str;
}

//Return next char
char nextChar()
{
	if (array_pointer < 0 || (array_pointer >= input_string.length()))
	{
		return 0;
	}
	current_char = input_string[array_pointer];
	array_pointer++;
	return current_char;
}

//Skip the space in the JSON file and return next real char
char nextRealChar()
{
	do
	{
		nextChar();
	} while (current_char == '\n' || current_char == '\r' || current_char == ' '|| current_char == '\0');
	if (current_char != 0 && (current_char < 32 || current_char == 127))
	{
        logic_error ex("Invalid char found");
        throw std::exception(ex);
	}
	return current_char;
}

//validate the string structure
void validateString()
{
	string special = "\"\\/bfnrtu";
	do
	{
		current_char = nextChar();
		if (current_char == '\\')
		{
			if (special.find(nextChar()) == string::npos)
			{
                logic_error ex("Invalid escape char found"); //fixed char after '\' check
                throw std::exception(ex);
			}
			if (current_char == 'u')
			{ //check unicode format 0-9 a-f A-F
				for (int i = 0; i < 4; i++)
				{
					nextChar(); // check unicode format
					if (current_char < 48 || (current_char > 57 && current_char < 65) || (current_char > 70 && current_char < 97) || current_char > 102)
					{
                        logic_error ex("Invalid hex found");
                        throw std::exception(ex);
					}
				}
			}
			else if (current_char == '"')
			{
				nextChar(); //make the '\"' correct while it comes with the '"'
			}
		}
	} while (current_char >= 32 && current_char != 34 && current_char != 127);
	if (current_char == 0)
	{
        logic_error ex("Unclosed quote found");
        throw std::exception(ex);
	}
	else if (current_char != 34)
	{
        logic_error ex("Invalid string found");
        throw std::exception(ex);
	}
}

//validate the number structure
void validateNumber()

{
	if (current_char == '-')
	{
		current_char = nextChar(); //it can begin with '-'
	}
	if (current_char > 48 && current_char <= 57)
	{
		while (current_char >= 48 && current_char <= 57)
		{
			current_char = nextChar(); //integer part cannot start with 0 and follow with other numbers
		}
	}
	else if (current_char == 48)
	{
		current_char = nextChar(); //integer part start with 0
	}
	else
	{
        logic_error ex("Invalid number found");
        throw std::exception(ex);
	}
	if (current_char == '.')
	{ //fraction
		current_char = nextChar();
		if (current_char >= 48 && current_char <= 57)
		{
			while (current_char >= 48 && current_char <= 57)
			{
				current_char = nextChar(); //after '.', it can be 0-9
			}
		}
		else
		{
            logic_error ex("Invalid number found");
            throw std::exception(ex);
		}
	}
	if (current_char == 'e' || current_char == 'E')
	{ //exponent
		current_char = nextChar(); //scientific notation first has 'e'/'E'
		if (current_char == '+' || current_char == '-')
		{
			current_char = nextChar(); //scientific notation then has '-'/'+'
		}
		if (current_char < 48 || current_char > 57)
		{
            logic_error ex("Invalid number found");
            throw std::exception(ex);
		}
		while (current_char >= 48 && current_char <= 57)
		{
			current_char = nextChar(); //it can have any digits next
		}
	}
	array_pointer--;
}

//validate the special value true/false/null
void validateTFN()
{
	string special = ",]#/";
	string sb = "";
	do
	{
		sb.append(1,current_char);
		current_char = nextChar();
	} while (current_char >= ' ' && special.find(current_char) == string::npos && current_char != 127);
	if (sb.compare("true")==1 && sb.compare("false")==1 && sb.compare("null")==1)
	{
		sb.erase(0);
        logic_error ex("Invalid true/false/null");
        throw std::exception(ex);
	}
	array_pointer--;
	sb.erase(0);
}

//validate the object structure
bool validateObject()
{
	nextRealChar();
	if (current_char == '}')
	{
		return true; //empty object, return true
	}
	else if (current_char == ',')
	{
        logic_error ex("Extra comma found"); //extra comma after '{'
		throw std::exception(ex);
	}
	while (true)
	{
		if (current_char == '}')
		{
            logic_error ex("Extra comma found"); //extra comma, this is testing while it has iterations
            throw std::exception(ex);
		}
		else if (current_char == '"')
		{
			validateString(); //string key
		}
		else
		{
			return false;
		}
		if (nextRealChar() != ':')
		{
			return false;
		}
		nextRealChar(); //go to the value
		if (current_char == ',')
		{
            logic_error ex("No values in key-value pair"); //No values in the pair
			throw std::exception(ex);
		}
		else if (current_char == '"')
		{
			validateString(); //string
		}
		else if (current_char == '-' || (current_char >= 48 && current_char <= 57))
		{
			validateNumber(); //number
		}
		else if (current_char == '{')
		{
			if (!validateObject())
			{ //object
				return false;
			}
		}
		else if (current_char == '[')
		{
			if (!validateArray())
			{ //array
				return false;
			}
		}
		else if (current_char == 't' || current_char == 'f' || current_char == 'n')
		{
			validateTFN(); //test the special value true/false/null
		}
		else
		{
			return false;
		}
		switch (nextRealChar())
		{
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

//validate the array structure
bool validateArray()
{
	nextRealChar();
	if (current_char == ']')
	{
		return true; //empty array, return true
	}
	else if (current_char == ',')
	{
        logic_error ex("Extra comma found"); //extra comma after '['
        throw std::exception(ex);
	}
	while (true)
	{
		if (current_char == ']')
		{
            logic_error ex("Extra comma found"); //extra comma, this is testing while it has iterations
            throw std::exception(ex);
		}
		else if (current_char == '"')
		{
			validateString(); //string
		}
		else if (current_char == '-' || (current_char >= 48 && current_char <= 57))
		{
			validateNumber(); //number
		}
		else if (current_char == '{')
		{
			if (!validateObject())
			{ //object
				return false;
			}
		}
		else if (current_char == '[')
		{
			if (!validateArray())
			{ //array
				return false;
			}
		}
		else if (current_char == 't' || current_char == 'f' || current_char == 'n')
		{
			validateTFN(); //test the special value true/false/null
		}
		else
		{
			return false;
		}
		switch (nextRealChar())
		{
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
bool isJSON(string input)
{
	try
	{
		array_pointer = 0;
		input_string = input;
		//find the next real char as the entry of the validator
		switch (nextRealChar())
		{
		case '[': //an array at the outset
			if (nextRealChar() == ']')
			{
				if (array_pointer < input.length() && nextRealChar() != input[array_pointer])
				{
					return false; //there are other chars after the outset array
				}
				return true; //there is nothing after the outset array
			}
			array_pointer--; //Go back one char and then go to the array validator
			if (validateArray() == true)
			{
				if (array_pointer < input.length() && nextRealChar() != input[array_pointer])
				{
					return false; //there are other chars after the outset array
				}
				return true; //there is nothing after the outset array
			}
			return false;
		case '{': //an object at the outset
			if (nextRealChar() == '}')
			{
				if (array_pointer < input.length() && nextRealChar() != input[array_pointer])
				{
					return false; //there are other chars after the outset object
				}
				return true; //there is nothing after the outset object
			}
			array_pointer--; //Go back one char and then go to the object validator part
			if (validateObject() == true)
			{
				if (array_pointer < input.length() && nextRealChar() != input[array_pointer])
				{
					return false; //there are other chars after the outset object
				}
				return true; //there is nothing after the outset object
			}
			return false;
		default:
			return false;
		}
	}
	catch (exception e)
	{
		return false;
	}
}

//Main function (Program entry)
int main(int argc,char *argv[])
{
	string file_name = argv[1];
	clock_t begin, end;
	double cost;
	begin = clock();
	try {
		string input_file = readFileToString(file_name);
		trim(input_file);
		if (isJSON(input_file) == true)
		{
			cout << "pass" << endl;
		}
		else {
			cout << "fail" << endl;
		}
	}
	catch (exception e)
	{
		cout << "fail" << endl;
	}
	end = clock();
	cost = (double)(end - begin) / CLOCKS_PER_SEC;
	cout << "constant CLOCKS_PER_SEC is: " << CLOCKS_PER_SEC << ", time cost is: " << cost << "secs" << endl;
	return 0;
}