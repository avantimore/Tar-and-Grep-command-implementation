#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include<errno.h>
#include <dirent.h>
// #include <libgen.h>
#include"arcFuns.h"
#include"compression.h"
#define SYMBOL_SIZE 256
#define BUFFER_SIZE 16384

#ifdef _WIN32
#include <direct.h> // For Windows-specific _mkdir() function
#define mkdir(dir, mode) _mkdir(dir)
#define DIR_SEPARATOR '\\'
#else
#include <sys/stat.h> // For mkdir() function on non-Windows platforms
#define DIR_SEPARATOR '/'
#endif

#ifndef PATH_MAX
#define PATH_MAX 260
#endif


Node* createNode(unsigned char symbol, unsigned int frequency) {
    Node* node = (Node*)malloc(sizeof(Node));
    node->symbol = symbol;
    node->frequency = frequency;
    node->left = NULL;
    node->right = NULL;
    return node;
}

void destroyNode(Node* node) {
    if (node == NULL)
        return;
    destroyNode(node->left);
    destroyNode(node->right);
    free(node);
}

void countSymbolFrequencies(const char* filePath, unsigned int* frequencies) {
    FILE* file = fopen(filePath, "rb");
    if (file == NULL) {
        printf("Error opening file: %s\n", filePath);
        exit(1);
    }

    unsigned char buffer[BUFFER_SIZE];
    size_t bytesRead;

    while ((bytesRead = fread(buffer, sizeof(unsigned char), BUFFER_SIZE, file)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            frequencies[buffer[i]]++;
        }
    }

    fclose(file);
}

Node* buildHuffmanTree(unsigned int* frequencies) {
    Node* forest[SYMBOL_SIZE];
    int numNodes = 0;

    for (unsigned int i = 0; i < SYMBOL_SIZE; i++) {
        if (frequencies[i] > 0) {
            Node* node = createNode(i, frequencies[i]);
            forest[numNodes++] = node;
        }
    }

    while (numNodes > 1) {
        // Find the two nodes with the lowest frequency
        int minIndex1 = 0;
        int minIndex2 = 1;

        if (forest[minIndex2]->frequency < forest[minIndex1]->frequency) {
            int temp = minIndex1;
            minIndex1 = minIndex2;
            minIndex2 = temp;
        }

        for (int i = 2; i < numNodes; i++) {
            if (forest[i]->frequency < forest[minIndex1]->frequency) {
                minIndex2 = minIndex1;
                minIndex1 = i;
            } else if (forest[i]->frequency < forest[minIndex2]->frequency) {
                minIndex2 = i;
            }
        }

        // Create a new node with the combined frequency
        Node* newNode = createNode(0, forest[minIndex1]->frequency + forest[minIndex2]->frequency);
        newNode->left = forest[minIndex1];
        newNode->right = forest[minIndex2];

        // Remove the processed nodes from the forest
        forest[minIndex1] = newNode;
        forest[minIndex2] = forest[numNodes - 1];
        numNodes--;
    }

    return forest[0];

}


void generateCodeTable(Node* node, char* code, int depth, CodeEntry* codeTable) {
    if (node->left == NULL && node->right == NULL) {
        code[depth] = '\0';
        codeTable[node->symbol].symbol = node->symbol;
        codeTable[node->symbol].code = strdup(code);
        return;
    }

    code[depth] = '0';
    generateCodeTable(node->left, code, depth + 1, codeTable);

    code[depth] = '1';
    generateCodeTable(node->right, code, depth + 1, codeTable);
}


