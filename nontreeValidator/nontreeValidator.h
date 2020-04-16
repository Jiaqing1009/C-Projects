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
#include <algorithm> 
using namespace std;

class nontreeValidator{
    private:
    char* input_string; //the char array (JSON string) which is waiting for validating
    vector<char> input_char; //the input which only contains [ or ]
    vector<int> node; //save the initial index
    vector<int> depth; //save the depth of initial index
    struct index_depth
    {
        int index;
        int depth;
    };
    vector<index_depth> index_depth_pairs; //store the index and depth pairs to sort

    public:
    //Read a file and transfer it to a char* by giving its file path
    char* read_file_to_char(char* file_path);

    //Validate the input JSON and generate a value vector
    int * validator();
    
    //Main function (Program entry)
	int main(int argc,char *argv[]);
};

#endif