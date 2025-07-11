/*
	File:		TextServices.h

	Copyright:	� 1983-1993 by Apple Computer, Inc.
				All rights reserved.

	Version:	System 7.1 for ETO #11
	Created:	Tuesday, March 30, 1993 18:00
	
	Modified:	Added Gestalt call for 'tsmv' as in Technote
				Fixed Prototype for Init and Close
				- fprefect@engin.umich.edu, 4/29/94

*/

#ifndef __TEXTSERVICES__
#define __TEXTSERVICES__

#ifndef __TYPES__
#include	<Types.h>
#endif

#ifndef __EVENTS__
#include	<Events.h>
#endif

#ifndef __MENUS__
#include	<Menus.h>
#endif

#ifndef __APPLEEVENTS__
#include	<AppleEvents.h>
#endif

#ifndef __ERRORS__
#include	<Errors.h>
#endif


#ifndef __COMPONENTS__
#include	<Components.h>
#endif


#define		gestaltTSMgrVersion			'tsmv'

#define		kTSMVersion					1				/* Version of the Text Services Manager */
#define		kTextService				'tsvc'		/* component type for the component description */
#define		kInputMethodService		'inpm'		/* component subtype for the component description */

#define		bTakeActiveEvent			15				/* bit set if the component takes active event */
#define		bScriptMask					0x00007F00	/* bit 8 - 14 */
#define		bLanguageMask				0x000000FF	/* bit 0 - 7  */
#define		bScriptLanguageMask		bScriptMask+bLanguageMask	/* bit 0 - 14  */
/*	�� ************************************** ��  */


/* Hilite styles ... */
typedef enum {
	kCursorPosition				=	1,					/* specify cursor position */
	kRawText							=	2,					/* specify range of raw text */
	kSelectedRawText				=	3,					/* specify range of selected raw text */
	kConvertedText					=	4,					/* specify range of converted text */
	kSelectedConvertedText		=	5					/* specify range of selected converted text */
} HiliteStyleType;

/*	�� ************************************** ��  */


/* Apple Event constants ... */

/* Event class ... */

#define		kTextServiceClass			kTextService

/* event ID ... */

#define		kUpdateActiveInputArea	'updt'		/* update the active Inline area */	
#define		kPos2Offset					'p2st'		/* converting global coordinates to char position */
#define		kOffset2Pos					'st2p'		/* converting char position to global coordinates */
#define		kShowHideInputWindow		'shiw'		/* show or hide the input window */

/* Event keywords ... */

#define		keyAETSMDocumentRefcon	'refc'		/* TSM document refcon, typeLongInteger		<#45> */


#define		keyAEServerInstance		'srvi'		/* component instance */
#define		keyAETheData				'kdat'		/* typeText */
#define		keyAEScriptTag				'sclg'		/* script tag */
#define		keyAEFixLength				'fixl'		/* fix len ?? */
#define		keyAEHiliteRange			'hrng'		/* hilite range array */
#define		keyAEUpdateRange			'udng'		/* update range array */
#define		keyAEClauseOffsets		'clau'		/* Clause Offsets array */

#define		keyAECurrentPoint			'cpos'		/* current point */
#define		keyAEDragging				'bool'		/* dragging falg */
#define		keyAEOffset					'ofst'		/* offset */
#define		keyAERegionClass			'rgnc'		/* region class */
#define		keyAEPoint					'gpos'		/* current point */
#define		keyAEBufferSize			'buff'		/* buffer size to get the text */
#define		keyAERequestedType		'rtyp'		/* requested text type */
#define		keyAEMoveView				'mvvw'		/* move view flag */
#define		keyAELength					'leng'		/* length */
#define		keyAENextBody				'nxbd'		/* next or previous body */


/* optional keywords for Offset2Pos					-- 28Mar92 <#38> */
#define		keyAETextFont				'ktxf'
#define		keyAETextPointSize		'ktps'
#define		keyAETextLineHeight		'ktlh'
#define		keyAETextLineAscent		'ktas'
#define		keyAEAngle					'kang'


