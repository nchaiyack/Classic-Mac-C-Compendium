#include "MetaGlobal.h"

typedef pascal void (*MDEFProc)(short msg, MenuHandle theMenu, Rect* menuRect,
								Point hitPt, short* itemID);

Ptr		GetA0(void) = { 0x2008 };

pascal void main(short msg, MenuHandle theMenu, Rect* menuRect, Point hitPt,
					short* itemID)
{
	StdHeaderHdl	us;
	Handle			mdefHandle;
	char			oldState;
	Handle			origMDEF;
	
	us = (StdHeaderHdl) RecoverHandle(GetA0());
	mdefHandle = (Handle) (**us).refCon;

	if (mdefHandle != NIL) {
		if (*mdefHandle == NIL)  {
			LoadResource(mdefHandle);
		}
		if (*mdefHandle != NIL) {
			origMDEF = (**theMenu).menuProc;
			(**theMenu).menuProc = mdefHandle;
			oldState = HGetState(mdefHandle);
			HLock(mdefHandle);
			((MDEFProc) *mdefHandle)(msg, theMenu, menuRect, hitPt, itemID);
			HSetState(mdefHandle, oldState);
			(**theMenu).menuProc = origMDEF;
			switch (msg) {
				case mChooseMsg:
					MenuDisable = ((long) (**theMenu).menuID << 16) + *itemID;
					break;
				case mPopUpMsg:
					menuRect->top = *itemID;
					break;
			}
			return;
		}
	}

	SysError(dsMDEFNotFound);
}
