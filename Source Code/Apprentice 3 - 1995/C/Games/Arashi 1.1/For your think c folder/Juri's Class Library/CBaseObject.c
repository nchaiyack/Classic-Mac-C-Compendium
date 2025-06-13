/*/
     Project Arashi: CBaseObject.c
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, February 9, 1993, 8:51
     Created: Friday, October 23, 1992, 16:22

     Copyright © 1992-1993, Juri Munkki
/*/

#include <CBaseObject.h>

void	CBaseObject::IBaseObject()
{
	lockCounter = 0;
}

void	CBaseObject::Lock()
{
	if(!lockCounter)
		HLock((Handle)this);
	lockCounter++;
}

void	CBaseObject::Unlock()
{
	if(lockCounter != 0)
	{	if(--lockCounter == 0)
		{	HUnlock((Handle)this);
		}
	}
}

void	CBaseObject::ForceUnlock()
{
	if(lockCounter > 1) lockCounter = 1;
	Unlock();
}	

void	CBaseObject::Dispose()
{
	delete(this);
}

/*
**	This is a utility method for all
**	my classes. The idea is to make
**	copying Handles that are part of
**	an object easy like this:
**
**		thing = CloneHandle(thing);
**
**	Note that you can't simply call
**	HandToHand(&thing), because thing
**	is an instance variable in a block
**	that might move.
*/
void	*CBaseObject::CloneHandle(
	void	*theHandle)
{
	short	state;
	
	state = HGetState(theHandle);
	HandToHand(&theHandle);
	if(theHandle)
		HSetState(theHandle, state);
	
	return theHandle;
}

/*
**	Never call this method, but override it,
**	if you are writing a class that should
**	duplicate handles when an object is cloned.
*/
void	CBaseObject::CloneFields()
{

}

/*
**	Call clone to make a deep copy of the object.
*/
void	*CBaseObject::Clone()
{
	CBaseObject	*myCopy;
	short		state;
	
	myCopy = this;
	
	state = HGetState(this);
	HandToHand(&myCopy);
	
	if(myCopy)
	{	HSetState(myCopy,state);
		myCopy->CloneFields();
	}
	
	return myCopy;
}

long	CBaseObject::HowMuchMemory()
{
	return GetHandleSize(this);
}