#ifndef _PROGRAM_GLOBALS_H_
#define _PROGRAM_GLOBALS_H_

#define CREATOR			'BrLr'
#define APPLICATION_NAME "\pBrailler"
#define	SAVE_TYPE		'TEXT'
#define	SAVE_VERSION	1

enum
{
	kAboutWindow=0,		/* about box */
	kOtherProductsWindow,	/* other products window */
	kHelpWindow,		/* help window */
	kMainWindow,		/* main graphics window */
	kFloatingWindow
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
	userCancelErr,
	
	kCantCreateFile,
	kCantOpenFileToSave,
	kDiskFull,
	kCantWriteFile,
	kCantOpenFileToLoad,
	kCantLoadFile
};

extern	short			gGrade;
extern	short			gOldImportGrade;
extern	Boolean			gDynamicScroll;

#endif
