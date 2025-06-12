/* CMyApplication.h */

#pragma once

#include "CApplication.h"
#include <Aliases.h>

#define TRACKERSERVER68020CREATOR ('ºTsp')
#define FRANKSEIDECREATOR ('ºTsq')
#define TRACKERSERVER68000CREATOR ('ºTsr')
#define ROSSETCREATOR ('ºTss')

#define ControlEventClass ('ºTsp')

#define ControlEvent ('ºCtl')
#define keyKeyPressCharacter ('ºKey')

#define ErrorEvent ('ºErr')
#define keyErrorIDNum ('ºEr#')

/* modifiers for open event */
#define keyAntiAliasing ('ºSaa')
#define keyStereoOn ('ºSso')
#define keySamplingRate ('ºSsr')
#define keyNumRepeats ('ºSnr')
#define keySpeed ('ºSsp')
#define keyStereoMix ('ºSsm')
#define keyLoudness ('ºSvl')
#define keyNumBits ('ºBit')
#define keySenderPSN ('ºPsn')


#define FatalErrorUnknown (0)
#define FatalErrorOutOfMemory (1)
#define FatalErrorInternalError (2)
#define FatalErrorCantOpenCompressedFiles (3)
#define FatalErrorCouldntOpenFile (4)
#define FatalErrorCouldntCloseFile (5)
#define FatalErrorNotASong (6)
#define FatalError68020NeededID (7)


/* this is to override anything you want to change about the default application */
struct	CMyApplication	:	CApplication
	{
		/* */			CMyApplication();
		void			InitMenuBar(void);
		void			DispatchOpenDocument(FSSpec* TheFSSpec);
		void			DoMenuOpen(void);
		void			DoMenuNew(void);
		void			EnableMenuItems(void);

		void			LaunchTracker(OSType TrackerCreator);
		void			KillPlayer(ProcessSerialNumber PlayerSN);
		void			SendMessage(ProcessSerialNumber PlayerSN, AppleEvent* Message);
		static	pascal	OSErr	MyHandleOBIT(AppleEvent* theAppleEvent,
								AppleEvent* reply, long handlerRefcon);
		static	pascal	OSErr	MyHandleERROR(AppleEvent* theAppleEvent,
								AppleEvent* reply, long handlerRefcon);
	};
