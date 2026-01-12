# nina-compiler
This is the official compiler for Waldbrand

Info/Warning: The Programming Language Waldbrand is still in Beta and only runnable on Windows, but I'm trying to bring it to Linux, macOS and the BSD-Distrories.

The requiments are:
- A running Windows Machine
- GCC installed via MinGW or MSYS2

1. Download the two .c files.
2. compile first nina_transpiliter.c with gcc nina_transpiler.c -o nina.exe
3. compile nina_gui.c with gcc nina_gui.c -o nina.exe -lcomdlg32 -mwindows
