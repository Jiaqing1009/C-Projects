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

vector<index_depth> index_depth_pairs;

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

//Override comparison function
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
            input_char.push_back(i);
            // node.push_back(node_count);
            // depth.push_back(depth_count--);
            depth_count--;
        }
    }
    //strlen(node) = strlen(depth)
    int array_size = node.size(); //the following arraies have the same size
    for(int i = 0 ; i < array_size; i++){
        index_depth local_pair  = {node[i],depth[i]};
        index_depth_pairs.push_back(local_pair);
    }
    sort(index_depth_pairs.begin(), index_depth_pairs.end(), compareDepth());
    //Calculate bfs
    int * bfs = NULL;
    bfs = new int [array_size];
    for (int i = 0; i < array_size; i++){
        bfs[index_depth_pairs[i].index] = i;
    }
    cout << "bfs" << endl;
    for(int n = 0; n < array_size; n++){
        cout << bfs[n] << " ";
    }
    cout << endl;
    //Calculate par
    int * par = NULL;
    par = new int [array_size];
    for(int i = 0; i < array_size; i++){
        par[i] = -1;
    }
    for(int i = 0; i < array_size; i++){
        if(depth[i-1]+1 == depth[i]){
            par[bfs[i]] = bfs[i-1];
        }
    }
    cout << "par" << endl;
    for(int n = 0; n < array_size; n++){
        cout << par[n] << " ";
    }
    cout << endl;
    //Calculate parent and count
    int * parent = NULL;
    parent = new int [array_size];
    for(int i = 0; i < array_size; i++){
        parent[i] = -1;
    }
    int * count = NULL;
    count = new int [array_size];
    for(int i = 0; i < array_size; i++){
        count[i] = -1;
    }
    int local_number;
    int local_count;
    for(int i = 1; i < array_size; i++){
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
    int * nchild = NULL;
    nchild = new int [array_size];
    for(int i = 0; i < array_size; i++){
        nchild[i] = 0;
    }
    for(int i = 0; i < array_size; i++){
        if(parent[i]!=-1){
            nchild[parent[i]]++;
        }
    }
    //Calculate alloc
    int * alloc = NULL;
    alloc = new int [array_size];
    alloc[0] = 0;
    for(int i = 1; i < array_size; i++){
        alloc[i] = alloc[i-1]+nchild[i-1]+1;
    }
    cout << "parent" << endl;
    for(int n = 0; n < array_size; n++){
        cout << parent[n] << " ";
    }
    cout << endl;
    cout << "count" << endl;
    for(int n = 0; n < array_size; n++){
        cout << count[n] << " ";
    }
    cout << endl;
    cout << "nchild" << endl;
    for(int n = 0; n < array_size; n++){
        cout << nchild[n] << " ";
    }
    cout << endl;
    cout << "alloc" << endl;
    for(int n = 0; n < array_size; n++){
        cout << alloc[n] << " ";
    }
    cout << endl;
    //Get the final result
    int value_size = input_char.size()-1;
    int * value = NULL;
    value = new int [value_size];
    for(int i = 0; i < value_size; i++){
        value[i] = 0;
    }
    for(int i = 0; i < array_size ; i++){
        if(nchild[i]!=0){
            value[alloc[i]] = nchild[i];   
        }
    }
    for(int i = 1; i < array_size ; i++){
        value[parent[i]+node[i]] = alloc[i];
    }
    cout << "Final result" << endl;
    for(int i = 0; i < value_size; i++){
        cout << value[i] << " ";
    }
    cout << endl;
    delete [] bfs;
    delete []par;
    delete []parent;
    delete []count;
    delete []nchild;
    delete []alloc;
    delete []value;
}

//Main function (Program entry)
int main(int argc,char *argv[]){
    input_string = read_file_to_char(argv[1]);
    cout << endl;
    cout << "The input is " << input_string << endl;
    validator();
    cout << "index" << endl;
    int j = 0;
    for(vector<int>::iterator j = node.begin(); j != node.end(); j++ ){
        cout << *j << " ";
    }
    cout << endl;
    cout << "depth" << endl;
    int k = 0;
    for(vector<int>::iterator k = depth.begin(); k != depth.end(); k++ ){
        cout << *k << " ";
    }
    cout << endl;
    cout << "index_depth pairs" << endl;
    int m = 0;
    for(vector<index_depth>::iterator m = index_depth_pairs.begin(); m != index_depth_pairs.end(); m++ ){
        cout << m->index << " " << m->depth <<endl;
    }
    cout << endl;
    return 0;
}