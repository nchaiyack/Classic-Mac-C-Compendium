/* CObject.c */

#include "CObject.h"
#include "Memory.h"


#define MemoryFillPattern (0x81)


void*		CObject::operator new (EXECUTE(register) unsigned long nBytes)
	{
		Handle	Temp;
		EXECUTE(register char*		Scan;)

		Temp = AllocHandle(nBytes);
		SetTag(Temp,"CObject");
		EXECUTE(
		/* this zeros the contents of all objects, making error detection easier */
		Scan = *Temp;
		while (nBytes > 0)
			{
				(*Scan) = MemoryFillPattern;
				Scan += 1;
				nBytes -= 1;
			})
		return Temp;
	}


void		CObject::operator delete(void* Object)
	{
		ReleaseHandle((Handle)Object);
	}
