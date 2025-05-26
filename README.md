# mylibc

## Test and build

### Run unit tests
```bash
./tools/build-and-run.sh test
```

### Debug 
```bash
./tools/build-and-run.sh debug 
```

### Release
```bash
./tools/build-and-run.sh release 
```

## Add to your project

### Add as a static library
- Compile in release mode as described above.
- Copy the contents of the `dist` folder to your project directory, say `mylibc-folder`.
- Add `mylibc.h` to your projects.
- Add `mylibc-folder` to the library search path a and link the library itself when compiling. For example:

```bash
clang main.c -Lmylibc-folder -lmylibc -o my_program"
./my_program
```

### Add the source code
All the source code is contained in the `src` folder, including the single header file `mylibc.h`.
