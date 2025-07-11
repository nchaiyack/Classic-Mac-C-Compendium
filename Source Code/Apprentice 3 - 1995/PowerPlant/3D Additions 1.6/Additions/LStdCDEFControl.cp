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
#include "LStdCDEFControl.h"

#include <PP_Messages.h>

/**************************************************************************
	CreateFromStream()								[public, static]
	
	Create a new button from the stream. Stream format is identical to
	LStdControl and LStdButton--the CDEF is specified in the control kind
	field (upper 12 bits)
**************************************************************************/
LStdCDEFButton *
LStdCDEFButton::CreateFromStream(
	LStream *	inStream)
{
	return new LStdCDEFButton(inStream);
}

/**************************************************************************
	LStdCDEFButton(void)							[public]
	
	Default constructor. Make an unusable pane. Uses system CDEF.
**************************************************************************/
LStdCDEFButton::LStdCDEFButton()
	: LStdControl((Int16) pushButProc)
{
}

/**************************************************************************
	LStdCDEFButton(const LStdCDEFButton&)			[public]
	
	Copy constructor. Uses same CDEF as original
**************************************************************************/
LStdCDEFButton::LStdCDEFButton(
	const LStdCDEFButton	&inOriginal)
	: LStdControl(inOriginal)
{
}
	
/**************************************************************************
	LStdCDEFButton(params)							[public]
	
	Create from parameters. Specify a CDEF id or 0 for system CDEF
**************************************************************************/
LStdCDEFButton::LStdCDEFButton(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	ResIDT			inTextTraitsID,
	Str255			inTitle,
	ResIDT			inCDEFid)
		: LStdControl(inPaneInfo, inValueMessage, 0, 0, 0,
			(inCDEFid << 4) + pushButProc,inTextTraitsID, inTitle, 0)
{
}
	
/**************************************************************************
	LStdCDEFButton(params & ControlHandle)			[public]
	
	Create a button from an existing control handle. Since the control
	handle already knows which CDEF to use, you don't need to supply
	a CDEF id for this constructor.
**************************************************************************/
LStdCDEFButton::LStdCDEFButton(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	ResIDT			inTextTraitsID,
	ControlHandle	inMacControlH)
	: LStdControl(inPaneInfo, inValueMessage, 0, 0, 0, pushButProc,
					inTextTraitsID, inMacControlH)
{
}

/**************************************************************************
	LStdCDEFButton(stream)							[public]
	
	Stream constructor. CDEF is specified in upper 12 bits of stream's
	controlKind field.
*************************************************************************/
LStdCDEFButton::LStdCDEFButton(
	LStream *	inStream)
	: LStdControl(inStream)
{
}

/**************************************************************************
	HotSpotResult()									[protected, virtual]
													[complete override ]

	Even though a click in a push button doesn't change its value,
	it should broadcast its value message
**************************************************************************/
void
LStdCDEFButton::HotSpotResult(
	Int16 inHotSpot)
{
	BroadcastValueMessage();
}

/**************************************************************************
	CreateFromStream()								[public, static]
	
	Create a new checkbox from a stream. The CDEF is specified in the
	upper 12 bits of the stream's control kind field
**************************************************************************/
LStdCDEFCheckBox *
LStdCDEFCheckBox::CreateFromStream(
	LStream *	inStream)
{
	return new LStdCDEFCheckBox(inStream);
}

/**************************************************************************
	LStdCDEFCheckBox(void)							[public]
	
	Create a nigh-useless pane
**************************************************************************/
LStdCDEFCheckBox::LStdCDEFCheckBox(void)
	: LStdControl()
{
	SetMaxValue(CheckBoxMax);
}

/**************************************************************************
	LStdCDEFCheckBox(const LStdCDEFCheckBox)		[public]
	
	Copy constructor. Uses same CDEF as original
**************************************************************************/
LStdCDEFCheckBox::LStdCDEFCheckBox(
	const LStdCDEFCheckBox	&inOriginal)
	: LStdControl(inOriginal)
{
}
	
/**************************************************************************
	LStdCDEEFCheckBox(params-o-rama)				[public]
	
	Create a checkbox from parameters
**************************************************************************/
LStdCDEFCheckBox::LStdCDEFCheckBox(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	Int32			inValue,
	ResIDT			inTextTraitsID,
	Str255			inTitle,
	ResIDT			inCDEFid)
	: LStdControl(inPaneInfo, inValueMessage, inValue, 0, CheckBoxMax,
				(inCDEFid << 4) + checkBoxProc, inTextTraitsID, inTitle, 0)
{
}
	
