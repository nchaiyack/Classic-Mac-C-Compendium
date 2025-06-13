/* EDStuff - Implementation of the ed-based file-transfer routines            */

#ifndef __EDSTUFF__
#define __EDSTUFF__

#ifndef __MUDDOC__
#include "MUDDoc.h"
#endif


pascal void EDReceive (TMUDDoc *theDoc, Str255 path, Str255 fileName,
		short vRefNum);
pascal void EDSend (TMUDDoc *theDoc, AppFile *theFile, Str255 path);

#endif
