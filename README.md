# the calculator program source code
# just run the commands, or just run the batch file!
```bash
windres resource.rc -o resource.o
```bash

```bash
gcc main.c resource.o -mwindows -lcomctl32 -o Calculator.exe
# that's all of steps you need!