/* optional keywords for Pos2Offset					-- 20May92 <#44> */
#define		keyAELeftSide				'klef'		/* type Boolean */

/* optional keywords for kShowHideInputWindow	-- <#48> */
#define		keyAEShowHideInputWindow 'shiw'		/* type Boolean */

/* for PinRange  */
#define		keyAEPinRange				'pnrg'		/* <#49> */

/* Desc type ... */

#define		typeComponentInstance	'cmpi'		/* server instance */
#define		typeTextRangeArray		'tray'		/* text range array */
#define		typeOffsetArray			'ofay'		/* offset array */
#define		typeIntlWritingCode		'intl'		/* script code */
#define		typeQDPoint					'QDpt'		/* QuickDraw Point */
#define		typeAEText					'tTXT'		/* Apple Event text */
#define		typeText						'TEXT'		/* Plain text */

#define		typeTextRange				'txrn'		/* <#49> */



/* error codes */

#define	tsmComponentNoErr				0				// component result = no error


#define	tsmUnsupScriptLanguageErr	-2500			//
#define	tsmInputMethodNotFoundErr	-2501			//
#define	tsmNotAnAppErr					-2502			// not an application error
#define	tsmAlreadyRegisteredErr		-2503			// want to register again error
#define	tsmNeverRegisteredErr		-2504			// app never registered error (not TSM aware)
#define	tsmInvalidDocIDErr			-2505			// invalid TSM documentation id
#define	tsmTSMDocBusyErr				-2506			// document is still active
#define	tsmDocNotActiveErr			-2507			// document is NOT active
#define	tsmNoOpenTSErr					-2508			// no open text service
#define	tsmCantOpenComponentErr		-2509			// can't open the component
#define	tsmTextServiceNotFoundErr	-2510			// no text service found
#define	tsmDocumentOpenErr			-2511			// there are open documents
#define	tsmUseInputWindowErr			-2512			// not TSM aware because we are using input window
#define	tsmTSHasNoMenuErr				-2513			// the text service has no menu
#define	tsmTSNotOpenErr				-2514			// text service is not open
#define	tsmComponentAlreadyOpenErr	-2515			// text service already opened for the document

#define	tsmInputMethodIsOldErr		-2516			// returned by GetDefaultInputMethod
#define	tsmScriptHasNoIMErr			-2517			// script has no imput method or is using old IM.
#define	tsmUnsupportedTypeErr		-2518			// unSupported interface type error
#define	tsmUnknownErr					-2519			// any other errors




/* Desc type constants */

typedef enum {
	kTSMOutsideOfBody					= 1,
	kTSMInsideOfBody					= 2,
	kTSMInsideOfActiveInputArea	= 3
} AERegionClassType;

typedef enum {
	kNextBody						= 1,
	kPreviousBody					= 2
} AENextBodyType;


/* Apple Event error definitions */

typedef enum {
	errOffsetInvalid				= -1800,
	errOffsetIsOutsideOfView	= -1801,
	errTopOfDocument				= -1810,
	errTopOfBody					= -1811,
	errEndOfDocument				= -1812,
	errEndOfBody					= -1813
} AppleEventErrorType;

/*	�� ************************************** ��  */


struct TextRange {					/* typeTextRange 		'txrn' */
	long	fStart;
	long	fEnd;
	short	fHiliteStyle;
};
typedef struct TextRange TextRange;
typedef TextRange *TextRangePtr;
typedef TextRangePtr *TextRangeHandle;


struct TextRangeArray {				/* typeTextRangeArray	'txra' */
	short		fNumOfRanges;			/* specify the size of the fRange array */
	TextRange	fRange[1];			/* when fNumOfRanges > 1, the size of this array has to be calculated */
};
typedef struct TextRangeArray TextRangeArray;
typedef TextRangeArray *TextRangeArrayPtr;
typedef TextRangeArrayPtr *TextRangeArrayHandle;


