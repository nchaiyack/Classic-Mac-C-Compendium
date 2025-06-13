#include <Types.h>
#include <Memory.h>
#include <QuickDraw.h>
#include <OSUtils.h>
#include <ToolUtils.h>
#include <Menus.h>
#include <Packages.h>
#include <Traps.h>
#include <Files.h>
#include <Aliases.h>
#include <AppleEvents.h>
#include <GestaltEqu.h>
#include <Processes.h>
#include <Fonts.h>
#include <OSEvents.h>
#include <Resources.h>
#include <Desk.h>
#include <Finder.h>

#include "standardgetfile.h"
#include "boxmakergetfile.h"

#include "boxmaker constants.h"
#include "boxmaker.h"
#include "preferences.cp"
#include "NameFileLocker.h"

#pragma template_access public

void main();

void main()
{
	Handle theSettings = Get1Resource( 'what', 128);
	if( theSettings != 0)
	{
		HLock( theSettings);
		char *set = *theSettings;
		
		const int FileLockAction = set[ 0];
		const int NameLockAction = set[ 1];
		
		ReleaseResource( theSettings);
		NameFileLocker it( FileLockAction, NameLockAction);
		it.run();
	} else {
		DebugStr( "\pCould not get 'what' resource");
	}
}

NameFileLocker::NameFileLocker( int theFileAction, int theNameAction)
	: boxmaker( 3000)
	, fileAction( theFileAction)
	, nameAction( theNameAction)
{	
}

void NameFileLocker::OpenDoc( Boolean opening)
{
	if( opening)
	{
		FInfo theInfo;
		
		switch( fileAction)
		{
			case kLockFile:
				(void)FSpSetFLock( &theFSSpec);
				break;
			
			case kUnlockFile:
				(void)FSpRstFLock( &theFSSpec);
				break;

			case kIgnoreFile:
				;
		}
		if( nameAction != kIgnoreName)
		{
			if( FSpGetFInfo( &theFSSpec, &theInfo) == noErr)
			{
				theInfo.fdFlags &= ~kHasBeenInited;
				
				if( nameAction == kLockName)
				{
					theInfo.fdFlags |= kNameLocked;
				} else {
					theInfo.fdFlags &= ~kNameLocked;
				}
				(void)FSpSetFInfo( &theFSSpec, &theInfo);
			}
		}
	}
}
