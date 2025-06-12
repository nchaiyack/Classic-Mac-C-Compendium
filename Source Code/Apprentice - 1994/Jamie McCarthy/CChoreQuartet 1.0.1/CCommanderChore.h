/*
 * CCommanderChore.h
 *
 */



/********************************/

#pragma once

/********************************/

#include "CSelfCancelableChore.h"

/********************************/

class CCommanderChore;

void doCommandViaIdleChore(long theCommand);
CCommanderChore *doCommandContinuouslyViaIdleChore(long theCommand);
void doCommandViaUrgentChore(long theCommand);

/********************************/



class CCommanderChore : public CSelfCancelableChore {
	
public:
	
	void			Perform(long *maxSleep);
	
	void			setCommand(long theCommand);
	long			getCommand(void);
	
	void			setSendsContinuously(Boolean theSendsContinuously);
	Boolean		getSendsContinuously(void);
	
	
protected:
	
	long			itsCommand;
	Boolean		sendsContinuously;
	
	
} ;
