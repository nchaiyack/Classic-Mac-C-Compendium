/*
 * CSelfCancelableChore.c
 *
 * A chore that can cancel itself.
 *
 * Chores that subclass off of this class, instead of CChore, have
 * one extra privilege and one extra responsibility.  The privilege
 * is that they can call cancelSelf()--they don't have to be canceled
 * from an outside source.  The responsibility is that they must
 * check reallyPerform() at the start of every Perform() method, and
 * immediately return without doing anything if it's FALSE.
 *
 * � Copyright 1992 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 *
 */



/********************************/

#include "CSelfCancelableChore.h"

/********************************/

#include "CCancelerChore.h"

/********************************/



void CSelfCancelableChore::cancelSelf(void)
{
	hasBeenCancelled = TRUE;
	
	if (!isUrgent) {
		CCancelerChore *myCanceler;
		myCanceler = new(CCancelerChore);
		myCanceler->setCancelee(this);
		gApplication->AssignUrgentChore(myCanceler);
	}
}



Boolean CSelfCancelableChore::reallyPerform(void)
{
	return !hasBeenCancelled;
}



void CSelfCancelableChore::setUrgent(Boolean theUrgent)
{
	isUrgent = (theUrgent != FALSE);
}



Boolean CSelfCancelableChore::getUrgent(void)
{
	return isUrgent;
}

