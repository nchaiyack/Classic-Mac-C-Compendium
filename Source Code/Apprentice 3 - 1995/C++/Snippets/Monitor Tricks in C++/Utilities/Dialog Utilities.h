// Dialog Utilities.h v1.0
//
// by Kenneth Worley
// Public Domain
//
// Contact me at:  America Online: KNEworley
//			internet: KNEworley@aol.com or kworley@fascination.com
//
// Functions to make working with dialog items, controls, and
// text fields (and the same in control panels) easier and
// more convenient.
//

#ifndef Dialog_Utilities_h
#define Dialog_Utilities_h

#ifdef __cplusplus
extern "C" {
#endif

// GetItemHandle returns the handle of an item in a dialog.
Handle GetItemHandle( DialogPtr dlg, short itemNo );

// GetOnOff returns the on/off status of a radio button or check box.
Boolean GetOnOff( DialogPtr dlg, short itemNo );

// SetOnOff turns a radio button or check box on or off.
void SetOnOff( DialogPtr dlg, short itemNo, Boolean turnOn );

// Toggle toggles the on/off value of a radio button or check box.
void Toggle( DialogPtr dlg, short itemNo );

// GetRadioButton returns the item number of the first radio button
// it finds turned on in the given range of items.
short GetRadioButton( DialogPtr dlg, short first, short last );

// SetRadioButton turns on the radio button whose item number is
// specified in target and makes sure the rest of the radio buttons
// in the given range are off.
void SetRadioButton( DialogPtr dlg, short first, short last, short target );

// SetChars sets a text field in a dialog to the text specified.
void SetChars( DialogPtr dlg, short itemNo, char *str, short len );

// GetChars returns the text and length of text in the specified field.
void GetChars( DialogPtr dlg, short itemNo, char *str, short *len );

// SetText sets a text field in a dialog to the text specified.
void SetText( DialogPtr dlg, short itemNo, Str255 str );

// GetText returns the text of the specified field.
void GetText( DialogPtr dlg, short itemNo, Str255 str );

// Activate control either activates or deactivates a control
// depending on the activate parameter.
void ActivateControl( DialogPtr dlg, short itemNo, Boolean activate );

#ifdef __cplusplus
}
#endif

#endif // Dialog_Utilities_h