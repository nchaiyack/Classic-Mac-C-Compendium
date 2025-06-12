#include "ep.const.h"
#include "ep.extern.h"
#include "DialogMgr.h"

DoEditMenu (item)
int		item;
{
long	result;

	switch (item) { 
	case 3: /* Cut */
	case 4: /* Copy */
		ZeroScrap();
		HLock(ePict);
		result = PutScrap(GetHandleSize(ePict),'PICT',*ePict);
		HUnlock(ePict);
		result = UnloadScrap();
 		break;
 	case 6: /* clear */
 		ZeroScrap();
 		break;
 	default:
 		SysBeep(3);
 		break;
	}
}
