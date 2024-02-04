#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NUM_OF_REGISTERS 16
#define REGISTER_WIDTH 32
#define INSTRUCTION_LEN 20
#define NUM_OF_OPCODES 19
#define LINE_SIZE 300
#define LABEL_SIZE 50
#define NIBBLE_NUM 5
#define ERROR "Oops :( Error on line "


typedef struct label { // linked list of labels
	char name[LABEL_SIZE + 1];
	int address;
	struct label* next;
} Label;

typedef struct word { // linked list of .word instructions sorted by address (increasing order)
	int address;
	int data;
	struct word* next;
} Word;


Label* new_label(char* name, int address); // creates a new label
Label* add_label(Label* head, Label* lbl); // adds a new label to the front of the linked list
Label* find(Label* head, char* name, int line_num); // returns a pointer to the label in the linked list
Word* new_word(int address, int data); // creates a new word
Word* add_word(Word* head, Word* wrd); // adds a new word to the linked list (sorted by address)
int str_to_decimal(char* str_num); // converts a string containing either a decimal repr. of a number or a hex number starting with 0x into an int
const char* assembler(char* line, Label* head, int line_num); // returns the machine code for one line in assembly
int indexof(char which, char* str, int line_num); // helper function used in assembler for decoding opcodes and registers
void free_label_list(Label* head); // function to empty label list
void free_word_list(Word* head); // function to emty word list


int main(int argc, char* argv[]) // use this to run from command prompt
{

	if (argc < 3)  // use this to run from command prompt
	{
		printf("Error! Not enough arguments entered.\n");
		exit(1);
	}


	FILE* input, * output;
	char line[LINE_SIZE + 1], asm_line[2 * (NIBBLE_NUM + 1)], * cmt_ptr;
	int pc = 0;
	Label* lbl_list = NULL, * new_lbl = NULL;
	Word* wrd_list = NULL, * new_wrd = NULL;

	input = fopen(argv[1], "r"); // use this to run from command prompt
	output = fopen(argv[2], "w"); // use this to run from command prompt


	if (input == NULL)
	{
		printf("Error opening file %s.\n", argv[1]);
		exit(1);
	}

	// FIRST PASS for recording labels and .word instructions
	while (fgets(line, sizeof(line), input) != NULL)
	{
		while (line[0] == '\t' || line[0] == ' ' || line[0] == '\f' || line[0] == '\v') // if line starts with tabs, remove them
		{
			memmove(line, line + 1, strlen(line));
		}

		if ((line[0] == '\n') || (line[0] == '#') || (line[0] == '\0')) // if line is empty or is a comment, ignore it
		{
			continue;
		}

		cmt_ptr = strchr(line, '#');
		if (cmt_ptr != NULL)
		{
			char* token = strtok(line, "#"); // ignore comments if present
		}

		if (strstr(line, "$imm") != NULL)  // line contains: I-type instruction
		{
			pc += 2;
		}
		else if (strchr(line, ':') != NULL) { // line contains: label
			char* token = strtok(line, ":"); // remove colon from label name
			new_lbl = new_label(line, pc); // store label and its address (i.e. corresponding pc) in the labels list
			lbl_list = add_label(lbl_list, new_lbl);
		}
		else if (strstr(line, ".word") != NULL) // line contains: .word instruction
		{
			char* token = strtok(line, " \t");
			char* wrd_address = strtok(NULL, " , \t");
			char* wrd_data = strtok(NULL, " , \t\n");
			new_wrd = new_word(str_to_decimal(wrd_address), str_to_decimal(wrd_data)); // store address and data that needs to be written into it
			wrd_list = add_word(wrd_list, new_wrd);
		}
		else // line contains: R-type instruction
		{
			pc++;
		}
	}

	// reopen input file for the second pass
	fclose(input);
	input = fopen(argv[1], "r"); // used when calling the program from command line

	if (input == NULL)
	{
		printf("Error opening file.\n");
		exit(1);
	}

	// SECOND PASS for translating assembly into machine code

	int line_num = 1; // line_num keeps track of the current line in case of errors

	while ((fgets(line, sizeof(line), input) != NULL) && (strstr(line, ".word") == NULL))
	{
		// the beginning is the same as in the first pass
		while (line[0] == '\t' || line[0] == ' ' || line[0] == '\f' || line[0] == '\v') // if line starts with tabs, remove them
		{
			memmove(line, line + 1, strlen(line));
		}

		if ((line[0] == '\n') || (line[0] == '#') || (line[0] == '\0')) // if line is empty or is a comment, ignore it
		{
			line_num++;
			continue;
		}

		cmt_ptr = strchr(line, '#');
		if (cmt_ptr != NULL)
		{
			char* token = strtok(line, "#"); // ignore comments if present
		}

		if (strchr(line, ':') != NULL) // line contains: label -> ignore it
		{
			line_num++;
			continue;
		}
		strcpy(asm_line, assembler(line, lbl_list, line_num)); // translate line into machine code
		fputs(asm_line, output);
		fputs("\n", output);
		line_num++;
	}

	Word* curr_word = wrd_list;
	char str_data[10];

	// if input contains .word instructions, execute them 
	while (curr_word != NULL) // while not all .word instructions have been executed
	{
		for (pc; pc < (curr_word->address); pc++) // if no data needs to be written, fill the line with zeros
		{
			fputs("00000\n", output);
		}
		sprintf(str_data, "%05X", curr_word->data); // write data from .word instruction 
		if (curr_word->data < 0) // ensures proper syntax of negative numbers
		{
			fputs(&str_data[3], output);
		}
		else
		{
			fputs(str_data, output);
		}
		fputs("\n", output);
		pc++;
		curr_word = curr_word->next; // move to the next .word instruction
	}

	fclose(input);
	fclose(output);
	free_label_list(lbl_list);
	free_word_list(wrd_list);

	printf("Yay! No errors encountered!\n");
	return 0;
}