void compressFile(const char* filePath, const char* outputFile) {
    unsigned int frequencies[SYMBOL_SIZE] = {0};
    countSymbolFrequencies(filePath, frequencies);

    Node* huffmanTree = buildHuffmanTree(frequencies);

    char code[SYMBOL_SIZE];
    CodeEntry codeTable[SYMBOL_SIZE];
    generateCodeTable(huffmanTree, code, 0, codeTable);

    FILE* input = fopen(filePath, "rb");
    FILE* output = fopen(outputFile, "ab");

    if (input == NULL) {
        printf("Error opening file: %s\n", filePath);
        exit(1);
    }

    if (output == NULL) {
        printf("Error creating output file: %s\n", outputFile);
        exit(1);
    }

    fwrite(frequencies, sizeof(unsigned int), SYMBOL_SIZE, output);

    unsigned char inputBuffer[BUFFER_SIZE];
    unsigned char outputBuffer[BUFFER_SIZE];
    unsigned int bufferIndex = 0;
    unsigned int bufferBits = 0;
    int bitIndex = 7;

    size_t bytesRead;
    size_t bytesWritten;

    while ((bytesRead = fread(inputBuffer, sizeof(unsigned char), BUFFER_SIZE, input)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            char* huffmanCode = codeTable[inputBuffer[i]].code;
            int codeLength = strlen(huffmanCode);

            for (int j = 0; j < codeLength; j++) {
                bufferBits |= (huffmanCode[j] - '0') << bitIndex;
                bitIndex--;

                if (bitIndex < 0) {
                    outputBuffer[bufferIndex++] = bufferBits;
                    bufferBits = 0;
                    bitIndex = 7;

                    if (bufferIndex >= BUFFER_SIZE) {
                        bytesWritten = fwrite(outputBuffer, sizeof(unsigned char), BUFFER_SIZE, output);
                        if (bytesWritten != BUFFER_SIZE) {
                            printf("Error writing to file.\n");
                            exit(1);
                        }
                        bufferIndex = 0;
                    }
                }
            }
        }
    }

    if (bitIndex < 7) {
        outputBuffer[bufferIndex++] = bufferBits;
    }

    if (bufferIndex > 0) {
        bytesWritten = fwrite(outputBuffer, sizeof(unsigned char), bufferIndex, output);
        if (bytesWritten != bufferIndex) {
            printf("Error writing to file.\n");
            exit(1);
        }
    }

    fclose(input);
    fclose(output);

    destroyNode(huffmanTree);

    
    printf("File compression complete. Compressed data written to: %s\n", outputFile);
}

void decompressFile(const char* filePath, const char* outputFile) {
    FILE* input = fopen(filePath, "rb");
    FILE* output = fopen(outputFile, "wb");

    if (input == NULL || output == NULL) {
        printf("Error opening file.\n");
        exit(1);
    }

    unsigned int frequencies[SYMBOL_SIZE];
    size_t bytesRead = fread(frequencies, sizeof(unsigned int), SYMBOL_SIZE, input);

    if (bytesRead != SYMBOL_SIZE) {
        printf("Error reading from file.\n");
        exit(1);
    }

    Node* huffmanTree = buildHuffmanTree(frequencies);

    unsigned char inputBuffer[BUFFER_SIZE];
    unsigned char outputBuffer[BUFFER_SIZE];
    unsigned int bufferIndex = 0;
    unsigned int bufferBits = 0;
    int bitIndex = 7;

    Node* currentNode = huffmanTree;
    size_t bytesWritten;

    while ((bytesRead = fread(inputBuffer, sizeof(unsigned char), BUFFER_SIZE, input)) > 0) {
        for (size_t i = 0; i < bytesRead; i++) {
            unsigned char byte = inputBuffer[i];

            for (int j = 7; j >= 0; j--) {
                if ((byte >> j) & 1) {
                    currentNode = currentNode->right;
                } else {
                    currentNode = currentNode->left;
                }

                if (currentNode->left == NULL && currentNode->right == NULL) {
                    outputBuffer[bufferIndex++] = currentNode->symbol;
                    currentNode = huffmanTree;

                    if (bufferIndex >= BUFFER_SIZE) {
                        bytesWritten = fwrite(outputBuffer, sizeof(unsigned char), BUFFER_SIZE, output);
                        if (bytesWritten != BUFFER_SIZE) {
                            printf("Error writing to file.\n");
                            exit(1);
                        }
                        bufferIndex = 0;
                    }
                }
            }
        }
    }

    if (bufferIndex > 0) {
        bytesWritten = fwrite(outputBuffer, sizeof(unsigned char), bufferIndex, output);
        if (bytesWritten != bufferIndex) {
            printf("Error writing to file.\n");
            exit(1);
        }
    }

    fclose(input);
    fclose(output);

    destroyNode(huffmanTree);

    printf("Decompression complete. Decompressed data written to: %s\n", outputFile);
}


