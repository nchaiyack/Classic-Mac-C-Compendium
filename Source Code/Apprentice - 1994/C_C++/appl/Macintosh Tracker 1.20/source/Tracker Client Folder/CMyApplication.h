/* CMyApplication.h */

#pragma once

#include "CApplication.h"
#include <Aliases.h>

#define TRACKERSERVER68020CREATOR ('�Tsp')
#define FRANKSEIDECREATOR ('�Tsq')
#define TRACKERSERVER68000CREATOR ('�Tsr')
#define ROSSETCREATOR ('�Tss')

#define ControlEventClass ('�Tsp')

#define ControlEvent ('�Ctl')
#define keyKeyPressCharacter ('�Key')

#define ErrorEvent ('�Err')
#define keyErrorIDNum ('�Er#')

/* modifiers for open event */
#define keyAntiAliasing ('�Saa')
#define keyStereoOn ('�Sso')
#define keySamplingRate ('�Ssr')
#define keyNumRepeats ('�Snr')
#define keySpeed ('�Ssp')
#define keyStereoMix ('�Ssm')
#define keyLoudness ('�Svl')
#define keyNumBits ('�Bit')
#define keySenderPSN ('�Psn')


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
