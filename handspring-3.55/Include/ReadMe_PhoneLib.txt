====================================
Handspring DTS Team
http://www.handspring.com/developers
====================================
***If you want to submit a bug report send an email to DevBugs@handspring.com
   Note that there will be no feedback from this email as it is 
   only for bug reports and not for submitting technical questions.
   For technical support, see:
   http://www.handspring.com/developers/tech_support.jhtml


----------------------------------------------
Handspring Phone Library Header Files
----------------------------------------------
December 12, 2002

1. Please refer to the GSM/GPRS Phone Library Documentation for 
   information about these header files.
   
2. The SMS Library included replaces the orignal GSM SMSLibrary.h.
   which has been rename GSM_SMSLibrary.h.
   The new library is very similar to the old one directory but it refers to 
   the new PhoneLib errors and trap number.
   
3. Ringtone.h is the new ringtone header file that supports Treo 300. Look at 
   the App Note describing how to use it.

4. To use the PhoneLib, simply include <PhoneGlobals.h>
   Instead of PhoneLib.h. PhoneGlobals will in turn include 
   everything needed to use the Phone Library.

KNOWN ISSUES:

1. PhoneLib.h includes PalmCompatibility.h

	That might generate some warnings or errors (re-define of some value) 
	during compilation if PalmCompatibility.h was already in one of your 
	source code file.

2. Warnings in CodeWarrior:
	You could get some warnings in CodeWarrior about identifier expected.
	That's because the last element of some structure ends with ; or ,
	It should not cause any problems and we'll try to update our code in 
	future releases

		Warning : identifier expected
		PhoneLibErrors.h line 266   } GSMErrorCode;
	
		Warning : identifier expected
		PhoneLib.h line 435   } PhnGPRSPDPType;
	
		Warning : identifier expected
		PhoneLib.h line 441   } PhnGPRSQOSType;


OTHER GOOD THINGS TO KNOW:

 - The Palm-Debugger for Treo 90 also works for the Treo 300. 
 	You can get it by downloading the latest Palm-GNU tools in our Tools 
	section on our website: http://www.handspring.com/developers/sw_dev.jhtml 

 - Most of the GSM/GPRS Phone library calls can be used for the Treo 300.
   The same code base was used to develop the Treo 300 CDMA Phone Libary. We 
   will clarify the changes/addition in the very close future. Simply read the
   very short "Phone API on Treo 300.doc" app note to understand the different
   event (which you will have to copy yourself at this time)

