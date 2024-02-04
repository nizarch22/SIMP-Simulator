#pragma once

// reading and separating of the HEX instruction file
char* readFile(char* filename); // simply reads the file and loads onto a char pointer
char** getSepArr(char* arr, char* sep); // separates the arr char array into many sub arrays (determined by sep char array) - then those sub arrays are loading onto a double pointer

// self-explanatory helper functions
void printd(int x);
void print(char str[]);
int getArrayLength(char* arr);
int get2dArrayLength(char** arr);
void free2DArray(char** arr);
char* copyArray(char* arr);





