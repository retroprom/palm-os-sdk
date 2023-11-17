;-------------------------------------------------------------------------------
;	FILE:			PalmSourceSimLibStubShadow.asm
;	COPYRIGHT:		Copyright (c) 2004 PalmSource, Inc., or its subsidiaries.
;					All rights reserved.
;	GENERATED ON:	Fri Sep 03 14:23:28 2004

;	DESCRIPTION:	
;					When debugging shared libraries for PalmSim, this file serves
;					as the "source" file to keep the Visual Studio Debugger in
;					source level debugging mode. This is a template of the
;					actual assembly language generated. There are few directories
;					in which the VS Debugger will look for source files when the
;					source file (specified in the object file) is not found.
;					This file is expected to end up in one of those special places.
;
;	WARNING:		Changing this file without the corresponding code changes in
;					the PalmSource Shared Library Generator are unadvisable.
;

;-------------------------------------------------------------------------------
; Function:    __PrvDispatchJump_<library name>
; Description: Non-preloaded shared library stub dispatcher template...
;-------------------------------------------------------------------------------
PUBLIC __PrvDispatchJump_<libname>
_TEXT	SEGMENT
__PrvDispatchJump_<libname>	PROC NEAR
	cmp         dword ptr [_dispatchTable_<libname>],0
	jne         dispatchInited
	push        edx
	push        ecx
	push        offset _dispatchTable_<libname>
	push        dword ptr [_moduleRefNum]
	push        offset _moduleDescriptor
	call        _SysLinkerStub
	add         esp,0Ch
	pop         ecx
	pop         edx
dispatchInited:
	mov         eax,dword ptr [_dispatchTable_<libname>]
	jmp         dword ptr [eax+edx*4]
__PrvDispatchJump_<libname> ENDP
_TEXT ENDS


;-------------------------------------------------------------------------------
; Function:    _ExportName
; Description:	Every exported function from the ".sld" file
;				has a template which looks like this.
;-------------------------------------------------------------------------------
PUBLIC _ExportName
_TEXT	SEGMENT
_ExportName	PROC NEAR
	mov         edx,<exportNameIndex>
	jmp         __PrvDispatchJump_<libname>
_ExportName ENDP
_TEXT ENDS









































































































































































































;-------------------------------------------------------------------------------
; Function:    __PrvDispatchJump_<library name>
; Description:	Preloaded shared library stub dispatcher template...
;-------------------------------------------------------------------------------
PUBLIC __PrvDispatchJump_<libname>
_TEXT	SEGMENT
__PrvDispatchJump_<libname>	PROC NEAR
	mov         eax,dword ptr [_dispatchTable_<libname> + (dispatchID -1) * 4]
	jmp         dword ptr [eax+edx*4]
__PrvDispatchJump_<libname> ENDP
_TEXT ENDS

