#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "mutate_block.h"
#include "process_file.h"

#define ONE_MIB (1024U * 1024U)

int process_file(FILE *input_file, FILE *output_file, off_t file_size, int percentage)
{
    size_t block_size;
    unsigned char *input_buffer;

    block_size = ((uint64_t)file_size <= ONE_MIB) ? (size_t)(file_size > 0 ? file_size : 1) : ONE_MIB;
    input_buffer = (unsigned char *)malloc(block_size);
    if (input_buffer == NULL)
    {
        fprintf(stderr, "Failed to allocate input buffer\n");
        return -1;
    }

    while (1)
    {
        size_t bytes_read;
        unsigned char *mutated = NULL;
        size_t mutated_size = 0;

        bytes_read = fread(input_buffer, 1, block_size, input_file);
        if (bytes_read == 0)
        {
            if (ferror(input_file))
            {
                fprintf(stderr, "Failed to read input file\n");
                free(input_buffer);
                return -1;
            }
            break;
        }

        if (mutate_block(input_buffer, bytes_read, percentage, &mutated, &mutated_size) != 0)
        {
            fprintf(stderr, "Failed to mutate block\n");
            free(input_buffer);
            return -1;
        }

        if (mutated_size > 0 && fwrite(mutated, 1, mutated_size, output_file) != mutated_size)
        {
            fprintf(stderr, "Failed to write output file\n");
            free(mutated);
            free(input_buffer);
            return -1;
        }

        free(mutated);
    }

    free(input_buffer);
    return 0;
}
