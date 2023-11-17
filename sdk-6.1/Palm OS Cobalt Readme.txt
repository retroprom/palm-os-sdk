Palm OS 6 SDK (ARM) - Readme File

IMPORTANT:  Read Before Using the Accompanying Software

= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =
  P a l m   O S   S o f t w a r e   D e v e l o p m e n t   K i t
= = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = = =

**********************************************************************
* ENHANCEMENTS AND FIXES -- WHAT'S NEW                               *
**********************************************************************

Building/running Protein RAM PIM apps
*************************************
Protein PIM apps in the SDK samples (sample names prefixed by "RAM_") 
are configured to be built/ran side by side with ROM (aka OS) PIM apps 
independently without having to rely on any of the OS components. You 
can build the PIM apps either using Eclipse IDE or command line "make" 
utility. Once you have built the PIM app, you have to copy the dll of 
the PIM app itself and also dlls of all the libraries that this 
PIM app is linked against, to the Simulator/debug or Simulator/release 
folder. After copying the dlls, start the simulator and drag 'n' drop 
the PIM app PRC on to the Simulator to install it. You can confirm that 
Sample PIM app is running independently without having to rely on any 
of the OS component, by creating the records both in the ROM and as 
well as sample PIM app and then cross checking the records of the two 
apps against each other.

Table below lists components required by each Sample PIM apps

PIM app	   	  Components
RAM_Todo     ->   RAM_SearchLib_Client.slib 
                  RAM_NoteViewLib_Client.slib

RAM_Memo     ->   RAM_PhoneBookIOLib_Client.slib
		  RAM_SearchLib_Client.slib 

RAM_Addres   ->   RAM_BooksLib_Client.slib 
                  RAM_SearchLib_Client.slib 
                  RAM_NoteViewLib_Client.slib
                  Load RAM_Todo Prc before loading RAM_Address Prc

RAM_DateBook ->   RAM_PhoneBookIOLib_Client.slib 
		  RAM_BooksLib_Client.slib
		  RAM_SearchLib_Client.slib
		  RAM_NoteViewLib_Client.slib
		  Load RAM_Todo.Prc before loading RAM_DateBook.Prc
		  Load RAM_BooksLib.Prc before loading RAM_DateBook.Prc
		  

***********************************************************
Changing build configuration of samples using Standard Make
***********************************************************
By default, all samples built as "Debug/Simulator" configuration. If 
you want build the SDK sample for a different configuration such as 
Release/Device etc. you have to edit the makefile manually and update 
the following makefile variables DEBUG_OR_RELEASE and TARGET_PLATFORM, 
according for more information please read comments in the makefile.

Using Standard Make, if you want to change the build configuration of an 
app which is dependent on other components, then you have to manually edit 
the makefiles of all the components to change their configuration. For 
example if you want to change the build configuration of a PIM app then 
you have to change the build configuration of all the dependent components 
as well, such as libraries that PIM app link.
For example if you want to change the build configuration of the DateBook, 
first you have to change the build configuration of the DateBook itself 
by editing the makefile variables mentioned in the above paragraph, then 
you have to change the build configurations of all the dependent components 
such as PhoneBookIOLib, BooksLib, SearchLib, and NoteViewLib. In DateBook 
case you also have to change the build configuration of the Todo PIM app 
as well because DateBook relies on it.


**********************************************************************
* INFORMATION REGARDING SAMPLES                                      *
**********************************************************************
SamplePinlet sample:
********************
After having built and installed the SamplePinlet application successfully 
for the Simulator. To run this application, tap and hold the right most 
button on the status bar for the input area for at least couple of seconds. 
The pinlet's pop up menu will be shown, now tap on "Sample" to launch the 
SamplePinlet application.

This sample demonstrates how to distinguish between dots and dashes.
Since dots and dashes are determined by the how long (in milliseconds) 
the pen is touching the screen.

**********************************************************************
* IMPORTANT INFORMATION REGARDING FUTURE OS COMPATIBILITY            *
**********************************************************************
TDB
**********************************************************************
* KNOWN ISSUES                                                       *
**********************************************************************

****BlueTooth and Simulator
Bluetooth samples are not supported on the Palm Simulator 6.0.1.


**********************************************************************
* DISCLAIMER                                                         *
**********************************************************************

****elfdump.exe
PalmSource ships the elfdump.exe as part of the compiler tool suite. 
This app is not fully tested and officially is not supported by 
PalmSource at this time. 

**********************************************************************
* INSTALLATION INSTRUCTIONS                                          *
**********************************************************************

+--------------------------------------------------------------------+
+ Installation Using Installers                                      +
+--------------------------------------------------------------------+
Windows 


PalmSource, Inc.
1240 Crossman Avenue
Sunnyvale, California 94089
(408) 400-3000
