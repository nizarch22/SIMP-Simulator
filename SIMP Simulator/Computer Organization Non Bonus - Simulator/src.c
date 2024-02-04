#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "stack.h"
#include "fileParser.h"

#define MAX_PC 4095
#define IMMEDIATE_REGISTER_NUM 1
#define STACK_POINTER_REGISTER_NUM 14
#define REGISTER_ZERO 0 

#define SINGLE_TRACE_SIZE 154 //PC=3, INSTR = 5, SPACES = 17, EACH NUMBER MAX = 8-> 15*8=128, AND A ZERO NULLIFIER=1
#define MEM_SIZE 4096
#define MAX_NUM 524287
#define MAX_IMM_NUM 1048575 

typedef struct
{
    char* errorBuffer;
} Error;

Error error;
int cycles = 0;
int MEM[MEM_SIZE] = { 0 };
int R[16] = { 0 }; // registers
int PC = 0; // program counter
int HALT = 0; // halt 'boolean' flag
int stackMem[STACK_MEM_SIZE] = { 0 }; // defined stackMem despite the 'infinite' space stack struct provides, because this is faster in most cases.
stack* sp; // stack pointer
char currentTrace[SINGLE_TRACE_SIZE]; // array of the trace line that needs to be output into the traces text file


typedef struct
{
    int opcode;
    int rd; // return register
    int rs; // first register
    int rt; // second register
    int imm;
} instruction;

// Instruction execution and analysis
instruction* constructInstruction(int instrNumber); // Takes the integer representation of the instruction and turns into an instruction parts with separate integer values. i.e. opcode, rd, rs, rt, and imm for I type
void loadInstructions(instruction** instructions, char** lines); // this functions loads the instructions from the memin.txt file onto the 'instructions' array.
void executeInstruction(instruction* instr); // executes the current instruction, instr.
void writeTrace(FILE* file); // outputs the trace into the file.
void setCurrentTrace(int currentPC, instruction* instr); // sets the currentTrace buffer
void writeOutputs(char* pathMemout, char* pathRegisters, char* pathCycles); // writes the other output files i.e. memout.txt, regout.txt, cycles.txt
char getType(instruction* instr); //determines the type of the instruction (R or I).
void inc(int x); // increments the PC according to x.

// Helper function - functions that are meant for string manipulation, and bitwise operations. These are self-explanatory.
char* intToBinary(unsigned int number);
void intFilter(int* number);
void uIntFilter(unsigned int* number);
int hexToInt(char* hex);
int hexToUint(char* hex);

// error functions - error 
void addError(char* errorMsg); // Adds an error message and executes the HALT sequence.
void reportError(); // Prints the current error (if there exists an error).
void overflowFilter(int num); // In case of loading a number that is over 20 bits long - a HALT sequence is initiated.
void overflowFilterImm(int immNumber); // In case of loading a number that is over 20 bits long into the immediate register - a HALT sequence is initiated.

// Memory functions
int loadMem(int index, int* val);
void storeMem(int index, int value);


int main(int argc, char* argv[])
{
    if (argc < 5)  // use this to run from command prompt
    {
        printf("Not enough arguments\n");
        exit(1);
    }
    //// debug
    //char* argv[] = { "empty", "C:\\Users\\buttm\\source\\repos\\memin.txt",
    //"C:\\Users\\buttm\\source\\repos\\memoutDEBUG1.txt",
    //"C:\\Users\\buttm\\source\\repos\\regoutDEBUG1.txt",
    //"C:\\Users\\buttm\\source\\repos\\traceDEBUG1.txt",
    //"C:\\Users\\buttm\\source\\repos\\cyclesDEBUG1.txt" };

    error.errorBuffer = NULL;

    sp = constructStack();
    stackInit(sp);

    char* contents = readFile(argv[1]);
    char** lines = getSepArr(contents, "\n");

    instruction** instructions = (instruction**)malloc((get2dArrayLength(lines) + 1) * sizeof(instruction*));
    loadInstructions(instructions, lines); // load instructions

    // creating file for trace
    char* tracePath = argv[4];
    FILE* fileTemp = fopen(tracePath, "w+");
    fclose(fileTemp);

    // writing into the trace file
    FILE* file = fopen(tracePath, "a");
    while (instructions[PC] != NULL && !HALT)
    {
        setCurrentTrace(PC, instructions[PC]);
        writeTrace(file);
        executeInstruction(instructions[PC]);
    }
    fclose(file);

    if (HALT)
    {
        reportError();
    }

    //outputs
    writeOutputs(argv[2], argv[3], argv[5]);

    // free memory
    free2DArray(lines);
    free2DArray(instructions);
    free(contents);
    if (error.errorBuffer != NULL)
        free(error.errorBuffer);

    freeStack();
    return 0;
}

