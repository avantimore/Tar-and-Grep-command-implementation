
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include "file.h"
#define MAX_LINE_LENGTH 100

#define ALPHABET_SIZE 26



TrieNode* createTNode() {                                            //create the node
    TrieNode* newNode = (TrieNode*)malloc(sizeof(TrieNode));
    if (newNode != NULL) {
        newNode->isEndOfWord = false;
        newNode->lines = NULL;
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            newNode->children[i] = NULL;
        }
    }
    return newNode;
}

void insertTWord(TrieNode* root, const char* word, int lineNumber) {
    TrieNode* current = root;

    for (int i = 0; word[i] != '\0'; i++) {
        int index = word[i] - 'a';

        if (current->children[index] == NULL) {
            current->children[index] = createTNode();
        }

        current = current->children[index];
    }

    current->isEndOfWord = true;

    LineNumber* newLine = (LineNumber*)malloc(sizeof(LineNumber));
    if (newLine == NULL) {
        printf("Memory allocation failed.\n");
        return;
    }
    newLine->number = lineNumber;
    newLine->next = current->lines;
    current->lines = newLine;
}

LineNumber* searchWord(TrieNode* root, const char* word, const char* option) {
    TrieNode* current = root;
    
        char modifiedWord[strlen(word) + 1];

    if (strcmp(option, "-c") == 0) {
        for (int i = 0; word[i] != '\0'; i++) {
            modifiedWord[i] = tolower((unsigned char)word[i]);
        }
    } else {
        for (int i = 0; word[i] != '\0'; i++) {
            modifiedWord[i] = word[i];
        }
    }
    modifiedWord[strlen(word)] = '\0';

    for (int i = 0; modifiedWord[i] != '\0'; i++) {
        int index = modifiedWord[i] - 'a';

        if (current->children[index] == NULL) {
            return NULL;
        }
    
    else{
    
    
    	for (int i = 0; word[i] != '\0'; i++) {
        	int index = word[i] - 'a';

        	if (current->children[index] == NULL) {
            	return NULL;
        	}

        	current = current->children[index];
    	}
	}  
    if (current != NULL && current->isEndOfWord) {
        return current->lines;
    }

    return NULL;
}
}

void searchInTrie(TrieNode* root, const char* word, const char* filename, const char* option) {
    LineNumber* lines = searchWord(root, word, option);

    if (lines == NULL) {
        printf("Word '%s' not found in the Trie.\n", word);
    } else {
        printf("Lines with word '%s' highlighted in file '%s':\n", word, filename);

        FILE* file = fopen(filename, "r");
        if (file == NULL) {
            printf("Failed to open the file.\n");
            return;
        }

        int currentLineNumber = 1;
        char line[MAX_LINE_LENGTH];

        while (fgets(line, sizeof(line), file) != NULL) {
            if(strcmp(option, "-n") == 0){
        		printf("Line no. %d: ", currentLineNumber); // Print the line number
        	}
            char* wordPtr = strtok(line, " \t\n");
            while (wordPtr != NULL) {
            
            	if (strcmp(option, "-c") == 0){
            		if (strcasecmp(wordPtr, word) == 0) {        // Perform case-insensitive comparison
                    printf("\033[1;31m%s\033[0m ", wordPtr);    // Set color to red 
                	}
                	else {
                    	printf("%s ", wordPtr);
                	}
                	wordPtr = strtok(NULL, " \t\n");
            	}
                
                else{
                	if (strcmp(wordPtr, word) == 0) {
                    	printf("\033[1;31m%s\033[0m ", wordPtr); // Set color to red 
                	} else {
                    	printf("%s ", wordPtr);
                	}
                	wordPtr = strtok(NULL, " \t\n");
            }
            }

            printf("\n");

            LineNumber* currentLine = lines;
            while (currentLine != NULL) {
                if (currentLine->number == currentLineNumber) {
                	/*if(strcmp(option, "-n") == 0){
                		printf("LineNo %d: %s ", currentLine->number,line); // Print the line number
                    break;
                	
                	}*/
                    break;
                }
                currentLine = currentLine->next;
            }

            currentLineNumber++;
        }

        fclose(file);
        printf("\n");
    }
    }


void freeTrie(TrieNode* node) {
    if (node != NULL) {
        for (int i = 0; i < ALPHABET_SIZE; i++) {
            freeTrie(node->children[i]);
        }
        LineNumber* currentLine = node->lines;
        while (currentLine != NULL) {
            LineNumber* nextLine = currentLine->next;
            free(currentLine);
            currentLine = nextLine;
        }
        free(node);
    }
}



