//==================================================================
// Failure.h										<tur 26-Feb-94>
//
//	Quickie implementation of a Failure module.
//
//==================================================================

#ifndef	__Failure__
#define	__Failure__

void Fail(OSErr ErrorID, char *FileName, short LineNum, Boolean Fatal);

void showFailure(ConstStr255Param unixStr);

#endif	/* __Failure__ */