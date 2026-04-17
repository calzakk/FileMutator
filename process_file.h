#ifndef PROCESS_FILE_H
#define PROCESS_FILE_H

#include <stdio.h>
#include <sys/types.h>

int process_file(FILE *input_file, FILE *output_file, off_t file_size, int percentage);

#endif
