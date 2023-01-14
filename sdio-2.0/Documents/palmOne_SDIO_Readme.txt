
          ----------------------------------------------

               palmOne SDIO Software Development Kit
                          Version 2.00
                     Readme - May 2005
          ----------------------------------------------

                Copyright (c) 2005 palmOne, Inc.


-----------------------------
Introduction to SDIO SDK 1.00
-----------------------------

The SDIO Software Development Kit allows developers
to create SDIO applications in conjunction with
the Palm OS SDK. It is an add-on to the Palm OS
SDK environment.

The SDIO SDK includes:
- SDIO Developer Guide
- SDIO Header Files
- SDIO Example Projects

The SDIO SDK version 2.00 is compatible with Palm OS SDK
version 5.X.

In addition to the SDIO SDK, you can install
debug versions of the SDIO Slot Drivers for some palmOne devices.
The debug slot drivers can be installed on a device
and assist the developer with SD/SDIO command tracing.
The SDIO debug versions of the slot drivers are included in this SDK.


-------------
Installation
-------------

To use the SDIO SDK Header Files with an existing Palm OS
SDK installation, please follow these steps:

1. Ensure the Palm OS SDK is installed and you are able to
   build Palm OS applications successfully.

2. Extract the SDIO SDK archive to your hard drive. You can
   locate the extracted files in any folder.

3. In your source code, include the files "SDIO.h" and
   "AutoRun.h".

4. Update your Target Settings to include the
   "SDIOIncs" folder. To do this, edit your
   project settings in Code Warrior. Set
   "Target-->Access Paths-->System Paths" to include
   "SDIOIncs". You should be able to "Add" the
   "SDIOIncs" entry to the list.

To check the installation, you can make sure the
"SDIOExample.mcp" project compiles.


-----------------
Technical Support
-----------------

Palm OS Support          - http://www.palmos.com/dev/tools/core.html
palmOne SDIO SDK Support - http://pluggedin.palmone.com


SDIO SDK 2.00 Readme Version 1.00

