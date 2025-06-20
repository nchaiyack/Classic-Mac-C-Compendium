#include <A4Stuff.h>

typedef	pascal long (*RealCDEVProc)(short, short, short, short,
						EventRecord*, void*, DialogPtr);

pascal long main(short message,short item,short numItems,short privValue,
			     EventRecord *e, void *cdev, DialogPtr dlg)
{
	long	result;
	long	oldA4 = SetCurrentA4();	//A4 must be set up for globals, strings, and 
									//inter-segment calls 
	Handle		resource;
	RealCDEVProc	realProc;
	
	resource = Get1Resource('cDEV', -4064);
	HLock(resource);
	realProc = (RealCDEVProc)*resource;
	result = (realProc)(message, item, numItems, privValue, e, cdev, dlg);

	// are we closing down in any way?
	switch (result)
		{
		case cdevGenErr:				// General error; gray cdev w/o alert*/
		case cdevMemErr:				// Memory shortfall; alert user please*/
		case cdevResErr:				// Couldn't get a needed resource; alert*/
		case cdevUnset:					// cdevValue is initialized to this*/
			HUnlock((Handle)resource);	// unlock the real cdev now
			ReleaseResource(resource);	// 	and release it
			break;
		}
	
	SetA4(oldA4);

	return(result);
}

