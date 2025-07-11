/*
	AppleEventUtils.c
	
	Functions to simplify Apple event processing
	
	Copyright � 1992�1993 by Paul M. Hoffman
	Send feedback to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.
	
	Created	11 Jul 1992	Extracted GetFSSpecsFromAEDescList from Dragon.c
	Modified	12 Sep 1992	Removed GetFSSpecsFromAEDescList � it's no longer needed in Dragonsmith
			16 May 1993	Added AEGetNthBoolean, AEGetKeyBoolean, etc.
			01 Aug 1993	Minor modifications
			14 Aug 1993	Added SendToSelf
			
*/

#include	"AppleEventUtils.h"
#include	"HandleUtils.h"

OSErr SendToSelf (AEEventClass eventClass , AEEventID eventID, AEDesc *directObject,
															AEDesc *reply, AESendMode sendMode)
{
	ProcessSerialNumber	selfPSN = { 0, kCurrentProcess };
	AEAddressDesc		selfAddress;
	AEDesc				ignoreReply;
	AppleEvent			theEvent;
	OSErr				err;
	
	err = AECreateDesc (typeProcessSerialNumber, (Ptr) &selfPSN, sizeof (ProcessSerialNumber), &selfAddress);
	err = AECreateAppleEvent (eventClass, eventID, &selfAddress, kAutoGenerateReturnID, kAnyTransactionID, &theEvent);
	if (reply == NULL)
		reply = &ignoreReply;
	if (directObject != NULL)
		err  = AEPutParamDesc (&theEvent, keyDirectObject, directObject);
	err = AESend (&theEvent, reply, sendMode, kAENormalPriority, kAEDefaultTimeout, NULL, NULL);
}

OSErr GotRequiredParams (AppleEvent *theEvent)
{
	DescType	returnedType;
	long			actualSize;
	OSErr		err;
	
	err = AEGetAttributePtr (theEvent, keyMissedKeywordAttr, typeWildCard, &returnedType, NULL, 0, &actualSize);
	if (err == errAEDescNotFound)
		return noErr;
	else if (err == noErr)
		return errAEEventNotHandled;
	else
		return err;
}

OSErr AEGetNthBoolean (AEDescList *listOrRecord, long index, AEKeyword *keyword, Boolean *result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetNthPtr (listOrRecord, index, typeBoolean, keyword, &typeCode, (Ptr) result, 1, &actualSize);
	if (err == noErr && actualSize == 0)
		return errAECoercionFail;
	else
		return err;
}

OSErr AEGetNthShort (AEDescList *listOrRecord, long index, AEKeyword *keyword, short *result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetNthPtr (listOrRecord, index, typeShortInteger, keyword, &typeCode, (Ptr) result, sizeof (short), &actualSize);
	if (err == noErr && actualSize < sizeof (short))
		return errAECoercionFail;
	else
		return err;
}

OSErr AEGetNthLong (AEDescList *listOrRecord, long index, AEKeyword *keyword, long *result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetNthPtr (listOrRecord, index, typeLongInteger, keyword, &typeCode, (Ptr) result, sizeof (long), &actualSize);
	if (err == noErr && actualSize < sizeof (long))
		return errAECoercionFail;
	else
		return err;
}

OSErr AEGetNthType (AEDescList *listOrRecord, long index, AEKeyword *keyword, DescType *result)
{
	DescType	typeCode, tempType = '    ';
	long			actualSize;
	OSErr		err;
	
	err = AEGetNthPtr (listOrRecord, index, typeType, keyword, &typeCode, (Ptr) &tempType, sizeof (long), &actualSize);
	if (err == noErr) {
		if (actualSize == 0)
			return errAECoercionFail;
		else
			*result = tempType;
	}
	return err;
}

OSErr AEGetNthStr255 (AEDescList *listOrRecord, long index, AEKeyword *keyword, StringPtr result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetNthPtr (listOrRecord, index, typeChar, keyword, &typeCode, (Ptr) &result[1], 255, &actualSize);
	if (err == noErr && actualSize > 255)
		return errAECoercionFail;
	if (err == noErr)
		result[0] = actualSize;
	else
		result[0] = 0;
	return err;
}

OSErr AEGetNthList (AEDescList *listOrRecord, long index, AEKeyword *keyword, AEDescList *result)
{
	return AEGetNthDesc (listOrRecord, index, typeAEList, keyword, result);
}

OSErr AEGetKeyBoolean (AERecord *record, AEKeyword keyword, Boolean *result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetKeyPtr (record, keyword, typeBoolean, &typeCode, (Ptr) result, 1, &actualSize);
	if (err == noErr && actualSize == 0)
		return errAECoercionFail;
	else
		return err;
}

OSErr AEGetKeyShort (AERecord *record, AEKeyword keyword, short *result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetKeyPtr (record, keyword, typeShortInteger, &typeCode, (Ptr) result, sizeof (short), &actualSize);
	if (err == noErr && actualSize < sizeof (short))
		return errAECoercionFail;
	else
		return err;
}

OSErr AEGetKeyLong (AERecord *record, AEKeyword keyword, long *result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetKeyPtr (record, keyword, typeLongInteger, &typeCode, (Ptr) result, sizeof (long), &actualSize);
	if (err == noErr && actualSize < sizeof (long))
		return errAECoercionFail;
	else
		return err;
}

OSErr AEGetKeyType (AERecord *record, AEKeyword keyword, DescType *result)
{
	DescType	typeCode, tempType = '    ';
	long			actualSize;
	OSErr		err;
	
	err = AEGetKeyPtr (record, keyword, typeType, &typeCode, (Ptr) &tempType, sizeof (long), &actualSize);
	if (err == noErr) {
		if (actualSize == 0)
			return errAECoercionFail;
		else
			*result = tempType;
	}
	return err;
}

OSErr AEGetKeyStr255 (AERecord *record, AEKeyword keyword, StringPtr result)
{
	DescType	typeCode;
	long			actualSize;
	OSErr		err;
	
	err = AEGetKeyPtr (record, keyword, typeChar, &typeCode, (Ptr) &result[1], 255, &actualSize);
	if (err == noErr && actualSize > 255)
		return errAECoercionFail;
	if (err == noErr)
		result[0] = actualSize;
	else
		result[0] = 0;
	return err;
}

OSErr AEGetKeyList (AERecord *record, AEKeyword keyword, AEDescList *result)
{
	return AEGetKeyDesc (record, keyword, typeAEList, result);
}

