#include <SonyCLIE.h>


UInt16 *ftrCardNoP, oldftrCardNo;
LocalID *ftrDbIDP, oldftrDbID;

UInt16** maskPP;
UInt16* oldmaskP;

#define NUMBER_OF_MASK 1
UInt16 mask[] = { sonyJogAstMaskType1,
                  NUMBER_OF_MASK,	// number of forms to mask
                  MainForm, sonyJogAstMaskUp | sonyJogAstMaskDown
                  	// mask JogUp and JogDown for the Main form
                  	// see SonyJogAssist.h for other mask values
                  	// <formID>, <mask>,
                  	// ...
                };

// ...


static Err AppStart(void)
{
	// ...

	SonySysFtrSysInfoP infoP;

	// get Sony system information
	if (!FtrGet(sonySysFtrCreator, sonySysFtrNumSysInfoP, (UInt32 *)&infoP))
	{

		// disable JogAssist
		if (infoP && (infoP->extn & sonySysFtrSysInfoExtnJogAst)
		    && !FtrGet(sonySysFtrCreator, sonySysFtrNumJogAstMaskP, (UInt32*) &maskPP))
		{

			// save the old mask to restore later
			oldmaskP = *maskPP;
			// set the JogAssist mask
			*maskPP = mask;

			// set JogAssist mask owner
			if (!FtrGet(sonySysFtrCreator, sonySysFtrNumJogAstMOCardNoP, (UInt32*) &ftrCardNoP)
			    && !FtrGet(sonySysFtrCreator, sonySysFtrNumJogAstMODbIDP, (UInt32*) &ftrDbIDP))
			{
				// save the old mask owner to restore later
				oldftrCardNo = *ftrCardNoP;
				oldftrDbID = *ftrDbIDP;
				// set the mask owner;
				SysCurAppDatabase(ftrCardNoP, ftrDbIDP);
			}
		}
	}

	// ...
}



static void AppStop(void)
{
	// ...

	// restore original JogAssist mask
	if(maskPP)	*maskPP = oldmaskP;

	// restore original JogAssist mask owner
	*ftrCardNoP = oldftrCardNo;
	*ftrDbIDP = oldftrDbID;

	// ...
}
