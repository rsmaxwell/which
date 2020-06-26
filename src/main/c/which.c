#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <time.h>
#include <limits.h>
#include <errno.h>
#include <io.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <windows.h>

#define ARRAY_SIZE(a)        sizeof(a) / sizeof(a[0])
#define zPATH_MAX            MAX_PATH
#define zSTAT_DEFAULT      {0,0,0,0,0,0,0,0,0,0,0}

#define LOCATE_VERSION          "3.0"

#define OPTION_VERBOSE       1
#define OPTION_SHOW_DATE     2
#define OPTION_SHOW_SIZE     4
#define OPTION_JAVA          8

#define zok                         0
#define zERROR_USAGE                1
#define zERROR_UNEXPECTED_ERROR     2
#define zERROR_PATH_NOT_FOUND       3
#define zERROR_FILE_NOT_FOUND       4
#define zERROR_MOD_NOT_FOUND        5
#define zERROR_INVALID_ACCESS       6
#define zERROR_BAD_EXE_FORMAT       7
#define zERROR_ACCESS_DENIED        8
#define zERROR_INTERNAL_ERROR       9
#define zERROR_INVALID_PARAMETER   10

/*********************************************************************/
/* Case insensitive string compare                                   */
/*********************************************************************/
int stricmp(const char *a, const char *b)
{
    return _stricmp(a, b);
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/
int zMalloc(int size, void * pointer) {
    * (void**) pointer = calloc(1, size);
    return zok;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/
int zFree(void * pointer) {
    free(* (void**) pointer);
    return zok;
}

/*********************************************************************/
/* Convert a system error to a return code                           */
/*********************************************************************/
int zMapRetcode(DWORD error) {
    return (int) error;
}

/*********************************************************************/
/*                                                                   */
/*********************************************************************/
char * zstrRC(int rc) {

    char *p = NULL;

    switch (rc) {
        case zok :                            p = "zok";                       break;
        case zERROR_USAGE :                   p = "zERROR_USAGE";              break;
        case zERROR_UNEXPECTED_ERROR :        p = "zERROR_UNEXPECTED_ERROR";   break;
        case zERROR_PATH_NOT_FOUND :          p = "zERROR_PATH_NOT_FOUND";     break;
        case zERROR_FILE_NOT_FOUND :          p = "zERROR_FILE_NOT_FOUND";     break;
        case zERROR_MOD_NOT_FOUND :           p = "zERROR_MOD_NOT_FOUND";      break;
        case zERROR_INVALID_ACCESS :          p = "zERROR_INVALID_ACCESS";     break;
        case zERROR_BAD_EXE_FORMAT :          p = "zERROR_BAD_EXE_FORMAT";     break;
        case zERROR_ACCESS_DENIED :           p = "zERROR_ACCESS_DENIED";      break;
        case zERROR_INTERNAL_ERROR :          p = "zERROR_INTERNAL_ERROR";     break;
        case zERROR_INVALID_PARAMETER :       p = "zERROR_INVALID_PARAMETER";  break;
        default:                              p = "?";                         break;
    }

    return p;
}

typedef HMODULE               zMODULE;
#define zMODULE_NULL    0

/*********************************************************************/
/* Load a module                                                     */
/*********************************************************************/
int zLoadModule( char     * errorBuffer
               , int        errorSize
               , char     * name
               , zMODULE  * module
               ) {
    int rc = zok;
    DWORD error = 0;

    *errorBuffer = '\0';
    *module = LoadLibrary(name);

    if (*module == NULL) {
        error = GetLastError();
        rc = zMapRetcode(error);
    }

    return rc;
}

/*********************************************************************/
/* Return the fully qualified path name associated with a module     */
/*********************************************************************/
int zQueryModuleName( zMODULE    module
                    , unsigned   size
                    , char     * name
                    ) {
    int rc = zok;
    DWORD result = 0;
    DWORD error = 0;

    result = GetModuleFileName(module, name, size);

    if (result == 0) {
        error = GetLastError();
        rc = zMapRetcode(error);
    }

    return rc;
}


/*********************************************************************/
/* Free a module                                                     */
/*********************************************************************/
int zFreeModule(zMODULE module) {
    int rc = zok;
    BOOL result = TRUE;
    DWORD error = 0;

    result = FreeLibrary( module );

    if (result == FALSE) {
        error = GetLastError();
        rc = zMapRetcode(error);
    }

    return rc;
}

/*********************************************************************/
/* Help                                                              */
/*********************************************************************/

int help ( char *version ) {
    int rc = zok;

    printf("WHICH version %s date %s by Richard Maxwell\n", version, __DATE__ );
    printf("Usage: WHICH [-b] [-x <ext>] [-v] [-j] <filename.ext>\n" );
    printf("Where -b    Basic mode.  Only show the file path\n" );
    printf("              Recognised extensions:\n" );
    printf("                EXE, COM, BAT      - current dir then PATH\n" );
    printf("                LIB, OBJ, DEF      - LIB\n" );
    printf("                HPP, H++, HXX, H   - INCLUDE\n" );
    printf("                SKE                - ROOTDIR\n" );
    printf("                DLL                - LoadLibrary then GetModuleFileName\n" );
    printf("                CLASS              - CLASSPATH\n" );
    printf("      -x <ext>  Force the extension to <ext>\n" );
    printf("      -j        Search for file on JAVA_LIB_PATH and attempt to load it\n" );
    printf("      -v        verbose\n" );

    return rc;
}

/*********************************************************************/
/* Determines whether the specified file exists and can be accessed  */
/* as specified by the value of mode.  When used with directories,   */
/* determines only whether the specified directory exists.           */
/*                                                                   */
/* In Windows NT, all directories have read and write access.        */
/*                                                                   */
/*  Mode   Meaning                                                   */
/*   00    Check only for the existence of the file.                 */
/*   02    Check for permission to write to the file.                */
/*   04    Check for permission to read from the file.               */
/*   06    Check for permission to read from and write to the file   */
/*********************************************************************/
int zAccess( char *pathname, int mode ) {
    int rc = zok;

    /*---------------------------------------------------------------*/
    /* Check the mode parameter                                      */
    /*---------------------------------------------------------------*/
    switch (mode) {
        case 0:
        case 2:
        case 4:
        case 6:
            break;

        default:
            rc = zERROR_INVALID_PARAMETER;
            break;
    }

    /*---------------------------------------------------------------*/
    /* Check the access to the file                                  */
    /*---------------------------------------------------------------*/
    if (rc == zok) {
        int result = _access( pathname, mode );

        if (result != 0) {
            int localErrno = errno;

            switch (localErrno) {
                case EACCES:    rc = zERROR_ACCESS_DENIED;     break;
                case ENOENT:    rc = zERROR_PATH_NOT_FOUND;    break;
                default:        rc = zERROR_INTERNAL_ERROR;    break;
            }
        }
    }

    return rc;
}

/*********************************************************************/
/* Fix the case of a pathname                                        */
/*********************************************************************/
int fixPathnameCase ( char *pathname ) {
    return zok;
}


/*********************************************************************/
/* Display a hit                                                     */
/*********************************************************************/
int displayHit(int options, char *pathname) {
    int rc = zok;
    struct stat buffer = zSTAT_DEFAULT;
    int result = 0;
    char destination[100] = "";
    struct tm *filetime = NULL;

    /*****************************************************************/
    /* Fix the case of the filename                                  */
    /*****************************************************************/
    rc = fixPathnameCase(pathname);

    if (rc == zok) {
        /*************************************************************/
        /* If necessary get file information                         */
        /*************************************************************/
        if ((options & OPTION_SHOW_DATE) || (options & OPTION_SHOW_SIZE)) {
            result = stat(pathname, &buffer);

            if (result != 0) {
                rc = zERROR_UNEXPECTED_ERROR;
                printf( "%s[%ld] result:%d errno:%ld\n", __FILE__, __LINE__, result, errno );
            }
        }

        /*************************************************************/
        /* Display the file date and time                            */
        /*************************************************************/
        if (options & OPTION_SHOW_DATE) {
            filetime = localtime(&buffer.st_mtime);

            strftime(destination, sizeof(destination), "%d/%m/%Y  %H:%M", filetime);

            printf("%s", destination);
            printf("    ");
        }

        /*************************************************************/
        /* Display the file size                                     */
        /*************************************************************/
        if (options & OPTION_SHOW_SIZE) {
            printf("%14ld", buffer.st_size);
            printf(" ");
        }

        /*************************************************************/
        /* Display the file pathname                                 */
        /*************************************************************/
        printf("%s\n", pathname);
    }

    return rc;
}

/*********************************************************************/
/* Search for the file in a path string                              */
/*********************************************************************/
int searchPathString2( int options, char *filename, char *pathstring, char *pathname ) {
    int rc = zERROR_PATH_NOT_FOUND;
    char *p = NULL;
    char *q = pathstring;

    /*****************************************************************/
    /* Search for the file                                           */
    /*****************************************************************/
    while (q && *q && ((rc == zERROR_PATH_NOT_FOUND) || (rc == zERROR_ACCESS_DENIED))) {
        p = pathname;

        /*-----------------------------------------------------------*/
        /* Skip over semi-colons                                     */
        /*-----------------------------------------------------------*/
        while (*q && (*q == ';')) {
            q++;
        }

        /*-----------------------------------------------------------*/
        /* Get the next path of the path                             */
        /*-----------------------------------------------------------*/
        while (*q && (*q != ';')) {
            *p++ = *q++;
        }

        sprintf( p, "\\%s", filename );

        /*-----------------------------------------------------------*/
        /* Look for the file                                         */
        /*-----------------------------------------------------------*/
        rc = zAccess(pathname, 0);
    }

    return rc;
}

/*********************************************************************/
/* Search for the file in a path string                              */
/*********************************************************************/
int searchPathString( int options, char *filename, char *pathstring ) {
    int rc = zok;
    char pathname[zPATH_MAX] = "";

    /*****************************************************************/
    /* Search for the file                                           */
    /*****************************************************************/
    rc = searchPathString2(options, filename, pathstring, pathname);

    /*****************************************************************/
    /* Display the results                                           */
    /*****************************************************************/
    switch (rc) {
        case zok:
            rc = displayHit(options, pathname);
            break;

        case zERROR_PATH_NOT_FOUND:
        case zERROR_ACCESS_DENIED:
            if (options & OPTION_VERBOSE) {
                printf( "File not found\n" );
            }  
            break;

        default:
            printf( "%s[%d] Unexpected error, %s\n", __FILE__, __LINE__, zstrRC(rc));
            break;
    }

    return rc;
}

/*********************************************************************/
/* Search for the file in a list of environment vatiables            */
/*********************************************************************/
int searchEnvironmentPath(int options, char *filename, int number, ...) {
    int rc = zERROR_PATH_NOT_FOUND;
    char *environmentVariable = NULL;
    char pathname[zPATH_MAX] = "";
    int i = 0;
    va_list arguments = NULL;
    va_start(arguments, number);

    /*****************************************************************/
    /* Search for the file                                           */
    /*****************************************************************/
    for (i = 0; (rc == zERROR_PATH_NOT_FOUND) && (i < number); i++) {
        environmentVariable = va_arg( arguments, char* );
        rc = searchPathString2(options, filename, getenv(environmentVariable), pathname);
    }

    /*****************************************************************/
    /* Display the results                                           */
    /*****************************************************************/
    switch (rc) {
        case zok:
            rc = displayHit(options, pathname);
            break;

        case zERROR_PATH_NOT_FOUND:
        case zERROR_ACCESS_DENIED:
            if (options & OPTION_VERBOSE) {
                printf( "File not found\n" );
            }
            break;

        default:
            printf( "%s[%d] Unexpected error, %s\n", __FILE__, __LINE__, zstrRC(rc));
            break;
    }

    va_end(arguments);

    return rc;
}

/*********************************************************************/
/* Search for the locale file                                        */
/*********************************************************************/
int searchLocalPath(int options, char *filename) {
    int rc = zok;
    int rc2 = zok;
    char *pathstring = NULL;
    char *locpath = getenv("LOCPATH");
    char *lang    = getenv("LANG");
    char *p = NULL;
    char *q = NULL;

    /*****************************************************************/
    /* Get local storage                                             */
    /*****************************************************************/
    rc = zMalloc(10000, &pathstring);

    /*****************************************************************/
    /* Add the LOCPATH to the LANG path                              */
    /*****************************************************************/
    if (rc == zok) {
        p = pathstring;
        q = locpath;

        while (q && *q) {
            while (q && *q && (*q != ';'))
            { *p++ = *q++; }

            if (*q == ';') {
                *q++;
                p += sprintf( p, "\\%s;", lang);
            }
        }
    }

    /*****************************************************************/
    /* Search for the file                                           */
    /*****************************************************************/
    if (rc == zok)   rc = searchPathString(options,  filename, pathstring);

    /*****************************************************************/
    /* Housekeeping                                                  */
    /*****************************************************************/
    if (pathstring) {
        rc2 = zFree(&pathstring);
        if (rc == zok)    rc = rc2;
    }

    return rc;
}

/*********************************************************************/
/* Search for the nlspath                                            */
/*********************************************************************/
int searchNLSPath(int options, char *filename) {
    int rc = zok;
    int rc2 = zok;
    char *pathstring = NULL;
    char *nlspath = getenv("NLSPATH");
    char *lang    = getenv("LANG");
    char *p = NULL;
    char *q = NULL;

    /*****************************************************************/
    /* Get local storage                                             */
    /*****************************************************************/
    rc = zMalloc(10000, &pathstring);

    /*****************************************************************/
    /* Add the NLSPATH to the LANG path                              */
    /*****************************************************************/
    if (rc == zok) {
        p = pathstring;
        q = nlspath;

        while (q && *q) {
            while (q && *q && (*q != '%'))
            { *p++ = *q++; }

            if (*q == '%') {
                *q++;

                if (toupper(*q) == 'L') {
                    p += sprintf( p, "%s;", lang );
                }
                else if (toupper(*q) != 'N') {
                    *p++ = *q;
                }

                *q++;
            }
        }
    }

    /*****************************************************************/
    /* Search for the file                                           */
    /*****************************************************************/
    if (rc == zok)   rc = searchPathString(options, filename, pathstring);

    /*****************************************************************/
    /* Housekeeping                                                  */
    /*****************************************************************/
    if (pathstring) {
        rc2 = zFree(&pathstring);
        if (rc == zok)    rc = rc2;
    }

    return rc;
}

/*********************************************************************/
/* Search for the file in the current directory then in the path     */
/* pointed to by the PATH environment variable                       */
/*********************************************************************/
int searchPath(int options, char *filename) {
    int rc = zok;
    char path[zPATH_MAX] = "";

    /*****************************************************************/
    /* Search for the file                                           */
    /*****************************************************************/
    _searchenv( filename, "PATH", path);

    /*****************************************************************/
    /* Display the results                                           */
    /*****************************************************************/
    if (*path == 0) {
        if (options & OPTION_VERBOSE) {
            printf("File not found\n");
        }    
        rc = zERROR_PATH_NOT_FOUND;
    }    
    else
        rc = displayHit(options, path);

    return rc;
}

/*********************************************************************/
/* Search for a DLL by loading it and querying the module name       */
/*********************************************************************/
int searchForDLL(int options, char *filename) {
    int rc = zok;
    int rc2 = zok;
    char buffer[300] = "";
    char name[300] = "";
    zMODULE hmod = zMODULE_NULL;

    /*****************************************************************/
    /* Get the name of the module                                    */
    /*****************************************************************/
    _splitpath ( filename, NULL, NULL, name, NULL );

    /*****************************************************************/
    /* Load the module                                               */
    /*****************************************************************/
    if (rc == zok) {
        rc = zLoadModule( buffer, sizeof(buffer), name, &hmod );

        switch (rc) {
            case zok:
            case zERROR_BAD_EXE_FORMAT:   /* OS/2 */
            case zERROR_FILE_NOT_FOUND:   /* OS/2 */
            case zERROR_MOD_NOT_FOUND:    /* NT   */
                if (options & OPTION_VERBOSE) {
                    printf( "File not found\n" );
                }    
                break;
            default: {
                printf( "%s[%d] zLoadModule --> %s\n", __FILE__, __LINE__, zstrRC(rc) );
                printf( "buffer:\"%s\"\n", buffer );
            }
        }
    }

    /*****************************************************************/
    /* Query the module filename                                     */
    /*****************************************************************/
    if (rc == zok) {
        rc = zQueryModuleName(hmod, sizeof(buffer), buffer);
        if (rc != zok)   printf( "%s[%d] zQueryModuleName --> %s\n", __FILE__, __LINE__, zstrRC(rc) );
    }

    /*****************************************************************/
    /* Free the module handle                                        */
    /*****************************************************************/
    if (rc == zok) {
        rc2 = zFreeModule(hmod);

        switch (rc2) {
            case zok:
            case zERROR_INVALID_ACCESS:
                break;
            default:
                rc = rc2;
                printf("%s[%d] zFreeModule --> %s\n", __FILE__, __LINE__, zstrRC(rc));
        }
    }

    /*****************************************************************/
    /* Display the results                                           */
    /*****************************************************************/
    if (rc == zok)   rc = displayHit(options, buffer);

    return rc;
}

/*********************************************************************/
/* Search for a library using on a path then attempt to load it      */
/*********************************************************************/
int searchAndLoadLibrary(int options, char *filename, int number, ...) {
    int rc = zok;
    int rc2 = zok;
    char *environmentVariable = NULL;
    char pathname[zPATH_MAX] = "";
    int i = 0;
    zMODULE hmod = zMODULE_NULL;
    va_list arguments = NULL;
    va_start(arguments, number);

    /*****************************************************************/
    /* Search for the file                                           */
    /*****************************************************************/
    rc = zERROR_PATH_NOT_FOUND;
    for (i = 0; (rc == zERROR_PATH_NOT_FOUND) && (i < number); i++) {
        environmentVariable = va_arg( arguments, char* );
        rc = searchPathString2(options, filename, getenv(environmentVariable), pathname);
    }

    /*****************************************************************/
    /* Display the results                                           */
    /*****************************************************************/
    switch (rc) {
        case zok:
            rc = displayHit(options, pathname);
            break;

        case zERROR_PATH_NOT_FOUND:
        case zERROR_ACCESS_DENIED:
            if (options & OPTION_VERBOSE) {
                printf( "File not found\n" );
            }
            break;

        default:
            printf( "%s[%d] Unexpected error, %s\n", __FILE__, __LINE__, zstrRC(rc));
            break;
    }

    va_end(arguments);

    /*****************************************************************/
    /* Attempt to load the library                                   */
    /*****************************************************************/
    if (rc == zok) {
        char buffer[300] = "";

        rc = zLoadModule(buffer, sizeof(buffer), pathname, &hmod);

        if (rc == zok) {
            printf("File was loaded successfully\n");
        }
        else {
            printf("File could not be loaded. rc = %d\n", rc);
            printf("buffer:\"%s\"\n", buffer);
        }
    }

    /*****************************************************************/
    /* Free the module handle                                        */
    /*****************************************************************/
    if (rc == zok) {
        rc2 = zFreeModule(hmod);

        switch (rc2) {
            case zok:
            case zERROR_INVALID_ACCESS:
                break;
            default:
                rc = rc2;
                printf("%s[%d] zFreeModule --> %s\n", __FILE__, __LINE__, zstrRC(rc));
        }
    }

    return rc;
}

/*********************************************************************/
/* Switch on the extension to find the file                          */
/*********************************************************************/
int switch_on_extension(int options, char *filename, char *extension) {
    int rc = zok;

    if (options & OPTION_JAVA) {
        rc = searchAndLoadLibrary(options, filename, 1, "JAVA_LIB_PATH");
    }
    else if (  !stricmp(extension, "EXE")
            || !stricmp(extension, "COM")
            || !stricmp(extension, "BAT")
			|| !stricmp(extension, "CMD")
            ) {
        rc = searchPath(options, filename);
    }
    else if (  !stricmp(extension, "LIB")
            || !stricmp(extension, "OBJ")
            || !stricmp(extension, "DEF")
            ) {
        rc = searchEnvironmentPath(options, filename, 1, "LIB");
    }
    else if (  !stricmp(extension, "HPP")
            || !stricmp(extension, "H++")
            || !stricmp(extension, "HXX")
            || !stricmp(extension, "H")
            ) {
        rc = searchEnvironmentPath(options, filename, 1, "INCLUDE");
    }
    else if (!stricmp(extension, "SKE")) {
        rc = searchEnvironmentPath(options, filename, 1, "ROOTDIR");
    }
    else if (!stricmp(extension, "DLL")) {
        rc = searchForDLL(options, filename);
    }
    else if (!stricmp(extension, "CLASS")) {
        rc = searchEnvironmentPath(options, filename, 1, "CLASSPATH");
    }
    else {
        printf( "Do not know how to search for files with extension \"%s\"\n", extension );
        rc = zERROR_USAGE;
    }

    return rc;
}

/*********************************************************************/
/* The main routine                                                  */
/*********************************************************************/
int main(int argc, char * argv[]) {
    int rc  = zok;
    int rc2 = zok;
    int option = 0;
    int index = 0;
    char *filename  = NULL;
    char *extension = NULL;
    int options = OPTION_SHOW_DATE | OPTION_SHOW_SIZE;
    int version  = 0;
    int required = 2;
                             
    /*****************************************************************/
    /* Get the command line options                                  */
    /*****************************************************************/
    index = 1;
    while ((rc == zok) && (index < argc)) {
        char * arg = argv[index];

        if (arg[0] == '-') {
            char option = arg[1];

            switch (option) {
              case 'b':
                    options &= ~(OPTION_SHOW_DATE | OPTION_SHOW_SIZE);
                    break;
              case 'h':
                    rc = zERROR_USAGE;
                    break;
              case 'v':
                    options |= OPTION_VERBOSE;
                    break;
              case 'j':
                    options |= OPTION_JAVA;
                    break;
              case 'x':
                    if (index < argc) {
                        index++;
                        extension = argv[index];
                    }
                    else {
                        printf("Missing value for option %s\n", arg);
                        rc = zERROR_UNEXPECTED_ERROR;
                    }
                    break;
              default:
                    printf("Syntax Error, option = %s\n", arg);
                    rc = zERROR_UNEXPECTED_ERROR;
                    break;
            }
        }
        else if (filename == NULL)
            filename = arg;
        else {
            printf("Unexpected argument %s\n", arg);
            rc = zERROR_USAGE;
        }
        index++;
    }

    /*****************************************************************/
    /* If necessary display usage help                               */
    /*****************************************************************/
    if (rc == zERROR_USAGE)     rc2 = help ( LOCATE_VERSION );

    /*****************************************************************/
    /* Get the extension                                             */
    /*****************************************************************/
    if (extension == NULL) {
        for (index = 0; (rc == zok) && (index < strlen(filename)); index++) {
            if (filename[index] == '.') {
                extension = &filename[index + 1];
            }
        }
    }

    /*****************************************************************/
    /* Display the filename                                          */
    /*****************************************************************/
    if (rc == zok) {
        if (extension)
            rc = switch_on_extension(options, filename, extension );
        else {
            printf( "The filename \"%s\" does not have an extension\n", filename );
            rc = zERROR_USAGE;
        }    
    }

    return rc;
}


