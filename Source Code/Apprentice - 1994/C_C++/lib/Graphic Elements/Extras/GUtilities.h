/*
	GUtilities.h
	
	General-purpose utility routines
	
*/

#ifndef GUTIL
#define GUTIL

//Get precompiled symbols if compiling under MPW
#ifdef applec
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#else
#include <Packages.h>
#endif

#define	mApple					128		/* Apple menu */
#define rUserAlert				129		/* General purpose note alert */

//Globals set by InitSystem

extern Str255		gAppName;
extern OSType		gSignature;
extern short		gAppResRef;
extern Boolean		gInBackground;

#ifdef __cplusplus
extern "C" {
#endif


//Init QD, Fonts, Windows, etc. and call MoreMasters mbCount times
void InitSystem( short mbCount );


//Loads menubar specified by mBarNum, adds DAs to Apple menu, displays menubar
Boolean LoadMenus( short mBarNum );

void TellUser(Str255 what, short errNum);

char *C2PStrCpy(char *Cstr, Str255 Pstr);

Boolean GetOpenFSSpec(SFTypeList *types, short nTypes, FSSpec *fileSpec);

Boolean GetSaveFSSpec(SFTypeList *types, FSSpec *fileSpec);

#ifdef __cplusplus
}
#endif


#endif