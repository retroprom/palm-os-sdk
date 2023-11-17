/*
 *  Linker.c	- public interface to MakeROM Linker
 *
 *  Copyright © 2001 metrowerks inc.  All rights reserved.
 *
 */

/* standard headers */
#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <string.h>

#ifdef __cplusplus
}
#endif

/* system headers */
#if macintosh
#include <Files.h>
#include <Strings.h>
#endif

/* compiler headers */
#include "DropInCompilerLinker.h"
#include "CompilerMapping.h"
#include "CWPluginErrors.h"
#include "CWRuntimeFeatures.h"

/* project headers */
#include "Panel.h"
#include "PlugUtils.h"

/* use standard CodeWarrior debugger */
#define kDebuggerCreator	'MWDB'

/* used for linking */
CWPluginContext myGlobalContext;

/* prototypes of local functions */
static CWResult	Link(CWPluginContext context);
static CWResult	Disassemble(CWPluginContext context);
static CWResult	GetTargetInfo(CWPluginContext context);


#if CW_USE_PRAGMA_EXPORT
#pragma export on
#endif

#pragma mark CWPlugin_GetDropInFlags
CWPLUGIN_ENTRY(CWPlugin_GetDropInFlags)(const DropInFlags** flags, long* flagsSize)
{
	static const DropInFlags sFlags = {
		kCurrentDropInFlagsVersion,
		CWDROPINLINKERTYPE,
		DROPINCOMPILERLINKERAPIVERSION_7,
		linkMultiTargAware,
		0,
		DROPINCOMPILERLINKERAPIVERSION
	};
	
	*flags = &sFlags;
	*flagsSize = sizeof(sFlags);
	
	return cwNoErr;
}

#pragma mark CWPlugin_GetDropInName
CWPLUGIN_ENTRY(CWPlugin_GetDropInName)(const char** dropinName)
{
	static const char* sDropInName = "MakeROM";
	
	*dropinName = sDropInName;
	
	return cwNoErr;
}

#pragma mark CWPlugin_GetDisplayName
CWPLUGIN_ENTRY(CWPlugin_GetDisplayName)(const char** displayName)
{
	static const char* sDisplayName = "MakeROM";
	
	*displayName = sDisplayName;
	
	return cwNoErr;
}

#pragma mark CWPlugin_GetPanelList
CWPLUGIN_ENTRY(CWPlugin_GetPanelList)(const CWPanelList** panelList)
{
	// +++Turn this on when the sample panel has been converted!
	static const char* sPanelName = kHandspringPanelName;
	static CWPanelList sPanelList = {kCurrentCWPanelListVersion, 1, &sPanelName};
	
	*panelList = &sPanelList;
	
	return cwNoErr;
}

#pragma mark CWPlugin_GetTargetList
CWPLUGIN_ENTRY(CWPlugin_GetTargetList)(const CWTargetList** targetList)
{
	static CWDataType sCPU = 'HAND';
	static CWDataType sOS = 'HAND';
	static CWTargetList sTargetList = {kCurrentCWTargetListVersion, 1, &sCPU, 1, &sOS};
	
	*targetList = &sTargetList;
	
	return cwNoErr;
}

#pragma mark CWPlugin_GetDefaultMappingList
CWPLUGIN_ENTRY(CWPlugin_GetDefaultMappingList)(const CWExtMapList** defaultMappingList)
{
	
	static CWExtensionMapping sExtension[2];
	
	sExtension[0].type = 0;
	strcpy(sExtension[0].extension,".doc\0");
	sExtension[0].flags = kLaunchable | kIgnored;
	
	sExtension[1].type = 0;
	strcpy(sExtension[1].extension,".prc\0");
	sExtension[1].flags = 0;
	
	static CWExtMapList sExtensionMapList = {kCurrentCWExtMapListVersion, 2, sExtension};
	
	*defaultMappingList = &sExtensionMapList;
	
	return cwNoErr;
}

#pragma mark CWPlugin_GetFamilyList
CWPLUGIN_ENTRY (CWPlugin_GetFamilyList)(const CWFamilyList** familyList)
{
	static CWFamily sFamily = {'HAND', "MakeROM Linker"};
	static CWFamilyList sFamilyList = {kCurrentCWFamilyListVersion, 1, &sFamily};
	
	*familyList = &sFamilyList;
	
	return cwNoErr;
}

#if CW_USE_PRAGMA_EXPORT
#pragma export off
#endif


/*
 *	main	-	main entry-point for Drop-In Sample linker
 *
 */

