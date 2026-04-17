cmake_minimum_required(VERSION 3.16)
project(filemutator C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

add_executable(filemutator
    filemutator.c
    process_file.c
    mutate_block.c
)

if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    target_compile_options(filemutator PRIVATE -Wall -Wextra -Wpedantic)
endif()
