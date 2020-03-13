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

//Skip the space in the JSON file
char nextRealChar()
{
	do
	{
		nextChar();
	} while (current_char == '\n' || current_char == '\r' || current_char == ' ' || current_char == 0);
	if (current_char != 0 && (current_char < 32 || current_char == 127))
	{
		throw exception("Invalid char found");
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
				throw exception("Invalid escape char found"); //fixed char after '\' check
			}
			if (current_char == 'u')
			{ //check unicode format 0-9 a-f A-F
				for (int i = 0; i < 4; i++)
				{
					nextChar(); // check unicode format
					if (current_char < 48 || (current_char > 57 && current_char < 65) || (current_char > 70 && current_char < 97) || current_char > 102)
					{
						throw exception("Invalid hex found");
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
		throw exception("Unclosed quote found");
	}
	else if (current_char != 34)
	{
		throw exception("Invalid string found");
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
		throw exception("Invalid number found");
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
			throw exception("Invalid number found");
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
			throw exception("Invalid number found");
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
	if (!sb.compare("true") && !sb.compare("false") && !sb.compare("null"))
	{
		sb.erase(0);
		throw exception("Invalid true/false/null");
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
		throw exception("Extra comma found"); //extra comma after '{'
	}
	while (true)
	{
		if (current_char == '}')
		{
			throw exception("Extra comma found"); //extra comma, this is testing while it has iterations
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
			throw exception("No values in key-value pair"); //No values in the pair
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
		throw exception("Extra comma found"); //extra comma after '['
	}
	while (true)
	{
		if (current_char == ']')
		{
			throw exception("Extra comma found"); //extra comma, this is testing while it has iterations
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
	catch (const std::runtime_error & e)
	{
		return false;
	}
}

int main()
{
	int count_fail = 0;
	int count_pass = 0;
	/*cout << "For 33 fail test cases:" << endl;
	for (int i = 1; i <= 33; i++)
	{
		try
		{
			string input_file = readFileToString("test/fail" + to_string(i) + ".json");
			if (isJSON(input_file) == false)
			{
				count_fail++;
			}
		}
		catch (exception e)
		{
			count_fail++;
		}
	}
	cout << count_fail << " fail and " << (33 - count_fail) << " pass" << endl;*/
	cout << "For 3 pass test cases:" << endl;
	for (int i = 1; i <= 3; i++)
	{
		try
		{
			string input_file = readFileToString("test/pass" + to_string(i) + ".json");
			if (isJSON(input_file) == true)
			{
				count_pass++;
			}
		}
		catch (exception e)
		{
		}
	}
	cout << count_pass << " pass and " << (3 - count_pass) << " fail" << endl;
	//Add one specific file here, just change the filepath
	cout << "For the given case:" << endl;
	string file = "test.txt";
	string str;
	str = readFileToString(file);
	bool flag = isJSON(str);
	if (flag == true) {
		cout << "pass" << endl;
	}
	else{
		cout << "fail" << endl;
	}
	return 0;
}