/*
 * CCommanderChore.c
 *
 * A chore that sends a command.
 *
 * There are three ways to send the command:  once with an urgent
 * chore, once with an idle chore, or continuously with an idle
 * chore.  I advise using the functions doCommandViaUrgentChore(),
 * doCommandViaIdleChore(), and doCommandContinuouslyViaIdleChore();
 * they'll do all the setting-up for you.  The latter returns the
 * chore in question, so that if you want to cancel it later,
 * you can.  The former two cancel themselves, so you don't need
 * to worry about them.
 *
 * � Copyright 1992-93 by Jamie R. McCarthy.  All rights reserved.
 * This code can be both distributed and used freely.
 * Internet: k044477@kzoo.edu			AppleLink: j.mccarthy
 *
 */



/********************************/

#include "CCommanderChore.h"

/********************************/



void doCommandViaIdleChore(long theCommand)
{
	CCommanderChore *theChore;
	theChore = new(CCommanderChore);
	theChore->setCommand(theCommand);
	theChore->setUrgent(FALSE);
	theChore->setSendsContinuously(FALSE);
	gApplication->AssignIdleChore(theChore);
}



CCommanderChore *doCommandContinuouslyViaIdleChore(long theCommand)
{
	CCommanderChore *theChore;
	theChore = new(CCommanderChore);
	theChore->setCommand(theCommand);
	theChore->setUrgent(FALSE);
	theChore->setSendsContinuously(TRUE);
	gApplication->AssignIdleChore(theChore);
}



void doCommandViaUrgentChore(long theCommand)
{
	CCommanderChore *theChore;
	theChore = new(CCommanderChore);
	theChore->setCommand(theCommand);
	theChore->setUrgent(TRUE);
	gApplication->AssignUrgentChore(theChore);
}



void CCommanderChore::Perform(long *maxSleep)
{
	if (!reallyPerform()) return;
	
	TRY {
		gGopher->DoCommand(itsCommand);
	} CATCH {
			/*
			 * If some kind of error occurred, forget about doing
			 * this command again.
			 */
		cancelSelf();
	} ENDTRY;
	
	if (!sendsContinuously) {
		itsCommand = cmdNull;
		cancelSelf();
	}
}



void CCommanderChore::setCommand(long theCommand)
{
	itsCommand = theCommand;
}



long CCommanderChore::getCommand(void)
{
	return itsCommand;
}



void CCommanderChore::setSendsContinuously(Boolean theSendsContinuously)
{
	sendsContinuously = (theSendsContinuously != FALSE);
}



Boolean CCommanderChore::getSendsContinuously(void)
{
	return sendsContinuously;
}
