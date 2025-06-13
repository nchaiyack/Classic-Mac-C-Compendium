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

#include "standardgetfile.h"
#include "boxmakergetfile.h"

#include "boxmaker constants.h"
#include "boxmaker.h"
#include "preferences.cp"
#include "touchshell.h"

#pragma template_access public

void main();

void main()
{
	touchsettings defaultSettings =
	{
		touchshell::kSecondButton,
		touchshell::kModDate
	};
	StringHandle prefsFileHandle = GetString( 128);
	touchshell it( *(Str255 *)*prefsFileHandle, defaultSettings);
	it.run();
}

touchshell::touchshell( Str255 prefsFileName, touchsettings &defaultsettings)
		: boxmaker( 3000)
		, touchprefs( prefsFileName, defaultsettings)
{	
	Handle times = Get1Resource( 'tims', 128);
	
	MoveHHi( times);
	HLock( times);
	
	theGrains = (long *)*times;
	
	GetDateTime( &starting_time);
	ChangeGrain( whichGrain);
	ChangeDate( whichDate);
}

void touchshell::OpenDoc( Boolean opening)
{
	if( opening)
	{
		if( whichDate != kModDate)
		{
			theCInfoPBRec.hFileInfo.ioFlCrDat = time_to_set;
		}
		if( whichDate != kCreatDate)
		{
			theCInfoPBRec.hFileInfo.ioFlMdDat = time_to_set;
		}
		theCInfoPBRec.hFileInfo.ioFDirIndex = 0;
		const OSErr result = PBSetCatInfoSync( &theCInfoPBRec);
		if( result != noErr)
		{
			SysBeep( 9);
		}
	}
}

void touchshell::HandleDialogEvent( short itemHit, DialogPtr theDialog)
{
	switch( itemHit)
	{
		case kSecondButton:
		case kMinuteButton:
		case kHourButton:
		case kHalfDayButton:
		case kDayButton:
		case kGroundZeroButton:
			ChangeGrain( itemHit);
			break;
			
		case kCreatDate:
		case kModDate:
		case kBothDates:
			ChangeDate( itemHit);
			break;
	}
}

void touchshell::ChangeGrain( int newGrain)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	for( int i = kSecondButton; i <= kGroundZeroButton; i++)
	{
		GetDialogItem( gMainDialog, i, &iType, &iHandle, &iRect);
		SetControlValue( (ControlHandle)iHandle, (i == newGrain));
	}
	whichGrain = newGrain;
	const long granularity = theGrains[ whichGrain - 1];
	
	time_to_set = granularity *
		((starting_time + granularity - 1) / granularity);
}

void touchshell::ChangeDate( int newDate)
{
	short	iType;
	Handle	iHandle;
	Rect	iRect;

	for( int i = kCreatDate; i <= kBothDates; i++)
	{
		GetDialogItem( gMainDialog, i, &iType, &iHandle, &iRect);
		SetControlValue( (ControlHandle)iHandle, (i == newDate));
	}
	whichDate = newDate;
}
