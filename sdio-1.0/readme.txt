
          ----------------------------------------------

               Palm SDIO Software Development Kit
                          Version 1.00
                     Readme - September 2001
          ----------------------------------------------

                Copyright (c) 2001 Palm, Inc.



----------------
Read Me Contents
----------------
- Introduction to SDIO SDK 1.00
- Installation 
- Technical Support


-----------------------------
Introduction to SDIO SDK 1.00
-----------------------------

The SDIO Software Development Kit allows developers
to create SDIO applications in conjunction with
the Palm OS SDK. It is an add-on to the Palm OS
SDK environment.

The SDIO SDK contains:
- Guide to Developing SDIO Peripherals
- SDIO Header Files
- SDIO Example Projects

The SDIO SDK version 1.00 is compatible with Palm OS SDK
version 4.0 for both Mac and Windows platforms.

In addition to the SDIO SDK, you should also obtain 
debug versions of the SDIO Slot Drivers for Palm OS. 
The debug slot drivers can be installed on a device 
and assist the developer with SD/SDIO command tracing.
The SDIO debug versions of the slot drivers are available
for download in the Plugged In area at 
http://www.palmos.com/dev/pluggedin.


-------------
Installation
-------------

To use the SDIO SDK Header Files with an existing Palm OS
SDK installation, please follow these steps:

1. Ensure the Palm OS SDK is installed and you are able to
   build Palm OS applications successfully.
   
2. Extract the SDIO SDK archive to your hard drive. You can
   locate the extracted files in any folder. The root folder
   of the SDIO SDK is "Palm SDIO SDK Windows/" or
   "Palm SDIO SDK Mac".
   
3. Copy the "Palm SDIO Support" folder and all its contents
   to the "CW for Palm OS R6" folder. The 
   "Palm SDIO Support" folder should be located in the same
   folder as the "Palm OS Support" folder.
   
4. In your source code, include the files "SDIO.h" and
   "AutoRun.h".
   
5. Update your Target Settings to include the
   "Palm SDIO Support" folder. To do this, edit your
   project settings in Code Warrior. Set 
   "Target-->Access Paths-->System Paths" to include
   "{Compiler}Palm SDIO Support". The setting
   "{Compiler}Palm OS Support" should already exist
   in this list. You should be able to "Add" the
   "{Compiler}Palm SDIO Support" entry to the list.
   
To check the installation, you can make sure the 
"SDIOExample.mcp" project compiles.


-----------------
Technical Support
-----------------

Palm OS Support - http://www.palmos.com/dev
Palm SDIO SDK Support - http://www.palmos.com/dev/pluggedin


SDIO SDK 1.00 Readme Version 1.01