pascal short	main(CWPluginContext context)
{
	short		result;
	long		request;
	
	if (CWGetPluginRequest(context, &request) != cwNoErr)
		return cwErrRequestFailed;
	
	result = cwNoErr;
		
	/* dispatch on linker request */
	switch (request)
	{
	case reqInitLinker:
		/* linker has just been loaded into memory */
		break;
		
	case reqTermLinker:
		/* linker is about to be unloaded from memory */
		break;
		
	case reqLink:
		/* build the final executable */
		result = Link(context);
		break;
		
	case reqDisassemble:
		/* disassemble object code for a given project file */
		result = Disassemble(context);
		break;
	
	case reqTargetInfo:
		/* return info describing target characteristics */
		result = GetTargetInfo(context);
		break;
		
	default:
		result = cwErrRequestFailed;
		break;
	}
	
	result = CWDonePluginRequest(context, result);
	
	/* return result code */
	return result;
}


static CWResult	Link(CWPluginContext context)
{
	CWMemHandle		prefsHand;
	HandspringPref	*prefsPtr;
	HandspringPref	prefsData;
	long		index;
	CWResult	err;
	long		filecount;

	/* load the relevant prefs */
	err = CWGetNamedPreferences(context, kHandspringPanelName, &prefsHand);
	if (err != cwNoErr)
		return (err);
		
	err = CWLockMemHandle(context, prefsHand, false, (void**)&prefsPtr);
	if (err != cwNoErr)
		return (err);
	
	prefsData = *prefsPtr;
	
	err = CWUnlockMemHandle(context, prefsHand);
	if (err != cwNoErr)
		return (err);

	/*
	 *	Once all initialization has been done, the principal interaction 
	 *	between the linker and the IDE occurs in a loop where the linker 
	 *	processes all files in the project.
	 */
	 
	err = CWGetProjectFileCount(context, &filecount);
	if (err != cwNoErr)
		return (err);
	
	char** lFiles;
	int    lFileCnt = 0;
	
	//Get the output directory and get some infos
	char* pOutput 		= GetPrcOutputName(context); // the output file
	char* pTargetDir 	= ExtractPath(pOutput);      // the output directory
	char* pOutfile 		= ExtractFile(pOutput);      // the file name
	char* pOutfileUp 	= ExtractFileUp(pOutfile);   // the file name without extention
	char* pOutfileDown 	= ExtractFileDown(pOutfile); // the file extention (.xxx)
	
	char pHdr[64];		// the hdr offset
	char pStack[64];	// the stack value
	char pVersion[64];	// the version number
	char pVersionL[3];
	char pVersionH[3];
	sprintf((char*)pHdr,"0x%.8x",prefsData.hdr_offset);
	sprintf((char*)pStack,"0x%.8x",prefsData.initial_stack);
	sprintf((char*)pVersion,"%.4d",prefsData.version);
	sprintf((char*)pVersionH,"%c%c",pVersion[0],pVersion[1]);
	sprintf((char*)pVersionL,"%c%c",pVersion[2],pVersion[3]);
	sprintf((char*)pVersion,"0x%.2x%.2x",atoi(pVersionH),atoi(pVersionL));
	
	//Count the number of prc files
	for (index = 0; (err == cwNoErr) && (index < filecount); index++)
	{
		CWProjectFileInfo	fileInfo;
		err = CWGetFileInfo(context, index, false, &fileInfo);
		if (err != cwNoErr)
			continue;
		
		CWFileSpec fSpec = fileInfo.filespec;
		char* fName = fSpecToChar(fSpec);
		
		char* ext = ExtractFileDown(fName);
		if ( stricmp(ext,".prc")==0 )
		{
			lFileCnt++;
		}
		free(ext);
		free(fName);
	}
	
	//Generate the array of prc files
	lFiles = new char*[lFileCnt];
	lFileCnt = 0;
	for (index = 0; (err == cwNoErr) && (index < filecount); index++)
	{
		CWProjectFileInfo	fileInfo;
		err = CWGetFileInfo(context, index, false, &fileInfo);
		if (err != cwNoErr)
			continue;
		
		CWFileSpec fSpec = fileInfo.filespec;
		char* fName = fSpecToChar(fSpec);
		
		char* ext = ExtractFileDown(fName);
		if ( stricmp(ext,".prc")==0 )
		{
			int x = strlen(fName);
			lFiles[lFileCnt] = new char[strlen(fName)+1];
			strcpy(lFiles[lFileCnt], fName);
			lFileCnt++;
		}
		free(ext);
		free(fName);
	}
	
	//Create an array corresponding which will tell if a file needs to be ignored
	//as a -romDB file, i.e it is overridden by a "add parameter" option
	short* lFilesUse = new short[lFileCnt];
	for (int i=0;i<lFileCnt;i++) lFilesUse[i]=1;
	
	//Once we have the array generate the command line ...
	char sCmdLine[4096];
	
	// Generate the basic cmd
	sprintf(sCmdLine,"MakeROM -op create "\
				"-hdr %s "\
				"-chStack %s "\
				"-chName \"%s\" "\
				"-chManuf \"%s\" "\
				"-chVersion %s "\
				"-romName \"%s\" "\
				"-autoSize "\
				"-o \"%s\""\
				,pHdr,pStack,prefsData.name,prefsData.manuf,pVersion,prefsData.name,pOutput);
				
	// Add the extra arguments
	if ( prefsData.cmdline )
	{
		char sCline[512];
		sprintf(sCline,"%s",prefsData.command_line);
		
		// Replace files like Starter.prc to $patch\\Starter.prc
		int nb = nbtoken(sCline);
		char** mytab = (char**)malloc((nb)*(sizeof(char*)));
		str2tab(sCline,mytab);
		for (int i=0;i<lFileCnt;i++)
		{
			char* lvStr = ExtractFile(lFiles[i]);
			if ( SetToFullPath(mytab,nb,lvStr,lFiles[i]) )
			{
				lFilesUse[i]=0;
			}
		}
		
		// Recreate the extra arguments line
		char sCline2[512];
		sCline2[0]='\0';
		for (int j=0;j<nb;j++)
		{
			char* hey = mytab[j];
			if ( (!strcmp(mytab[j]," ")) || (!strcmp(mytab[j],"")) ) { continue; }
			strcat(sCline2," ");
			strcat(sCline2,mytab[j]);
		}
		FreeCharMatrix(mytab,nb);
		// end of replace
		
		strcat(sCmdLine,sCline2);
	}
	
	// Add all the prc files to use
	char sPrcFile[512];
	int i = 0;
	for (i=0;i<lFileCnt;i++)
	{
		if ( !lFilesUse[i] ) { continue; }
		sprintf(sPrcFile," -romDB \"%s\"",lFiles[i]);
		strcat(sCmdLine,sPrcFile);
	}
	
	//Create the argv array for the makerom command
	int nbtok = nbtoken(sCmdLine);
	char** mytab = (char**)malloc((nbtok)*(sizeof(char*)));
	str2tab(sCmdLine,mytab);
	
	myGlobalContext = context;
	
	int ret = makerom_main(nbtok,mytab);

	//Got an error ?
	if ( ret ) 
	{
		DisplayError("ROM creation failed.");
		err = S_FALSE;
		goto Exit;
		
	} else {
		err = S_OK;
	}
	
	//Split the ROM file and generate the ressource file
	HRESULT lerr = CreateRomFiles(pOutput, ExtractFileUp(pOutput), pOutfileDown, 16000);
	if ( lerr == S_FALSE )
	{
		DisplayError("ROM resource creation failed.");
		err = S_FALSE;
	}
	
	Exit:

	FreeCharMatrix(lFiles,lFileCnt);
	FreeCharMatrix(mytab,nbtok);
	free(pTargetDir);
	free(pOutfile);
	free(pOutfileUp);
	free(pOutfileDown);
	free(lFilesUse);

	return (err);
}


