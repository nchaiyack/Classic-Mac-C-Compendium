/*
 * CCancelerChore.h
 *
 */



/********************************/

#pragma once

/********************************/

#include <CChore.h>

/********************************/



class CCancelerChore : public CChore {
	
public:
	
	void			Perform(long *maxSleep);
	
	void			setCancelee(CChore *theCancelee);
	CObject		*getCancelee(void);
	
	
protected:
	
	CChore		*itsCancelee;
	
	
} ;
