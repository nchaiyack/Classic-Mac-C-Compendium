#include <SetUpA4.h>

#define kErrorInstallStrings	27500		// must match Global.h
	#define kBaseText				1

#define Min(x, y)		((x) < (y) ? (x) : (y))

StringPtr			gAlertString;
NMRec				gNotificationRecord;

pascal void			MyResponse(NMRecPtr myRecord);
pascal StringPtr	PLstrcat(StringPtr str1, StringPtr str2);

//--------------------------------------------------------------------------------

void main(Handle usHandle, short errStringIndex)
{
	Str255			tempString;

	RememberA0();
	SetUpA4();

	if (errStringIndex > 0) {
		DetachResource(usHandle);
	
		gAlertString = (StringPtr) NewPtrSys(sizeof(Str255));
		GetIndString(gAlertString, kErrorInstallStrings, kBaseText);
		GetIndString(tempString, kErrorInstallStrings, errStringIndex);
		PLstrcat(gAlertString, tempString);
	
		gNotificationRecord.qType = nmType;
		gNotificationRecord.nmMark = 0;
		gNotificationRecord.nmIcon = NULL;
		gNotificationRecord.nmSound = NULL;
		gNotificationRecord.nmStr = (StringPtr) gAlertString;
		gNotificationRecord.nmResp = (ProcPtr) MyResponse;
		gNotificationRecord.nmRefCon = (long) usHandle;
		
		NMInstall(&gNotificationRecord);
	}
	
	RestoreA4();
}


//--------------------------------------------------------------------------------

pascal void MyResponse(NMRecPtr myRecord)
{
	NMRemove(myRecord);
	DisposePtr(myRecord->nmStr);
	DisposeHandle((Handle) myRecord->nmRefCon);
}


//--------------------------------------------------------------------------------

pascal StringPtr	PLstrcat(StringPtr str1, StringPtr str2)
{
	StringPtr		dest = str1;
	StringPtr		src = str2;
	short			destLength = *dest;
	short			srcLength = *src++;
	short			spaceLeft;
	long			amtToCopy;

	spaceLeft = 255 - destLength;
	amtToCopy = Min(spaceLeft, srcLength);
	*dest++ += amtToCopy;
	BlockMove(src, dest + destLength, amtToCopy);

	return str1;
}


