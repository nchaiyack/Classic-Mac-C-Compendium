#ifndef __MYUTILS__
#define __MYUTILS__

#ifndef __TYPES__
#include <Types.h>
#endif
#ifndef __PROCESSES__
#include <Processes.h>
#endif

#define BlockClear(ptr, size)	BlockSet(ptr, size, 0);

#ifdef __cplusplus
extern "C" {
#endif
	typedef ProcessSerialNumber PSN, *PSNPtr, **PSNHnd;


	Boolean	SameProcesses(ProcessSerialNumber* p1, ProcessSerialNumber* p2);
	void	BlockSet(void* ptr, long len, char what);
	OSErr	DebugIfErr(OSErr theErr);
	void	PStrCat(Str255 base, Str255 append);
	void	MBPrintf(char* form, ...);

#ifdef __cplusplus
}
#endif


#endif