#include <stdlib.h>

#include <Windows.h>
#include <QDOffscreen.h>
#include <Memory.h>
#include <Fonts.h>
#include <Packages.h>
#include <SegLoad.h>
#include <ToolUtils.h>
#include <TextEdit.h>
#include <Files.h>

#include "grafport.h"
#include "gworld.h"

gworld::gworld( int breedte, int hoogte, int diepte, CTabHandle cTable)
{
	if( (diepte != 1) && (diepte !=  2) && (diepte !=  4)
		&& (diepte != 8) && (diepte != 16) && (diepte != 32))
	{
		DebugStr( "\pgworld::gworld: illegal depth passed");
		exit( EXIT_FAILURE);
	}
	if( (hoogte <= 0) || (breedte <= 0))
	{
		DebugStr( "\pgworld::gworld: illegal width and/or height passed");
		exit( EXIT_FAILURE);
	}
	myRect.top    = 0;
	myRect.left   = 0;
	myRect.bottom = hoogte;
	myRect.right  = breedte;
	
	de_diepte     = diepte;
	
	if( NewGWorld( &myGWorldPtr, de_diepte, &myRect, cTable, nil, 0) != noErr)
	{
		//
		// retry in temporary memory. We do not set the 'AllowPurgePixels' bit.
		// This is not the proper thing to do, but doing it this way is better than
		// failing immediately.
		//
		(void)NewGWorld( &myGWorldPtr, de_diepte, &myRect, cTable, nil, useTempMem);
	}
	myGDHandle = GetGWorldDevice( myGWorldPtr);
	
	const PixMapHandle myPixMapH = GetGWorldPixMap( myGWorldPtr);

	(void)LockPixels( myPixMapH);

	HLock( (Handle)myPixMapH); 
		
	myPix = (*myPixMapH);
}

gworld::~gworld()
{
	const PixMapHandle myPixMapH = GetGWorldPixMap( myGWorldPtr);
	HUnlock( (Handle)myPixMapH);
	UnlockPixels( myPixMapH);
	DisposeGWorld( myGWorldPtr);
}

OSErr gworld::dump( short defile) const
{
	const int rowChars = (myPix->rowBytes & 0x2FFF);
	const int numRows   = myPix->bounds.bottom - myPix->bounds.top;
	long totalChars = rowChars * numRows;
	const void *curAddress = (const void *)myPix->baseAddr;
	return FSWrite( defile, &totalChars, curAddress);
}

OSErr gworld::load( short defile) const
{
	const int rowChars = (myPix->rowBytes & 0x2FFF);
	const int numRows   = myPix->bounds.bottom - myPix->bounds.top;
	long totalChars = rowChars * numRows;
	void *curAddress = (void *)myPix->baseAddr;
	return FSRead( defile, &totalChars, curAddress);
}
