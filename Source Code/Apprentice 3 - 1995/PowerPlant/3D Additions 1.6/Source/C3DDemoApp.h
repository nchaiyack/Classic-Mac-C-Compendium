// ===========================================================================
//	C3DDemoApp.h
// ===========================================================================

#pragma once

#include <LApplication.h>

class	LWindow;

class	C3DDemoApp : public LApplication {
public:
						C3DDemoApp();
	virtual 			~C3DDemoApp();
	
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
							
protected:
	LWindow 	*mDisplayWindow;
};