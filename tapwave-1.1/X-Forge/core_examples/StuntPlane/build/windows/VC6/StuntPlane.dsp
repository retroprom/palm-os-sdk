# Microsoft Developer Studio Project File - Name="StuntPlane" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=StuntPlane - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "StuntPlane.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "StuntPlane.mak" CFG="StuntPlane - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "StuntPlane - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "StuntPlane - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "StuntPlane - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "NDEBUG"
# ADD RSC /l 0x40b /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "StuntPlane - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /I "..\..\..\include" /I "..\..\..\..\..\xforge\include" /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /D "XFC_PLATFORM_DESKTOPWINDOWS" /YX /FD /GZ /c
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x40b /d "_DEBUG"
# ADD RSC /l 0x40b /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 xfcored.lib xfutild.lib zlib.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib winmm.lib wsock32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept /libpath:"..\..\..\..\..\xforge\lib\windows"

!ENDIF 

# Begin Target

# Name "StuntPlane - Win32 Release"
# Name "StuntPlane - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=..\..\..\source\GameScreen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Level.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\MenuScreen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Mesh.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\Plane.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\SplashScreen.cpp
# End Source File
# Begin Source File

SOURCE=..\..\..\source\StuntPlane.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=..\..\..\include\Buffers.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\GameScreen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Level.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\MenuScreen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Mesh.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Node.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Objects.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Plane.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Screen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\SplashScreen.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\Sprite.h
# End Source File
# Begin Source File

SOURCE=..\..\..\include\StuntPlane.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
