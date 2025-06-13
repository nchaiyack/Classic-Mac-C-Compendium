// ===========================================================================
// 3DTextFields.h	  ©1995 J. Rodden, DD/MF & Associates. All rights reserved
// ===========================================================================
// Provides 3D text fields (inset) as proposed in Develop #15.

#pragma once

#include <LCaption.h>
#include <LEditField.h>

// ===========================================================================
// Basic Edit Field (uses 3DUtilities set colors)
// ===========================================================================

class C3DEditField {
public:
	enum { class_ID = '3Ded' };

	static LEditField*	CreateFromStream(LStream *inStream);

	static LEditField*	CreateFromData(const SPaneInfo& inPaneInfo,
							Str255 inString, ResIDT inTextTraitsID,
							Int16 inMaxChars, Boolean inHasWordWrap,
							KeyFilterFunc inKeyFilter, LCommander* inSuper);
};


class C3DCaption {
public:
	enum { class_ID = '3Dca' };

	static LCaption*	CreateFromStream(LStream *inStream);

	static LCaption*	CreateFromData(const SPaneInfo &inPaneInfo,
										Str255 inString,
										ResIDT inTextTraitsID);
};