int instructionToInt(instruction* instruc)
{
    instruction instr = *instruc;

    int result = 0;
    result += instr.rt;
    result += instr.rs << 4;
    result += instr.rd << 8;
    result += instr.opcode << 12;

    return result;
}

void executeInstruction(instruction* instr)
{
    int opcode = instr->opcode;
    int rd, rs, rt, imm;
    rd = instr->rd, rs = instr->rs, rt = instr->rt, imm = instr->imm;

    int intTemp;
    unsigned int uintTemp;
    // immediate value handling
    overflowFilterImm(imm);
    intFilter(&imm);
    R[IMMEDIATE_REGISTER_NUM] = imm;
    if (HALT)
        return;

    // Handling PC
    if (getType(instr) == 'I')
    {
        cycles += 2;
        inc(2);
    }
    else
    {
        cycles++;
        inc(1);
    }

    switch (opcode)
    {
    case 0:
        R[rd] = R[rs] + R[rt];
        overflowFilter(R[rd]);
        break;
    case 1:
        R[rd] = R[rs] - R[rt];
        overflowFilter(R[rd]);
        break;
    case 2:
        R[rd] = R[rs] * R[rt];
        overflowFilter(R[rd]);
        break;
    case 3:
        R[rd] = R[rs] & R[rt];
        overflowFilter(R[rd]);
        break;
    case 4:
        R[rd] = R[rs] | R[rt];
        overflowFilter(R[rd]);
        break;
    case 5:
        R[rd] = R[rs] ^ R[rt];
        overflowFilter(R[rd]);
        break;
    case 6://sll
        R[rd] = R[rs] << R[rt];
        overflowFilter(R[rd]);
        intFilter(&R[rd]);
        break;
    case 7://sra
        // sign extension
        intTemp = R[rs];
        if (R[rs]<0)
        {
            intTemp *= -1;
        }
        R[rd] = intTemp >> R[rt];
        if (R[rs] < 0)
        {
            R[rd] *= -1;
        }
        break;
    case 8://srl
        // logical shift
        uintTemp = R[rs];
        uIntFilter(&uintTemp);
        R[rd] = uintTemp >> R[rt];

        break;
    case 9://beq
        if (R[rs] == R[rt])
            PC = R[rd];
        break;
    case 10:
        if (R[rs] != R[rt])
            PC = R[rd];
        break;
    case 11:
        if (R[rs] < R[rt])
            PC = R[rd];
        break;

    case 12:
        if (R[rs] > R[rt])
            PC = R[rd];
        break;
    case 13:
        if (R[rs] <= R[rt])
            PC = R[rd];
        break;
    case 14:
        if (R[rs] >= R[rt])
            PC = R[rd];
        break;

    case 15: //jal
        R[rd] = PC;
        PC = R[rs];
        break;
    case  16://lw 
        if (getMemType(instr) == STACK_POINTER_REGISTER_NUM)
        {
            if (R[rs] + R[rt] < 1)
            {
                addError("Memory index access violation!\n");
                return;
            }
            if (R[rs] + R[rt] > getStackHeight()+STACK_MEM_SIZE-1)
            {
                addError("Memory index access violation!\n");
                return;
            }
            loadStack(R[rs] + R[rt], &R[rd], stackMem);
        }
        else
        {
            if (R[rs] + R[rt] > 4095)
            {
                addError("Memory index access violation!\n");
                return;
            }
            loadMem(R[rs] + R[rt], &R[rd]);
        }
        cycles++;
        break;

    case 17: //sw
        if (getMemType(instr) == STACK_POINTER_REGISTER_NUM)
        {
            if (R[rs] + R[rt] < 1)
            {
                addError("Memory index access violation of the stack!\n");
                return;
            }
            storeStack(R[rs] + R[rt], R[rd], stackMem);
        }
        else
        {
            if (R[rs] + R[rt] < 0)
            {
                addError("Memory index access violation!\n");
                return;
            }
            // check for overflow
            if (R[rs] + R[rt] > 4095)
            {
                addError("Overflow Error! Memory index access violation!\n");
                return;
            }
            storeMem(R[rs] + R[rt], R[rd]);
        }
        cycles++;
        break;
    case 18: //halt
        HALT = 1;
        break;
    default:
        break;
    }

    if (R[REGISTER_ZERO] != 0)
    {
        addError("Value Assignment Error! Cannot modify the value of $zero register.");
        return;
    }

    if (PC < 0 || PC>4095)
    {
        addError("Value Assignment Error! Invalid input to the PC.");
        return;
    }
    if (R[IMMEDIATE_REGISTER_NUM] != imm)
    {
        addError("Value Assignment Error! Cannot modify the value of $imm register.");
        return;
    }
}

