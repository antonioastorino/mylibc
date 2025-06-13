# mylibc

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
> NOTE: do NOT use this mode if your projects is using the `_TEST` and `_MEMORY_CHECK` flags to run unit tests
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
> NOTE: do NOT use this mode if your projects is using the `_TEST` and `_MEMORY_CHECK` flags to run unit tests
This will compile the project as an archive file with
- no entry point
- optimization level 3
- no unit test.
```bash
./tools/build-and-run.sh lib
```

### Module
> NOTE: use this mode if your projects is using the `_TEST` and `_MEMORY_CHECK` flags to run unit tests
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
