#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "mutate_block.h"

#define MAX_OPS_PER_TYPE 10
#define MAX_TOTAL_OPS 30

typedef struct
{
    unsigned char *data;
    unsigned char *inserted;
    size_t size;
    size_t capacity;
} MutableBlock;

static size_t random_between(size_t min_inclusive, size_t max_inclusive)
{
    if (max_inclusive <= min_inclusive)
    {
        return min_inclusive;
    }
    return min_inclusive + (size_t)(rand() % (int)(max_inclusive - min_inclusive + 1));
}

static size_t ceil_percent(size_t value, int percentage)
{
    uint64_t numerator = (uint64_t)value * (uint64_t)percentage + 99U;
    size_t result = (size_t)(numerator / 100U);
    if (result == 0U && value > 0U)
    {
        return 1U;
    }
    return result;
}

static int init_block(MutableBlock *block,
                      const unsigned char *input,
                      size_t input_size,
                      size_t capacity)
{
    if (capacity < input_size)
    {
        return -1;
    }

    block->data = (unsigned char *)malloc(capacity > 0 ? capacity : 1);
    block->inserted = (unsigned char *)malloc(capacity > 0 ? capacity : 1);
    if (block->data == NULL || block->inserted == NULL)
    {
        free(block->data);
        free(block->inserted);
        block->data = NULL;
        block->inserted = NULL;
        return -1;
    }

    if (input_size > 0)
    {
        memcpy(block->data, input, input_size);
    }
    memset(block->inserted, 0, input_size);
    block->size = input_size;
    block->capacity = capacity;
    return 0;
}

static void free_block(MutableBlock *block)
{
    free(block->data);
    free(block->inserted);
    block->data = NULL;
    block->inserted = NULL;
    block->size = 0;
    block->capacity = 0;
}

static int insert_random_bytes(MutableBlock *block, size_t position, size_t length)
{
    size_t tail_size;
    size_t i;

    if (length == 0 || position > block->size)
    {
        return -1;
    }
    if (block->size + length > block->capacity)
    {
        return -1;
    }

    tail_size = block->size - position;
    if (tail_size > 0)
    {
        memmove(block->data + position + length, block->data + position, tail_size);
        memmove(block->inserted + position + length, block->inserted + position, tail_size);
    }

    for (i = 0; i < length; ++i)
    {
        block->data[position + i] = (unsigned char)(rand() % 256);
        block->inserted[position + i] = 1;
    }
    block->size += length;
    return 0;
}

static int delete_bytes(MutableBlock *block, size_t position, size_t length)
{
    size_t tail_start;
    size_t tail_size;

    if (length == 0 || position >= block->size || position + length > block->size)
    {
        return -1;
    }

    tail_start = position + length;
    tail_size = block->size - tail_start;
    if (tail_size > 0)
    {
        memmove(block->data + position, block->data + tail_start, tail_size);
        memmove(block->inserted + position, block->inserted + tail_start, tail_size);
    }
    block->size -= length;
    return 0;
}

static int modify_bytes(MutableBlock *block, size_t position, size_t length)
{
    size_t i;

    if (length == 0 || position >= block->size || position + length > block->size)
    {
        return -1;
    }

    for (i = 0; i < length; ++i)
    {
        block->data[position + i] = (unsigned char)(rand() % 256);
    }
    return 0;
}

int mutate_block(const unsigned char *input,
                 size_t input_size,
                 int percentage,
                 unsigned char **out_data,
                 size_t *out_size)
{
    MutableBlock block;
    size_t delta;
    size_t min_size_allowed;
    size_t max_size_allowed;
    size_t capacity;
    int insert_ops = 0;
    int delete_ops = 0;
    int modify_ops = 0;

    if (input_size == 0)
    {
        *out_data = NULL;
        *out_size = 0;
        return 0;
    }

    delta = ceil_percent(input_size, percentage);
    if (delta > input_size)
    {
        delta = input_size;
    }

    min_size_allowed = input_size - delta;
    max_size_allowed = input_size + delta;
    capacity = input_size + delta;

    if (init_block(&block, input, input_size, capacity) != 0)
    {
        return -1;
    }

    while (1)
    {
        int can_insert = (insert_ops < MAX_OPS_PER_TYPE) && (block.size < max_size_allowed);
        int can_delete = (delete_ops < MAX_OPS_PER_TYPE) && (block.size > min_size_allowed);
        int can_modify = (modify_ops < MAX_OPS_PER_TYPE) && (block.size > 0);
        int options_count = can_insert + can_delete + can_modify;
        int pick;
        int total_ops;

        if (options_count == 0)
        {
            break;
        }

        pick = (int)random_between(1, (size_t)options_count);

        if (can_insert)
        {
            if (pick == 1)
            {
                size_t max_insert_len = max_size_allowed - block.size;
                size_t insert_len = random_between(1, max_insert_len);
                size_t position = random_between(0, block.size);

                if (insert_random_bytes(&block, position, insert_len) != 0)
                {
                    free_block(&block);
                    return -1;
                }
                ++insert_ops;
            }
            --pick;
        }
        if (can_delete)
        {
            if (pick == 1)
            {
                size_t max_delete_len = block.size - min_size_allowed;
                size_t delete_len = random_between(1, max_delete_len);
                size_t max_start = block.size - delete_len;
                size_t position = random_between(0, max_start);

                if (delete_bytes(&block, position, delete_len) != 0)
                {
                    free_block(&block);
                    return -1;
                }
                ++delete_ops;
            }
            --pick;
        }
        if (can_modify && pick == 1)
        {
            size_t position = random_between(0, block.size - 1);
            size_t max_modify_len = block.size - position;
            size_t modify_len = random_between(1, max_modify_len);

            if (modify_bytes(&block, position, modify_len) != 0)
            {
                free_block(&block);
                return -1;
            }
            ++modify_ops;
        }

        total_ops = insert_ops + delete_ops + modify_ops;
        if (total_ops >= MAX_TOTAL_OPS)
        {
            break;
        }
        if ((rand() % MAX_TOTAL_OPS) < total_ops)
        {
            break;
        }
    }

    if (insert_ops + delete_ops + modify_ops < 1 || insert_ops + delete_ops + modify_ops > MAX_TOTAL_OPS)
    {
        free_block(&block);
        return -1;
    }
    if (block.size < min_size_allowed || block.size > max_size_allowed)
    {
        free_block(&block);
        return -1;
    }

    *out_data = (unsigned char *)malloc(block.size > 0 ? block.size : 1);
    if (*out_data == NULL)
    {
        free_block(&block);
        return -1;
    }
    if (block.size > 0)
    {
        memcpy(*out_data, block.data, block.size);
    }
    *out_size = block.size;

    free_block(&block);
    return 0;
}
