
#include "PlugUtils.h"

#ifdef __cplusplus
extern "C" {
#endif

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#ifdef __cplusplus
}
#endif

// displays an error message in the codewarrior window
void DisplayError(char* mystr) 
{
	extern CWPluginContext myGlobalContext;
	CWMessageRef* pRef = (CWMessageRef*)NULL;
	const char* line1 = mystr;
	const char* line2 = (const char*)NULL;
	CWReportMessage(myGlobalContext,pRef,line1,line2,messagetypeError,0);
}

// counts the number of tokens of a string
int nbtoken(char* cmd)
{
	char* cmdp = cmd;
	char* cmds = cmd;
	int i = 0;
	
	const int INIT      = 0;
	const int READQUOTE = 1;
	const int READSPACE = 2;
	const int WRITE     = 3;
	int state = READSPACE;

	while (1) {

	switch ( state )
	{
	case INIT :	
		    if ( (cmdp[0]=='"') ) { state = READQUOTE; };
			if ( (cmdp[0]==' ') ) { state = READSPACE; };
			cmdp++;
			if ( (cmdp[0]=='"') ) { state = READQUOTE; cmds++; cmdp++; };
			break;
	case READQUOTE :
			if ( (cmdp[0]=='"') ) { state = WRITE; };
			cmdp++;
			break;
	case READSPACE : 
			if ( (cmdp[0]==' ') ) { state = WRITE; };
			cmdp++;
			break;
	case WRITE : 
			i++;
			cmds=cmdp;
			cmdp--;
			if ( (cmdp[0]=='"') ) { cmds++; cmdp++; };
			state = INIT;
			break;
	default : break;
	}
	
	if ( cmdp[0]=='\0' ) { 
		i++;
		break;
	}

	}
	return i;
}

// converts a string into an array containing each tokens
void str2tab(char* cmd, char** mytabtest)
{
	char* cmdp = cmd;
	char* cmds = cmd;
	int strl;
	int i = 0;
	
	const int INIT      = 0;
	const int READQUOTE = 1;
	const int READSPACE = 2;
	const int WRITE     = 3;
	int state = READSPACE;

	while (1) {

	switch ( state )
	{
	case INIT :	
		    if ( (cmdp[0]=='"') ) { state = READQUOTE; };
			if ( (cmdp[0]==' ') ) { state = READSPACE; };
			cmdp++;
			if ( (cmdp[0]=='"') ) { state = READQUOTE; cmds++; cmdp++; };
			break;
	case READQUOTE :
			if ( (cmdp[0]=='"') ) { state = WRITE; };
			cmdp++;
			break;
	case READSPACE : 
			if ( (cmdp[0]==' ') ) { state = WRITE; };
			cmdp++;
			break;
	case WRITE : 
			strl = cmdp-cmds-1;
			mytabtest[i] = (char*)malloc(strl+1);
			strncpy(mytabtest[i],cmds,strl);
			mytabtest[i][strl]='\0';
			i++;
			cmds=cmdp;
			cmdp--;
			if ( (cmdp[0]=='"') ) { cmds++; cmdp++; };
			state = INIT;
			break;
	default : break;
	}
	
	if ( cmdp[0]=='\0' ) { 
		if ( cmdp[-1]=='"' ) {
			strl = cmdp-cmds-1;
		} 
		else
		{ 
			strl = cmdp-cmds; 
		}
		mytabtest[i] = (char*)malloc(strl+1);
		strncpy(mytabtest[i],cmds,strl);
		mytabtest[i][strl]='\0';
		break; 
	}

	}
	
}

//MISC

char *strdup(const char *str) 
{
	return (_strdup(str));
}

int strnicmp(const char *s1, const char *s2, int n) 
{
	return (_strnicmp(s1,s2,n));
}

int stricmp(const char *s1, const char *s2)
{
	return (_stricmp(s1,s2));
}

// returns the path of a file : C:\foo\foo.txt -> C:\foo\ 
char* ExtractPath(char* file)
{
	char* result = (char*)malloc(strlen(file)+1);
	strcpy(result,file);
	char* pchar = (char*)result + strlen(result);
	
	while ( (pchar--) && (pchar>result) )
	{
		if ( (pchar[0]==kDirectoryDelimiter) )
		{
			pchar[1]='\0';
			break;
		}
	}

	return result;
}

// returns the filename of a file : C:\foo\foo.txt -> foo.txt
char* ExtractFile(char* file)
{
	char* result = (char*)malloc(strlen(file)+1);
	strcpy(result,file);
	char* pchar = (char*)result + strlen(result);
	
	while ( (pchar--) && (pchar>result) )
	{
		if ( (pchar[0]==kDirectoryDelimiter) )
		{
			strcpy(result,pchar+1);
			break;
		}
	}

	return result;
}

