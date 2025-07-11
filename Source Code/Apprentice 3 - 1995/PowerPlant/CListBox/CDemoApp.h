// =================================================================================
//	CDemoApp.h						�1994 AG Group, Inc. All rights reserved.
// =================================================================================
//	CDemoApp.cp			<- double-click + Command-D to see class implementation

#pragma once

#include <LDocApplication.h>

#define __CDEMOAPP__


class CDemoApp : public LApplication {

public:
						CDemoApp(void);
	virtual 			~CDemoApp(void);
	
	virtual Boolean		ObeyCommand(CommandT inCommand, void *ioParam = nil);
	virtual void		FindCommandStatus(CommandT inCommand,
							Boolean &outEnabled, Boolean &outUsesMark,
							Char16 &outMark, Str255 outName);
};