// ============================================================================
//	3DAttachments.cp   �1995 J. Rodden, DD/MF & Associates. All rights reserved
// ============================================================================
//	A few Attachment classes to provide a 3D interface.
//	There are attachements that allow programatic customization of color usage
//	and others that use the settings defined by the 3DUtilities package.

#include "3DAttachments.h"

#include <3DDrawingUtils.h>
#include <UDrawingState.h>

// ============================================================================
// � C3DBkgdAttachment									    C3DBkgdAttachment �
// ============================================================================
//	Paints the background in the background grey color.

C3DBkgdAttachment::C3DBkgdAttachment(
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
}


void
C3DBkgdAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect theRect = *((Rect*) ioParam);
	RGBColor bkgdColor;
	U3DDrawingUtils::Get3DBackColor(&bkgdColor);
  ::RGBBackColor(&bkgdColor);
  ::EraseRect(&theRect);
}


// ============================================================================
// � C3DEdgeAttachment									    C3DEdgeAttachment �
// ============================================================================
//	Draws a 3D style border on some or all edges within the Frame of a Pane
//		For use only with msg_DrawOrPrint
//
//	Use light color for inULColor and dark color for inLRColor to create
//	an embossed/raised appearance. Switch color argument ordering to create
//	embedded/engraved appearance.
//	Default colors correspond to those recommended in Develop #15.

C3DEdgeAttachment::C3DEdgeAttachment(
	PenState		*inPenState,
	SBooleanRect	&inEdges,
	const RGBColor	&inULColor,
	const RGBColor	&inLRColor,
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
	mPenState = *inPenState;
	mEdges	 = inEdges;
	mULColor = inULColor;
	mLRColor = inLRColor;
}


void
C3DEdgeAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect theRect = *((Rect*) ioParam);
	StColorPenState	savePenState;		// Will save and restore pen state

	SetPenState(&mPenState);

	if ( mEdges.left ) {
		U3DDrawingUtils::Draw3DVLine( theRect.left, theRect.top, theRect.bottom,
										&mULColor, &mLRColor);
	}

	if ( mEdges.top ) {
		U3DDrawingUtils::Draw3DHLine( theRect.top, theRect.left, theRect.right,
										&mULColor, &mLRColor);
	}

	if ( mEdges.right ) {
		U3DDrawingUtils::Draw3DVLine( theRect.right - 2*mPenState.pnSize.h,
										theRect.top, theRect.bottom,
										&mULColor, &mLRColor);
	}

	if ( mEdges.bottom ) {
		U3DDrawingUtils::Draw3DHLine( theRect.bottom - 2*mPenState.pnSize.v,
										theRect.left, theRect.right,
										&mULColor, &mLRColor);
	}
}


// ============================================================================
// � C3DInsetEdgeAttachment						 	   C3DInsetEdgeAttachment �
// ============================================================================

C3DInsetEdgeAttachment::C3DInsetEdgeAttachment(
	SBooleanRect &inEdges,
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
	mEdges	 = inEdges;
}


void
C3DInsetEdgeAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	PenState thePenState;
	Rect theRect = *((Rect*) ioParam);
	
	U3DDrawingUtils::Get3DPenState(&thePenState);

	if ( mEdges.left ) {
		U3DDrawingUtils::Draw3DInsetVLine( theRect.left, theRect.top, theRect.bottom);
	}

	if ( mEdges.top ) {
		U3DDrawingUtils::Draw3DInsetHLine( theRect.top, theRect.left, theRect.right);
	}

	if ( mEdges.right ) {
		U3DDrawingUtils::Draw3DInsetVLine( theRect.right - 2*thePenState.pnSize.h,
											theRect.top, theRect.bottom);
	}

	if ( mEdges.bottom ) {
		U3DDrawingUtils::Draw3DInsetHLine( theRect.bottom - 2*thePenState.pnSize.v,
											theRect.left, theRect.right);
	}
}


// ============================================================================
// � C3DRaisedEdgeAttachment					 	  C3DRaisedEdgeAttachment �
// ============================================================================

C3DRaisedEdgeAttachment::C3DRaisedEdgeAttachment(
	SBooleanRect &inEdges,
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
	mEdges	 = inEdges;
}


void
C3DRaisedEdgeAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	PenState thePenState;
	Rect theRect = *((Rect*) ioParam);
	
	U3DDrawingUtils::Get3DPenState(&thePenState);
	
	if ( mEdges.left ) {
		U3DDrawingUtils::Draw3DRaisedVLine( theRect.left, theRect.top, theRect.bottom);
	}

	if ( mEdges.top ) {
		U3DDrawingUtils::Draw3DRaisedHLine( theRect.top, theRect.left, theRect.right);
	}

	if ( mEdges.right ) {
		U3DDrawingUtils::Draw3DRaisedVLine( theRect.right - 2*thePenState.pnSize.h,
											theRect.top, theRect.bottom);
	}

	if ( mEdges.bottom ) {
		U3DDrawingUtils::Draw3DRaisedHLine( theRect.bottom - 2*thePenState.pnSize.v,
											theRect.left, theRect.right);
	}
}


// ============================================================================
// � C3DFrameAttachment									   C3DFrameAttachment �
// ============================================================================
//	Draws a 3D style border within the Frame of a Pane
//		For use only with msg_DrawOrPrint
//
//	Use light color for inULColor and dark color for inLRColor to create
//	an embossed/raised appearance. Switch color argument ordering to create
//	embedded/engraved appearance.
//	Default colors correspond to those recommended in Develop #15.

