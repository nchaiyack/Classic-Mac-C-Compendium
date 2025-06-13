/*/
     Project Arashi: CBaseObject.h
     Major release: Version 1.1, 7/22/92

     Last modification: Tuesday, February 9, 1993, 8:51
     Created: Friday, October 23, 1992, 16:22

     Copyright © 1992-1993, Juri Munkki
/*/

#pragma once

class	CBaseObject : indirect
{
public:
	short	lockCounter;

	void	IBaseObject();
	void	Lock();
	void	Unlock();	
	void	ForceUnlock();
	void	Dispose();
	void	*Clone();
	void	CloneFields();
	void	*CloneHandle(void *theHandle);
	
	long	HowMuchMemory();
};