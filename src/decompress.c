#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


int fill_codetable(FILE* fp, char** codetable){
    int orig_bytes = 0;
    fscanf(fp, "%d", &orig_bytes);

    int ending_char = fgetc(fp);
    char temp[256]; // max length for the codetable: 255 (+1 ending char)

    while (!feof(fp)) {
        int i = 0;

        ending_char = fgetc(fp);
        int ascii = ending_char;

        ending_char = fgetc(fp);
        if (ending_char != '0' && ending_char != '1')
            break;

        /* if not end, get binary code */
        while(ending_char != '.') {
            temp[i] = ending_char;
            i++;
            ending_char = fgetc(fp);
        }
        temp[i] = '\0';

        codetable[ascii] = (char*) malloc(sizeof(char) * (i + 1));
        if (codetable[ascii] == NULL) {
            printf("Memory allocation failed!");
            exit(-1);
        }
        strcpy(codetable[ascii], temp);
    }
    return orig_bytes;
}

void update_decompression_loading(int until, int orig_bytes) {
	char* loading_char = ".";
	int loading_dots = 10;
	if (until % ((orig_bytes / loading_dots) + 1) == 0) // +1 for %0 safety, it doesn't really matter
        printf(loading_char);
}

char* check_codes(FILE* new_file, char* to_decode, char** codetable, int orig_bytes, int* until) {
     bool end = false;
     while(!end) {
        end = true;

        for (int ascii = 0; ascii < 256; ascii++) {
            if (codetable[ascii] != NULL && strncmp(to_decode, codetable[ascii], strlen(codetable[ascii])) == 0) {

                fputc(ascii, new_file);
                (*until)++;

				/* if it's already decoded, only ending chars are left */
                if (*until == orig_bytes)
                    break;

                update_decompression_loading(*until, orig_bytes);

                int code_length = strlen(codetable[ascii]);
                char* temp = (char*) malloc(sizeof(char) * (strlen(to_decode) - code_length + 1));
                if (temp == NULL) {
                    printf("Memory allocation failed!");
                    exit(-1);
                }
                strcpy(temp, &to_decode[code_length]);
                free(to_decode);
                to_decode = temp;

                end = false;
            }
        }
     }
     return to_decode;
}

void write_decoded(FILE* new_file, FILE* orig_file, char** codetable, int orig_bytes) {
    int until = 0;
    char* to_decode = (char*) malloc(sizeof(char));
    if (to_decode == NULL) {
        printf("Memory allocation failed!");
        exit(-1);
    }

    to_decode[0] = '\0';
    int ascii_char = getc(orig_file);
    while (!feof(orig_file)) {

        char* ascii_as_bits = (char*) malloc(sizeof(char) * (8 + 1));
        if (ascii_as_bits == NULL) {
            printf("Memory allocation failed!");
            exit(-1);
        }

        ascii_as_bits[8] = '\0';
        for (int i = 7; i >= 0; i--) { // char to bits
            ascii_as_bits[i] = '0' + (ascii_char % 2);
            ascii_char /= 2;
        }

        /* add to  the to_decode array */
        char* temp = (char*) malloc(sizeof(char) * (strlen(to_decode) + strlen(ascii_as_bits) + 1));
        if (temp == NULL) {
            printf("Memory allocation failed!");
            exit(-1);
        }
        strcpy(temp, to_decode);
        strcat(temp, ascii_as_bits);
        free(to_decode);
        free(ascii_as_bits);
        to_decode = temp;
		
        /* search for codes, if found: remove and return the rest of to_decode */
        to_decode = check_codes(new_file, to_decode, codetable, orig_bytes, &until);
        ascii_char = getc(orig_file);
    }
    free(to_decode);

    /* free table */
    for(int i = 0; i < 256; i++) {
        if(codetable[i] != NULL)
            free(codetable[i]);
    }

}

int decompress(char* file_name) {
    int i;
    for (i = strlen(file_name); file_name[i] != '.'; i--);
    char* extension = &file_name[i];
    if (strcmp(extension, ".hm") != 0) {
        printf("\nInvalid file extension. Valid extension: .hm \n");
        exit(-1);
    }

    FILE* orig_file;
    orig_file = fopen(file_name, "rb");
    if (orig_file == NULL) {
        perror("\nFile not found");
        exit(-1);
    }

    char* codetable[256] = {0};
    int orig_bytes = fill_codetable(orig_file, codetable);

    /* add new extension */
    strcpy(extension, "_out.txt");
    FILE* new_file;
    new_file = fopen(file_name, "wt");
    if (new_file == NULL) {
        perror("\nCouldn't open file");
        exit(-1);
    }

    write_decoded(new_file, orig_file, codetable, orig_bytes);

    fclose(new_file);
    fclose(orig_file);
    printf("\nDecompression was successful!\n");
	printf("Created file: %s\n", file_name);

    return 0;
}
