#include "sm4.h"
#include <stdio.h>
#include <string.h>

void print_hex(const uint8_t *data, size_t len) {
    for (size_t i = 0; i < len; i++) {
        printf("%02x", data[i]);
        if (i % 4 == 3 && i != len - 1) {
            printf(" ");
        }
    }
    printf("\n");
}

void print_block(const char *label, const uint8_t *block) {
    printf("%s: ", label);
    print_hex(block, SM4_BLOCK_SIZE);
}
