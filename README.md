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
This will compile the project as an object file with
- no entry point
- optimization level 3
- no unit test.
```bash
./tools/build-and-run.sh release 
```

### Static library 
This will compile the project as an archive file with
- no entry point
- optimization level 3
- no unit test.
```bash
./tools/build-and-run.sh lib
```

## Add to your project

### Add as a static library
- Compile in RELEASE or LIB mode as described above
- Follow the instructions printed after successful compilation

### Add the source code
All the source code is contained in the `src` folder, including the only header file `mylibc.h`.
