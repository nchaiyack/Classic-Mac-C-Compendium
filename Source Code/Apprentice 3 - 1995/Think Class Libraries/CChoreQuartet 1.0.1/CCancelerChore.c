/*
 * CCancelerChore.c
 *
 * A chore that cancels another chore.  Assign this as
 * an urgent chore.
 *
 * � Copyright 1992 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 *
 */



/********************************/

#include "CCancelerChore.h"

/********************************/

#include <CList.h>

/********************************/



void CCancelerChore::Perform(long *maxSleep)
{
	ASSERT(itsCancelee != NULL);
	ASSERT(gApplication->itsIdleChores->Includes(itsCancelee));
	gApplication->CancelIdleChore(itsCancelee);
	itsCancelee = NULL;
}



void CCancelerChore::setCancelee(CChore *theCancelee)
{
	itsCancelee = theCancelee;
}



CObject *CCancelerChore::getCancelee(void)
{
	return itsCancelee;
}


