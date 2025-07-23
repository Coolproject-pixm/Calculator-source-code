# the calculator program source code
#### just run the commands, or just run the batch file!
```bash
windres resource.rc -o resource.o
```
# linking it as .exe file
```
gcc main.c resource.o -mwindows -lcomctl32 -o Calculator.exe
```
# end
- this is how it works!
- like I told you! it would create a Window application! and app.manifest makes the program work! and have styles and it wouldn't be outdated after all!
### just remember to run the command tools or the batch file like I told you!
