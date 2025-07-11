#ifndef __GLOBAL__
#define __GLOBAL__

#include "MetaGlobal.h"

//#define BREAK_ON_ENTRY

#define kACURResource			27000

#define kMenubarCDEF			90
#define kZeroCDEF				96

#define kExtraDITL				27000
	#define kMenubar				1			// add to result of CountDITL
#define kAboutDialog			27001
	#define kAboutOK				1
	#define kAboutTitle				2
	#define kAuthorText				3
	#define kVersionText			4
#define kFindDialog				27002
	#define	kFindOK					1
	#define kFindCancel				2
	#define kFindLabel				3
	#define kFindEditText			4
#define kNewFolderDialog		27003
	#define kNewFolderOK			1
	#define kNewFolderCancel		2
	#define kNewFolderLabel			3
	#define kNewFolderEditText		4
#define kNoFilesFoundAlert		27004
	#define kNoFilesOK				1
	#define kNoFilesText			2
#define kErrorAlert				27005
	#define kErrorOK				1
	#define kErrorExplain			2
	#define kErrorNumber			3
#define kTrashAlert				27006
#define kConfigureDialog		27007
	#define kConfigOK				1
	#define kConfigCancel			2
	#define kConfigLinePICT1		3
	#define kConfigLinePICT2		4
	#define kConfigLinePICT3		5
	#define kConfigLinePICT4		6
	#define kConfigSortText			7
	#define kConfigNameRadio		8
	#define kConfigAgeRadio			9
	#define kConfigPermItemsAtTop	10
	#define kConfigHowManyText		11
	#define kConfigHowManyUser		12
	#define kConfigArrowPICT		13
	#define kConfigHotKeyText		14
	#define kConfigHotKeyCheck		15
	#define kConfigHotKeyUser		16
	#define kConfigNoCEText			17
	#define kConfigTakenText		18
	#define kConfigIncludeWhatText	19
	#define kConfigIncludeVolCheck	20
	#define kConfigIncludeParCheck	21
#define kPermanentDialog		27008
	#define kPermOK					1
	#define kPermCancel				2
	#define kPermText				3
	#define kPermListUser			4
#define kGetFolderDialog		27009
	#define	kSelectItem				10

#define	kOKIcon					27000
#define kPrefsIcon				27001
#define kDocumentIcon			27002
#define	kBadIcon				27003
#define kCheckBoxOffIcon		27004
#define kCheckBoxOnIcon			27005

#define kCheckLDEF				90

#define kMBAR					27000
	#define kDirectoryMenuResID		27000
	#define kFileMenuResID			27001
	#define kOtherMenuResID			27002

#define kArrowPICT				27001
#define kDownArrowPICT			27002
#define kUpArrowPICT			27003
#define kGrayBox				27004

#define kProcShowINIT			0
#define kProcExternalInit		1
#define kProcNotification		2

#define kDownArrowResID			-3990		// from system file
#define kDownArrow					0			// zero based

#define kStrings				27000
	#define kPrefsFile				1
	#define kWordMore				2
	#define kSelectString			3
	#define kDesktopName			4
#define kErrorStrings			27001
	#define kCreatingDirectory		1
	#define kSearching				2
	#define kNoStartVolume			3
	#define kGetDirInfo				4
	#define kReinstallingPack3		5
	#define kArrayIndexOOB			6
	#define kCorruptList			7
	#define kCantMakeMenu			8
	#define kAddingBogMenuName		9
	#define kCantFindFileForMenu	10
	#define kPutOnPuppet			11
	#define kAddingEmptyName		12
	#define kCreatingWD				13
	#define kMenuNILInAddCommon		14
	#define kMEOCatInfoErr			15
	#define kMEOFCBInfoErr			16
#define kErrorInstallStrings	27500
	#define kBaseText				1
	#define kErrNoExternInit		2
	#define kErrNeedSystem7			3
	#define kOptionKeyWasDown		4
	#define kFindFolderFailed		5
	#define kErrGetFCBInfo			6
	#define kMemFullErr				7


