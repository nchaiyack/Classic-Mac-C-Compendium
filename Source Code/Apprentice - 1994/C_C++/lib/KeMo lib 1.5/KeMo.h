/*
 *  KeMo.h
 *
 *  Copyright (c) 1992,1993 Dan Costin.  All rights reserved.
 *
 */

#define KeMoAllDevs		0x0001		/* flags for KeMoSelect */
#define KeMoDev2		0x0004
#define KeMoKey			0x0004
#define KeMoDev3		0x0008
#define KeMoMouse 		0x0008
#define KeMoDev4		0x0010
#define KeMoTablet		0x0010
#define KeMoDev8		0x0100
#define KeMoDev9		0x0200
#define KeMoDevA		0x0400
#define KeMoDevB		0x0800
#define KeMoDevC		0x1000
#define KeMoDevD		0x2000
#define KeMoDevE		0x4000
#define KeMoDevF		0x8000
#define KeMoOneKey		0x00010000
#define KeMoAllKeys		0x00020000
#define KeMoOneMouse	0x00040000
#define KeMoAllMice		0x00080000

#define KeMoDown		0x00		/* flags and transitions for KeMoWait */
#define KeMoUp			0x01
#define KeMoUpDown		0x02
#define KeMoTimedOut	0x04

#define KeMoNoTimeOut 	-1L			/* no timeout for KeMoWait */

#define KeMoQuiet		0x0001		/* flags for KeMoInit */
#define KeMoNoAlert		0x0002
#define KeMoAltKeys		0x0004
#define KeMoNoKeys		0x0008
#define KeMoNoTimer		0x0010
#define KeMoNoSync		0x0020
#define KeMoNoTCheck	0x0040
#define KeMoNoCorrection	0x0080	/* flag for KeMoTimerStop */
#define KeMoNoMBarInit	0x0100
#define KeMoAvoidSysHeap	0x0200		/* use this flag if Synch initialization
											freezes up your 68040 or later */

#define KeMoErrorVeryOldSystem 		-501	/* errors */
#define KeMoErrorOldSystem 			-502
#define KeMoErrorNotADB 			-503
#define KeMoErrorNotInitialized		-504
#define KeMoErrorBadArgs			-505
#define KeMoErrorUnsuccessful		-506
#define KeMoErrorNoSuchDevice		-507
#define KeMoErrorTimerRunning		-508
#define KeMoErrorTimerNotRunning	-509
#define KeMoErrorTimerNotInit		-510
#define KeMoErrorTimerOff			-511
#define KeMoErrorImproperTest		-512
#define KeMoErrorNoHeapAlloc		-513
#define KeMoErrorNoSync				-514
#define KeMoErrorGestalt			-515
#define KeMoErrorNoCorrection		-516
#define KeMoErrorUnknownDevType		-517
#define KeMoErrorOneDevOnly			-518
#define KeMoErrorMBarHidden			-519
#define KeMoErrorMBarShows			-520
#define KeMoNotPressed				-521

#define KeMoErrorMisc				-599

	/* definitions for the more arbitrary ASCII codes returned by KeMoCode2Asc */
						
enum { K0 = 0, K1, K2, K3, K4, K5, K6, K7, K8, K9,
		F1 = 11, F2, F3, F4, F5, F6, F7, F8, F9, F10, F11, F12, F13, F14, F15,
		KSLASH = 'A', TAB, COMMAND, DELETE, ESC, /* NO F */
		POWER = 'G', HELP, /* NO I OR J */
		KENTER = 'K', CAPS, KMINUS, NUMCLEAR, OPTIONL, KPERIOD, KEQUAL,
		RETURN, SHIFTL, CTLL, PGUP, DOWN, PGDOWN, DEL, HOME, END,
		LEFT = '<', RIGHT = '>', UP = '^',
		SHIFTR = '@', CTLR = '#', OPTIONR = '$'
	};

			/* structure whose address is to be sent to KeMoWait */
typedef struct {
	char key, key2;
	char updown, updown2;
	} KeMoParms;
	
			/* function prototypes */
pascal long KeMoInit(short flags);
pascal long KeMoSelect(long flags);
pascal long KeMoReset(void);
pascal long KeMoAccuracy(void);
pascal long KeMoWait(short flags, long timeout, KeMoParms *parms);
pascal long KeMoDelay(long timeout);
pascal long KeMoTimerTest(void);
pascal long KeMoSetDeviceType(short devnum, short flags);

pascal long KeMoSync(short when);

pascal long KeMoTimerStart(void);
pascal long KeMoTimerStop(short flags);

pascal short KeMoCode2Asc(short code);

pascal long KeMoHideMBar(void);
pascal long KeMoShowMBar(void);

pascal long KeMoQuitAllApps(void);

/* available globals, set-up by calling KeMoInit() */
			/* KeMoDevArray: each position is 1 if there's a device at that address,
					0 otherwise, index from 0 to 15 */
extern char KeMoDevArray[], 
			/* KeMoDevType: each position has the original ADB device handler ID 
					for the device: usually 2 or 3 for keyboards, 1 for mouse,
					index from 0 to 15 */
	 		KeMoDevType[];

