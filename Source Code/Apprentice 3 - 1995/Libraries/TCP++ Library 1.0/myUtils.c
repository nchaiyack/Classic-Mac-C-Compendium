
#include "MyUtils.h"

#ifndef __PACKAGES__
#include <Packages.h>
#endif
#ifndef __STDARG__
#include <StdArg.h>
#endif
#ifndef __STDIO__
#include <StdIO.h>
#endif
#ifndef __STRINGS__
#include <Strings.h>
#endif

void
BlockSet(void* ptr, long len, char what)
{
	while (len--)
	{
		*((char*) ptr) = what;
		ptr = (void*)((long)ptr+1);
	}
}

OSErr
DebugIfErr(OSErr theErr)
{
	if (theErr != noErr) {
		Str255	pstr;
		
		NumToString((long) theErr, pstr);
		DebugStr(pstr);
	}
	
	return theErr;
}

void
PStrCat(Str255 base, Str255 append)
{
	int	lenBase = Length(base);
	int	lenAppend = Length(append);
	int	j;
	
	for (j = 1; j <= lenAppend; j++) {
		base[lenBase + j] = append[j];
	}
	
	base[0] = lenBase + lenAppend;
}

void
MBPrintf(char* form, ...)
{
	va_list ap;
	char	str[256];
	
	va_start(ap, form);
	vsprintf(str, form, ap);
	va_end(ap);
	
	DebugStr(c2pstr(str));
}

Boolean
SameProcesses(ProcessSerialNumber* p1, ProcessSerialNumber* p2)
{
	Boolean same;
	SameProcess(p1, p2, &same);
	return same;
}
