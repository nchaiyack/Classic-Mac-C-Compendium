/*
 * CDisposerChore.h
 *
 */



/********************************/

#pragma once

/********************************/

#include <CChore.h>

/********************************/

void disposeViaUrgentChore(CObject *theDisposee);

/********************************/



class CDisposerChore : public CChore {
	
public:
	
	void			Perform(long *maxSleep);
	
	void			setDisposee(CObject *theDisposee);
	CObject		*getDisposee(void);
	
	
protected:
	
	CObject		*itsDisposee;
	
	
} ;
