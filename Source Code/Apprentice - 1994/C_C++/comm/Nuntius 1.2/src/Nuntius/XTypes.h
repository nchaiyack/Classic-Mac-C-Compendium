// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// XTypes.h

#define __XTYPES__

struct HandleOffset
{
	Handle fH;
	long fOffset;
};

struct OffsetLength
{
	long fOffset;
	long fLength;
};
typedef OffsetLength *OffsetLengthPtr;

struct HandleOffsetLength
{
	Handle fH;
	long fOffset;
	long fLength;
};

void CopyHolToCStr255(HandleOffsetLength hol, CStr255 &text);
