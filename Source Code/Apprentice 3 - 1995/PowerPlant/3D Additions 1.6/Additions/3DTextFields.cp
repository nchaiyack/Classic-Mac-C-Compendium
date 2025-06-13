// ===========================================================================
// 3DTextFields.cp	  ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// Provides 3D text fields (inset) as proposed in Develop #15.

#include "3DTextFields.h"

#include <3DDrawingUtils.h>
#include <3DPanes.h>

#include <LStream.h>

static void EmbedIn3DPanel(LStream *inStream, short inNumLines = 1);
static void EmbedIn3DPanel(SPaneInfo& ioPaneInfo, short inNumLines = 1);

// ---------------------------------------------------------------------------
//		¥ CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a C3DEditField
//	pane from a resource


LEditField*
C3DEditField::CreateFromStream(
	LStream *inStream)
{
	EmbedIn3DPanel(inStream);

	// Then create and return the LEditField
	return new LEditField(inStream);
}

// ---------------------------------------------------------------------------
//		¥ CreateFromData [static]
// ---------------------------------------------------------------------------
//	Construct from data structures.

LEditField*
C3DEditField::CreateFromData(
	const SPaneInfo& inPaneInfo,
	Str255			 inString,
	ResIDT			 inTextTraitsID,
	Int16			 inMaxChars,
	Boolean			 inHasWordWrap,
	KeyFilterFunc	 inKeyFilter,
	LCommander*		 inSuper)
{
	SPaneInfo thePaneInfo = inPaneInfo;
	EmbedIn3DPanel(thePaneInfo);
	
	// Then create and return the LEditField
	return new LEditField(thePaneInfo,inString,inTextTraitsID,inMaxChars,
				 			true,inHasWordWrap,inKeyFilter,inSuper);
}


// ---------------------------------------------------------------------------
//		¥ CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a C3DCaption
//	pane from a resource


LCaption*
C3DCaption::CreateFromStream(
	LStream *inStream)
{
	EmbedIn3DPanel(inStream,3);
	
	// Then create and return the LEditField
	return new LCaption(inStream);
}

// ---------------------------------------------------------------------------
//		¥ CreateFromData [static]
// ---------------------------------------------------------------------------
//	Construct from data structures.

LCaption*
C3DCaption::CreateFromData(
	const SPaneInfo &inPaneInfo,
	Str255 inString,
	ResIDT inTextTraitsID)
{
	SPaneInfo thePaneInfo = inPaneInfo;
	EmbedIn3DPanel(thePaneInfo,3);
	
	// Then create and return the LEditField
	return new LCaption( thePaneInfo, inString, inTextTraitsID);
}


// ---------------------------------------------------------------------------
//		¥ EmbedIn3DPanel [static]
// ---------------------------------------------------------------------------

void EmbedIn3DPanel(LStream *inStream, short inNumLines)
{
	// First intercept the stream to build a 3DPanel
	SPaneInfo thePaneInfo;
	inStream->PeekData(&thePaneInfo, sizeof(SPaneInfo));
	LPane* the3DPanel = new C3DPanel(thePaneInfo,true);

	// Now alter the SPaneInfo to shrink within the 3DPanel
	PenState thePenState;
	U3DDrawingUtils::Get3DPenState(&thePenState);
	thePaneInfo.width -= inNumLines*2*thePenState.pnSize.h;
	thePaneInfo.height -= inNumLines*2*thePenState.pnSize.v;
	thePaneInfo.left += inNumLines*thePenState.pnSize.h;
	thePaneInfo.top += inNumLines*thePenState.pnSize.v;
	
	// Rewrite the SPaneInfo back onto the inStream
	inStream->WriteData(&thePaneInfo, sizeof(SPaneInfo));
	inStream->SetMarker(-(sizeof(SPaneInfo)), streamFrom_Marker);
}

// ---------------------------------------------------------------------------
//		¥ EmbedIn3DPanel [static]
// ---------------------------------------------------------------------------

void EmbedIn3DPanel(SPaneInfo& ioPaneInfo, short inNumLines)
{
	// First create a 3DPanel
	LPane* the3DPanel = new C3DPanel( ioPaneInfo, true);

	// Now alter the SPaneInfo to shrink inPaneInfo within the 3DPanel
	PenState thePenState;
	U3DDrawingUtils::Get3DPenState(&thePenState);
	
	ioPaneInfo.width -= inNumLines*2*thePenState.pnSize.h;
	ioPaneInfo.height -= inNumLines*2*thePenState.pnSize.v;
	ioPaneInfo.left += inNumLines*thePenState.pnSize.h;
	ioPaneInfo.top += inNumLines*thePenState.pnSize.v;
}
