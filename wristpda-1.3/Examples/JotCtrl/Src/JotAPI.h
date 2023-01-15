// LaunchCode Features
#define	ENABLE_JOT				0x0001
#define	DISABLE_JOT				0x0002
#define	ENABLE_WRITING			0x0004
#define	DISABLE_WRITING			0x0008
#define	ENABLE_INKING			0x0010
#define	DISABLE_INKING			0x0020
#define	ENABLE_MODEMARK			0x0040
#define	DISABLE_MODEMARK 		0x0080

#define JOTCREATOR			'JWJT'



/*
 *----------------------------------------------------------------------------
 * Sample to turn off inking and writing on the screen
 *----------------------------------------------------------------------------
 */
/*
UInt16				cardNo;
LocalID				dbID = NULL;
DmSearchStateType	searchState;
UInt32				fFeature;

fFeature = DISABLE_WRITING | DISABLE_INKING;
DmGetNextDatabaseByTypeCreator(true, &searchState, sysFileTApplication, JOTCREATOR, true, &cardNo, &dbID);
SysAppLaunch(cardNo, dbID, 0, 32801, (char *)&fFeature, NULL);
*/