// ============================================================================
//	3DAttachments.h	   ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ============================================================================
//	A few Attachment classes to provide a 3D interface.
//	There are attachements that allow programatic customization of color usage
//	and others that use the settings defined by the 3DUtilities package.

#pragma once

#include <3DGrays.h>

#include <LAttachment.h>
#include <LPane.h>

// ============================================================================
// ============================================================================
// ¥ C3DBkgdAttachment								 	    C3DBkgdAttachment ¥
// ============================================================================

class	C3DBkgdAttachment : public LAttachment {
public:
			C3DBkgdAttachment(Boolean inExecuteHost = true);

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ============================================================================
// ¥ C3DEdgeAttachment								 	    C3DEdgeAttachment ¥
// ============================================================================

class	C3DEdgeAttachment : public LAttachment {
public:
			C3DEdgeAttachment(PenState *inPenState, SBooleanRect& inEdges,
								const RGBColor &inULColor = kWhite,
								const RGBColor &inLRColor = kShadowGray,
								Boolean inExecuteHost = true);

protected:
	PenState		mPenState;
	SBooleanRect	mEdges;
	RGBColor		mULColor;
	RGBColor		mLRColor;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DInsetEdgeAttachment						 	   C3DInsetEdgeAttachment ¥
// ============================================================================

class	C3DInsetEdgeAttachment : public LAttachment {
public:
			C3DInsetEdgeAttachment( SBooleanRect& inEdges, Boolean inExecuteHost = true);

protected:
	SBooleanRect	mEdges;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DRaisedEdgeAttachment					 	  C3DRaisedEdgeAttachment ¥
// ============================================================================

class	C3DRaisedEdgeAttachment : public LAttachment {
public:
			C3DRaisedEdgeAttachment( SBooleanRect& inEdges, Boolean inExecuteHost = true);

protected:
	SBooleanRect	mEdges;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ============================================================================
// ¥ C3DFrameAttachment								 	   C3DFrameAttachment ¥
// ============================================================================

class	C3DFrameAttachment : public LAttachment {
public:
			C3DFrameAttachment(PenState *inPenState,
								const RGBColor &inULColor = kWhite,
								const RGBColor &inLRColor = kShadowGray,
								Boolean inExecuteHost = true);

protected:
	PenState		mPenState;
	RGBColor		mULColor;
	RGBColor		mLRColor;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DInsetFrameAttachment					 	  C3DInsetFrameAttachment ¥
// ============================================================================

class	C3DInsetFrameAttachment : public LAttachment {
public:
			C3DInsetFrameAttachment(Boolean inExecuteHost = true);

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DRaisedFrameAttachment					 	 C3DRaisedFrameAttachment ¥
// ============================================================================

class	C3DRaisedFrameAttachment : public LAttachment {
public:
			C3DRaisedFrameAttachment(Boolean inExecuteHost = true);

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ============================================================================
// ¥ C3DBorderAttachment							 	  C3DBorderAttachment ¥
// ============================================================================

class	C3DBorderAttachment : public LAttachment {
public:
			C3DBorderAttachment(PenState *inPenState,
								const RGBColor &inULColor = kWhite,
								const RGBColor &inLRColor = kShadowGray,
								Boolean inExecuteHost = true);

protected:
	PenState		mPenState;
	RGBColor		mULColor;
	RGBColor		mLRColor;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DInsetBorderAttachment					 	 C3DInsetBorderAttachment ¥
// ============================================================================

class	C3DInsetBorderAttachment : public LAttachment {
public:
			C3DInsetBorderAttachment(Boolean inExecuteHost = true);

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DRaisedBorderAttachment					 	C3DRaisedBorderAttachment ¥
// ============================================================================

class	C3DRaisedBorderAttachment : public LAttachment {
public:
			C3DRaisedBorderAttachment(Boolean inExecuteHost = true);

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ============================================================================
// ¥ C3DPanelAttachment								  	   C3DPanelAttachment ¥
// ============================================================================

class	C3DPanelAttachment : public LAttachment {
public:
			C3DPanelAttachment(PenState *inPenState,
								const RGBColor &inBKColor = kBackgroundGray,
								const RGBColor &inULColor = kWhite,
								const RGBColor &inLRColor = kShadowGray,
								Boolean inFrameIt = false,
								Boolean inExecuteHost = true);

protected:
	PenState	mPenState;
	RGBColor	mBKColor;
	RGBColor	mULColor;
	RGBColor	mLRColor;
	Boolean		mFrameIt;

	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DInsetPanelAttachment					  	  C3DInsetPanelAttachment ¥
// ============================================================================

class	C3DInsetPanelAttachment : public LAttachment {
public:
			C3DInsetPanelAttachment(Boolean inExecuteHost = true);

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};


// ============================================================================
// ¥ C3DRaisedPanelAttachment					  	 C3DRaisedPanelAttachment ¥
// ============================================================================

class	C3DRaisedPanelAttachment : public LAttachment {
public:
			C3DRaisedPanelAttachment(Boolean inExecuteHost = true);

protected:
	virtual void	ExecuteSelf(MessageT inMessage, void *ioParam);
};