struct OffsetArray {					/* typeOffsetArray		'offa' */
	short	fNumOfOffsets;				/* specify the size of the fOffset array */
	long	fOffset[1];					/* when fNumOfOffsets > 1, the size of this array has to be calculated */
};
typedef struct OffsetArray OffsetArray;
typedef OffsetArray *OffsetArrayPtr;
typedef OffsetArrayPtr *OffsetArrayHandle;

/*	�� ************************************** ��  */


/* extract Script/Language code from Component flag ... */
#define		mGetScriptCode(cdRec)		((ScriptCode)	((cdRec.componentFlags & bScriptMask) >> 8))
#define		mGetLanguageCode(cdRec)		((LangCode)		cdRec.componentFlags & bLanguageMask)


typedef		void	*TSMDocumentID;
/*	�� ************************************** ��  */


/*
 *	Text Service Info List
 */
struct TextServiceInfo {
 Component			fComponent;
 Str255				fItemName;
};

typedef struct TextServiceInfo TextServiceInfo;
typedef TextServiceInfo *TextServiceInfoPtr;


struct TextServiceList {
 short					fTextServiceCount;				/* number of entries in the 'fServices' array */
 TextServiceInfo		fServices[1];						/* Note: array of 'TextServiceInfo' records follows */
};

typedef struct TextServiceList TextServiceList;
typedef TextServiceList *TextServiceListPtr; 
typedef TextServiceListPtr *TextServiceListHandle; 


struct ScriptLanguageRecord {
 ScriptCode		fScript;
 LangCode		fLanguage;
};

typedef struct ScriptLanguageRecord ScriptLanguageRecord;


struct ScriptLanguageSupport {
 short						fScriptLanguageCount;		/* number of entries in the 'fScriptLanguageArray' array */
 ScriptLanguageRecord	fScriptLanguageArray[1];	/* Note: array of 'ScriptLanguageRecord' records follows */
};

typedef struct ScriptLanguageSupport ScriptLanguageSupport;
typedef ScriptLanguageSupport *ScriptLanguageSupportPtr; 
typedef ScriptLanguageSupportPtr *ScriptLanguageSupportHandle; 

/*	�� ************************************** ��  */




#define	TSMTrapNum		0xAA54

/* ��	Text Services Manager function selectors ... */

/* ��	High level routines ... */
#define	kNewTSMDocument				0		// create new TSM aware document
#define	kDeleteTSMDocument			1		// delete TSM aware document
#define	kActivateTSMDocument			2		// activate TSM aware document
#define	kDeactivateTSMDocument		3		// deactivate TSM aware document
#define	kTSMEvent						4		// pass all events to TSM
#define	kTSMMenuSelect					5		// pass menu selection to TSM
#define	kSetTSMCursor					6		// set the cursor
#define	kFixTSMDocument				7		// terminate text services
#define	kGetServiceList				8		// get all text service components
#define	kOpenTextService				9		// open  the text service
#define	kCloseTextService				10		// close the text service
#define	kSendAEFromTSMComponent		11		// send Apple Event to client


/* ��	Utilities ... */

#define	kSetDefaultInputMethod		12		// 
#define	kGetDefaultInputMethod		13		// 0x0D
#define	kSetTextServiceLanguage		14		// Called by Script Mgr
#define	kGetTextServiceLanguage		15		// 
#define	kUseInputWindow				16		// 
#define	kNewServiceWindow				17		// 
#define	kCloseServiceWindow			18		// 
#define	kGetFrontServiceWindow		19		// 


#define	kInitTSMAwareApplication	20		// claim TSM aware application
#define	kCloseTSMAwareApplication	21		// application deregisters itself as TSM aware

#define	kFindServiceWindow			23


/*	�� ************************************** ��  */




/* High level TSM routines .. */

pascal OSErr NewTSMDocument(short numOfInterface,		OSType supportedInterfaceTypes[],
									 TSMDocumentID *idocID,		long	 refcon)
