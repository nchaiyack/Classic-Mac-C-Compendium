// ===========================================================================
// 3DPanes.cp		  �1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// Provides 3D panes (raised and inset) as proposed in Develop #15.
//
// There are two types of 3DPanes: panels, in which the whole pain is drawn
// into, and frames, which only draw a box around the pane.
//
// There are also two implementations of each type of pane. The first uses the
// colors stored via 3DUtilities so that all 3DPane colors can be controlled
// in unison at runtime. The second is entirely resource based and gets all its 
// color information from a Constructor template resource.

#include "3DPanes.h"

#include "3DAttachments.h"
#include "3DDrawingUtils.h"

#include <LStream.h>
#include <PP_Types.h>
#include <UDrawingUtils.h>

// local (static) prototype

static RGBColor	ByteToGray( Byte inByte);


// ===========================================================================
// � C3DPanel														C3DPanel �
// ===========================================================================

// ---------------------------------------------------------------------------
//		� CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a C3DPanel
//	pane from a resource

C3DPanel *
C3DPanel::CreateFromStream(
	LStream *inStream)
{
	return (new C3DPanel(inStream));
}

// ---------------------------------------------------------------------------
//		� C3DPanel
// ---------------------------------------------------------------------------
//	The default constructor.

C3DPanel::C3DPanel()
	: LPane(), mIsInset(false)
{
}

// ---------------------------------------------------------------------------
//		� C3DPanel
// ---------------------------------------------------------------------------
//	Construct from data structures.

C3DPanel::C3DPanel(const SPaneInfo &inPaneInfo, const Boolean &inIsInset)
	: LPane(inPaneInfo), mIsInset(inIsInset)
{
}

// ---------------------------------------------------------------------------
//		� C3DPanel
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor.

C3DPanel::C3DPanel(LStream *inStream)
	: LPane(inStream)
{
	inStream->ReadData(&mIsInset, sizeof(Boolean));
}

// ---------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------

void
C3DPanel::DrawSelf()
{
	Rect	 	theRect;
	RGBColor	theColor;
	
	CalcLocalFrameRect(theRect);

	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
			if (mIsInset) {
				::Draw3DInsetPanel(&theRect);
				::Get3DLightColor(&theColor);
			} else {
				::Draw3DRaisedPanel(&theRect);
				::Get3DBackColor(&theColor);
			}
			::RGBBackColor(&theColor);	// set background color for text captions
		} else {
			::FrameRect(&theRect);
		}
	}
}



// ===========================================================================
// � C3DFrame														C3DFrame �
// ===========================================================================

// ---------------------------------------------------------------------------
//		� CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a C3DFrame
//	pane from a resource


C3DFrame *
C3DFrame::CreateFromStream(
	LStream *inStream)
{
	return (new C3DFrame(inStream));
}

// ---------------------------------------------------------------------------
//		� C3DFrame
// ---------------------------------------------------------------------------
//	The default constructor.

C3DFrame::C3DFrame()
	: LPane(), mIsInset(false)
{
}

// ---------------------------------------------------------------------------
//		� C3DFrame
// ---------------------------------------------------------------------------
//	Construct from data structures.

C3DFrame::C3DFrame(const SPaneInfo &inPaneInfo, const Boolean &inIsInset)
	: LPane(inPaneInfo), mIsInset(inIsInset)
{
}

// ---------------------------------------------------------------------------
//		� C3DFrame
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor.

C3DFrame::C3DFrame(LStream *inStream)
	: LPane(inStream)
{
	inStream->ReadData(&mIsInset, sizeof(Boolean));
}

// ---------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------

void
C3DFrame::DrawSelf()
{
	Rect	 	theRect;
	RGBColor	theColor;
	
	CalcLocalFrameRect(theRect);

	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
			if (mIsInset) {
				::Draw3DInsetFrame(&theRect);
			} else {
				::Draw3DRaisedFrame(&theRect);
			}
			::Get3DBackColor(&theColor);
			::RGBBackColor(&theColor);	// set background color for text captions
		} else {
			::FrameRect(&theRect);
		}
	}
}



// ===========================================================================
// � CThreeDPanel												CThreeDPanel �
// ===========================================================================

// ---------------------------------------------------------------------------
//		� CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a CThreeDPanel
//	pane from a resource

CThreeDPanel *
CThreeDPanel::CreateFromStream(
	LStream *inStream)
{
	return (new CThreeDPanel(inStream));
}

// ---------------------------------------------------------------------------
//		� CThreeDPanel
// ---------------------------------------------------------------------------
//	The default constructor.

