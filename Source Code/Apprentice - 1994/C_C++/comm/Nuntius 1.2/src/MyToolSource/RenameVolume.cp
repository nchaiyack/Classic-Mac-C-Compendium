#include <Files.h>
#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>


#if 0
FUNCTION PBSetVInfo (paramBlock:��HParmBlkPtr; async:��BOOLEAN) :��OSErr;

��� Use the XRef menu to refer to Technical Note #204.���

Trap macro    _SetVolInfo

Parameter block
  -->    12    ioCompletion  pointer
  <--    16    ioResult      word
  -->    18    ioNamePtr     pointer
  -->    22    ioVRefNum     word
  -->    30    ioVCrDate     long word
  -->    34    ioVLsMod      long word
  -->    38    ioVAtrb       word
  -->    52    ioVClpSiz     long word
  -->    72    ioVBkUp       long word
  -->    76    ioVSeqNum     word
  -->    90    ioVFndrInfo   32 bytes


FUNCTION PBHGetVInfo (paramBlock:��HParmBlkPtr; async:��BOOLEAN) :��OSErr;

Trap macro    _HGetVInfo

Parameter block
  -->    12    ioCompletion  pointer
  <--    16    ioResult      word
  <->    18    ioNamePtr     pointer
  <->    22    ioVRefNum     word
  -->    28    ioVolIndex    word
  <--    30    ioVCrDate     long word
  <--    34    ioVLsMod      long word
  <--    38    ioVAtrb       word
  <--    40    ioVNmFls      word
  <--    42    ioVBitMap     word
  <--    44    ioVAllocPtr   word
  <--    46    ioVNmAlBlks   word
  <--    48    ioVAlBlkSiz   long word
  <--    52    ioVClpSiz     long word
  <--    56    ioAlBlSt      word
  <--    58    ioVNxtFNum    long word
  <--    62    ioVFrBlk      word
  <--    64    ioVSigWord    word
  <--    66    ioVDrvInfo    word
  <--    68    ioVDRefNum    word
  <--    70    ioVFSID       word
  <--    72    ioVBkUp       long word
  <--    76    ioVSeqNum     word
  <--    78    ioVWrCnt      long word
  <--    82    ioVFilCnt     long word
  <--    86    ioVDirCnt     long word
  <--    90    ioVFndrInfo   32 bytes
#endif

void PrintUsage()
{
	fprintf(stderr, "### RenameVolume  oldName  newName\n");
	fprintf(stderr, "### Bad or missing parameter\n");
	exit(1);
}

void FailOSErr(OSErr err, const char *msg)
{
	if (err == noErr)
		return;
	fprintf(stderr, "### RenameVolume: got error = %ld\n", long(err));
	fprintf(stderr, "### At: %s\n", msg);
	exit(1);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
		PrintUsage();
	Str255 oldName, newName;
	strcpy((char*)oldName, argv[1]);
	strcpy((char*)newName, argv[2]);
//	fprintf(stderr, "oldName = '%s', newName = '%s'\n", oldName, newName);
	c2pstr((char*)oldName);
	c2pstr((char*)newName);
	HParamBlockRec pb;
	memset(&pb, sizeof(pb), 1);
	pb.volumeParam.ioNamePtr = oldName;
	pb.volumeParam.ioVRefNum = 32000;
	pb.volumeParam.ioVolIndex = -1; // use name and vrefnum
	FailOSErr(PBHGetVInfoSync(&pb), "PBHGetVInfoSync");
	pb.volumeParam.ioNamePtr = newName;
	FailOSErr(PBSetVInfoSync(&pb), "PBSetVInfoSync");
	return 0;
}