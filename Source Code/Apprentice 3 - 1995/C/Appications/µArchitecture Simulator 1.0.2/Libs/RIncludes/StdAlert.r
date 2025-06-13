/*
Copyright © 1993,1994 by Fabrizio Oddone
••• ••• ••• ••• ••• ••• ••• ••• ••• •••
This source code is distributed as freeware: you can copy, exchange, modify this
code as you wish. You may include this code in any kind of application: freeware,
shareware, or commercial, provided that full credits are given.
You may not sell or distribute this code for profit.
*/

#define A					13    // white space between most elements
#define B					23    // white space to left and right of icon

//#define NumTextLines		4    // number of lines of text in the alert
#define NumAltTextLines		0

#define LineHeight			16    // height of a single line of Chicago-12
#define ButtonHeight		20    // standard button height

#ifdef	__USVersion__
#define LongestBtnName		41    // width of “Cancel” in Chicago-12
#define DontSaveBtnName		67    // width of “Don’t Save” in Chicago-12
#endif	__USVersion__

#ifdef	__VersioneItaliana__
#define LongestBtnName		46    // width of “Annulla” in Chicago-12
#define DontSaveBtnName		96    // width of “Non Registrare” in Chicago-12
#endif	__VersioneItaliana__

#define LongestButtonName	LongestBtnName    // placeholder
#define DontSaveButtonName	DontSaveBtnName    // placeholder

#define DontSaveButtonWidth	(DontSaveButtonName + 18)
#define ButtonWidth			(LongestButtonName + 18)

#define AlrtWidth			354    // chosen to make the right margin = A
#define AlertWidth			AlrtWidth    // placeholder

#define IconLeft			(B - 3)
#define IconRight			(IconLeft + 32)
#define IconTop				(A - 3)
#define IconBottom			(IconTop + 32)

#define TextLeft			(IconRight + (B - 1))
#define TextRight			(AlertWidth - (A - 3))
#define TextTop				(A - 6)
#define TextBottom			(TextTop + (NumTextLines * LineHeight))

#define AltTextTop			TextBottom
#define AltTextBottom		(AltTextTop + (NumAltTextLines * LineHeight))

#define ButtonTop			(AltTextBottom + A)
#define ButtonBottom		(ButtonTop + ButtonHeight)
#define ActionButtonRight	(AlertWidth - (A - 3))
#define ActionButtonLeft	(ActionButtonRight - ButtonWidth)
#define CancelButtonRight	(ActionButtonLeft - A)
#define CancelButtonLeft	(CancelButtonRight - ButtonWidth)
#define DontSaveButtonRight	(DontSaveButtonLeft + DontSaveButtonWidth)
#define DontSaveButtonLeft	TextLeft
#define ThirdButtonRight	(CancelButtonLeft - A)
#define ThirdButtonLeft		(ThirdButtonRight - ButtonWidth)
#define FourthButtonRight	(ThirdButtonLeft - A)
#define FourthButtonLeft	(FourthButtonRight - ButtonWidth)

#define AlertHeight			(ButtonBottom + (A - 3))

#define AlertTop			70
#define AlertLeft			20
#define AlertBottom			(AlertTop+AlertHeight)
#define AlertRight			(AlertLeft+AlertWidth)