CThreeDPanel::CThreeDPanel()
	: LPane(),
	  mBackgroundColor(kBackgroundGray),
	  mUpperLeftColor(kWhite),
	  mLowerRightColor(kShadowGray),
	  mHasBox(false)
{
}

// ---------------------------------------------------------------------------
//		� CThreeDPanel
// ---------------------------------------------------------------------------
//	Construct from data structures.

CThreeDPanel::CThreeDPanel(const SPaneInfo &inPaneInfo,
				 ThreeDPaneInfo &inThreeDPaneInfo)
	: LPane(inPaneInfo)
{
	mBackgroundColor	= ByteToGray( inThreeDPaneInfo.backgroundColor );
	mUpperLeftColor		= ByteToGray( inThreeDPaneInfo.upperLeftColor );
	mUpperLeftColor		= ByteToGray( inThreeDPaneInfo.lowerRightColor );
	mHasBox				= inThreeDPaneInfo.hasBox;
}

// ---------------------------------------------------------------------------
//		� CThreeDPanel
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor.

CThreeDPanel::CThreeDPanel(LStream *inStream)
	: LPane(inStream)
{
	ThreeDPaneInfo	thePaneInfo;
	inStream->ReadData(&thePaneInfo, sizeof(ThreeDPaneInfo));

	mBackgroundColor	= ByteToGray( thePaneInfo.backgroundColor );
	mUpperLeftColor		= ByteToGray( thePaneInfo.upperLeftColor );
	mLowerRightColor	= ByteToGray( thePaneInfo.lowerRightColor );
	mHasBox				= thePaneInfo.hasBox;
}

// ---------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------

void
CThreeDPanel::DrawSelf()
{
	Rect	 	theRect;
	
	CalcLocalFrameRect(theRect);

	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
			::Draw3DPanel( &theRect, &mBackgroundColor, &mUpperLeftColor,
									 &mLowerRightColor, mHasBox);
			::RGBBackColor(&mBackgroundColor);	// set background color for text captions
		} else {
			::FrameRect(&theRect);
		}
	}
}



// ===========================================================================
// � CThreeDFrame														CThreeDFrame �
// ===========================================================================

// ---------------------------------------------------------------------------
//		� CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a CThreeDFrame
//	pane from a resource

CThreeDFrame *
CThreeDFrame::CreateFromStream(
	LStream *inStream)
{
	return (new CThreeDFrame(inStream));
}

// ---------------------------------------------------------------------------
//		� CThreeDFrame
// ---------------------------------------------------------------------------
//	The default constructor.

CThreeDFrame::CThreeDFrame()
	: LPane(),
	  mUpperLeftColor(kShadowGray),
	  mLowerRightColor(kWhite)
{
}

// ---------------------------------------------------------------------------
//		� CThreeDCThreeDFramePane
// ---------------------------------------------------------------------------
//	Construct from data structures.

CThreeDFrame::CThreeDFrame(const SPaneInfo &inPaneInfo,
				 ThreeDFrameInfo &inThreeDFrameInfo)
	: LPane(inPaneInfo)
{
	mUpperLeftColor		= ByteToGray( inThreeDFrameInfo.upperLeftColor );
	mUpperLeftColor		= ByteToGray( inThreeDFrameInfo.lowerRightColor );
}

// ---------------------------------------------------------------------------
//		� CThreeDFrame
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor.

CThreeDFrame::CThreeDFrame(LStream *inStream)
	: LPane(inStream)
{
	ThreeDFrameInfo	theFrameInfo;
	inStream->ReadData(&theFrameInfo, sizeof(ThreeDFrameInfo));

	mUpperLeftColor		= ByteToGray( theFrameInfo.upperLeftColor );
	mLowerRightColor	= ByteToGray( theFrameInfo.lowerRightColor );
}

// ---------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------

void
CThreeDFrame::DrawSelf()
{
	Rect	 	theRect;
	
	CalcLocalFrameRect(theRect);

	U3DDrawingUtils::Draw3DFrame( &theRect, &mUpperLeftColor, &mLowerRightColor);
}



// ---------------------------------------------------------------------------
//		� ByteToGray
// ---------------------------------------------------------------------------
// Convert a single byte to one of the 16 basic grays.

static RGBColor ByteToGray( Byte inByte)
{
	Int16		tempVal = 0;
	
	tempVal = (inByte * 0x1000) + (inByte * 0x0100) + (inByte * 0x0010) + inByte;
	
	RGBColor	theGray = { tempVal, tempVal, tempVal};
	
	return theGray;
}



// ===========================================================================
// � C3DPanelT													   C3DPanelT �
// ===========================================================================

