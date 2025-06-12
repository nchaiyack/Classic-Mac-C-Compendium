/* CObject.h */

#pragma once

struct	CObject	:	indirect
	{
		void*		operator new (unsigned long);
		void		operator delete (void*);
	};
