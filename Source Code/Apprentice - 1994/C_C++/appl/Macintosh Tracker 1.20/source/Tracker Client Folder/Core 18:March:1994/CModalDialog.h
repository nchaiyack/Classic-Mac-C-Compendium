/* CModalDialog.h */

#pragma once

#include "CWindow.h"

#define DoAllowMenus (True)
#define DontAllowMenus (False)

struct	CModalDialog	:	CWindow
	{
		MyBoolean*	GoAwayLocation;
		EXECUTE(MyBoolean Initialized;)

		/* */		~CModalDialog();
		void		IModalDialog(LongPoint Start, LongPoint Extent, MyBoolean AllowMenus);
		void		MakeMyGrafPtr(LongPoint Start, LongPoint Extent);
		void		DoEventLoop(void);
	};
