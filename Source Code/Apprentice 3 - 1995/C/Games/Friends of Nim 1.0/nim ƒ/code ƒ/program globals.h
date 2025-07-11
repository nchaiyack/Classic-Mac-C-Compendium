#ifndef _PROGRAM_GLOBALS_H_
#define _PROGRAM_GLOBALS_H_

#define USE_SOUNDS				1
#define USE_MUSIC				1
#define USE_SPEECH				0
#define USE_DRAG				0
#define USE_MERCUTIO			0
#define USE_PRINTING			1
#define SUPPORT_STYLED_TEXT		0
#define USE_SAVE				0

#define CREATOR			'Nim.'
#define APPLICATION_NAME "\pFriends of Nim"
#define	SAVE_TYPE		'Game'
#define	SAVE_VERSION	1

enum
{
	kAboutWindow=0,			/* about box */
	kOtherProductsWindow,	/* other products window */
	kHelpWindow,			/* help window */
	kMainWindow				/* main graphics window */
};

enum ErrorTypes
{
	allsWell=0,
	
	/* shell errors */
	kNoMemory,
	kNoMemoryAndQuitting,
	kProgramIntegrityNotVerified,
	kProgramIntegritySet,
	kSystemTooOld,
	userCancelErr
};

typedef short		ErrorTypes;

#endif
