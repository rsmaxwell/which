# which


## Setup

Install Visual Studio 2017 installed and add the tools to the environment 

``` batch
set MSVS=C:\Program Files (x86)\Microsoft Visual Studio\2017\Community
call "%MSVS%\VC\Auxiliary\Build\vcvarsx86_amd64"
``` 

## build

In the project directory, run the "build" command

``` batch
build 
``` 

The executable will be built as:  ".\target\dist\which.exe"

## usage

```
Usage: WHICH [-b] [-x <ext>] [-v] [-j] <filename.ext>
Where -b    Basic mode.  Only show the file path
              Recognised extensions:
                EXE, COM, BAT      - current dir then PATH
                LIB, OBJ, DEF      - LIB
                HPP, H++, HXX, H   - INCLUDE
                SKE                - ROOTDIR
                DLL                - LoadLibrary then GetModuleFileName
                CLASS              - CLASSPATH
      -x <ext>  Force the extension to <ext>
      -j        Search for file on JAVA_LIB_PATH and attempt to load it
      -v        verbose
```
