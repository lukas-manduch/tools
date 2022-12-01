// Convert raw hex data to binary format.
//
// Takes 1 or 2 arguments.
//	1. Out file. Input is on stdin
//	2. in file, out file

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TEST 0
#define USAGE "Converts raw hex data to binary. \n"\
	"Takes one or two arguments:\n"\
	"program <outfile>   -- input is now supplied via STDIN\n"\
	"program <infile> <outfile>\n"

// Return -1 for error or 0-15 for hex character
int hexchar_to_int(int character) {
	if (character >= '0' && character <= '9')
		return character - '0';
	if (character >= 'a' && character <= 'f')
		return character - 'a' + 10;
	if (character >= 'A' && character <= 'F')
		return character - 'A' + 10;
	return -1;
}

// Code returned will be program exit code
int run(FILE* input_stream, FILE* output_stream) {
	uint64_t position = 0;
	_Bool invalid_found = 0;
	int a = -1; // Converted value of first (of 2) hex characters will be here
	while (1) {
		int character = fgetc(input_stream);
		int numeric_value = hexchar_to_int(character);
		position++;
		// Handle success
		if (numeric_value != -1) {
			if (a == -1) { // This is first (of 2) hex char
				a = numeric_value;
			}
			else { // We have one byte, write it
				int fput_ret = fputc((char)(16*a + numeric_value), output_stream);
				a = -1;
				if (fput_ret == EOF) {
					fprintf(stderr, "Error writing output\n");
					exit(1);
				}
			}
		}
		// Handle EOF
		if (character == EOF) {
			if (feof(input_stream)) {
				if (a != -1) {
					fprintf(stderr, "Error, odd number of hex characters!\n");
					return 1;
				}
				return 0;
			}
			fprintf(stderr, "Error writing to stream\n");
			return 1;
		}
		// Handle error
		if (numeric_value == -1 && !invalid_found && !isspace(character)) {
			fprintf(stderr, "Bad character at position %ld\n", position);
			invalid_found = 1;
		}
	}
	return 0;
}

int main(int argc, char** argv) {
	FILE *input_stream = NULL;
	switch(argc) {
		case 2:
			input_stream = stdin;
			// If we get only --help/-h then don't treat it as
			// output file, but show help instead
			if (strcmp(argv[1], "-h") == 0 || 
					strcmp(argv[1], "--help") == 0) {
				printf(USAGE);
				exit(0);
			}
			break;
		case 3:
			input_stream = fopen(argv[1], "r");
			if (input_stream == NULL) {
				perror("Cannot open input file for reading");
				exit(1);
			}
			break;
		default:
			printf(USAGE);
			exit(1);
	}
	FILE* output_file = fopen(argv[argc-1], "wb");
	if (output_file == NULL) {
		perror("Cannot write to output");
		exit(1);
	}
	int ret = run(input_stream, output_file);
	fclose(output_file);
	fclose(input_stream);
	return ret;
}