int binaryToInt(char str[], int a, int length)
{
    int b = length - 1;
    int result = 0;

    int n;
    for (int i = 0; i < length; i++)
    {
        n = a + i;
        result += (int)(str[n] - 48) * (int)pow(2, (b - i)); //(1<<(b-i))
    }

    return result;
}


char* intToBinary(unsigned int number)
{
    // int to binary, taking only the first 20 bits (from the right).
    char* result = (char*)malloc(21);
    unsigned int a;
    for (int i = 0; i < 20; i++)
    {
        a = number;
        // 
        a >>= i;
        a <<= 31;
        a >>= 31;

        result[19 - i] = (char)(a + 48);

    }
    result[20] = '\0';
    return result;
}

char getType(instruction* instr)
{
    // $imm register number = 1
    if (instr->rd == IMMEDIATE_REGISTER_NUM || instr->rs == IMMEDIATE_REGISTER_NUM || instr->rt == IMMEDIATE_REGISTER_NUM)
        return 'I';
    return 'R';

}
void inc(int x)
{
    PC += x;
    if (PC > MAX_PC)
    {
        addError("Max PC value exceeded! Cannot execute any more instructions.");
    }
}

int loadMem(int index, int* val)
{
    *val = MEM[index];
}
void storeMem(int index, int value)
{
    MEM[index] = value;
}

int getMemType(instruction* instr)
// determine it is stack, or data memory
{
    if (instr->rd == STACK_POINTER_REGISTER_NUM || instr->rs == STACK_POINTER_REGISTER_NUM || instr->rt == STACK_POINTER_REGISTER_NUM)
        return STACK_POINTER_REGISTER_NUM;
    return 0; // data memory
}

void writeOutputs(char* pathMemout, char* pathRegisters, char* pathCycles)
{
    // Registers
    FILE* file;

    file = fopen(pathMemout, "w");
    if (!file)
    {
        printf("[ERROR] Failed to open file!\n");
        exit(1);
    }
    for (int i = 0; i < MEM_SIZE-1; i++)
        fprintf(file, "%05X\n", MEM[i]);
    fprintf(file, "%05X", MEM[4095]);   // Insuring that we do not have an extra new line at the bottom of the file.
    fclose(file);                       //

    file = fopen(pathRegisters, "w");
    if (!file)
    {
        printf("[ERROR] Failed to open file!\n");
        exit(1);
    }

    for (int i = 2; i < 15; i++)
    {
        fprintf(file, "%08X\n", R[i]);
    }
    fprintf(file, "%08X", R[15]);   // Insuring that we do not have an extra new line at the bottom of the file.
    fclose(file);                   //

    file = fopen(pathCycles, "w");
    if (!file)
    {
        printf("[ERROR] Failed to open file!\n");
        exit(1);
    }
    fprintf(file, "%d", cycles);
    fclose(file);
}

