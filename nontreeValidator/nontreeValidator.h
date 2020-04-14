#ifndef NONTREEVALIDATOR_H_
#define NONTREEVALIDATOR_H_

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <cstring>
#include <stdlib.h>
#include <vector>
#include<stdio.h>
using namespace std;

class nontreeValidator{
    private:
    char* input_string; //the char array (JSON string) which is waiting for validating

    public:

    //Read a file and transfer it to a char* by giving its file path
    char* read_file_to_char(char* file_path);

    //Main function (Program entry)
	int main(int argc,char *argv[]);
};

#endif