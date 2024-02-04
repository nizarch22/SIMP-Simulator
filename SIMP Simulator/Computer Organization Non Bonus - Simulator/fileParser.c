#pragma warning(disable:4996) // as some functions in readFile are deprecated, there was a need to get rid of the deprecation warnings.
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>


#define HEX_NUM_ZLEN 6 // zero nullified hex number length
#define MAX_LINE_LEN 500*2 // I instruction has 2 lines. Max number of SIMP instructions are 500 lines. // randomly assigned

#define MAX_FILE_CHARACTERS 6*1000 // HEX_NUM_ZLEN*MAX_LINE_LEN (randomly assigned)


//#define FILE_PATH "C:\\Users\\buttm\\code\\MIPS sim\\test.asm"


void print(char str[])
{
	printf("%s\n", str);
}
void printd(int x)
{
	printf("%d\n", x);
}


char* copyArray(char* arr) // strcpy causes an array from strtok to disappear.
{
	char* res = (char*)malloc(getArrayLength(arr) + 1);
	int c = 0;
	while (arr[c] != '\0')
	{
		res[c] = arr[c];
		c++;
	}
	res[c] = '\0';
	return res;
}

int get2dArrayLength(char** arr)
{
	int result = 0;
	while (arr[result] != NULL)
	{
		result++;
	}
	return result;

}

int getArrayLength(char* arr)
{
	int result = 0;
	while (arr[result] != '\0')
	{
		result++;
	}
	return result;

}

char* readFile(char* filename)
{
	char* contents = (char*)malloc(MAX_FILE_CHARACTERS);
	int length = 0;
	FILE* file = fopen(filename, "r");
	if (!file)
	{
		printf("[Error] Failed to open file!\n");
		exit(1);
	}

	while (!feof(file))
	{
		contents[length] = fgetc(file);
		length++;
	}
	fclose(file);

	contents[length - 1] = '\0';
	return contents;
}

char** getSepArr(char* arr, char* sep)
{
	char** result = (char**)malloc(MAX_FILE_CHARACTERS * sizeof(char*));
	char* token = strtok(arr, sep);

	char** temp = result;
	while (token != NULL)
	{
		*temp = copyArray(token);
		token = strtok(NULL, sep);
		temp++;
	}
	*temp = NULL; // add a null pointer to signify the end of array.
	return result;
}
void free2DArray(char** arr)
{
	int i = 0;
	while (arr[i] != NULL) {
		free(arr[i]);
		i++;
	}
	free(arr[i]);
	free(arr);
}




//char** getInstructions()
//{
//	// retrieve file contents
//	char* contents = readFile(FILE_PATH);
//	// get all line of contents
//	char** lines = getSepArr(contents, "\n");
//	int numOfLines = get2dArrayLength(lines);
//
//
//	char** instructions = (char**)malloc(MAX_FILE_SIZE * sizeof(char*));
//	char** temp = instructions;
//	int count = 0;
//	while (lines[count] != NULL)
//	{
//		char* line = lines[count];
//
//		print(line);
//
//		char* instruction = (char*)malloc(MAX_INSTR_LEN + 1);
//
//		int i = 0;
//		while (line[i] != ' ')
//		{
//			if (line[i] == '\0')
//			{
//				break;
//			}
//			instruction[i] = line[i];
//			i++;
//		}
//		instruction[i] = '\0';
//
//		instructions[count] = instruction;
//		count++;
//	}
//	instructions[count] = NULL;
//
//	free(contents);
//	free2DArray(lines);
//
//	return instructions;
//
//
//
//}


//char** getParametersList()
//{
//	// retrieve file contents
//	char* contents = readFile(FILE_PATH);
//	// get all line of contents
//	char** lines = getSepArr(contents, "\n");
//	int numOfLines = get2dArrayLength(lines);
//
//	char*** parametersList = (char***)malloc(MAX_FILE_SIZE * sizeof(char*));
//
//	int count = 0;
//	while (lines[count] != NULL)
//	{
//		char* line = lines[count];
//		// skipping over the instruction part
//		int i = 0;
//		while (line[i] != ' ')
//		{
//			if (line[i] == '\0')
//				break;
//			i++;
//		}
//
//		char** parameters = (char**)malloc((MAX_PARAMETERS + 1) * sizeof(char*));
//
//		int a, b;
//		a = 0; b = 0;
//
//		while (line[i] != '\0')
//		{
//			b = 0;
//			parameters[a] = (char*)malloc(MAX_PAR_LEN + 1);
//			while (line[i] != ',' && line[i] != '\0')
//			{
//				if (line[i] == ' ') // remove spaces from parameters.
//				{
//					i++;
//					continue;
//				}
//				parameters[a][b] = line[i];
//				b++;
//				i++;
//			}
//			parameters[a][b] = '\0';
//			a++;
//			if (line[i] == '\0')
//				break;
//			i++;
//		}
//		parameters[a] = NULL;
//
//		parametersList[count] = parameters;
//		count++;
//	}
//	parametersList[count] = NULL;
//
//	free(contents);
//	free2DArray(lines);
//
//	return parametersList;
//
//}
