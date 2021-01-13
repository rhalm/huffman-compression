#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "compress.h"
#include "decompress.h"

int compress(char* file_name);
int decompress(char* file_name);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Please provide the correct arguments! E.g. huffman compress text.txt\n");
        return -1;
    }
    else  {
        if (strcmp(argv[1], "compress") == 0) {
            printf("Compression has started");
            return compress(argv[2]);
        }
        else if (strcmp(argv[1], "decompress") == 0) {
            printf("Decompression has started");
            return decompress(argv[2]);
        }
        else {
            printf("Wrong argument: \"%s\" (2)", argv[1]);
            return -1;
        }
    }
}


