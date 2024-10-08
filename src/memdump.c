#include "memdump.h"

#include <stdio.h>
#include <time.h>

/**
 * @brief Dump memory in hex format
 *
 * @param desc description of the memory mem dumped
 * @param addr pointer to the initial memory address to dump
 * @param len length of the memory to dump
 *
 * @see https://gist.github.com/domnikl/af00cc154e3da1c5d965 for the original code
 */
void hexDump(char *desc, void *addr, int len) {
    int i;
    unsigned char buff[17];
    unsigned char *pc = (unsigned char *)addr;
    // Output description if given.
    if (desc != NULL)
        printf("%s:\n", desc);
    // Process every byte in the data.
    for (i = 0; i < len; i++) {
        // Multiple of 16 means new line (with line offset).
        if ((i % 16) == 0) {
            // Just don't print ASCII for the zeroth line.
            if (i != 0)
                printf("  %s\n", buff);
            // Output the offset.
            printf("  %04x ", i);
        }
        // Now the hex code for the specific character.
        printf(" %02x", pc[i]);
        // And store a printable ASCII character for later.
        if ((pc[i] < 0x20) || (pc[i] > 0x7e)) {
            buff[i % 16] = '.';
        } else {
            buff[i % 16] = pc[i];
        }
        buff[(i % 16) + 1] = '\0';
    }
    // Pad out last line if not exactly 16 characters.
    while ((i % 16) != 0) {
        printf("   ");
        i++;
    }
    // And print the final ASCII bit.
    printf("  %s\n", buff);
}
