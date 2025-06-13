/*
 * CDisposerChore.c
 *
 * A chore that disposes of an object.  Assign this as
 * an urgent chore.
 *
 * © Copyright 1992-93 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 *
 */



/********************************/

#include "CDisposerChore.h"

/********************************/



void disposeViaUrgentChore(CObject *theDisposee)
{
	CDisposerChore *theChore;
	theChore = new(CDisposerChore);
	theChore->setDisposee(theDisposee);
	gApplication->AssignUrgentChore(theChore);
}



void CDisposerChore::Perform(long *maxSleep)
{
	ForgetObject(itsDisposee);
}



void CDisposerChore::setDisposee(CObject *theDisposee)
{
	ASSERT(member(theDisposee, CObject));
	itsDisposee = theDisposee;
}



CObject *CDisposerChore::getDisposee(void)
{
	return itsDisposee;
}

