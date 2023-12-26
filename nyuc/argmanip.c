#ifndef _ARGMANIP_H_
#define _ARGMANIP_H_
#include "argmanip.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
// argc: the size of the input 
// argv: array format for the input variables
// manip: the function you want to run
// you will make a “copy” of the argument list7. 

char **manipulate_args(int argc,
 const char *const *argv, int (*const manip)(int)){
   
	char **sentence = malloc( (argc+1) *sizeof(char*)); // array of strings, the last element is null
	for(int index = 0; index< argc;index++){ // last number is null, so we use < args
		int size = strlen(argv[index]);
		sentence[index] = malloc((size +1 ) * sizeof(char));
        const char *const lista = argv[index];
		for(int element = 0; element< size; element++){
			sentence[index][element] =  (*manip)(lista[element]); 
		}
	}
	return sentence;


}




void free_copied_args(char **args, ...){
	va_list arg;
    va_start(arg, *args);
    while (*args != NULL) {
    	char** temp = va_arg(arg, char**);
        char* string = *temp;
        int index = 0;
        printf("%d", string[index]);
        while(string[index]){
        	free(&string[index]);
        	index++;
        }
        free(string);
        args++;
    }  
    va_end(arg);


}
    /*
    	while(){

    	}
    	int length = strlen(*temp);
        for(int index = 0; index < length;index++){ 
             int size = strlen(*temp);
             char * lista = *temp;
             for(int element = 0; element< size;element++){ 
                 free(lista[element]);
             free(lista);
		} */

#endif
