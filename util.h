//util.h allows for users to create a new compressed file of another file and to decompress
//an encoded file into a new readable text file. This file also allows for the encoding and
//ddecoding to be done step by step. With the menu options, the frequency map, encoding tree,
//encoding map, encode, and decode can all be done separately as well as compression and 
//decompression. The frequency map, encoding tree, encoding map also are printed it so users
//can see what they all look like. The point of util.h is to save memory. By sending a 
//compressed file rather than the text file a lot of memory is saved and download time is also
//saved if the file is rather large.

#pragma once

typedef hashmap hashmapF;
typedef unordered_map <int, string> hashmapE;

struct HuffmanNode {
    int character;
    int count;
    HuffmanNode* zero;
    HuffmanNode* one;
};

struct compare
{
    bool operator()(const HuffmanNode *lhs,
        const HuffmanNode *rhs)
    {
        return lhs->count > rhs->count;
    }
};
//----------------------------------------------------------
void freeTree(HuffmanNode* node) { //Deletes a tree to save memory
    if(node == nullptr){
        return;
    }
    else{  //Post Order Traversal is best for deleting nodes
        freeTree(node->zero);
        freeTree(node->one);
        delete node;
    }
    
}
//----------------------------------------------------------
void buildFrequencyMap(string filename, bool isFile, hashmapF &map) { //Finds the frequency of every character in a file or invalid file
    char c;
    if(isFile){ //Checks if file is valid
        ifstream inFS(filename);
        while(inFS.get(c)){ //Loops through file character by character
            if(map.containsKey(c)){  //If map contains character
                int counter = map.get(c); 
                map.put(c, ++counter); //adds 1 to the frequency 
            }
            else{
                map.put(c, 1); 
            }
       } 
    }
    else{
        string str = filename;
        for(char c: str){ //Loops through the name of the file 
            if(map.containsKey(c)){ 
                int counter = map.get(c);
                map.put(c, ++counter);
            }
            else{
                map.put(c, 1);
            }
        }
    }
    map.put(PSEUDO_EOF, 1); //Adds the end of file to the map
}
//----------------------------------------------------------
HuffmanNode* buildEncodingTree(hashmapF &map) { //Builds a tree that has characters stored in all the leaf nodes using 0 and 1 as left and right
    priority_queue<HuffmanNode*, vector<HuffmanNode*>, compare> pq;

    for(int key: map.keys()){ //Looping through the keys in the map
        HuffmanNode* curr = new HuffmanNode; //Creating a new node and storing count and character
        curr->count = map.get(key);
        curr->character = key;
        curr->one = nullptr;
        curr->zero = nullptr;
        pq.push(curr); //Pushing the node in the priority queue
    }

    while(pq.size()!=1){ //Looping through priority queue till root
        HuffmanNode* zero = pq.top();
        pq.pop();
        HuffmanNode* one = pq.top(); 
        pq.pop();
        HuffmanNode* parent = new HuffmanNode; //Parent node created holds the combined frequency of both nodes
        int combinedCount = (zero->count) + (one->count);
        parent->count = combinedCount;
        parent->character = NOT_A_CHAR; //Parent node does not hold a character
        parent->zero = zero;
        parent->one = one;

        pq.push(parent); //Parent node is pushed in priority queue

    }

    HuffmanNode* first = pq.top(); //Node stored holds all the info for the Tree
    pq.pop(); // Entire priority queue is deleted
    if(!pq.empty()){
        cout<<"Not Empty\n";
    }

    return first; 
}
//----------------------------------------------------------
void _buildEncodingMap(HuffmanNode* node, hashmapE &encodingMap, string str, HuffmanNode* prev) { //BuildEncodingMap helper function
    if(node->character != NOT_A_CHAR){ //Checks for leaf nodes
        encodingMap[node->character] = str; //Stores the binary sequency for the character in the encodingMap
        return;
    }
    if(node->zero != nullptr){ //PreOrder Traversal for essentially copying information
        _buildEncodingMap(node->zero, encodingMap, str + '0', prev); //Adds 0 to the string
    }
    if(node->one != nullptr){
        _buildEncodingMap(node->one, encodingMap, str + '1', prev); //Adds 1 to the string
    }
}
//----------------------------------------------------------
hashmapE buildEncodingMap(HuffmanNode* tree) { //Builds a map with the character and its binary sequency using the encoding tree
    hashmapE encodingMap;
    _buildEncodingMap(tree, encodingMap, "", nullptr); //Calls buildEcodingMap helper 
    
    return encodingMap;
}
//----------------------------------------------------------
string encode(ifstream& input, hashmapE &encodingMap, ofbitstream& output, int &size, bool makeFile) { //Returns a binary string that has the information for a 
    char c; //txt file when decoded
    string str;
    while(input.get(c)){ //Loops through the input by character
        str += encodingMap[c]; //encodingMap[c] returns the binary sequence for each letter, which builds onto the string
        for(int i =0; i<encodingMap[c].size(); i++){
            size++; //The amount of 1s and 0s for each letter is added onto the size
        }
    }
    str+= encodingMap[PSEUDO_EOF]; //End of file
    for(int i=0; i<encodingMap[PSEUDO_EOF].size(); i++){
        size++; //binary size of end of file
    }

    string temp;
    int bit;
    for(int i=0; i<str.size(); i++){
        temp = (str[i]); //char to string 
        bit = stoi(temp); //string to int 
        output.writeBit(bit); //Adds to the output
    }

    return str;
}
//----------------------------------------------------------
string decode(ifbitstream &input, HuffmanNode* encodingTree, ofstream &output) { //Decodes binary string into readable text
    string str;
    string char1;
    HuffmanNode* root = encodingTree; //temp node to reset
    while(!input.eof()){ //Loop through the input
        int bit = input.readBit();
        if(bit == 0){ //Node moves based on next input
            root= root->zero;
        }
        else{
            root = root->one;
        }
        if(root->character != NOT_A_CHAR){ //Checks if node is a lead
          if(root->character == PSEUDO_EOF){
            break;
          }
            output.put(root->character); //Adds to output
            str += root->character; //Character is added onto the string 
            root = encodingTree; //Node is reset
        }   
    }
    return str;
}
//----------------------------------------------------------
string compress(string filename) { //Compresses file and creates encoded file
    hashmapF frequencyMap;
    HuffmanNode* encodingTree;
    hashmapE encodingMap;
    //Frequency map, encoding tree, and encoding map is created 
    buildFrequencyMap(filename, true, frequencyMap);
    encodingTree = buildEncodingTree(frequencyMap);
    encodingMap = buildEncodingMap(encodingTree);

    ofbitstream output(filename +".huf"); //File created
    ifstream input(filename);
    stringstream ss;

    ss<<frequencyMap;
    output<<frequencyMap; 
    int size = 0;
    string codeStr = encode(input, encodingMap, output, size, true); //String becomes an encoded string 
    freeTree(encodingTree);
    output.close();

    return codeStr; 
}
//---------------------------------------------------------- 
string decompress(string filename) { //Turns encoded string into readable text file
    ifbitstream input(filename);

    int pos = filename.find(".txt");
    filename = filename.substr(0, pos);
    ofstream output(filename + "_unc.txt"); //Nw file created
    hashmapF frequencyMap;
    HuffmanNode* encodingTree;
    input>>frequencyMap;

    encodingTree = buildEncodingTree(frequencyMap); //Encoded Tree is made
    
    string decodeStr  = decode(input, encodingTree, output); //String becomes readable text
    freeTree(encodingTree); //Memory deleted
    output.close();
    
    return decodeStr; 
}
//----------------------------------------------------------