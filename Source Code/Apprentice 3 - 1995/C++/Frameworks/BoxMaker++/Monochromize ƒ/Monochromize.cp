#include <assert.h>

#include <fstream.h>

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

#include <Windows.h>
#include <QDOffscreen.h>
#include <SegLoad.h>
#include <TextEdit.h>

#include "grafport.h"
#include "gworld.h"

#include "standardgetfile.h"
#include "boxmakergetfile.h"

#include "boxmaker constants.h"
#include "boxmaker.h"
#include "Monochromize.h"

void main();

void main()
{
	eight_to_one it;
	it.run();
}

void eight_to_one::OpenDoc( Boolean opening)
{
	short refNum;
	FSpOpenDF( &theFSSpec, fsRdWrPerm, &refNum);
	
	long picture_size;
	OSErr result;
	
	result = GetEOF( refNum, &picture_size);
	
	picture_size -= 0x0200;

	result = SetFPos( refNum, fsFromStart, 0x0200); 

	PicHandle thePICT = (PicHandle)NewHandle( picture_size);
	HLock( (Handle)thePICT);
		result = FSRead( refNum, &picture_size, (char *)*thePICT);
		Rect PICTRect = (**thePICT).picFrame;
	HUnlock( (Handle)thePICT);
	
	OffsetRect( &PICTRect, -PICTRect.left, -PICTRect.top);
	
	gworld offscreen( PICTRect.right, PICTRect.bottom, 1);
	offscreen.use();
	DrawPicture( thePICT, &PICTRect);
	DisposeHandle( (Handle)thePICT);
	
	thePICT = OpenPicture( &PICTRect);
	offscreen.copyfrom( offscreen);
	ClosePicture();
	long new_size = GetHandleSize( (Handle)thePICT);
	//
	// Keep the old header
	//
	result = SetFPos( refNum, fsFromStart, 0x0200); 
	result = SetEOF( refNum, new_size + 0x200);
	result = FSWrite( refNum, &new_size, (char *)*thePICT);
	KillPicture( thePICT);
	result = FSClose( refNum);
}

void eight_to_one::DoMenu( long retVal)
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
				
				case kPrefsItem:	// _not_ kQuitItem!!
					SendQuitToSelf();
					break;
			}
	}
}
