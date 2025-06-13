//////
///// Select Multiple Files Demo 1.0 ¥ By Eddy J. Gurney / ©1994 The Eccentricity Group
////
/// See "Select Multiple Files.c" for more information.
//

#pragma once

// Function prototypes
extern void ErrorHandler(StringPtr);

// I don't like #define's...

/* Character codes */
enum {
	homeKey = 0x01,
	enterKey = 0x03,
	endKey,
	helpKey,
	deleteKey = 0x08,
	tabKey,
	pageUpKey = 0x0b,
	pageDownKey,
	returnKey,
	anyFunctionKey = 0x10,
	escapeKey = 0x1b,
	leftArrow = 0x1c,
	rightArrow = 0x1d,
	upArrow = 0x1e,
	downArrow = 0x1f
};

/* Other useful constants */
enum {
	kControlInactive = 255,
	kControlActive = 0,
	kListInset = -1,
	kScrollBarWidth = 16,
	kScrollBarAdjust = (kScrollBarWidth - 1)
};

enum {
	rErrorAlert = 500,
	rTotalDialog = 501
};

enum {
	rSFMultipleFileDialog = 509,
	dSFMultipleLine1 = 10,
	dSFMultipleSelectedFilesList,
	dSFMultipleOpenButton,
	dSFMultipleAddAllButton,
	dSFMultipleRemoveButton,
	dSFMultiplePromptStaticText,
	dSFMultipleLine2
};

enum {
	rChangingButtonStrings = 509,
	kOpenFileButton = 1,
	kOpenListButton
};
