#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <unistd.h>

#include "process_file.h"

static int parse_percentage(const char *text, int *out_percentage)
{
    char *end = NULL;
    long value = strtol(text, &end, 10);
    if (*text == '\0' || end == NULL || *end != '\0')
    {
        return -1;
    }
    if (value < 1 || value > 100)
    {
        return -1;
    }
    *out_percentage = (int)value;
    return 0;
}

int main(int argc, char **argv)
{
    const char *input_path;
    int percentage;
    FILE *input_file = NULL;
    FILE *output_file = NULL;
    struct stat st;
    off_t file_size;
    size_t output_path_len;
    char *output_path;
    int status = EXIT_FAILURE;

    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <file-pathname> <percentage 1-100>\n", argv[0]);
        return EXIT_FAILURE;
    }

    input_path = argv[1];
    if (parse_percentage(argv[2], &percentage) != 0)
    {
        fprintf(stderr, "Invalid percentage: must be an integer between 1 and 100\n");
        return EXIT_FAILURE;
    }

    if (stat(input_path, &st) != 0)
    {
        fprintf(stderr, "Failed to stat input file '%s': %s\n", input_path, strerror(errno));
        return EXIT_FAILURE;
    }
    file_size = st.st_size;

    output_path_len = strlen(input_path) + strlen(".mutated") + 1;
    output_path = (char *)malloc(output_path_len);
    if (output_path == NULL)
    {
        fprintf(stderr, "Failed to allocate output path\n");
        return EXIT_FAILURE;
    }
    snprintf(output_path, output_path_len, "%s.mutated", input_path);

    input_file = fopen(input_path, "rb");
    if (input_file == NULL)
    {
        fprintf(stderr, "Failed to open input file '%s': %s\n", input_path, strerror(errno));
        goto cleanup;
    }

    output_file = fopen(output_path, "wb");
    if (output_file == NULL)
    {
        fprintf(stderr, "Failed to open output file '%s': %s\n", output_path, strerror(errno));
        goto cleanup;
    }

    srand((unsigned int)(time(NULL) ^ (unsigned int)getpid()));

    if (process_file(input_file, output_file, file_size, percentage) != 0)
    {
        goto cleanup;
    }

    status = EXIT_SUCCESS;

cleanup:
    if (output_file != NULL)
    {
        fclose(output_file);
    }
    if (input_file != NULL)
    {
        fclose(input_file);
    }
    free(output_path);
    return status;
}
