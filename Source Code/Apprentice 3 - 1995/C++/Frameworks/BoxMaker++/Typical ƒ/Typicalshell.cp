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
#include <LowMem.h>

#include "boxmaker constants.h"
#include "standardgetfile.h"
#include "boxmakergetfile.h"

#include "boxmaker.h"
#include "preferences.cp"
#include "typicalshell.h"

#pragma template_access public

void main();

void main()
{
	typicalshell it;
	it.run();
}

const int    typicalshell::kOurQuitItem = 3;
const OSType typicalshell::dontChange   = '****';

typicalshell::typicalshell() : boxmaker( 3000)
{
	#if NEW_HEADERS_AVAILABLE
		const StringPtr appname = LMGetCurApName();
	#else
		//
		// For some reason 'curApName' is not declared under SC++
		// This doesn't bother me; I know how to find it, anyway.
		// #include <LoMem.h>
		// const StringPtr appname = (StringPtr)curApName;
		const StringPtr appname = (StringPtr)0x910;
	#endif	
	if( *appname < 8)
	{
		SysBeep( 9);
		SysBeep( 9);
		SysBeep( 9);
		ExitToShell();
	}
	#ifdef NEW_HEADERS_AVAILABLE
		BlockMoveData( &appname[ 1], &typeToSet   , sizeof( typeToSet));
		BlockMoveData( &appname[ 5], &creatorToSet, sizeof( creatorToSet));
	#else
		BlockMove( &appname[ 1], &typeToSet   , sizeof( typeToSet));
		BlockMove( &appname[ 5], &creatorToSet, sizeof( creatorToSet));
	#endif
}

void typicalshell::OpenDoc( Boolean opening)
{
	if( opening)
	{
		if( creatorToSet != dontChange)
		{
			theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdCreator = creatorToSet;
		}
		if( typeToSet != dontChange)
		{
			theCInfoPBRec.hFileInfo.ioFlFndrInfo.fdType = typeToSet;
		}
		const OSErr result = PBSetCatInfoSync( &theCInfoPBRec);
		if( result != noErr)
		{
			SysBeep( 9);
		}
	}
}

void typicalshell::DoMenu( long retVal)
{
	const short menuID = HiWord( retVal);
	const short itemID = LoWord( retVal);
	switch( menuID)
	{
		case kAppleMenuID:
			DoAppleMenu( itemID);
			break;
			
		case kFileMenuID:
			switch( itemID)
			{
				case kSelectFileItem:
					SelectFile();
					break;
			
				case kOurQuitItem:		// NB: not kQuitItem!
					SendQuitToSelf();
					break;
			}
			break;
		
		default:
			break;			
	}
	HiliteMenu( 0);
}