static CWResult	Disassemble(CWPluginContext context)
{
	char				buff[200];
	CWProjectFileInfo	fileInfo;
	CWMemHandle			objectData;
	long				objectSize;
	size_t				bufflen;
	CWResult			err;
	long				mainFileNumber;
	CWMemHandle			output;
	
	/* get info about the file we have to disassemble */
	err = CWGetMainFileNumber(context, &mainFileNumber);
	if (err != cwNoErr)
		return (err);
		
	err = CWGetFileInfo(context, mainFileNumber, false, &fileInfo);
	
	if (err == cwNoErr)
	{
		if (fileInfo.hasobjectcode || fileInfo.hasresources)
		{
			/* the file has either object code and/or resources */
			
			/* load the object code or resource fork image */
			err = CWLoadObjectData(context, mainFileNumber, &objectData);
			
			if (err == cwNoErr)
			{
				/* at this point we process the object code */
				err = CWGetMemHandleSize(context, objectData, &objectSize);
				
				/* release the object code when we're done */
				err = CWFreeObjectData(context, mainFileNumber, objectData);
			}
			else
				objectSize = 0;

#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
			sprintf(buff, "file name: %#s\robject data size: %ld\r", 
					fileInfo.filespec.name, objectSize);
#endif
#if CWPLUGIN_HOST == CWPLUGIN_HOST_WIN32
			sprintf(buff, "file name: %s\robject data size: %ld\r", 
					fileInfo.filespec.path, objectSize);
#endif
		}
		else
		{
			/*
			 *	The file contains neither object code nor resources.  It may 
			 *	be a doc file or some such.
			 */
			
#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
			sprintf(buff, "file name: %#s\rcontains no object code nor resources\r", 
					fileInfo.filespec.name);
#endif
#if CWPLUGIN_HOST == CWPLUGIN_HOST_WIN32
			sprintf(buff, "file name: %s\rcontains no object code nor resources\r", 
					fileInfo.filespec.path);
#endif
		}
		
		bufflen = strlen(buff) + 1;
		
		err = CWAllocMemHandle(context, bufflen, true, &output);
		if (err == cwNoErr)
		{
			void* p;
			err = CWLockMemHandle(context, output, false, &p);
			if (err == cwNoErr)
			{
				CWNewTextDocumentInfo docinfo;
				
				memcpy(p, buff, bufflen);
				CWUnlockMemHandle(context, output);
			
				memset(&docinfo, 0, sizeof(docinfo));
				docinfo.text = output;
				err = CWCreateNewTextDocument(context, &docinfo);
			}
		}		
	}
	
	return (err);
}


