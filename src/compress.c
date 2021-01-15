#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "huffman.h"

int update_char_frequencies(FILE* orig_file, int* char_freq) {
    int orig_bytes = 0;
    int c = fgetc(orig_file);

    while(!feof(orig_file)) {
           char_freq[c]++; // count frequency of character
           orig_bytes++; // and how many bytes the file is
           c = fgetc(orig_file);
    }
    return orig_bytes;
}

int write_codetable(FILE* to_file, int orig_length, char** codetable) {
    int written_bytes = 0;
    fprintf(to_file, "%d", orig_length);
	
    while (orig_length != 0) {
        written_bytes++; // add as many bytes as long the orig_length string would be
        orig_length /= 10; 
    }
	
    char ending_char = 'x';
    fputc(ending_char, to_file);
    written_bytes++;

    /* write ascii-code pairs */
    for (int ascii_code = 0; ascii_code < 256; ascii_code++) {
        if (codetable[ascii_code] != 0) {
            fputc(ascii_code, to_file);
            written_bytes++;
            for (int i = 0; i < strlen(codetable[ascii_code]); i++) {
                fputc(codetable[ascii_code][i], to_file);
                written_bytes++;
            }
			char end_char = '.';
            fputc(end_char, to_file);
            written_bytes++;
        }
    }
	
	char* table_end_chars = "xx";
    fputs(table_end_chars, to_file);
    written_bytes += 2;

    return written_bytes;
}

int bits_to_ascii(char* bits) {
	int starting_char = '0';
	int bit_per_byte = 8;
	int multiplier = 128;
	
	int ascii_code = 0;
	for (int i = 0; i < bit_per_byte; i++) {
		if (bits[i] == '\0')
			break;
		ascii_code += (bits[i] - starting_char) * multiplier;
		multiplier /= 2;
	}
	return ascii_code;
}

void update_cmd_loading(int written_bytes, int orig_bytes) {
	char* loading_char = ".";
	int loading_dots = 10;
	if (written_bytes % ((orig_bytes / loading_dots) + 1) == 0) // +1 for %0 safety, it doesn't really matter
        printf(loading_char);
}

int write_compressed_text(FILE* to_file, FILE* orig_file, int orig_bytes, char** codetable) {
    int written_bytes = 0;
    rewind(orig_file);
	
    char* bits_to_write = (char*) malloc(sizeof(char));
    if (bits_to_write == NULL) {
        printf("Memory allocation failed!");
        exit(-1);
    }

	char starting_char = '\0';
    bits_to_write[0] = starting_char;
    int c = fgetc(orig_file);
    while(!feof(orig_file)) {
        /* add new char code to bits_to_write */
        char* temp = (char*) malloc(sizeof(char) * (strlen(bits_to_write) + strlen(codetable[c]) + 1));
        if (temp == NULL) {
            printf("Memory allocation failed!");
            exit(-1);
        }
		
        strcpy(temp, bits_to_write);
        strcat(temp, codetable[c]);

        free(bits_to_write);
        bits_to_write = temp;

        int bits_per_byte = 8;
		while (strlen(bits_to_write) >= bits_per_byte) { // group bits to bytes if possible and write it to file
			
			char grouped_bits_to_byte = bits_to_ascii(bits_to_write);
			fputc(grouped_bits_to_byte, to_file);
			written_bytes++;
			update_cmd_loading(written_bytes, orig_bytes);
			

			/* delete first 8 bits from array */
			char* temp = (char*) malloc(sizeof(char) * (strlen(bits_to_write) - 8) + 1);
			if (temp == NULL) {
				printf("Memory allocation failed!");
				exit(-1);
			}
			strcpy(temp, &bits_to_write[8]);
			free(bits_to_write);
			bits_to_write = temp;
		}

        c = fgetc(orig_file);
    }
    if (strlen(bits_to_write) != 0)
        fputc(bits_to_ascii(bits_to_write), to_file);

    free(bits_to_write);
    return written_bytes;
}

char* check_extension(char** file_name_ref) {
	char* file_name = *file_name_ref;
	int i;
    for (i = strlen(file_name); file_name[i] != '.'; i--);

    char* extension = &file_name[i];

    if (strcmp(extension, ".txt") != 0) {
        printf("\nInvalid file extension. Valid extension: .txt\n");
        exit(-1);
    }
	return extension;
}

int compress(char* file_name) {
    char* extension = check_extension(&file_name);
	
    FILE* orig_file;
    orig_file = fopen(file_name, "rt"); // open to read
    if (orig_file == NULL) {
        perror("\nFile not found");
        exit(-1);
    }

    int char_freq[256] = {0};
    int length = update_char_frequencies(orig_file, char_freq);

    char* codetable[256] = {0};
    huffman(char_freq, codetable);

    /* add new extension */
    strcpy(extension, ".hm");
    FILE* to_file;
    to_file = fopen(file_name, "wb");
    if (to_file == NULL) {
        perror("\nCouldn't create a new file.");
        exit(-1);
    }

    int byte_table = write_codetable(to_file, length, codetable);
    int byte_coded = write_compressed_text(to_file, orig_file, length, codetable);
    printf("\nCompression was successful!\n\nBytes: %d -> %d (codetable) + %d (compressed text) = %d\nSize: %d%% of the original\n", length, byte_table, byte_coded, byte_table + byte_coded, (byte_table + byte_coded) * 100 / length);
	printf("Created file: %s\n", file_name);
	
    fclose(orig_file);
    fclose(to_file);
    return 0;
}
