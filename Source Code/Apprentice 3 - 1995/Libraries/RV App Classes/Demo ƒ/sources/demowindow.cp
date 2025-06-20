#include <Drag.h>
#include <QuickDraw.h>
#include <QDOffscreen.h>
#include <Scrap.h>

#include "rsrcIDs.h"
#include "demoapp rsrcIDs.h"

#include "memobject.h"
#include "handle.h"
#include "qdobject.h"
#include "recordable.h"
#include "rect.h"
#include "picture.h"
#include "region.h"

#include "grafport.h"
#include "windowrecord.h"
#include "window.h"
#include "demowindow.h"

#include "draghandler.h"

demowindow::demowindow( short rsrcID, short PICT_ID)
	: window( rsrcID)
	, picture( PICT_ID)
{
}

demowindow::demowindow( short rsrcID, const FSSpec &theFile)
	: window( rsrcID)
	, picture( theFile)
{
}

demowindow::~demowindow()
{
}

void demowindow::HandleUpdate()
{
	this->grafport::Erase();
	this->Draw();
	this->DrawGrowIcon();
}

void demowindow::HandleMouseDownInContent( const EventRecord &theEvent)
{
	if( WaitMouseMoved( theEvent.where))
	{
		dragsender envelop;
		//
		// since the data is in a handle we might be better of using a send proc
		//
		// ?? MoveHHi( this->thehandle); ??
		//
		HLock( this->thehandle);

			(void)envelop.AddFlavor( 'PICT', *this->thehandle, GetHandleSize( this->thehandle));
			
			(void)envelop.SetBounds( this->picFrame());
			
			region dragRegion( this->picFrame());
			
			dragRegion.convert_to_frame();
	
			this->Set();
			
			Point p = { 0, 0};
			
			LocalToGlobal( &p);
			
			dragRegion.Offset( p.h, p.v);

			OSErr result = envelop.Track( theEvent, dragRegion);
		HUnlock( this->thehandle);
	}
}

void demowindow::HandleMenuSelection( long selection)
{
	const short theMenu = HiWord( selection);
	const short theItem = LoWord( selection);

	switch( theMenu)
	{
		case kAppleMenuID:
		case kFileMenuID:
			DebugStr( "\pdemowindow::HandleMenuSelection: Huh??");
			break;

		case kEditMenuID:
			HandleEditMenu( selection);
			break;			
	}	

}

OSErr demowindow::HandleDrop( DragReference theDragRef)
{
	OSErr result = dragNotAcceptedErr;

	DragAttributes dragFlags;
	OSErr attribResult = GetDragAttributes( theDragRef, &dragFlags);
	
	if( (attribResult == noErr) && !(dragFlags & dragInsideSenderWindow))
	{
		unsigned short numItems;
		CountDragItems( theDragRef, &numItems);
		for( int index = 1; index <= numItems; index++)
		{
			ItemReference theItem;
			(void)GetDragItemReferenceNumber( theDragRef, index, &theItem);
	
			FlavorFlags theFlags;
			const OSErr got_a_PICT = GetFlavorFlags( theDragRef, theItem, 'PICT', &theFlags);
	
			if( got_a_PICT == noErr)
			{
				long dataSize;
				(void)GetFlavorDataSize( theDragRef, theItem, 'PICT', &dataSize);
	
				Handle newPICT = NewHandle( dataSize);
	
				HLock( newPICT);
					(void)GetFlavorData( theDragRef, theItem, 'PICT', *newPICT, &dataSize, 0L);
				HUnlock( newPICT);
				
				this->Invalidy( this->picFrame());
				KillPicture( (PicHandle)(this->thehandle));
				this->thehandle = newPICT;
				this->Invalidy( this->picFrame());
				
				result = noErr;
			} else {
				const OSErr got_TEXT = GetFlavorFlags( theDragRef, theItem, 'TEXT', &theFlags);
		
				if( got_TEXT == noErr)
				{
					long dataSize;
					(void)GetFlavorDataSize( theDragRef, theItem, 'TEXT', &dataSize);
		
					handle theTEXT( dataSize);

					theTEXT.Lock();
						(void)GetFlavorData( theDragRef, theItem, 'TEXT', *theTEXT(), &dataSize, 0L);

						this->record();
							TETextBox( *theTEXT(), dataSize, &portRect(), teJustLeft);
						this->stop();
					theTEXT.Unlock();

					this->Invalidy();

					result = noErr;
				}
			}
		}
	}
	return result;
}

void demowindow::HandleEditMenu( short item)
{
	switch( item)
	{
		case iUndo:
		case iCut:
		case iClear:
			DebugStr( "\pdemowindow::HandleEditMenu : iUndo/iCut/iClear?");
			break;
		
		case iCopy:
			(void)ZeroScrap();
			this->Lock();
				(void)PutScrap( GetHandleSize( thehandle), 'PICT', *thehandle);
			this->Unlock();
			break;
			
		case iPaste:
			{
				long offset;
				long length = GetScrap( 0L, 'PICT', &offset);
				
				if( length < noErr)
				{
					length = GetScrap( 0L, 'TEXT', &offset);
					if( length < noErr)
					{
						SysBeep( 9);
					} else {
						handle newTEXT;
						length = GetScrap( newTEXT(), 'TEXT', &offset);
						newTEXT.Lock();
							this->record();
								TETextBox( *newTEXT(), length, &portRect(), teJustLeft);
							this->stop();
						newTEXT.Unlock();
						this->Invalidy();
					}
				} else {
					picture newPICT;
					(void)GetScrap( (Handle)newPICT(), 'PICT', &offset);
					*(picture *)this = newPICT;
					this->Invalidy();
				}
			}
			break;
	}
}
