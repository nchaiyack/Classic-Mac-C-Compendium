// ©1994-95, Matthew E. Axsom, All Rights Reserved
#include <A4Stuff.h>
#include <Devices.h>

#ifndef	_H_TControlPanel
#include "TControlPanel.h"
#endif

// to be declared in your cdev code (see example CDEV.cp file)
extern long	runable(void);
extern TControlPanel *makeCDEV(short numItems,DialogPtr cp);

// main entry point for control panel
pascal long main(short message,short item,short numItems,short /*privateValue*/,
			     EventRecord *e, TControlPanel *cdev, DialogPtr d)
{
	// set up a4 so we can access the globals
	long oldA4=SetCurrentA4();
	
	// return code
	long 	result=0;
	
	switch (message) {
		// do initialization
		case initDev:
			if ((long)cdev == cdevUnset) {
				cdev = makeCDEV(numItems,d);
				cdev->Init();		// call init method
			}
			break;
				
		// control panel is closing
		case closeDev:
			result=cdevUnset;
			break;
		
		// can we run? return 1 if so, else 0
		case macDev:
			result=runable();
			break;
		
		// it's not an init, open, or close message
		default:
			if ((long)cdev != cdevUnset) {
				// copy over this event and call the action proc
				cdev->fEvent = e;			
				result=cdev->actions(message,item);
			}
			break;
	}
	
	// If this is macDev, then we do not need any additional error checking.  The result
	// from runable is just what we need.
	if (message != macDev) {
		// if there is no error code then make sure we return
		// the cdev as the result, as per IM:MMT pg 8-30
		if (result == 0)
			result = (long)cdev;
		else {
			// there's either an err, or we are closing.  In either case, delete the cdev
			cdev->Close();	// call close routine.
			delete cdev;	// delete the object
			
			// if we are deleting then result is cdevUnset and there is no err.  Otherwise it
			// must be an error code returned by one of the functions.
			
			// make sure the error message conforms to one of the standard return values.
			// If the message doesn't match one of the standard ones, return a
			// generic error that simply closes the cdev w/no message to the user.
			// See NIM:More Mac Toolbox pg 8-47
			switch (result) {
				case cdevGenErr:
				case cdevResErr:
				case cdevUnset:
					break;
				
				// translate internal out-of-memory code to OS acceptable
				// out-of-memory code.  See define of cdevFWMemErr for additional
				// explanation.
				case cdevFWMemErr:
					result=cdevMemErr;
					break;
					
				default:
					result=cdevGenErr;
					break;
			}
		}
	}
	
	// restore a4 before exiting
	SetA4(oldA4);
		
	return(result);
}