static CWResult	GetTargetInfo(CWPluginContext context)
{
	CWTargetInfo	targ;
	CWMemHandle		prefsHand;
	HandspringPref	prefsData;
	HandspringPref	*prefsPtr;
	CWResult		err;
	
	memset(&targ, 0, sizeof(targ));
	
	err = CWGetOutputFileDirectory(context, &targ.outfile);
	targ.outputType 		= linkOutputFile;
	targ.symfile			= targ.outfile;	/* location of SYM file				*/
	targ.linkType			= exelinkageFlat;
	targ.targetCPU			= 'HAND';
	targ.targetOS			= 'HAND';
	
	/* load the relevant prefs */
	err = CWGetNamedPreferences(context, kHandspringPanelName, &prefsHand);
	if (err != cwNoErr)
		return (err);
	
		err = CWLockMemHandle(context, prefsHand, false, (void**)&prefsPtr);
	if (err != cwNoErr)
		return (err);
	
	prefsData = *prefsPtr;
	
	err = CWUnlockMemHandle(context, prefsHand);
	if (err != cwNoErr)
		return (err);
	
#if CWPLUGIN_HOST == CWPLUGIN_HOST_MACOS
	targ.outfileCreator		= 'MWIE';
	targ.outfileType		= 'TEXT';
	targ.debuggerCreator	= kDebuggerCreator;	/* so IDE can locate our debugger	*/

	/* we put output file in same folder as project, but with name stored in prefs */
	targ.outfile.name[0] = strlen((const char*)prefsData.outfile);
	BlockMoveData(prefsData.outfile, targ.outfile.name + 1, targ.outfile.name[0]);
	
	/* we put SYM file in same folder as project, but with name stored in prefs */
	BlockMoveData(targ.outfile.name, targ.symfile.name, StrLength(targ.outfile.name)+1);
	{
#if TARGET_API_MAC_CARBON
		char temp[256];
		p2cstrcpy(temp, targ.symfile.name);
		strcat(temp, ".SYM");
		c2pstrcpy(targ.symfile.name, temp);
#else
		char* cstr;
		cstr = p2cstr(targ.symfile.name);
		strcat(cstr, ".SYM");
		c2pstr(cstr);
#endif
	}
#endif

#if CWPLUGIN_HOST == CWPLUGIN_HOST_WIN32
	targ.debugHelperIsRegKey = true;
	*(long*)targ.debugHelperName = kDebuggerCreator;
	targ.debugHelperName[4] = 0;
	strcat(targ.outfile.path, "\\");
	strcat(targ.outfile.path, (const char*)prefsData.outfile);
	strcpy(targ.symfile.path, targ.outfile.path);
	strcat(targ.symfile.path, ".SYM");
#endif

	targ.runfile			= targ.outfile;
	targ.linkAgainstFile	= targ.outfile;

	/* we can only run applications */
	//targ.canRun = (prefsData.projtype == kProjTypeApplication);
	targ.canRun = 0;
	
	/* we can only debug if we have a SYM file */
	//targ.canDebug = prefsData.linksym;	
	targ.canDebug = 0;
	
	err = CWSetTargetInfo(context, &targ);
	
	return err;
}

