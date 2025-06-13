/******************************************************************************
 CCicnButton.h

		Interface for CicnButton class
		
	SUPERCLASS = CIconPane
	
	Copyright © 1992 Object Factory Incorporated. All rights reserved.

	Modified 	12/93 by Martin R. Wachter

 ******************************************************************************/

#pragma once

#include <CIconPane.h>

enum {								// Button kinds
	kSAPushButton,
	kSACheckBox,
	kSARadioButton
};

enum {								// Highlight styles
	kSAStateHilight,				//	 Highlight by drawing different picture
	kSADimHilight					//	 Highlight by dimming (somehow)
};

enum {								// Ways to indicate button is "on"
	kSAStateOn,						//	 Show on by drawing different picture
	kSABorderOn						//	 Show on by drawing border around button
};

enum {								// Whether or not needs update to highlight
	kSANoUpdate,					//	 Nope, just draw it
	kSANeedsUpdate					//	 Must update. Call the artist!
};

enum {								// Graphical states in order
	kSAOffState,
	kSAOffHiliteState,
	kSAOnState,
	kSAOnHiliteState
};

class CCicnButton : public CIconPane
{	
public:
	Str32		itsLabels[4];		// labels used for each icon state
									// 1 - off (normal)
									// 2 - offHilite
									// 3 - on
									// 4 - onHilite
	
	RGBColor	itsColors[4];		// colors to draw itsLables in for each
									// of the four states.
							
	short		itsShadow;			// pixel width of the label's drop shadow
	
	short		buttonKind;			// Button kind
	Boolean		colorHilite;		// TRUE if hilight with color
	Boolean		outlineOn;			// TRUE if "on" button has border
	short		value;				// Control value, 0 or 1
	short		groupID;			// Group ID used for radio buttons

									// Arrays where CIconPane has single values
	short		itsIconID[4];
	char		itsIcon[4][kIconBytes];
	CIconHandle	itsCicnH[4];

	Point		borderPen;			// Saved border width and height
	

                                    /** Init/Dispose **/
	void 	ICicnButton(CView *anEnclosure, CBureaucrat *aSupervisor,
					short aHEncl, short aVEncl,
					SizingOption aHSizing, SizingOption aVSizing,
					short aButtonKind, short hilightStyle, short onStyle,
					short offID, short offHiliteID, short onID, short onHiliteID);

	void			ICicnButtonX(short offHiliteID, short onID, short onHiliteID);
	
	virtual void CCicnButton::SetLabel(short strListID, 
							short offIndex, short offHiliteIndex, short onIndex, short onHiliteIndex,
							short fontNumber, short fontFace, short fontSize, short itsShadow);
	
	virtual void 	SetStateIcons(short offHiliteID, short onID, short onHiliteID);
	virtual void 	Dispose(void);
	virtual void 	DrawIcon(Boolean fHilite);
	virtual void	DoDrawIcon(Boolean fHilite);
	virtual void	AdjustCursor(Point where, RgnHandle mouseRgn);
	virtual Boolean	Track(void);
	virtual void	SetLabelColors(RGBColor offColor, RGBColor offHiliteColor, RGBColor onColor, RGBColor onHiliteColor);
	virtual void	DrawLabel(Rect *fBounds, short state);
	virtual void	SetShadow(short aShadow);
	virtual short	GetShadow(short aShadow);
    virtual void	ChangeValue(short aValue);
	virtual void 	SetValue(short aValue);
	virtual short	GetValue(void);
	virtual void 	SetGroupID(short anID);
	virtual short	GetGroupID(void);
	virtual void 	SetButtonKind(short aKind);
	virtual short	GetButtonKind(void);
	virtual void 	FixupBorder(void);

	virtual short	CalcDrawState(Boolean fHilite);
	virtual void	SetEnable(void);
	virtual void	SetDisable(void);
	virtual short	GetEnable(void);
	virtual void 	ResetIconGroup(void);

private:

	CursHandle		itsPointerHand;
	
};