// WARNING: The following is experimental and not currently supported.
// ===========================================================================
//	Once template support is available (in CW5), this templatized version may
//	be better. With it you can typedef your own color styles and minimize
//	data duplication. For instance:
//
//		typedef C3DPaneT<kBackgroundGray,kWhite,kShadowGray,false>	CThreeDRaisedPane;
//		typedef C3DPaneT<kWhite,kShadowGray,kWhite,true>			CThreeDInsetPane;
//		typedef C3DFrameT<kWhite,kShadowGray>						CThreeDRaisedFrame;
//		typedef C3DFrameT<kShadowGray,kWhite>						CThreeDInsetFrame;
//
//	would give you raised and inset panes and frames respectively using the
//	coloring scheme proposed in Develop #15. (For each of your own typedef'ed
//	versions, be sure to use a distinct class id.)
//
//	NOTE: I haven't tested this version since I don't have access to a template compiler :)

//#ifdef __Templates

// ===========================================================================
// � C3DPanelT													   C3DPanelT �
// ===========================================================================

// ---------------------------------------------------------------------------
//		� CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a CToolbar
//	window from a resource

template <RGBColor TBKColor, RGBColor TULColor, RGBColor TLRColor, Boolean TFrameIt>
C3DPanelT<TBKColor,TULColor,TLRColor,TFrameIt>*
C3DPanelT<TBKColor,TULColor,TLRColor,TFrameIt>::CreateFromStream(
	LStream *inStream)
{
	return (new C3DPanelT<TBKColor,TULColor,TLRColor>(inStream));
}

// ---------------------------------------------------------------------------
//		� C3DPanelT
// ---------------------------------------------------------------------------
//	The default constructor does nothing.

template <RGBColor TBKColor, RGBColor TULColor, RGBColor TLRColor, Boolean TFrameIt>
C3DPanelT<TBKColor,TULColor,TLRColor,TFrameIt>::C3DPanelT()
{
}

// ---------------------------------------------------------------------------
//		� C3DPanelT
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor just constructs the base class.

template <RGBColor TBKColor, RGBColor TULColor, RGBColor TLRColor, Boolean TFrameIt>
C3DPanelT<TBKColor,TULColor,TLRColor,TFrameIt>::C3DPanelT(LStream *inStream)
	: LPanel(inStream)
{
}

// ---------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------

template <RGBColor TBKColor, RGBColor TULColor, RGBColor TLRColor, Boolean TFrameIt>
void
C3DPanelT<TBKColor,TULColor,TLRColor,TFrameIt>::DrawSelf()
{
	Rect	 	theRect;
	
	CalcPortFrameRect(theRect);

	St3DDeviceLoop	theLoop(theRect);
	
	while (theLoop.Next()) {
		if (theLoop.CurrentDeviceIs3DCapable()) {
			::Draw3DPanel( &theRect, &TBKColor, &TULColor, &TLRColor, TFrameIt);
	
			::RGBBackColor(&TBKColor);	// set background color for text captions
		} else {
			::FrameRect(&theRect);
		}
	}
}



// ===========================================================================
// � C3DFrameT													   C3DFrameT �
// ===========================================================================

// ---------------------------------------------------------------------------
//		� CreateFromStream [static]
// ---------------------------------------------------------------------------
//	This is the function you register with URegistrar to create a C3DFrame
//	pane from a resource


template <RGBColor TULColor, RGBColor TLRColor>
C3DFrameT<TULColor,TLRColor>*
C3DFrameT<TULColor,TLRColor>::CreateFromStream(
	LStream *inStream)
{
	return (new C3DFrameT(inStream));
}

// ---------------------------------------------------------------------------
//		� C3DFrameT
// ---------------------------------------------------------------------------
//	The default constructor does nothing.

template <RGBColor TULColor, RGBColor TLRColor>
C3DFrameT<TULColor,TLRColor>::C3DFrameT()
{
}

// ---------------------------------------------------------------------------
//		� C3DFrameT
// ---------------------------------------------------------------------------
//	The construct-from-stream constructor just constructs the base class.

template <RGBColor TULColor, RGBColor TLRColor>
C3DFrameT<TULColor,TLRColor>::C3DFrameT(LStream *inStream)
	: LPane(inStream)
{
}

// ---------------------------------------------------------------------------
//		� DrawSelf
// ---------------------------------------------------------------------------

template <RGBColor TULColor, RGBColor TLRColor>
void
C3DFrameT<TULColor,TLRColor>::DrawSelf()
{
	Rect	 	theRect;
	
	CalcPortFrameRect(theRect);

	U3DDrawingUtils::Draw3DRect( &theRect, &TULColor, &TLRColor);
}


//#endif	// __Templates