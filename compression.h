typedef struct Node {
    unsigned char symbol;
    unsigned int frequency;
    struct Node* left;
    struct Node* right;
} Node;

typedef struct {
    unsigned char symbol;
    char* code;
} CodeEntry;
Node* createNode(unsigned char symbol, unsigned int frequency);
char* getFileNameFromPath(const char* path);
void destroyNode(Node* node);
void countSymbolFrequencies(const char* filePath, unsigned int* frequencies) ;
Node* buildHuffmanTree(unsigned int* frequencies);
void generateCodeTable(Node* node, char* code, int depth, CodeEntry* codeTable) ;
void compressFile(const char* inputFile, const char* outputFile);
void decompressFile(const char* inputFile, const char* outputFile);
void compressDirectory(const char* inputDir);
void decompressDirectory(const char* inputDir);

