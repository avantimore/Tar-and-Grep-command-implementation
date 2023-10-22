#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
//#include <conio.h>
#include"modes.h"
#include"arcFuns.h"
#include"compression.h"
#include "file.h"
#include "directory.h"
#define MAX_LINE_LENGTH 100

#define ALPHABET_SIZE 26



int main(int argc, char *argv[]){


	if (argv[1][0] == 't'){     // implementation of tar command if "tar" is the second commandline argument  
	
    	if(argc<3){
        	printf("Please insert a valid command");
        	return 0;
    	}
    	char mode = findMode(argc,argv);
			if(mode=='c'){
		    	char *destFileName = argv[3];

		    for(int i=4;i<argc;i++){
		      char* fileName = argv[i]; 
		        archiveFiles(fileName,destFileName);
		    }
		    const char* outputPath = "output.tar";
		    compressFile(destFileName, outputPath);
		    printf("File copied successfully.\n");
    	}

    	else if(mode=='x'){
        	char *inFileName = argv[3];
		    const char* outputFile = "output.txt";
		    decompressFile(inFileName, outputFile);
		    extractFiles(outputFile);
		    printf("Files extracted successfully.\n");
    	}
    	else if(mode=='e'){
    	    char* inputDirPath = argv[3];

        	compressDirectory(inputDirPath);
        	printf("Directory compressed successfully.\n");
    	}
    	else if(mode=='d'){
    	    const char* inputDirPath = argv[3];

        	decompressDirectory(inputDirPath) ;
        	printf("Directory decompressed successfully.\n");
    	}
    	else if(mode=='t'){
    	    char *inFileName = argv[3];

		    const char* outputFile = "op.txt";
		    const char* extension = strrchr(inFileName, '.');
		    if(extension != NULL){
		        if (strcmp(extension, ".tar") == 0){
		            decompressFile(inFileName, outputFile);
		            searchFileNames(outputFile); 
		        }
		        else{
		            searchFileNames(inFileName); 
		        }
		    }
		    
   	 		}
    	else{
    	    printf("Please check whether you have entered the right command!");
    	}
   }
   
   else if(argv[1][0] == 'g'){                                          // implement grep command if the secondcommandline argument is "grep"
   			if (argc == 3){

		}
	
	    if (argc < 5 && argc !=3) {
	        printf("Insufficient command line arguments.\n");
	        printf("Usage: %s [grep] [option] [word] [filename]\n", argv[0]);
	        return 1;
	    }
	    
	    // ------if option contains rv like for file------------------
	    
    
	    if (strcmp(argv[2], "-rv") == 0){                               //if the option provided is "-rv" then perform operations related to searching in directory
	
			if(argc < 6){
				printf("Insufficient arguments. Usage: %s <search_word> <directory_name>\n", argv[1]);
			}
			const char *search_word = argv[3];
	    	const char *directory_name = argv[4];

	    	DIR *dir = opendir(directory_name);
	    	if (dir == NULL) {
	        	printf("Error opening directory: %s\n", directory_name);
	     	   return 1;
	    	}

	    	char directory_path[256];
	    	realpath(directory_name, directory_path);
	    	closedir(dir);
	
	    	printf("Searching for '%s' in directory '%s'\n\n", search_word, directory_path);
	
	    	search_word_in_directory(directory_path, search_word);
    

    
    
	    }
    
	    else{
    
    
    
    
    //-----------------------------

	    	TrieNode* root = createTNode();
	    	const char* word = argv[3];
	    	const char* filename = argv[4];
	    	const char* option = argv[2];
	    
      

    
	    	if (strcmp(argv[2], "-i") == 0) {             //if the option provided is "-i" then perform operations related to searching in files
	  			  
			}


    
	    	char *fillename = argv[4];
	   		// printf("\nhiiiii\n");
	    
	   	 	FILE* file = fopen(argv[4], "r");
	    	if (file == NULL) {
	    		if(argc !=3){
	        		printf("Failed to open the file.\n");
	        		}
	        	return 1;
	    	}

	    	char line[100];
	    	int lineNumber = 1;

	    	while (fgets(line, sizeof(line), file) != NULL) {
	        	char* token = strtok(line, " \t\n");
	        	while (token != NULL) {
	        	    insertTWord(root, token, lineNumber);
	            	token = strtok(NULL, " \t\n");
	        	}
	        	lineNumber++;
	    	}

	    	fclose(file);

	    	searchInTrie(root, word, filename, option);

	    	freeTrie(root);
	    	
	    	}
	    	
	    
  }
  
  else{                                                                     //if any other arguments are passed rather than grep or tar
  		printf("Your passed the commandline argument other than tar or grep  !!");
  		}
	    	return 0;	
	
   }
   