Label* new_label(char* name, int address)
{
	Label* lbl = (Label*)malloc(sizeof(Label));
	if (lbl != NULL) // allocation succeeded
	{
		strcpy(lbl->name, name);
		lbl->address = address;
		lbl->next = NULL;
	}
	return lbl;
}

Label* add_label(Label* head, Label* lbl)
{
	lbl->next = head;
	return lbl;
}

Label* find(Label* head, char* name, int line_num)
{
	while (head != NULL && strcmp(head->name, name) != 0)
		head = head->next;

	if (head == NULL)
	{
		printf("%s%d! Cannot find label %s.\n", ERROR, line_num, name);
		exit(1);
	}
	return head;
}

Word* new_word(int address, int data)
{
	Word* wrd = (Word*)malloc(sizeof(Word));
	if (wrd != NULL) // allocation succeeded
	{
		wrd->address = address;
		wrd->data = data;
		wrd->next = NULL;
	}
	return wrd;
}

Word* add_word(Word* head, Word* wrd)
{
	Word* curr, * prev = NULL;
	if (head == NULL)
	{
		return wrd;
	}
	if (wrd->address < head->address)
	{
		wrd->next = head;
		return wrd;
	}
	curr = head;
	while ((curr != NULL) && (wrd->address > curr->address))
	{
		prev = curr;
		curr = curr->next;
	}
	if (prev != NULL)
	{
		prev->next = wrd;
	}
	wrd->next = curr;
	return head;
}

int str_to_decimal(char* str_num)
{
	int result;

	if (strstr(str_num, "0x") != NULL) // string contains int in hex repr.
	{
		result = strtol(str_num, NULL, 16);
	}
	else // string contains int in decimal repr. 
	{
		result = atoi(str_num);
	}
	return result;
}

int indexof(char which, char* str, int line_num)
{
	if (str == NULL)
	{
		printf("%s%d", ERROR, line_num);
		exit(1);
	}

	int i = 0;
	int index = -1;
	char opcodes[NUM_OF_OPCODES][NIBBLE_NUM + 1] = { "add", "sub", "mul", "and", "or", "xor", "sll", "sra", "srl", "beq", "bne",
	"blt", "bgt", "ble", "bge", "jal", "lw", "sw", "halt" };

	char registers[NUM_OF_REGISTERS][NIBBLE_NUM + 1] = { "$zero", "$imm", "$v0", "$a0", "$a1", "$a2", "$a3", "$t0", "$t1", "$t2",
		"$s0", "$s1", "$s2", "$gp", "$sp", "$ra" };

	if (which == 'o') // find number of opcode (its index in the "matrix" of opcodes) 
	{
		while (index == -1)
		{
			if (strcmp(opcodes[i], str) == 0)
			{
				index = i;
			}
			i++;
			if (i > NUM_OF_OPCODES)
			{
				printf("%s%d! %s is not a valid opcode.\n", ERROR, line_num, str);
				exit(1);
			}
		}
	}
	else // find number of register (its index in the "matrix" of registers) 
	{
		while (index == -1)
		{
			if (strcmp(registers[i], str) == 0)
			{
				index = i;
			}
			i++;
			if (i > NUM_OF_REGISTERS)
			{
				printf("%s%d! %s is not a valid register.\n", ERROR, line_num, str);
				exit(1);
			}
		}
	}
	return index;
}

const char* assembler(char* line, Label* head, int line_num)
{
	int i, imm;
	char j[10], instructions[NIBBLE_NUM + 1][2 * (NIBBLE_NUM + 1)], type = 'R';
	// instructions[][] will contain the separate hex translations of opocodes, registers, and constants

	char* token = strtok(line, " , \t"); // identify the first token
	sprintf(instructions[0], "%02X", indexof('o', token, line_num)); // instructions[0] contains the opcode

	for (i = 1; i < 4; i++)
	{
		token = strtok(NULL, " , \t\n");
		if (strcmp(token, "$imm") == 0) type = 'I';
		sprintf(instructions[i], "%X", indexof('r', token, line_num));
		// instructions[1] contains rd, instructions[2] contains rs, instructions[3] contains rt
	}


	if (type == 'R') // for R-type, ignore imm field
	{
		strcat((strcat((strcat(instructions[0], instructions[1])), instructions[2])), instructions[3]);
	}

	else // for I-type, write imm value on the next line
	{
		token = strtok(NULL, ", \n\t");
		if (token == NULL || token[0] == '$' || isspace(token[0]))
		{
			printf("%s%d! Imm value either missing or invalid.\n", ERROR, line_num);
			exit(1);
		}
		if (token[0] > 64) // imm field contains a label if the first character is a letter, i.e. its ASCII value > 64
		{
			sprintf(instructions[4], "%05X", find(head, token, line_num)->address);
		}
		else // imm field contains a constant
		{
			imm = str_to_decimal(token);
			sprintf(j, "%05X", imm);
			if (imm < 0) // ensures proper syntax of negative numbers
			{
				strcpy(instructions[4], &j[3]);
			}
			else
			{
				strcpy(instructions[4], j);
			}
		}

		strcat(strcat((strcat((strcat((strcat(instructions[0], instructions[1])), instructions[2])), instructions[3])), "\n"), instructions[4]);

	}

	return instructions[0];
}

void free_label_list(Label* head)
{
	while (head) {
		Label* old = head;
		head = head->next;
		free(old);
	}
}

void free_word_list(Word* head)
{
	while (head) {
		Word* old = head;
		head = head->next;
		free(old);
	}
}



