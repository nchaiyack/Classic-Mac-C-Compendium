#ifndef _PROGRAM_GLOBALS_H_
#define _PROGRAM_GLOBALS_H_

enum
{
	kAboutWindow=0,		/* about box */
	kAboutMSGWindow,	/* "About MSG" splash screen */
	kOtherMSGWindow,	/* other MSG products list */
	kHelpWindow,		/* help window */
	kNotePad,			/* scratch pad window */
	kMainWindow			/* main graphics window */
};

#define		NUM_WINDOWS				6		/* total number of windows (see above enum) */

enum ErrorTypes
{
	allsWell=0,
	
	/* shell errors */
	kNoMemory,
	kNoMemoryAndQuitting,
	kProgramIntegrityNotVerified,
	kProgramIntegritySet,
	kSystemTooOld,
	userCancelErr,
	
	/* program-specific errors */
	kNoDictionaries,
	kNoFiveLetterCustom,
	kNoSixLetterCustom,
	kNoCustomAtAll,
	kCantGetFiveLetterComputerWord,
	kCantGetSixLetterComputerWord,
	kCantGetFiveLetterHumanWord,
	kCantGetSixLetterHumanWord,
	kCantGetFiveLetterCustomWord,
	kCantGetSixLetterCustomWord,
	kCantSaveFiveLetterCustomWord,
	kCantSaveSixLetterCustomWord,
	kCantCreateFile,
	kCantOpenFileToSave,
	kCantWriteFile,
	kCantOpenFileToLoad,
	kCantLoadFile,
	kBadChecksum,
	kSaveVersionNotSupported,
	kNoMoreCustomWords,
	kDiskFull
};

#define CREATOR			'Jot2'
#define APPLICATION_NAME "\pJotto ]["
#define	SAVE_TYPE		'SvGm'
#define CUSTOM_TYPE		'TEXT'
#define	SAVE_VERSION	2

#define MAX_TRIES		15

extern	char			gComputerWord[6];
extern	char			gHumanWord[MAX_TRIES+1][6];
extern	char			gNumRight[MAX_TRIES];
extern	short			gNumHumanWords[2];
extern	short			gNumComputerWords[2];
extern	unsigned char	gNumTries;
extern	unsigned char	gWhichChar;
extern	unsigned char	gNumLetters;

extern	char			gAllowDup;
extern	char			gNonWordsCount;
extern	char			gAnimation;

extern	short			gWhichWipe;
extern	short			gLastWipe;

#endif
