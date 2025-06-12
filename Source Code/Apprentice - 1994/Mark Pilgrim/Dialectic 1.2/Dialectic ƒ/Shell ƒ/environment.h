/**********************************************************************\

File:		environment.h

Purpose:	This is the header file for environment.c

\**********************************************************************/

extern	Boolean			gHasAppleEvents;
extern	Boolean			gHasFSSpecs;
extern	Boolean			gStandardFile58;
extern	Boolean			gHasColorQD;
extern	Boolean			gHasPowerManager;
extern	Boolean			gHasNotificationManager;
extern	Boolean			gSystemSevenOrLater;
extern	Boolean			gWaitNextEventAvailable;
extern	Boolean			gHasGestalt;

extern	int				gForegroundWaitTime;
extern	int				gBackgroundWaitTime;
extern	Boolean			gIsInBackground;
extern	Boolean			gInProgress;
extern	Boolean			gDone;
extern	Boolean			gFrontWindowIsOurs;

Boolean InitTheEnvironment(void);