// returns the extention of a file : foo.txt -> .txt
char* ExtractFileDown(char* file)
{
	char* result = (char*)malloc(strlen(file)+1);
	strcpy(result,file);
	char* pchar = (char*)result + strlen(result);
	
	strcpy(result,"\0");
	while ( (pchar--) && (pchar>result) )
	{
		if ( (pchar[0]=='.') )
		{
			strcpy(result,pchar);
			break;
		}
	}

	return result;
}

// returns the upper name of a file : foo.txt -> foo
char* ExtractFileUp(char* file)
{
	char* result = (char*)malloc(strlen(file)+1);
	strcpy(result,file);
	char* pchar = (char*)result + strlen(result);
	
	while ( (pchar--) && (pchar>result) )
	{
		if ( (pchar[0]=='.') )
		{
			pchar[0]='\0';
			break;
		}
	}

	return result;
}

// replace filename $file in the array $tab by it's full path version
int SetToFullPath(char** tab, int nbtok, char* file, char* fullfile)
{
	char ofile[64];
	char* ffile = new char[255];
	int res = 0;
	
	ffile[0]='\0';
	strcat(ffile,"\"");
	strcat(ffile,fullfile);
	strcat(ffile,"\"");
	
	sprintf(ofile,"\"%s\"",file);
	for( int i = 0; i<nbtok; i++)
	{
		if ( !_stricmp(tab[i],file) || !_stricmp(tab[i],ofile))
		{
			free(tab[i]);
			tab[i]=ffile;
			res = 1;
		}
	}
	return res;
}


// split $file in packets of $chuncksize bytes
HRESULT SplitFile( char* file, char* outfile, char* ext, int chuncksize, int* num)
{
	HRESULT result = S_OK;

	void* buffer = malloc(chuncksize);
	long numreadtotal = 0;
	long numwrittentotal = 0;
	long numread = 0;
	long numwritten = 0;
	char tgtfile[255];
	int  nbfile = 1;
	FILE *stream = (FILE*)NULL;
	FILE *target = (FILE*)NULL;
	
	stream = fopen(file, "rb");
	if ( stream )
	{
		while ( !feof(stream) )
		{
			numread = 0;
			numwritten = 0;

			sprintf((char*)tgtfile, "%s%d%s", outfile, nbfile, ext);
			target = fopen(tgtfile, "wb");
			if ( target ) 
			{
				numread = fread(buffer, 1, chuncksize, stream);
				numreadtotal += numread;
				numwritten = fwrite( buffer, 1, numread, target );
				numwrittentotal += numwritten;
				
      			fclose( target );
				nbfile++;
			}
			else 
			{
				fclose(stream);
				return S_FALSE;
			}
		}
	} 
	else 
	{
		return S_FALSE;
	}

	fclose( stream );
	free( buffer );
	*num = --nbfile;
	return result;
}

// create the rom resource file
HRESULT CreateRomResources(char* outfile, char* ext, int numfiles, char* resource)
{
	HRESULT result = S_OK;

	FILE* stream = (FILE*)NULL;
	char buffer[255];

	stream = fopen(resource, "w+t");

	if ( stream )
	{
		for (int i = 0; i<numfiles; i++)
		{
			sprintf(buffer,"read 'RomI' (%d) \"%s%d%s\"; \n"
					,i+1,outfile,i+1,ext);
			fwrite(buffer, sizeof(char), strlen(buffer), stream);
		}

	} else {
		return S_FALSE;
	}
	
	fclose( stream );
	return result;
}

// create the rom files and the rom resource file
HRESULT CreateRomFiles( char* file, char* outfile, char* ext, int chuncksize)
{
	HRESULT result = S_OK;
	int numfiles = 0;
	char tgtfile[255];

	if ( SplitFile(file,outfile,ext,chuncksize, &numfiles) == S_OK )
	{
		sprintf(tgtfile,"%s.r",outfile);
		if ( CreateRomResources(ExtractFile(outfile),ext,numfiles,tgtfile) == S_FALSE )
		{
			return S_FALSE;
		}

	} else {
		return S_FALSE;
	}

	return result;
}

// free and array of pointers
void FreeCharMatrix( char** tab, int len )
{
	for (int i = len-1; i>=0; i--) { free(tab[i]); tab[i] = (char*)NULL; }
	free(tab);
}

