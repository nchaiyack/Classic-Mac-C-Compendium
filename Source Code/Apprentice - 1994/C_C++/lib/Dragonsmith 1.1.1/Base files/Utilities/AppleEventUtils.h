/*
	AppleEventUtils.h
*/

#pragma once

#include	<AppleEvents.h>

OSErr SendToSelf (AEEventClass eventClass , AEEventID eventID, AEDesc *directObject,
															AEDesc *reply, AESendMode sendMode);

OSErr GotRequiredParams (AppleEvent *theEvent);

OSErr AEGetNthBoolean (AEDescList *listOrRecord, long index, AEKeyword *keyword, Boolean *result);
OSErr AEGetNthShort (AEDescList *listOrRecord, long index, AEKeyword *keyword, short *result);
OSErr AEGetNthLong (AEDescList *listOrRecord, long index, AEKeyword *keyword, long *result);
OSErr AEGetNthType (AEDescList *listOrRecord, long index, AEKeyword *keyword, DescType *result);
OSErr AEGetNthStr255 (AEDescList *listOrRecord, long index, AEKeyword *keyword, StringPtr result);
OSErr AEGetNthList (AEDescList *listOrRecord, long index, AEKeyword *keyword, AEDescList *result);

OSErr AEGetKeyBoolean (AERecord *record, AEKeyword keyword, Boolean *result);
OSErr AEGetKeyShort (AERecord *record, AEKeyword keyword, short *result);
OSErr AEGetKeyLong (AERecord *record, AEKeyword keyword, long *result);
OSErr AEGetKeyType (AERecord *record, AEKeyword keyword, DescType *result);
OSErr AEGetKeyStr255 (AERecord *record, AEKeyword keyword, StringPtr result);
OSErr AEGetKeyList (AERecord *record, AEKeyword keyword, AEDescList *result);
