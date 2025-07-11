// ===========================================================================
//	C3DDemoWin.cp
// ===========================================================================

#include "C3DDemoWin.h"

#include <LStream.h>
#include <UDrawingUtils.h>

#include <3DGrays.h>
#include <3DAttachments.h>
#include <3DDrawingUtils.h>

// ---------------------------------------------------------------------------
//		� Create3DDemoWinStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a C3DDemoWin
//	window from a resource


C3DDemoWin *
C3DDemoWin::CreateFromStream(
	LStream *inStream)
{
	return (new C3DDemoWin(inStream));
}

// ---------------------------------------------------------------------------
//		� C3DDemoWin
// ---------------------------------------------------------------------------
//	The default constructor does nothing.

C3DDemoWin::C3DDemoWin()
{
}

// ---------------------------------------------------------------------------
//		� C3DDemoWin
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor just constructs the base class.

C3DDemoWin::C3DDemoWin(LStream *inStream)
	: LWindow(inStream)
{
}

// ---------------------------------------------------------------------------
//		� ~C3DDemoWin
// ---------------------------------------------------------------------------
//	The default destructor does nothing.

C3DDemoWin::~C3DDemoWin()
{
}

// ---------------------------------------------------------------------------
//		� FinishCreateSelf
// ---------------------------------------------------------------------------

void
C3DDemoWin::FinishCreateSelf()
{
	PenState	thePenState;
	thePenState.pnSize.h = 1;
	thePenState.pnSize.v = 1;
	thePenState.pnMode = srcCopy;
	thePenState.pnPat = qd.black;

	AddAttachment(new C3DPanelAttachment(&thePenState));
}

// ---------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------

void
C3DDemoWin::DrawSelf()
{
	U3DDrawingUtils::Draw3DInsetHLine( 190,  40, 200);
	U3DDrawingUtils::Draw3DInsetVLine(  20, 180, 220);

	U3DDrawingUtils::Draw3DRaisedHLine( 210,  40, 200);
	U3DDrawingUtils::Draw3DRaisedVLine( 220, 180, 220);
	
	Rect	oval = { 170, 65, 182, 77};
	
	U3DDrawingUtils::Draw3DInsetOvalPanel(&oval);
	
	::OffsetRect( &oval, 20, 0);
	U3DDrawingUtils::Draw3DRaisedOvalPanel(&oval);
	
	::OffsetRect( &oval, 20, 0);
	U3DDrawingUtils::Draw3DInsetOvalBorder(&oval);
	
	::OffsetRect( &oval, 20, 0);
	U3DDrawingUtils::Draw3DRaisedOvalBorder(&oval);
	
	::OffsetRect( &oval, 20, 0);
	U3DDrawingUtils::Draw3DInsetOvalFrame(&oval);
	
	::OffsetRect( &oval, 20, 0);
	U3DDrawingUtils::Draw3DRaisedOvalFrame(&oval);
}
