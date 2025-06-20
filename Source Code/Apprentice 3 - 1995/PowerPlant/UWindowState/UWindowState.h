// ===========================================================================
//	UWindowState.h						   �1994 J. Rodden, DD/MF & Associates
// ===========================================================================
// Utilities for saving and restoring a window's state (position, size, and
// zoom state) in a file. Typically usefull for document windows as per
// Apple's User Interface Guidelines. Use when opening and saving a document.
//
// All rights reserved. You may use this code in any application, recognition
// would be appreciated but is not required.

#pragma once

class LFile;
class LWindow;

class UWindowState {
public:
	static void RestoreWindowState( LFile* inFile, LWindow* inWindow, short inResID = 0);
	static void SaveWindowState( LFile* inFile, LWindow* inWindow, short inResID = 0);
};