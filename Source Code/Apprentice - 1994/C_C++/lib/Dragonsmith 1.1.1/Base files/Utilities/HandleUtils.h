/*
	HandleUtils.h
*/

#pragma once

OSErr CopyHandle (Handle srcHndl, Handle destHndl);
OSErr ResizeHandle (Handle h, long newSize);
Handle BigHandle (long *actualSize);
Handle AnyHandle (long size);
OSErr HandleToScrap (Handle h, ResType type);
char SmartHLock (Handle h);