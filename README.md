# FileMutator

`filemutator` reads an input file in blocks and writes a mutated output file with the suffix `.mutated`.

Note the source code within this repo was generated entirely by AI, following instructions provided by me.

## Requirements

- C compiler with C11 support
- CMake 3.16+

## Build

Debug build:

```bash
./build-debug.sh
```

Release build:

```bash
./build-release.sh
```

The executable will be generated in the selected build directory:

- `build/debug/filemutator`
- `build/release/filemutator`

## Usage

```bash
./filemutator <input-file> <percentage>
```

- `input-file`: path to the source file
- `percentage`: integer from 1 to 100

Output file path is always:

`<input-file>.mutated`

## Mutation Behavior

- Input is opened read-only, output is written to a separate file.
- If file size is 1 MiB or less, it is processed in-memory as one block.
- If file size is greater than 1 MiB, it is processed in 1 MiB blocks.
- For each block, `delta = ceil(P% of input block size)` is calculated.
- Output block size is constrained to stay within `input_size - delta` and `input_size + delta`.
- A single mutation loop randomly chooses insert/delete/modify operations:
  - insert: random contiguous bytes (1+), only if size stays <= `input_size + delta`
  - delete: random contiguous bytes (1+), only if size stays >= `input_size - delta`
  - modify: random contiguous bytes (1+) in-place
- Per block limits:
  - at most 10 insertion operations
  - at most 10 deletion operations
  - at most 10 modification operations
  - total operations are between 1 and 30
- After each iteration, the loop may exit probabilistically:
  - compute `total_ops = insert_ops + delete_ops + modify_ops`
  - draw a random value in `[0, 29]`
  - exit if the random value is less than `total_ops`

## MIT License

Copyright (c) 2026 Graham Bull

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
