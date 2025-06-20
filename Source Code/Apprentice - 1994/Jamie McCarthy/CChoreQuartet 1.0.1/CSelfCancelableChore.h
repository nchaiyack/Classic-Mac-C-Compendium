/*
 * CSelfCancelableChore.h
 *
 */



/********************************/

#pragma once

/********************************/

#include <CChore.h>

/********************************/



class CSelfCancelableChore : public CChore {
	
public:
	
	void			cancelSelf(void);
	Boolean		reallyPerform(void);
	
	void			setUrgent(Boolean theUrgent);
	Boolean		getUrgent(void);
	
protected:
	
	Boolean		isUrgent;
	Boolean		hasBeenCancelled;
	
	
} ;
