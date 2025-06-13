/* this file is needed only if you want to compile suntar 1.1 or later with
A pre-System 7.0 version of Think-C: since that's what we have done,
it must be skipped to compile suntar on a newer release
(Think-C 5.0): these damned compilers are stupid, they
don't accept that a type or function be declared twice, even if
the two declarations are absolutely identical. And obviously
these declarations do exist in new header files.
*/

#ifndef THINK_C_5


#ifndef	_MacTypes_
#include "MacTypes.h"
#endif

typedef unsigned char Str63[64];
typedef short ScriptCode;

typedef struct FSSpec {
	short	vRefNum;
	long	parID;
	Str63	name;
} FSSpec;
typedef FSSpec *FSSpecPtr;

struct StandardFileReply {
    Boolean sfGood;
    Boolean sfReplacing;
    OSType sfType;
    FSSpec sfFile;
    ScriptCode sfScript;
    short sfFlags;
    Boolean sfIsFolder;
    Boolean sfIsVolume;
    long sfReserved1;
    short sfReserved2;
};
typedef struct StandardFileReply StandardFileReply;


pascal void CustomPutFile(Str255,Str255,StandardFileReply *,short,Point,ProcPtr,ProcPtr,short *,ProcPtr,void *)
    = {0x3F3C,0x0007,0xA9EA}; 


pascal OSErr FSMakeFSSpec( short, long, StringPtr,FSSpecPtr) = {0x303C,0x0001,0xAA52};


#endif

pascal OSErr ResolveAliasFile(FSSpec*,Boolean,Boolean *,Boolean	*) = { 0x700C,0xA823};
