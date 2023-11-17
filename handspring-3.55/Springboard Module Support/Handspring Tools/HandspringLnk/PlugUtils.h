
#ifndef _PLUGUTILS_H
#define _PLUGUTILS_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdarg.h>
#include <extras.h>

#ifdef __cplusplus
}
#endif

/* compiler headers */
#include "DropInCompilerLinker.h"
#include "CompilerMapping.h"
#include "CWPluginErrors.h"
#include "CWRuntimeFeatures.h"

#include "MWCom.h"

#if CW_HOSTOS == CW_MSWIN
#define kDirectoryDelimiter			'\\'
#elif CW_HOSTOS == CW_MACOS
#define kDirectoryDelimiter			':'
#else
#define kDirectoryDelimiter			'/'
#endif


//declarations
int makerom_main(int argc, char* argv[]);

int nbtoken(char*);
void str2tab(char*, char**);

void DisplayError(char*);

char *strdup(const char *);
int strnicmp(const char *, const char *, int);
int stricmp(const char *, const char *);

char* fSpecToChar(CWFileSpec fSpec);

char* ExtractPath(char*);
char* ExtractFile(char*);
char* ExtractFileUp(char*);
char* ExtractFileDown(char*);
char* GetPrcOutputName(CWPluginContext);
int SetToFullPath(char**, int, char*, char*);

void FreeCharMatrix( char** , int );

HRESULT CreateRomResources(char* , char* , int , char* );
HRESULT SplitFile( char* , char* , char* , int , int* );
HRESULT CreateRomFiles( char* , char* , char* , int);

#ifdef macintosh
OSErr FSpPathFromLocation(FSSpec*, int*, Handle*);
#endif

//global vars
#define MYSTRLENGTH 255
static char mystr[MYSTRLENGTH];

#endif