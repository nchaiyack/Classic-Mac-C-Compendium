#include "Remember.h"

OSErr				HandleToFSSpec(FileObjectHdl objHandle, FSSpec *spec);
void				FSSpecToHandle(FSSpec *spec, FileObjectHdl objHandle);

void				ReplyToFSSpec(void *reply, FSSpec *spec);
void				FSSpecToReply(FSSpec *spec, void *reply);

void				ReplyToHandle(void *reply, FileObjectHdl objHandle);
void				HandleToReply(FileObjectHdl objHandle, void *reply);

void				SelectionToFSSpec(ListHandle theList, FSSpec *spec);
void				SFEntryToFSSpec(Ptr entry, FSSpec* spec);
