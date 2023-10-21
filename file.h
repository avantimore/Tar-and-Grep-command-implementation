#include <stdbool.h>
#define MAX_LINE_LENGTH 100
#define ALPHABET_SIZE 26

typedef struct TrieNode {
    struct TrieNode* children[ALPHABET_SIZE];
    bool isEndOfWord;
    struct LineNumber* lines;
}TrieNode;

typedef struct LineNumber {
    int number;
    struct LineNumber* next;
}LineNumber;


TrieNode* createTNode();

void insertTWord(TrieNode* root, const char* word, int lineNumber);

LineNumber* searchWord(TrieNode* root, const char* word, const char* option);

void searchInTrie(TrieNode* root, const char* word, const char* filename, const char* option) ;

void freeTrie(TrieNode* node);

void usage();