= { 0x303C,kNewTSMDocument, TSMTrapNum };					/* MOVE.W	#selector,D0		*/

pascal OSErr DeleteTSMDocument(TSMDocumentID idocID)
= { 0x303C, kDeleteTSMDocument, TSMTrapNum };

pascal OSErr ActivateTSMDocument(TSMDocumentID idocID)
= { 0x303C, kActivateTSMDocument, TSMTrapNum };

pascal OSErr DeactivateTSMDocument(TSMDocumentID idocID)
= { 0x303C, kDeactivateTSMDocument, TSMTrapNum };

pascal Boolean TSMEvent(EventRecord *event) 									// pass all events to TSM (removed TSMDocumentID #35)
= { 0x303C, kTSMEvent, TSMTrapNum };

pascal Boolean TSMMenuSelect(long menuResult) 								// pass menu selection to TSM (removed TSMDocumentID #35)
= { 0x303C, kTSMMenuSelect, TSMTrapNum };

pascal Boolean SetTSMCursor(Point mousePos) 									// set the cursor (removed TSMDocumentID #35)
= { 0x303C, kSetTSMCursor, TSMTrapNum };

pascal OSErr FixTSMDocument(TSMDocumentID idocID) 							// terminate text services
= { 0x303C, kFixTSMDocument, TSMTrapNum };

pascal OSErr GetServiceList(short			numOfInterface,
							OSType						supportedInterfaceTypes[],
							TextServiceListHandle	*serviceInfo,
							long							*seedValue) 					// get all text services
= { 0x303C, kGetServiceList, TSMTrapNum };

pascal OSErr OpenTextService(TSMDocumentID idocID, Component aComponent, ComponentInstance *aComponentInstance) 
= { 0x303C, kOpenTextService, TSMTrapNum };									// open the text service (return instance)


pascal OSErr CloseTextService(TSMDocumentID idocID, ComponentInstance aComponentInstance)
= { 0x303C, kCloseTextService, TSMTrapNum };								 	// close the text service (return instance)



/* Sending callback AppleEvents ... */
pascal OSErr SendAEFromTSMComponent(AppleEvent		*theAppleEvent,	// should be a pointer
							AppleEvent				*reply, 
							AESendMode				sendMode,
							AESendPriority			sendPriority,
							long						timeOutInTicks,
							IdleProcPtr				idleProc,
							EventFilterProcPtr	filterProc )
= { 0x303C, kSendAEFromTSMComponent, TSMTrapNum };

pascal OSErr InitTSMAwareApplication(void) 										// app registers itself as TSM aware
= { 0x303C, kInitTSMAwareApplication, TSMTrapNum };


pascal OSErr CloseTSMAwareApplication(void) 										// app deregisters itself when quit
= { 0x303C, kCloseTSMAwareApplication, TSMTrapNum };




/* ��  Utilities ... */
pascal OSErr SetDefaultInputMethod ( Component  ts, ScriptLanguageRecord *slRecordPtr )
= { 0x303C, kSetDefaultInputMethod, TSMTrapNum };

pascal OSErr GetDefaultInputMethod ( Component *ts, ScriptLanguageRecord *slRecordPtr )
= { 0x303C, kGetDefaultInputMethod, TSMTrapNum };

pascal OSErr SetTextServiceLanguage( ScriptLanguageRecord *slRecordPtr )
= { 0x303C, kSetTextServiceLanguage, TSMTrapNum };

pascal OSErr GetTextServiceLanguage( ScriptLanguageRecord *slRecordPtr )
= { 0x303C, kGetTextServiceLanguage, TSMTrapNum };

pascal OSErr UseInputWindow( TSMDocumentID idocID, Boolean useWindow )
= { 0x303C, kUseInputWindow, TSMTrapNum };

pascal OSErr NewServiceWindow(void			*wStorage,
								const Rect			*boundsRect,
								ConstStr255Param	title,
								Boolean				visible,
								short					theProc,
								WindowPtr			behind,
								Boolean				goAwayFlag,
								ComponentInstance	ts,
								WindowPtr			*window )
