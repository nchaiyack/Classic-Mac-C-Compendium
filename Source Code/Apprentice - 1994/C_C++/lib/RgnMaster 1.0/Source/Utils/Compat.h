// Compat.h Header File

#include <GestaltEqu.h>


typedef struct MacEnviron {
	Boolean gestaltAvail;
	Boolean WNEAvail;
	long sysVersion;
	short cpu;
	short fpu;
	Boolean hasColor;
	short pixDepth;
	Boolean hasTempMem;
	Boolean hasSoundChip;
};

extern MacEnviron gEnviron;

void CheckEnviron();