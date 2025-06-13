// ===========================================================================
// 3DPanes.h		  ©1995 J. Rodden, DD/MF & Associates. All rights reserved
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

#pragma once

#include <LPane.h>

struct ThreeDPaneInfo {
	Byte	backgroundColor;
	Byte	upperLeftColor;
	Byte	lowerRightColor;
	Boolean hasBox;
};

struct ThreeDFrameInfo {
	Byte	upperLeftColor;
	Byte	lowerRightColor;
};


// ===========================================================================
// Basic Panel (uses 3DUtilities set colors)
// ===========================================================================

class C3DPanel : public LPane {
public:
	enum { class_ID = '3Dpa' };

	static C3DPanel*	CreateFromStream(LStream *inStream);

					C3DPanel();
					C3DPanel(const SPaneInfo &inPaneInfo, const Boolean &inIsInset);
					C3DPanel(LStream *inStream);

protected:
	virtual void	DrawSelf();	

	Boolean		mIsInset;
};

// ===========================================================================
// Basic Frame (uses 3DUtilities set colors)
// ===========================================================================

class C3DFrame : public LPane {
public:
	enum { class_ID = '3Dfm' };

	static C3DFrame*	CreateFromStream(LStream *inStream);

					C3DFrame();
					C3DFrame(const SPaneInfo &inPaneInfo, const Boolean &inIsInset);
					C3DFrame(LStream *inStream);

protected:
	virtual void	DrawSelf();	

	Boolean		mIsInset;
};


// ===========================================================================
// Resource defined 3D panel
// ===========================================================================

class CThreeDPanel : public LPane {
public:
	enum { class_ID = '3DPA' };

	static CThreeDPanel*	CreateFromStream(LStream *inStream);

					CThreeDPanel();
					CThreeDPanel(const SPaneInfo &inPaneInfo,
							ThreeDPaneInfo &inThreeDPaneInfo);
					CThreeDPanel(LStream *inStream);
	
protected:
	virtual void	DrawSelf();

	RGBColor	mBackgroundColor;
	RGBColor	mUpperLeftColor;
	RGBColor	mLowerRightColor;
	Boolean		mHasBox;
};

// ===========================================================================
// Resource defined 3D frame
// ===========================================================================

class CThreeDFrame : public LPane {
public:
	enum { class_ID = '3DFM' };

	static CThreeDFrame*	CreateFromStream(LStream *inStream);

					CThreeDFrame();
					CThreeDFrame(const SPaneInfo &inPaneInfo,
							 ThreeDFrameInfo &inThreeDFrameInfo);
					CThreeDFrame(LStream *inStream);
	
protected:
	virtual void	DrawSelf();

	RGBColor	mUpperLeftColor;
	RGBColor	mLowerRightColor;
};


// WARNING: The following is experimental and not currently supported.
// ===========================================================================
//	Once template support is available (in CW5), this templatized version may
//	be better. With it you can typedef your own color styles and minimize
//	data duplication. For instance:
//
//		typedef C3DPane<kBackgroundGray,kWhite,kShadowGray,false>	CThreeDRaisedPane;
//		typedef C3DPane<kWhite,kShadowGray,kWhite,true>				CThreeDInsetPane;
//		typedef C3DFrame<kWhite,kShadowGray>						CThreeDRaisedFrame;
//		typedef C3DFrame<kShadowGray,kWhite>						CThreeDInsetFrame;
//
//	would give you raised and inset panes and frames respectively using the
//	coloring scheme proposed in Develop #15. (For each of your own typedef'ed
//	versions, be sure to use a distinct class id.)
//
//	NOTE: I haven't tested this version since I don't have access to a template compiler :)

//#ifdef __Templates

template <RGBColor TBKColor, RGBColor TULColor, RGBColor TLRColor, Boolean TFrameIt>
class C3DPanelT : public LPane {
public:
	enum { class_ID = '3DPT' };

	static C3DPanelT<TBKColor,TULColor,TLRColor,TFrameIt>*
					CreateFromStream(LStream *inStream);

					C3DPanelT();
					C3DPanelT(LStream *inStream);
	
protected:
	virtual void	DrawSelf();	

private:

};

template <RGBColor TULColor, RGBColor TLRColor>
class C3DFrameT : public LPane {
public:
	enum { class_ID = '3DFT' };

	static C3DFrameT<TULColor,TLRColor>*
					CreateFromStream(LStream *inStream);

					C3DFrameT();
					C3DFrameT(LStream *inStream);
	
protected:
	virtual void	DrawSelf();	

private:

};

//#endif	// __Templates