// returns the target output file
char* GetPrcOutputName(CWPluginContext context) {
	CWTargetInfo tInfo;
	CWGetTargetInfo(context, &tInfo);
	CWFileSpec fSpec = tInfo.outfile;
	char* fName = fSpecToChar(fSpec);
	return(fName);
}

// returns the filename of a CWFileSpec file
char* fSpecToChar(CWFileSpec fSpec) 
{
	char* fName = (char*)NULL;
	#ifdef macintosh
		int lvLength = 0;
		Handle fullPath;
		FSpPathFromLocation(&fSpec,&lvLength,&fullPath);
		//fName = (char*)fSpec.name;
		fName = (char*)*fullPath;
	#else
		fName = fSpec.path;
	#endif
	char* sfile = strdup(fName);
	return sfile;
}

#ifdef macintosh

OSErr FSpPathFromLocation(
    FSSpec *spec,               /* The location we want a path for. */
    int *length,                /* Length of the resulting path. */
    Handle *fullPath)           /* Handle to path. */
{
    OSErr err;
    FSSpec tempSpec;
    CInfoPBRec pb;
        
    *fullPath = NULL;
        
    /* 
     * Make a copy of the input FSSpec that can be modified.
     */
    BlockMoveData(spec, &tempSpec, sizeof(FSSpec));
        
    if (tempSpec.parID == fsRtParID) {
        /* 
         * The object is a volume.  Add a colon to make it a full 
         * pathname.  Allocate a handle for it and we are done.
         */
        tempSpec.name[0] += 2;
        tempSpec.name[tempSpec.name[0] - 1] = ':';
        tempSpec.name[tempSpec.name[0]] = '\0';
                
        err = PtrToHand(&tempSpec.name[1], fullPath, tempSpec.name[0]);
    } else {
        /* 
         * The object isn't a volume.  Is the object a file or a directory? 
         */
        pb.dirInfo.ioNamePtr = tempSpec.name;
        pb.dirInfo.ioVRefNum = tempSpec.vRefNum;
        pb.dirInfo.ioDrDirID = tempSpec.parID;
        pb.dirInfo.ioFDirIndex = 0;
        err = PBGetCatInfoSync(&pb);

        if ((err == noErr) || (err == fnfErr)) {
            /* 
             * If the file doesn't currently exist we start over.  If the
             * directory exists everything will work just fine.  Otherwise we
             * will just fail later.  If the object is a directory, append a
             * colon so full pathname ends with colon.
             */
            if (err == fnfErr) {
                BlockMoveData(spec, &tempSpec, sizeof(FSSpec));
            } else if ( (pb.hFileInfo.ioFlAttrib & ioDirMask) != 0 ) {
                tempSpec.name[0] += 1;
                tempSpec.name[tempSpec.name[0]] = ':';
            }
                        
            /* 
             * Create a new Handle for the object - make it a C string.
             */
            tempSpec.name[0] += 1;
            tempSpec.name[tempSpec.name[0]] = '\0';
            err = PtrToHand(&tempSpec.name[1], fullPath, tempSpec.name[0]);
            if (err == noErr) {
                /* 
                 * Get the ancestor directory names - loop until we have an 
                 * error or find the root directory.
                 */
                pb.dirInfo.ioNamePtr = tempSpec.name;
                pb.dirInfo.ioVRefNum = tempSpec.vRefNum;
                pb.dirInfo.ioDrParID = tempSpec.parID;
                do {
                    pb.dirInfo.ioFDirIndex = -1;
                    pb.dirInfo.ioDrDirID = pb.dirInfo.ioDrParID;
                    err = PBGetCatInfoSync(&pb);
                    if (err == noErr) {
                        /* 
                         * Append colon to directory name and add 
                         * directory name to beginning of fullPath.
                         */
                        ++tempSpec.name[0];
                        tempSpec.name[tempSpec.name[0]] = ':';
                                                
                        (void) Munger(*fullPath, 0, NULL, 0, &tempSpec.name[1],
                                tempSpec.name[0]);
                        err = MemError();
                    }
                } while ( (err == noErr) &&
                        (pb.dirInfo.ioDrDirID != fsRtDirID) );
            }
        }
    }
    
    /*
     * On error Dispose the handle, set it to NULL & return the err.
     * Otherwise, set the length & return.
     */
    if (err == noErr) {
        *length = GetHandleSize(*fullPath) - 1;
    } else {
        if ( *fullPath != NULL ) {
            DisposeHandle(*fullPath);
        }
        *fullPath = NULL;
        *length = 0;
    }

    return err;
}

#endif
