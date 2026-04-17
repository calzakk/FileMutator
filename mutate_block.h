#ifndef MUTATE_BLOCK_H
#define MUTATE_BLOCK_H

#include <stddef.h>

int mutate_block(const unsigned char *input,
                 size_t input_size,
                 int percentage,
                 unsigned char **out_data,
                 size_t *out_size);

#endif
