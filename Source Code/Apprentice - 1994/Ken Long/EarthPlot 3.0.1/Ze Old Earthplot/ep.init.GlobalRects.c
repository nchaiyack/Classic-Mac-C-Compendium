#include "Quickdraw.h"
#include "ep.extern.h"

initGlobalRects()
{
	SetRect(&latSBRect,		-1,118,164,134);
	SetRect(&lonSBRect,		-1,198,164,214);
	SetRect(&altSBRect,		-1,279,164,295);

	SetRect(&latDataRect,	75, 62,175, 82);
	SetRect(&lonDataRect,	82,142,175,162);
	SetRect(&altDataRect,	73,222,175,242);

	SetRect(&northRect,		 5, 94, 74,114);
	SetRect(&southRect,		75, 94,145,114);
	SetRect(&eastRect,		 5,174, 74,194);
	SetRect(&westRect,		75,174,145,194);

	SetRect(&mileRect,		5,255,65,274);
	SetRect(&kmRect,		66,254,175,274);

	SetRect(&iconRect,		10,10,42,42);

	SetRect(&controlWindowRect,	 12, 30+13, 175, 324+13);
	SetRect(&earthWindowRect,	195, 30+13, 499, 324+13);

}