#define kOpenAnyFile		'****'
#define kApplicationSig		'APPL'
#define kUnknownSig			'????'


typedef struct {
	long		typeOrDirID;
	short		driveNumber;
	short		vRefNum;
	long		parID;
	char		fileAttributes;
	char		eightOh;
	short		finderFlags;
	short		sicnID;
	short		textJustThing;		// $FF00 or $00FF
	short		readWritePriv;
	short		scriptStuff[4];
	char		name[64];
} StdFileListRec, *StdFileListRecPtr;

typedef struct {
	long		typeOrDirID;
	long		signatureOrSomethingElse;		// Mac Easy Open adds this
	short		driveNumber;
	short		vRefNum;
	long		parID;
	char		fileAttributes;
	char		eightOh;
	short		finderFlags;
	short		sicnID;
	short		textJustThing;		// $FF00 or $00FF
	short		readWritePriv;
	short		scriptStuff[4];
	char		name[64];
} StdFileListRec2, *StdFileListRecPtr2;


typedef struct AuxControlRec {
	struct AuxControlRec**	nextAux;
	ControlHandle			theControl;
	StdHeaderHdl			realCDEF;
} AuxControlRec, *AuxControlRecPtr, **AuxControlRecHdl;


#pragma parameter __D0 HiWrd(__D0)
short HiWrd(long)
	= { 0x4840 };			// SWAP D0
#define LoWrd(aLong)	((short)(aLong))
long MergeShorts(short high, short low)
	= { 0x2017 };			// MOVE.L     (A7),D0

#define IsGetFile(selector) ((selector&1)==0)
#define IsCustomCall(selector) (((selector-1)&2)==0)
#define IsNewCall(selector) (selector >= standardPutFile)


extern void main(void);
#define SetUpA4()		do { asm { move.l a4,-(sp) } asm { lea main,a4 } } while (0)
#define RestoreA4()		do { asm { move.l (sp)+,a4 } } while (0)


extern FSSpec*			gMe;


/* Returns a pointer to the Dialog Manager's standard dialog filter */
#if 0
pascal OSErr GetStdFilterProc(ModalFilterProcPtr *theProc )
	= { 0x303C, 0x0203, 0xAA68 };

/* Indicates to the dialog manager which item is default.  Will then alias the return */
/* & enter keys to this item, and also bold border it for you (yaaaaa!) */		
pascal OSErr SetDialogDefaultItem(DialogPtr theDialog, short newItem) 
	= { 0x303C, 0x0304, 0xAA68 };		

/* Indicates which item should be aliased to escape or Command - . */
pascal OSErr SetDialogCancelItem(DialogPtr theDialog, short newItem)
	= { 0x303C, 0x0305, 0xAA68 };

/* Tells the dialog manager that there is an edit line in this dialog, and */ 
/* it should track and change to an I-Beam cursor when over the edit line */
pascal OSErr SetDialogTracksCursor(DialogPtr theDialog, Boolean tracks)
	= { 0x303C, 0x0306, 0xAA68 };
#endif

#if 0
typedef short DITLMethod;
enum {
	overlayDITL,
	appendDITLRight,
	appendDITLBottom
};

#define DITLGlue(selector) \
	{	0x3F3C, selector,	/* MOVE.W    #$0402,-(A7)	; push on selector		*/\
		0x204F,				/* MOVEA.L   A7,A0			; get pointer to parms	*/\
		0xA08B,				/* _CommToolboxDispatch								*/\
		0x544F }			/* ADDQ.W    #$2,A7			; remove the selector	*/

void		AppendDITL(DITLMethod method, Handle theHandle, DialogPtr theDialog)
	= DITLGlue(0x0402);

short		CountDITL(DialogPtr theDialog)
	= DITLGlue(0x0403);

void		ShortenDITL(short numberItems, DialogPtr theDialog)
	= DITLGlue(0x0404);

#endif

#endif