void compressDirectory(const char* inputDir) {
    DIR* dir = opendir(inputDir);
    if (dir == NULL) {
        printf("Error opening directory: %s\n", inputDir);
        exit(1);
    }

    // Determine the parent directory path
    char parentDir[PATH_MAX];
    size_t inputDirLen = strlen(inputDir);
    size_t lastSeparatorIndex = 0;

    for (size_t i = inputDirLen - 1; i > 0; i--) {
        if (inputDir[i] == '/' || inputDir[i] == '\\') {
            lastSeparatorIndex = i;
            break;
        }
    }

    strncpy(parentDir, inputDir, lastSeparatorIndex);
    parentDir[lastSeparatorIndex] = '\0';

    // Create a new directory for compressed files
    char outputDir[PATH_MAX];
    // snprintf(outputDir, sizeof(outputDir), "%s_compressed", parentDir);
    snprintf(outputDir, sizeof(outputDir), "%s%s.tar", parentDir, inputDir );
    if (mkdir(outputDir, 0777) == -1 && errno != EEXIST) {
        printf("Error creating directory: %s\n", outputDir);
        exit(1);
    }

    struct dirent* entry;
    char inputPath[PATH_MAX];
    char outputPath[PATH_MAX];
    const char* filename = "output.txt";
    
    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        printf("Error opening file.\n");
        return ;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(inputPath, sizeof(inputPath), "%s/%s", inputDir, entry->d_name);
        snprintf(outputPath, sizeof(outputPath), "%s/%s.tar", outputDir, entry->d_name);

        struct stat st;
        if (stat(inputPath, &st) == -1) {
            printf("Error getting file status.\n");
            exit(1);
        }

        if (S_ISDIR(st.st_mode)) {
            compressDirectory(inputPath);
        } else if (S_ISREG(st.st_mode)) {
            archiveFiles(inputPath, filename);
        }
    }
    compressFile(filename,outputPath);

    closedir(dir);
    fclose(file);

    printf("Directory compression complete. Compressed files written to: %s\n", outputDir);
}

void decompressDirectory(const char* inputDir) {
    DIR* dir = opendir(inputDir);
    if (dir == NULL) {
        printf("Error opening directory: %s\n", inputDir);
        exit(1);
    }

    // Determine the parent directory path
    char parentDir[PATH_MAX];
    size_t inputDirLen = strlen(inputDir);
    size_t lastSeparatorIndex = 0;

    for (size_t i = inputDirLen - 1; i > 0; i--) {
        if (inputDir[i] == '/' || inputDir[i] == '\\') {
            lastSeparatorIndex = i;
            break;
        }
    }

    strncpy(parentDir, inputDir, lastSeparatorIndex);
    parentDir[lastSeparatorIndex] = '\0';


    struct dirent* entry;
    char inputPath[PATH_MAX];
    char outputPath[PATH_MAX];

    const char* outputFile = "output.txt";
    while ((entry = readdir(dir)) != NULL) {
        if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
            continue;
        }

        snprintf(inputPath, sizeof(inputPath), "%s/%s", inputDir, entry->d_name);

        struct stat st;
        if (stat(inputPath, &st) == -1) {
            printf("Error getting file status.\n");
            exit(1);
        }
        decompressFile(inputPath, outputPath);
        
    }
       extractFiles(outputPath); 
        
    closedir(dir);

    printf("Directory decompression complete.");
}
