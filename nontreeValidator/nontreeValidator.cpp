#include "nontreeValidator.h"

char* input_string; //the char array (JSON string) which is waiting for validating
vector<char> input_char; //the input which only contains [ or ]
vector<int> node;
vector<int> depth;
vector<int> dfs;
vector<int> dfs_index;
vector<int> parent;
vector<int> count;
vector<int> nchild;
vector<int> alloc;
vector<int> value;

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

//Validate the input JSON and generate a value vector
void validator(){
    int node_count = 0;
    int depth_count = 0;
    for(int i = 0 ; i < strlen(input_string) ; i++){
        if(input_string[i]=='['){
            input_char.push_back(i);
            node.push_back(node_count++);
            depth.push_back(depth_count++);
        }
        else if(input_string[i]==']'){
            // input_char.push_back(i);
            // node.push_back(node_count);
            // depth.push_back(depth_count--);
            depth_count--;
        }
    }
    int bfs_count = 0;
}

//Main function (Program entry)
int main(int argc,char *argv[]){
    input_string = read_file_to_char(argv[1]);
    cout << "The input is " << input_string << endl;
    validator();
    int j = 0;
    for(vector<int>::iterator j = node.begin(); j != node.end(); j++ ){
        cout << *j << " ";
    }
    cout << endl;
    int k = 0;
    for(vector<int>::iterator k = depth.begin(); k != depth.end(); k++ ){
        cout << *k << " ";
    }
    cout << endl;
    return 0;
}