// Copyright © 1992 Peter Speck, speck@dat.ruc.dk. All rights reserved.
// XTypes.cp

#include "XTypes.h"
#include "Tools.h"

#pragma segment MyTools

void CopyHolToCStr255(HandleOffsetLength hol, CStr255 &text)
{
#if qDebug
	if (hol.fLength < 0 || hol.fLength > 255)
	{
		fprintf(stderr, "Invalid length (%ld) for hol->CStr255 conversion\n", hol.fLength);
		ProgramBreak(gEmptyString);
	}
#endif
	text.Length() = short(Min(250, hol.fLength));
	if (text.Length())
		BytesMove(*hol.fH + hol.fOffset, &text[1], text.Length());
}