void intFilter(int* number)
{
    // turns int number into 20 bit representation
    int num = *number;

    char* numBinary = intToBinary(num);
    num = binaryToInt(numBinary, 0, 20);
    if (numBinary[0] == '1')
    {
        num -= pow(2, 20);
    }

    free(numBinary);
    *number = num;
}

void uIntFilter(unsigned int* number)
{
    // turns uint number into 20 bit representation
    unsigned int num = *number;
    char* numBinary = intToBinary(num);
    num = binaryToInt(numBinary, 0, 20);

    free(numBinary);
    *number = num;
}

void setCurrentTrace(int currentPC, instruction* instr)
{
    int instructionNumber = instructionToInt(instr);
    sprintf(currentTrace, "%03X %05X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X %08X", currentPC, instructionNumber, R[0], R[1], R[2], R[3], R[4], R[5], R[6], R[7],
        R[8], R[9], R[10], R[11], R[12], R[13], R[14], R[15]);
}

void addError(char* errorMsg)
{
    HALT = 1;
    int length = getArrayLength(errorMsg);
    error.errorBuffer = (char*)malloc(length + 1);
    strcpy(error.errorBuffer, errorMsg);
}

void reportError()
{
    if (error.errorBuffer != NULL)
        printf("[ERROR] %s\n", error.errorBuffer);
}

void writeTrace(FILE* file)
{
    // adding to the already existing file
    fputs(currentTrace, file);
    fputs("\n", file);
}

int hexToInt(char* hex)
{
    int result = (int)strtol(hex, NULL, 16); // turn hex into int
    return result;
}

int hexToUint(char* hex)
{
    int result = (int)strtol(hex, NULL, 16); // turn hex into int
    return result;
}

void overflowFilter(int num)
{
    if (num > MAX_NUM || num < -(MAX_NUM + 1))
    {
        addError("Overflow Error! Cannot store values that exceed 20 bits in length.");
    }
}

void loadInstructions(instruction** instructions, char** lines)
{
    int countInstructions = 0;
    int countLines = 0;
    while (lines[countLines] != NULL)
    {
        char* instructionHexBuffer = lines[countLines];

        int instrNumber = hexToInt(instructionHexBuffer);

        instruction* instruction = constructInstruction(instrNumber);
        instructions[countInstructions] = instruction;
        if (getType(instruction) == 'I')
        {
            // type I
            countLines++;
            char* immHexBuffer = lines[countLines];

            int immNumber = hexToInt(immHexBuffer); 
            instruction->imm = immNumber;

            // filling an 'empty' or useless instruction to make up for the gap in the array that the I instruction creates 
            countInstructions++;
            instructions[countInstructions] = constructInstruction(0);
            instructions[countInstructions]->opcode = -1;
        }
        else
        {
            // R type
            instruction->imm = 0;
        }

        countInstructions++;
        countLines++;
    }
    instructions[countInstructions] = NULL;
}

instruction* constructInstruction(int instrNumber)
{
    instruction* result = malloc(sizeof(instruction));

    char* instrStr = intToBinary(instrNumber);
    int opcode = binaryToInt(instrStr, 0, 8);
    int rd = binaryToInt(instrStr, 8, 4);
    int rs = binaryToInt(instrStr, 12, 4);
    int rt = binaryToInt(instrStr, 16, 4);

    instruction instruc = { opcode, rd, rs ,rt, 0 };
    *result = instruc;

    free(instrStr);
    return result;
}

void overflowFilterImm(int immNumber)
{
    if (immNumber > MAX_IMM_NUM)
    {
        addError("Overflow Error! Cannot store values that exceed 20 bits in length into the immediate register.");
    }
}