= { 0x303C, kNewServiceWindow, TSMTrapNum };

pascal OSErr CloseServiceWindow(WindowPtr window )
= { 0x303C, kCloseServiceWindow, TSMTrapNum };

pascal OSErr GetFrontServiceWindow(WindowPtr *window )
= { 0x303C, kGetFrontServiceWindow, TSMTrapNum };

pascal short FindServiceWindow(Point thePoint,WindowPtr *theWindow)
= { 0x303C, kFindServiceWindow, TSMTrapNum };



/* ��	Low level routines which are dispatched directly to the Component Manager ... */

#define	kCMGetScriptLangSupport		0x0001		// Component Manager call selector 1
#define	kCMInitiateTextService		0x0002		// Component Manager call selector 2
#define	kCMTerminateTextService		0x0003		// Component Manager call selector 3
#define	kCMActivateTextService		0x0004		// Component Manager call selector 4
#define	kCMDeactivateTextService	0x0005		// Component Manager call selector 5
#define	kCMTextServiceEvent			0x0006		// Component Manager call selector 6
#define	kCMGetTextServiceMenu		0x0007		// Component Manager call selector 7
#define	kCMTextServiceMenuSelect	0x0008		// Component Manager call selector 8
#define	kCMFixTextService				0x0009		// Component Manager call selector 9
#define	kCMSetTextServiceCursor		0x000A		// Component Manager call selector 10
#define	kCMHidePaletteWindows		0x000B		// Component Manager call selector 11			<#37>


/* ��  Low level TSM routines ... */

pascal ComponentResult GetScriptLanguageSupport(ComponentInstance ts, ScriptLanguageSupportHandle *scriptHdl)
 = {0x2F3C,0x04,kCMGetScriptLangSupport,0x7000,0xA82A};			/* selector = 1 */


pascal ComponentResult InitiateTextService(ComponentInstance ts)
 = {0x2F3C,0x00,kCMInitiateTextService,0x7000,0xA82A};			/* selector = 2 */


pascal ComponentResult TerminateTextService(ComponentInstance ts)
 = {0x2F3C,0x00,kCMTerminateTextService,0x7000,0xA82A};			/* selector = 3 */


pascal ComponentResult ActivateTextService(ComponentInstance ts)
 = {0x2F3C,0x00,kCMActivateTextService,0x7000,0xA82A};			/* selector = 4 */


pascal ComponentResult DeactivateTextService(ComponentInstance ts)
 = {0x2F3C,0x00,kCMDeactivateTextService,0x7000,0xA82A};			/* selector = 5 */


pascal ComponentResult TextServiceEvent(ComponentInstance ts, short numOfEvents, EventRecord *event)
 = {0x2F3C,0x06,kCMTextServiceEvent,0x7000,0xA82A};				/* selector = 6 */


pascal ComponentResult GetTextServiceMenu(ComponentInstance ts, MenuHandle *serviceMenu)
 = {0x2F3C,0x4,kCMGetTextServiceMenu,0x7000,0xA82A};				/* selector = 7 */


pascal ComponentResult TextServiceMenuSelect(ComponentInstance ts, MenuHandle serviceMenu, short item)
 = {0x2F3C,0x06,kCMTextServiceMenuSelect,0x7000,0xA82A};			/* selector = 8 */


pascal ComponentResult FixTextService(ComponentInstance ts)
 = {0x2F3C,0x00,kCMFixTextService,0x7000,0xA82A};					/* selector = 9 */


pascal ComponentResult SetTextServiceCursor(ComponentInstance ts, Point mousePos)
 = {0x2F3C,0x04,kCMSetTextServiceCursor,0x7000,0xA82A};			/* selector = 10 */


pascal ComponentResult HidePaletteWindows(ComponentInstance ts)
 = {0x2F3C,0x00,kCMHidePaletteWindows,0x7000,0xA82A};				/* selector = 11					<#37> */



#endif