/**************************************************************************
	LStdCDEFCheckBox(params & ControlHandle)		[public]
	
	Create a checkbox from parameters and a control handle. Since the
	control handle already specifies a CDEF, you don't need to supply one
**************************************************************************/
LStdCDEFCheckBox::LStdCDEFCheckBox(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	Int32			inValue,
	ResIDT			inTextTraitsID,
	ControlHandle	inMacControlH)
	: LStdControl(inPaneInfo, inValueMessage, inValue, 0, CheckBoxMax,
					checkBoxProc, inTextTraitsID, inMacControlH)
{
}

/**************************************************************************
	LStdCDEFCheckBox(stream)						[public]
	
	Stream constructor. CDEF is in the upper 12 bits of stream's
	control kind field
**************************************************************************/
LStdCDEFCheckBox::LStdCDEFCheckBox(
	LStream *	inStream)
	: LStdControl(inStream)
{
}
	
/**************************************************************************
	HotSpotResult()									[protected, virtual]
	
	Toggle value.
**************************************************************************/
void
LStdCDEFCheckBox::HotSpotResult(
	Int16	inHotSpot)
{
	const Int32	OldValue	= GetValue();
#ifndef	TRISTATE_TEST
	Int32		newValue	= (OldValue == 0);
#else
	Int32		newValue	= OldValue + 1;
	if (newValue > CheckBoxMax) {	
		newValue = 0;
	}
#endif
	SetValue(newValue);
}


/**************************************************************************
	CreateFromStream()								[public, static]
	
	Create a radio button from a stream. Stream specifies CDEF id in
	upper 12 bits of control kind field
**************************************************************************/
LStdCDEFRadioButton *
LStdCDEFRadioButton::CreateFromStream(
	LStream *	inStream)
{
	return new LStdCDEFRadioButton(inStream);
}
	
/**************************************************************************
	LStdCDEFRadioButton(void)						[public]
	
	Create a nigh-useless pane
**************************************************************************/
LStdCDEFRadioButton::LStdCDEFRadioButton(void)
	: LStdControl()
{
	SetMaxValue(RadioMax);
}

/**************************************************************************
	LStdCDEFRadioButton(params-o-rama)				[public]
	
	Create a radio button.
**************************************************************************/
LStdCDEFRadioButton::LStdCDEFRadioButton(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	Int32			inValue,
	ResIDT			inTextTraitsID,
	Str255			inTitle,
	ResIDT			inCDEFid,
	Boolean			indented)
	: LStdControl(inPaneInfo, inValueMessage, inValue, 0, RadioMax,
			(inCDEFid << 4) + radioButProc + (indented ? indentHighlighted : 0),
			inTextTraitsID, inTitle, 0)
{
}

/**************************************************************************
	LStdCDEFRadioButton(params & ControlHandle)		[public]
	
	Create a radio button. CDEF is specified in ControlHandle.
**************************************************************************/
LStdCDEFRadioButton::LStdCDEFRadioButton(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	Int32			inValue,
	ResIDT			inTextTraitsID,
	ControlHandle	inMacControlH)
	: LStdControl(inPaneInfo, inValueMessage, inValue, 0, RadioMax,
					radioButProc, inTextTraitsID, inMacControlH)
{
}

/**************************************************************************
	LStdCDEFRadioButton(const LStdCDEFRadioButton&)	[public]
	
	Copy constructor.
**************************************************************************/
LStdCDEFRadioButton::LStdCDEFRadioButton(
	const LStdCDEFRadioButton	&inOriginal)
	: LStdControl(inOriginal)
{
}

/**************************************************************************
	LStdCDEFButton(LStream*)						[public]
	
	Stream constructor
**************************************************************************/
LStdCDEFRadioButton::LStdCDEFRadioButton(
	LStream *	inStream)
	: LStdControl(inStream)
{
}

/**************************************************************************
	HotSpotResult()									[protected, virtual]
													[complete override ]
	Change value to 1 and broadcast the change so
	radio groups can adapt
**************************************************************************/
void
LStdCDEFRadioButton::HotSpotResult(
	Int16	inHotSpot)
{
	SetValue(1);					// Turn button ON
	BroadcastMessage(msg_ControlClicked, (void*) this);
}

/**************************************************************************
	LStdCDEFIconButton(params)						[public]
**************************************************************************/
LStdCDEFIconButton::LStdCDEFIconButton(
	const SPaneInfo	&inPaneInfo,
	MessageT		inValueMessage,
	ResIDT			inIconID,
	ResIDT			inCDEFid)
	: LStdControl(inPaneInfo, inValueMessage, inIconID, min_Int16, max_Int16,
			(inCDEFid << 4) + iconButProc, 0, 0, 0)
{
}

/**************************************************************************
	HotSpotResult()									[protected, virtual]
													[complete override ]
	like a pushButton, fire off a broadcast on click
**************************************************************************/
void
LStdCDEFIconButton::HotSpotResult(
	Int16	inHotSpot)
{
	BroadcastValueMessage();	
}
