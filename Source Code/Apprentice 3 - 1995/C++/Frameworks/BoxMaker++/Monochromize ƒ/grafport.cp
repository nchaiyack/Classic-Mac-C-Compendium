#include <stdlib.h>

#include <Windows.h>
#include <QDOffscreen.h>

#include <Fonts.h>
#include <Packages.h>
#include <SegLoad.h>
#include <ToolUtils.h>
#include <TextEdit.h>

#include "grafport.h"

const grafport *grafport::currentport = 0;

grafport::~grafport()
{
	if( currentport == this)
	{
		//
		// about to delete the current port. To be safe,
		// set the current port to something else.
		//
		SetPort( qd.thePort);
		currentport = 0;
	}
}

void grafport::use() const
{
	if( currentport != this)
	{
		currentport = this;
		SetGWorld( myGWorldPtr, myGDHandle);
		// CheckForError( QDError(), "SetGWorld");
	}
}

void grafport::copyfrom( const grafport &source, const Rect &origRect,
			const Rect &destRect, const short mode, const RgnHandle maskRgn) const
{
	use();
	CopyBits( (BitMapPtr)(source.myPix), (BitMapPtr)myPix,
						&origRect, &destRect, mode, maskRgn);
}

void grafport::copyfrom( const grafport &source, const Rect &origRect,
						const short mode, const RgnHandle maskRgn) const
{
	use();
	CopyBits( (BitMapPtr)(source.myPix), (BitMapPtr)myPix,
						&origRect, &myRect, mode, maskRgn);
}

void grafport::copyfrom( const grafport &source, const grafport &mask,
					const short mode, const RgnHandle maskRgn) const
{
	use();
	CopyDeepMask( (BitMapPtr)(source.myPix), (BitMapPtr)(mask.myPix), (BitMapPtr)myPix,
					&source.myRect, &mask.myRect, &myRect, mode, maskRgn);
}

void grafport::copyfrom( const grafport &source, const grafport &mask, const Rect &origRect,
					const Rect &maskRect, const Rect &destRect,
					const short mode, const RgnHandle maskRgn) const
{
	use();
	CopyDeepMask( (BitMapPtr)(source.myPix), (BitMapPtr)(mask.myPix), (BitMapPtr)myPix,
						&origRect, &maskRect, &destRect, mode, maskRgn);
}

void grafport::scroll( short dh, short dv) const
{
	use();
	RgnHandle updateRgn = NewRgn();
	ScrollRect( &myGWorldPtr->portRect, dh, dv, updateRgn);
	DisposeRgn( updateRgn);
}

PicHandle grafport::getPICT() const
{
	use();
	PicHandle thePICT = OpenPicture( &myGWorldPtr->portRect);
	CopyBits( (BitMapPtr)myPix, (BitMapPtr)myPix, &myRect, &myRect, srcCopy + ditherCopy, 0L);
	ClosePicture();
	return thePICT;
}

void grafport::setentries( short start, short count, ColorSpec *theTable) const
{
	use();
	::SetEntries( start, count, theTable);
}

void grafport::SetColorTable( short resID) const
{
	CTabHandle theColorTable = GetCTable( resID);
	SetColorTable( theColorTable);
}

void grafport::SetColorTable( CTabHandle theColorTable) const
{
	DisposeCTable( myPix->pmTable);
	myPix->pmTable = theColorTable;
	GDeviceChanged( myGDHandle);
}
