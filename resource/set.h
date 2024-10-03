#ifndef SET
#define SET
#include <stdio.h>
#include <string.h>
#include "./shFile.h"
#include <stdbool.h>
#define MAX_SET_SIZE 100 
#define MAX_STR_LEN 100   
void display_set(char set[][MAX_STR_LEN], int size) {
    printf("Set of strings:\n");
    for (int i = 0; i < size; i++) {
        printf("%s\n", set[i]);
    }
}
bool is_present(char set[][MAX_STR_LEN], int size, const char *str){
     for (int i = 0; i < size; i++) {
        if (strcmp(set[i], str) == 0) {
            return true;  
        }
    }
    return false;  
}
bool add_to_shared_set(const char *str) {
    
    // return false;
    if (!is_present(shared_set, *shared_set_size, str)) {
        strcpy(shared_set[*shared_set_size], str);  
        (*shared_set_size)++;     
        display_set(shared_set, *shared_set_size);
        return true;        
    }else return false;
}
#endif 
