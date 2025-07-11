// Compat.h Header File

#ifndef COMPAT_H_
#define COMPAT_H_

#include <GestaltEqu.h>


typedef struct {
	Boolean gestaltAvail;
	Boolean WNEAvail;
	long sysVersion;
	short cpu;
	short fpu;
	Boolean hasColor;
	Boolean hasCQD;
	short pixDepth;
	Boolean hasTempMem;
	Boolean hasSoundChip;
} MacEnviron;

extern MacEnviron gEnviron;

void CheckEnviron();

#endif // COMPAT_H_