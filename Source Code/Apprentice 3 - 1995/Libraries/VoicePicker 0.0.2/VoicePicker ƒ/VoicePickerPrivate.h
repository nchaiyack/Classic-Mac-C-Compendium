/*
	File:			VoicePickerPrivate.h

	Contains:		Private header file for VoicePicker library routines.
				
	Written by:	Luigi Belverato
				P.O. Box 19,
				20080 Basiglio MI,
				Italy

	Copyright:	©1995 Luigi Belverato
	
	Change History (most recent first):
				06/23/95	0.0.1

	Notes: 		This code uses Apple's Universal Interfaces for C version 2.01f.
	
				Send bug reports to lbelvera@micronet.it
				(if I don't reply within a few days use snail mail as I might have changed internet provider)
*/

//-----------------------------------------------------------------------
// this text document was formatted with Geneva 9, tab size 4

#define kBaseResID		5000

//Strings
#define kSTRxResID		kBaseResID
#define kSTRxVoiceUnav	1
#define kSTRxMale		2
#define kSTRxNeuter	3
#define kSTRxFemale	4
#define kSTRxMessage	5

//Default values. These are used if the VPpr resource is not available.
#define kVPprResType	'VPpr'
#define kVPprResID		kBaseResID

#define krateMin		10
#define krateMax		400
#define kpitchMin		0
#define kpitchMax		100
#define kDialWait		6

//VoicePicker DLOG
#define kDLOGVoicePicker	kBaseResID

#define kVPokBtn		1
#define kVPcancelBtn	2
#define kVPvoicePopup	3
#define kVPmsgStr		4
#define kVPgenderStr	5
#define kVPageStr		6
#define kVPlanguageStr	7
#define kVPregionStr	8
#define kVPrateStat		9
#define kVPpitchStat		10
#define kVPrateDial		11
#define kVPpitchDial		12
#define kVPtryBtn		13
#define kVPstopBtn		14
#define kVPsampleEdit	15

#define kNoArrowPict	kBaseResID
#define kUpArrowPict	kBaseResID+1
#define kDownArrowPict	kBaseResID+2

#define kNoArrow		0
#define kUpArrow		1
#define kDownArrow	2


typedef struct VoiceList VoiceList, **VoiceListH;
struct VoiceList
{
	VoiceSpec		voice;
	VoiceListH		previousH;
	VoiceListH		nextH;
};

typedef struct VPpreferences VPpreferences, **VPpreferencesH;
struct VPpreferences
{
	short 	rateMin;
	short 	rateMax;
	short	pitchMin;
	short 	pitchMax;
	short 	dialWait;
};

//Function Prototypes

static void BuildVoicesList(	VoiceListH			*firstVoiceH,
						short			*lastPopupValue,
						VoiceData			*passedVoiceData);
				
static void BuildVoicesMenu(	MenuHandle		theMenu,
						VoiceListH			*firstVoiceH);
					
static void GetIndVoiceInfo(	short			item,
						VoiceDescription	*info,
						VoiceListH			firstVoiceH);
					
static void AdjustItems(		DialogPtr			theDialog,
						VoiceListH			firstVoiceH,
						VoiceData			*passedVoiceData,
						SpeechChannel		*channel,
						Boolean			usePassedValues);
					
static void DestroyVoicesList(	VoiceListH			firstVoiceH);

static Boolean PointerInside(	Rect				*theRect);

static void ChangeButton(		Rect				theRect,
						short			thePictID);
					
static void AdjustScrollerText(	DialogPtr			theDialog,
						short			which,
						long				value,
						SpeechChannel		channel);
					
static pascal void DialProc(	DialogPtr			theDialog,
						short			theItem);
					
static void HandleDial(		DialogPtr			theDialog,
						short			whichDial,
						SpeechChannel		channel,
						VoiceData			*passedVoiceData);
					
static Boolean IsCompatible(	void);

static void GetDefault(		void);

/* end of file: VoicePickerPrivate.h */