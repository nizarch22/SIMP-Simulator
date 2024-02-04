#pragma once
#define STACK_MEM_SIZE 8192

typedef struct stack
{
    int value;
    struct stack* prev;
    struct stack* next;
} stack;

//stack header
void stackInit(stack* sp); // initiliazing the stack pointer to the desired pointer.
void loadStack(int index, int* value, int mem[]); // loads a value from the stack
void storeStack(int index, int value, int mem[]); // stores a value onto the stack
int getStackHeight(); // gets the number of stack that are available for use
void addStacks(int amount); // constructs many structs that are linked together
stack* getStackPointer(int index); // retrieves the desired stack dependent on index
stack* constructStack(); // constructs a stack
stack* constructStackWithValue(int value); // construct a stack with an initial value
int getStackValue(stack* s, int index); // retrieves the desired stack value dependent on index
void freeStack(); // frees the stacks' heap memory