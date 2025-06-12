/* Cdev interface for Felix. June 30th, 1993 */
/* Uses the THINK C supplied object class, which makes things very simple */

#include "cdev.h"

class FelixCdev : public cdev {
	public : 
	virtual void ItemHit (short item);	
};


/* Runnable - should the cdev appear in the Control Panel? This implements the "macDev" message. */
/* Here we demand System 7 */

Boolean Runnable()
{
	OSErr 		e;
	SysEnvRec 	env;

	e = SysEnvirons(1, &env);
	return (env.systemVersion >= 0x700);
}

/* New - create the cdev object */

cdev* New()
{
	return (new (FelixCdev));
}

/* ItemHit has to be overriden to define our behavior. Here we have a single button */

void FelixCdev::ItemHit(short item)
{
	StandardFileReply		reply;
	SFTypeList			dummy;
	Handle				h;

	if (item == 1) {
		StandardGetFile(nil, -1, dummy, &reply);
		if (reply.sfGood) {
			h = GetResource('DDIR', 0);
			if (!ResError()) {
				* (long*) (*h) = reply.sfFile.parID;
				ChangedResource(h);
			}
			else SysBeep(0);
		}
	}
}