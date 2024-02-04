#include <stdlib.h>
#include <stdio.h>

#include "stack.h"

stack* s;
int height = 0;


void stackInit(stack* sp)
{
    s = sp;
}
stack* constructStack()
{
    stack* result = (stack*)malloc(sizeof(stack));
    result->next = NULL;
    result->prev = NULL;
    return result;
}

stack* constructStackWithValue(int value)
{
    stack* result = (stack*)malloc(sizeof(stack));
    result->value = value;
    result->next = NULL;
    result->prev = NULL;
    return result;
}

int getStackValue(stack* s, int index)
{
    // go down by index into the memory
    int result;
    stack* temp = s;
    for (int i = 0; i < index; i++)
        temp = temp->next;
    result = temp->value;

    return result;
}

void addStacks(int amount)
{
    stack* temp = s;
    while (temp->next != NULL)
    {
        temp = temp->next;
    }

    for (int i = 0; i < amount; i++)
    {
        // constructStack, without specifying value.
        stack* newStack = constructStack();

        temp->next = newStack;

        temp = temp->next;
        temp->prev = newStack;
    }
    height+=amount;
}

stack* getStackPointer(int index)
{
    stack* result = s;

    if (index < 0)
    {
        for (int i = 0; i < index; i++)
        {
            result = result->prev;
        }
        return result;
    }

    for (int i = 0; i < index; i++)
    {
        result = result->next;
    }
    return result;
}

void freeStack()
{
    while (s != NULL)
    {
        stack* temp = s->next;
        free(s);
        s = temp;
    }
    height = 0;

}

int getStackHeight()
{
    return height;
}

void storeStack(int index, int value, int mem[])
{
    int stackIndex = index - 1;
    // if index > stack height -> create more stacks
    if (stackIndex < STACK_MEM_SIZE)
    {
        mem[stackIndex] = value;
        return;
    }
    int newIndex = index - STACK_MEM_SIZE;
    if (newIndex > getStackHeight())
    {
        addStacks(newIndex - getStackHeight());
    }
    stack* temp = getStackPointer(newIndex);
    temp->value = value;
}



void loadStack(int index, int* value, int mem[])
{
    int stackIndex = index - 1;
    if (index < STACK_MEM_SIZE)
    {
        *value = mem[stackIndex];
        return;
    }
    int newIndex = index - STACK_MEM_SIZE + 1;
    stack* temp = getStackPointer(newIndex);
    *value = temp->value;
}
