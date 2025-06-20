#ifndef _PROGRAM_GLOBALS_H_
#define _PROGRAM_GLOBALS_H_

#define USE_SOUNDS				0
#define USE_MUSIC				1
#define USE_SPEECH				1
#define USE_DRAG				1
#define USE_MERCUTIO			0
#define USE_PRINTING			1
#define SUPPORT_STYLED_TEXT		0

#define CREATOR			'Kant'
#define APPLICATION_NAME "\pKant Generator Pro"
#define	SAVE_TYPE		'TEXT'
#define BUILD_TYPE		'rsrc'
#define	SAVE_VERSION	1

enum
{
	kAboutWindow=0,		/* about box */
	kOtherProductsWindow,
	kHelpWindow,		/* help window */
	kMainWindow,		/* main document window */
	kBuildWindow		/* module editor window */
};

#define	NUM_WINDOWS		5		/* total number of windows (see above enum) */

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
	kCantLoadFile,
	kFileTooLarge,
	
	kModuleNotFound,
	kCantBuildReferenceList,
	kCantFindModulesFolder,
	kCantCreateNewModule,
	kCantOpenModule,
	kCantDeleteReference,
	kCantDeleteInstantiation,
	kDuplicateReferenceName,
	kCantCreateReference,
	kCantCreateInstantiation,
	kCantReplaceReference,
	kCantReplaceInstantiation,
	
	kParserNullReferenceName,
	kParserReferenceNotFound,
	kParserBadChoiceFormat,
	kParserBadOptionalFormat,
	kParserNoInstantiations,
	kParserIllegalBackslash
};

typedef short		ErrorTypes;

extern	Boolean			gAlwaysResolve;
extern	short			gSpeedDelay;
extern	FSSpec			gModuleFS;
extern	Boolean			gUseDefault;
extern	Boolean			gDynamicScroll;
extern	Boolean			gStartFromTop;
extern	Boolean			gIgnoreCase;
extern	Str255			gFindString;
extern	Str255			gReplaceString;
extern	short			gLastFindPosition;
extern	Boolean			gShowAllRefs;
extern	Boolean			gIconifyMenus;
extern	Boolean			gShowMessageBox;
extern	Boolean			gShowToolbar;
extern	Boolean			gFastResolve;
extern	Boolean			gIgnoreErrors;

#endif