C3DFrameAttachment::C3DFrameAttachment(
	PenState	*inPenState,
	const RGBColor	&inULColor,
	const RGBColor	&inLRColor,
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
	mPenState = *inPenState;
	mULColor = inULColor;
	mLRColor = inLRColor;
}


void
C3DFrameAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect			theRect = *((Rect*) ioParam);
	StColorPenState	savePenState;		// Will save and restore pen state
	
	SetPenState(&mPenState);
	U3DDrawingUtils::Draw3DFrame( &theRect, &mULColor, &mLRColor);
}


// ============================================================================
// � C3DInsetFrameAttachment					 	  C3DInsetFrameAttachment �
// ============================================================================

C3DInsetFrameAttachment::C3DInsetFrameAttachment(
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
}


void
C3DInsetFrameAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect	theRect = *((Rect*) ioParam);

	U3DDrawingUtils::Draw3DInsetFrame(&theRect);
}


// ============================================================================
// � C3DRaisedFrameAttachment					 	 C3DRaisedFrameAttachment �
// ============================================================================

C3DRaisedFrameAttachment::C3DRaisedFrameAttachment(
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
}


void
C3DRaisedFrameAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect	theRect = *((Rect*) ioParam);

	U3DDrawingUtils::Draw3DRaisedFrame(&theRect);
}


// ============================================================================
// � C3DBorderAttachment								  C3DBorderAttachment �
// ============================================================================
//	Draws a 3D style border within the Frame of a Pane
//		For use only with msg_DrawOrPrint
//
//	Use light color for inULColor and dark color for inLRColor to create
//	an embossed/raised appearance. Switch color argument ordering to create
//	embedded/engraved appearance.
//	Default colors correspond to those recommended in Develop #15.

C3DBorderAttachment::C3DBorderAttachment(
	PenState	*inPenState,
	const RGBColor	&inULColor,
	const RGBColor	&inLRColor,
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
	mPenState = *inPenState;
	mULColor = inULColor;
	mLRColor = inLRColor;
}


void
C3DBorderAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect			theRect = *((Rect*) ioParam);
	StColorPenState	savePenState;		// Will save and restore pen state
	
	SetPenState(&mPenState);
	U3DDrawingUtils::Draw3DBorder( &theRect, &mULColor, &mLRColor, false);
}


// ============================================================================
// � C3DInsetBorderAttachment					 	 C3DInsetBorderAttachment �
// ============================================================================

C3DInsetBorderAttachment::C3DInsetBorderAttachment(
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
}


void
C3DInsetBorderAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect	theRect = *((Rect*) ioParam);

	U3DDrawingUtils::Draw3DInsetBorder(&theRect);
}


// ============================================================================
// � C3DRaisedBorderAttachment					 	C3DRaisedBorderAttachment �
// ============================================================================

C3DRaisedBorderAttachment::C3DRaisedBorderAttachment(
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
}


void
C3DRaisedBorderAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect	theRect = *((Rect*) ioParam);

	U3DDrawingUtils::Draw3DRaisedBorder(&theRect);
}


// ============================================================================
// � C3DPanelAttachment								  	   C3DPanelAttachment �
// ============================================================================
//	Draws a panel with a 3D style border and background color
//		For use only with msg_DrawOrPrint
//
//	Use light color for inULColor and dark color for inLRColor to create
//	an embossed/raised appearance. Switch color argument ordering to create
//	embedded/engraved appearance.
//	Default colors correspond to those recommended in Develop #15.

C3DPanelAttachment::C3DPanelAttachment(
	PenState	*inPenState,
	const RGBColor	&inBKColor,
	const RGBColor	&inULColor,
	const RGBColor	&inLRColor,
	Boolean			inFrameIt,
	Boolean		 	inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
	mPenState = *inPenState;
	mBKColor = inBKColor;
	mULColor = inULColor;
	mLRColor = inLRColor;
	mFrameIt = inFrameIt;
}


void
C3DPanelAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect			theRect = *((Rect*) ioParam);
	StColorPenState	savePenState;		// Will save and restore pen state
	
	SetPenState(&mPenState);
	U3DDrawingUtils::Draw3DPanel( &theRect, &mBKColor, &mULColor, &mLRColor, mFrameIt);
}


// ============================================================================
// � C3DInsetPanelAttachment					 	  C3DInsetPanelAttachment �
// ============================================================================

C3DInsetPanelAttachment::C3DInsetPanelAttachment(
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
}


void
C3DInsetPanelAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect	theRect = *((Rect*) ioParam);

	U3DDrawingUtils::Draw3DInsetPanel(&theRect);
}


// ============================================================================
// � C3DRaisedPanelAttachment					 	 C3DRaisedPanelAttachment �
// ============================================================================

C3DRaisedPanelAttachment::C3DRaisedPanelAttachment(
	Boolean		 inExecuteHost)
		: LAttachment(msg_DrawOrPrint, inExecuteHost)
{
}


void
C3DRaisedPanelAttachment::ExecuteSelf(
	MessageT	inMessage,
	void		*ioParam)
{
	Rect	theRect = *((Rect*) ioParam);

	U3DDrawingUtils::Draw3DRaisedPanel(&theRect);
}


