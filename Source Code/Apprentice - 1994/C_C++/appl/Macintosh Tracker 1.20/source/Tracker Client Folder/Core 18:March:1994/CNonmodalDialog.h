/* CNonmodalDialog.h */

#pragma once

#include "CModalDialog.h"


struct	CNonmodalDialog	:	CModalDialog
	{
		void		MakeMyGrafPtr(LongPoint Start, LongPoint Extent);
	};
