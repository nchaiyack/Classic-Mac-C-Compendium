/*
	Public domain by Zig Zichterman
*/
/*
	LStdCDEFControl
	
	A collection of classes that derive from LStdControl but
	use a CDEF other than 0. Useful mainly as CDEF-capable
	replacements for
		LStdButton
		LStdCheckBox
		LStdRadioButton
		
	Also, because my 3D CDEF implements icon buttons, you may
	want to replace
		LCicnButton
		
	12/07/94	zz	initial write
*/
#pragma once

#include <LStdControl.h>

const ResIDT indentHighlighted = 0x04;

class LStdCDEFButton : public LStdControl {
public:
	enum {	class_ID = '3Dpb' };

	static	LStdCDEFButton*	CreateFromStream(LStream *inStream);
	
			LStdCDEFButton();
		
			LStdCDEFButton(const LStdCDEFButton	&inOriginal);
			
			LStdCDEFButton(const SPaneInfo	&inPaneInfo,
							MessageT		inValueMessage,
							ResIDT			inTextTraitsID,
							Str255			inTitle,
							ResIDT			inCDEFid);
			
			LStdCDEFButton(const SPaneInfo	&inPaneInfo,
							MessageT		inValueMessage,
							ResIDT			inTextTraitsID,
							ControlHandle	inMacControlH);
			
			LStdCDEFButton(LStream *inStream);

protected:
	virtual void	HotSpotResult(Int16 inHotSpot);
};

class LStdCDEFCheckBox : public LStdControl {
public:
	enum {	class_ID = '3Dcb' };
	enum {	TristateValue	= 2,
			CheckBoxMax		= TristateValue		};

	static LStdCDEFCheckBox*	CreateFromStream(LStream *inStream);
			
			LStdCDEFCheckBox(void);
		
			LStdCDEFCheckBox(const LStdCDEFCheckBox	&inOriginal);
			
			LStdCDEFCheckBox(const SPaneInfo	&inPaneInfo,
								MessageT		inValueMessage,
								Int32			inValue,
								ResIDT			inTextTraitsID,
								Str255			inTitle,
								ResIDT			inCDEFid);
			
			LStdCDEFCheckBox(const SPaneInfo	&inPaneInfo,
								MessageT		inValueMessage,
								Int32			inValue,
								ResIDT			inTextTraitsID,
								ControlHandle	inMacControlH);
			
			LStdCDEFCheckBox(LStream *inStream);
			
protected:			
		virtual void	HotSpotResult(Int16	inHotSpot);
};

class LStdCDEFRadioButton : public LStdControl {
public:
	enum {	class_ID = '3Drb' };
	enum {	TristateValue	= 2,
			RadioMax		= TristateValue		};

	static LStdCDEFRadioButton*	CreateFromStream(LStream *inStream);
	
			LStdCDEFRadioButton(void);
		
			LStdCDEFRadioButton(const SPaneInfo	&inPaneInfo,
								MessageT		inValueMessage,
								Int32			inValue,
								ResIDT			inTextTraitsID,
								Str255			inTitle,
								ResIDT			inCDEFid,
								Boolean			indented = true);
			
			LStdCDEFRadioButton(const SPaneInfo	&inPaneInfo,
								MessageT		inValueMessage,
								Int32			inValue,
								ResIDT			inTextTraitsID,
								ControlHandle	inMacControlH);
			
			LStdCDEFRadioButton(const LStdCDEFRadioButton &inOriginal);
			
			LStdCDEFRadioButton(LStream *inStream);
	
protected:
	virtual void	HotSpotResult(Int16	inHotSpot);
};

class LStdCDEFIconButton : public LStdControl {
public :
	enum {	iconButProc	= 4	};
		
			LStdCDEFIconButton(const SPaneInfo	&inPaneInfo,
								MessageT		inValueMessage,
								ResIDT			inIconID,
								ResIDT			inCDEFid);
		
	void	SetIconID(ResIDT inIconID) { SetValue(inIconID); }
			
protected:
	virtual void	HotSpotResult(Int16	inHotSpot);
};
