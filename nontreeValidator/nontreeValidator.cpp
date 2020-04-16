#include "nontreeValidator.h"

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

//Override comparison function of sort
struct compareDepth{
    inline bool operator()(index_depth& a, index_depth& b){
        if(a.depth == b.depth){
            return a.index < b.index;
        }
        else{
            return a.depth < b.depth;
        }
    }
};

//Validate the input JSON and generate a value vector
int * validator(){
    int node_count = 0;
    int depth_count = 0;
    //Calculate node and depth, the first scan
    for(int i = 0 ; i < strlen(input_string) ; i++){
        if(input_string[i]=='['){
            input_char.push_back(i);
            node.push_back(node_count++);
            depth.push_back(depth_count++);
        }
        else if(input_string[i]==']'){
            input_char.push_back(i);
            depth_count--;
        }
    }
    //the following arrays have the same size
    int array_size = node.size();
    //Store the index and depth pairs prepared to sort
    for(int i = 0 ; i < array_size; i++){
        index_depth local_pair  = {node[i],depth[i]};
        index_depth_pairs.push_back(local_pair);
    }
    //the only one sort operation
    sort(index_depth_pairs.begin(), index_depth_pairs.end(), compareDepth());
    //create arrays based on the size of node vector
    int * bfs = NULL;
    bfs = new int [array_size];
    int * par = NULL;
    par = new int [array_size];
    //Initialize the par array
    for(int i = 0; i < array_size; i++){
        par[i] = -1;
    }
    int * parent = NULL;
    parent = new int [array_size];
    //Initialize the parent array
    for(int i = 0; i < array_size; i++){
        parent[i] = -1;
    }
    int * count = NULL;
    count = new int [array_size];
    //Initialize the count array
    for(int i = 0; i < array_size; i++){
        count[i] = -1;
    }
    int * nchild = NULL;
    nchild = new int [array_size];
    //Initialize the nchild array
    for(int i = 0; i < array_size; i++){
        nchild[i] = 0;
    }
    int * alloc = NULL;
    alloc = new int [array_size];
    //Initialize the alloc[0]
    alloc[0] = 0;
    //Calculate bfs, the first scatter
    for (int i = 0; i < array_size; i++){
        bfs[index_depth_pairs[i].index] = i;
    }
    //Calculate par, the second scatter
    for(int i = 0; i < array_size; i++){
        if(depth[i-1]+1 == depth[i]){
            par[bfs[i]] = bfs[i-1];
        }
    }
    //Calculate parent, count and nchild, the second scan
    int local_number;
    int local_count;
    for(int i = 0; i < array_size; i++){
        //Calculate parent and count
        if(i != 0){
            if(par[i]!= -1){
                local_number = par[i];
                parent[i] = local_number;
                local_count = 1;
                count[i] = local_count;
                local_count++;
            }
            else{
                parent[i] = local_number;
                count[i] = local_count;
                local_count++;
            }
        }
        //Calculate nchild
        if(parent[i]!=-1){
            nchild[parent[i]]++;
        }
    }
    //Calculate alloc, the third scan
    for(int i = 1; i < array_size; i++){
        alloc[i] = alloc[i-1]+nchild[i-1]+1;
    }
    //Put results together and generate the final result
    int value_size = input_char.size()-1;
    int * value = NULL;
    value = new int [value_size];
    for(int i = 0; i < value_size; i++){
        value[i] = 0;
    }
    //the third scatter
    for(int i = 0; i < array_size ; i++){
        if(nchild[i]!=0){
            value[alloc[i]] = nchild[i];   
        }
    }
    //the fourth scatter
    for(int i = 1; i < array_size ; i++){
        value[parent[i]+node[i]] = alloc[i];
    }
    delete [] bfs;
    delete []par;
    delete []parent;
    delete []count;
    delete []nchild;
    delete []alloc;
    return value;
}

//Main function (Program entry)
int main(int argc,char *argv[]){
    input_string = read_file_to_char(argv[1]);
    cout << "Input" << endl;
    cout << input_string << endl;
    int * result = validator();
    cout << "Final result" << endl;
    for(int i = 0; i < input_char.size()-1; i++){
        cout << result[i] << " ";
    }
    cout << endl;
    return 0;
}