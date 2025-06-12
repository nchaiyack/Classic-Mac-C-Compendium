/* CDocument.h */

#pragma once

#include "EventLoop.h"

struct	CDocument	:	CIdle
	{
		/* */			CDocument();
		/* */			~CDocument();
		void			DoNewFile(void);
		void			DoOpenFile(FSSpec* TheSpec);
		void			DoPrint(void);
		MyBoolean	GoAway(void);
	};
