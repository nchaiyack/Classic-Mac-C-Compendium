/* CAboutWindow.h */

#pragma once

#include "CWindow.h"
#include "CSimpleButton.h"

struct	CAboutWindow	:	CWindow
	{
		long			MomentOfInstantiation;
		MyBoolean	AutoFlag;

		/* */			~CAboutWindow();
		void			IAboutWindow(MyBoolean TheAutoFlag);
		void			DoIdle(long TimeSinceLastEvent);
		void			GoAway(void);
		MyBoolean	DoMenuCommand(ushort MenuCommandValue);
		void			EnableMenuItems(void);
	};


struct	CAboutWindowPicture	:	CViewRect
	{
		void			IAboutWindowPicture(CWindow* TheWindow);
		void			DoUpdate(void);
	};
