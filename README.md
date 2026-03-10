# mylibc

This project contains useful utilities for generic C projects:
- Class-like string utilities
- Class-like string array utilities (e.g., useful to split strings)
- Class-like JSON deserializer utilities
- number parser
- TCP utilities
- HashMap utilities

The unit test environment embeds a memory-leak checker, making use of `my_memory.c` functions. Those tests are also useful examples on how to use the various utilities.

## Test and build

### Run unit tests
This will compile and run the unit test.
```bash
./tools/build-and-run.sh test
```

### Debug 
This will compile the unit test and start the debugger (lldb or gdb).
```bash
./tools/build-and-run.sh debug 
```

### Release
> NOTE: do NOT use this mode if your project is using the `_TEST` and `_MEMORY_CHECK` compiler flags


This will compile the project as an object file with
- no entry point
- optimization level 3
- no unit test.
```bash
./tools/build-and-run.sh release 
```
Compile your project using, for example
```
clang src/<your-main-file>.c \
    mylibc/dist/mylibc.o \
    -Imylibc/dist \
    -Wall -Wextra '-std=c2x' -pedantic '-fsanitize=address' -O3 \
    -o <your-executable-name>
```


### Static library 
> NOTE: do NOT use this mode if your project is using the `_TEST` and `_MEMORY_CHECK` compiler flags


This will compile the project as an archive file with
- no entry point
- optimization level 3
- no unit test.
```bash
./tools/build-and-run.sh lib
```

### Module
> NOTE: use this mode if your project is using the `_TEST` and `_MEMORY_CHECK` compiler flags


This will compile the project as an object file with
- no entry point
- optimization level 0
- unit test present
```bash
./tools/build-and-run.sh module
```
Compile your project as follows.
```
clang src/<your-main-file>.c \
    mylibc/dist/mylibc.o \
    -Imylibc/dist \
    -Wall -Wextra '-std=c2x' -pedantic '-fsanitize=address' -O0 \
    -g -D_TEST -D_MEMORY_CHECK \
    -o <your-executable-name>

## Add to your project

### Add as a static library
- Compile in RELEASE or LIB mode as described above
- Follow the instructions printed after successful compilation

### Add the source code
All the source code is contained in the `src` folder, including the only header file `mylibc.h`.
