@echo off
REM -- First make map file from Microsoft Visual C++ generated resource.h
echo // MAKEHELP.BAT generated Help Map file.  Used by FATFLASH.HPJ. >"..\hlp\FATFlash.hm"
echo. >>"..\hlp\FATFlash.hm"
echo // Commands (ID_* and IDM_*) >>"..\hlp\FATFlash.hm"
makehm ID_,HID_,0x10000 IDM_,HIDM_,0x10000 resource.h >>"..\hlp\FATFlash.hm"
echo. >>"..\hlp\FATFlash.hm"
echo // Prompts (IDP_*) >>"..\hlp\FATFlash.hm"
makehm IDP_,HIDP_,0x30000 resource.h >>"..\hlp\FATFlash.hm"
echo. >>"..\hlp\FATFlash.hm"
echo // Resources (IDR_*) >>"..\hlp\FATFlash.hm"
makehm IDR_,HIDR_,0x20000 resource.h >>"..\hlp\FATFlash.hm"
echo. >>"..\hlp\FATFlash.hm"
echo // Dialogs (IDD_*) >>"..\hlp\FATFlash.hm"
makehm IDD_,HIDD_,0x20000 resource.h >>"..\hlp\FATFlash.hm"
echo. >>"..\hlp\FATFlash.hm"
echo // Frame Controls (IDW_*) >>"..\hlp\FATFlash.hm"
makehm IDW_,HIDW_,0x50000 resource.h >>"..\hlp\FATFlash.hm"
REM -- Make help for Project FATFLASH


echo Building Win32 Help files
start /wait hcw /C /E /M "..\hlp\FATFlash.hpj"
if errorlevel 1 goto :Error
if not exist "..\hlp\FATFlash.hlp" goto :Error
if not exist "..\hlp\FATFlash.cnt" goto :Error
echo.
if exist Debug\nul copy "..\hlp\FATFlash.hlp" Debug
if exist Debug\nul copy "..\hlp\FATFlash.cnt" Debug
if exist Release\nul copy "..\hlp\FATFlash.hlp" Release
if exist Release\nul copy "..\hlp\FATFlash.cnt" Release
echo.
goto :done

:Error
echo ..\hlp\FATFlash.hpj(1) : error: Problem encountered creating help file

:done
echo.
