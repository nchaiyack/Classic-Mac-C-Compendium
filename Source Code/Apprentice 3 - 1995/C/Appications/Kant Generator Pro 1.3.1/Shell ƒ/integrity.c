#include "integrity.h"
#include "memory layer.h"

#define TAG_OFFSET		136L
#define THE_TAG			0x16435934
#define kIntegrityType	'Intg'
#define kIntegrityID	0
#define kIntegrityName	"\p"
#define kExcludedTypes	7

static	ResType		gExcludedType[kExcludedTypes]={'ICN#','icl8','icl4','ics#','ics8','ics4','SIZE'};

static	unsigned long CalculateIntegrityHandleSize(void);
static	Boolean TypeIsExcludedQQ(ResType theType);
static	void FillIntegrityHandle(Handle integrityHandle);
static	OSErr WriteIntegrityHandle(Handle integrityHandle);
static	Boolean VerifyIntegrity(void);
static	short CountRelevantTypes(void);

Boolean DoIntegrityCheck(Boolean *programIntegritySet)
/* called first thing; can be used either to check program integrity (if it has
   been installed) or to install the integrity checker (if the SHIFT key is
   help down during program launch).  Returns FALSE if program integrity is
   not verified; programIntegritySet is TRUE if this procedure installs the
   integrity checker. */
{
	short			thisFile;
	long			count;
	long			tag;
	KeyMap			rawKeys;
	unsigned short	theKeys[8];
	Handle			integrityHandle;
	
	*programIntegritySet=FALSE;
	GetKeys(rawKeys);
	Mymemcpy((Ptr)theKeys, (Ptr)rawKeys, sizeof(rawKeys));
	FlushVol(0L, 0);		/* just to be on the safe side */
	
	/* interestingly enough, the new-and-improved functions to open a resource fork
	   (HOpenRF & FSpOpenRF) won't work here.  They check permissions and won't
	   allow write permission to an already-open resource fork -- so we won't be
	   able to install the integrity checker on the fly if we Do The Right Thing�.
	   Luckily, the old-and-inferior function (OpenRF) doesn't ask permission... */
	OpenRF(LMGetCurApName(), 0, &thisFile);
	
	SetFPos(thisFile, 1, TAG_OFFSET);
	count=4L;
	FSRead(thisFile, &count, (Ptr)(&tag));	/* read long-word tag */
	if (tag!=THE_TAG)	/* if not equal to our tag, integrity checker not installed */
	{
		if (theKeys[3]&1)	/* if SHIFT key down, install integrity checker */
		{
			integrityHandle=NewHandleClear(CalculateIntegrityHandleSize());
			FillIntegrityHandle(integrityHandle);
			if (WriteIntegrityHandle(integrityHandle)!=noErr)
				return FALSE;
			
			SetFPos(thisFile, 1, TAG_OFFSET);
			count=4L;
			tag=THE_TAG;
			/* store tag so we know integrity checker is installed */
			if (FSWrite(thisFile, &count, (Ptr)(&tag))!=noErr)
				return FALSE;
			
			*programIntegritySet=TRUE;		/* so we know we've installed it */
		}
		
		return TRUE;
	}
	else
	{
		return VerifyIntegrity();
	}
}

static	Boolean TypeIsExcludedQQ(ResType theType)
{
	short			i;
	
	for (i=0; i<kExcludedTypes; i++)
		if (gExcludedType[i]==theType)
			return TRUE;
	
	return FALSE;
}

static	unsigned long CalculateIntegrityHandleSize(void)
{
	short			i;
	short			numTypes, numTypesToWorryAbout;
	unsigned long	totalSize;
	ResType			thisType;
	
	numTypes=Count1Types();
	numTypesToWorryAbout=CountRelevantTypes();
	
	totalSize=sizeof(short)+(sizeof(ResType)+sizeof(short))*numTypesToWorryAbout;
	for (i=1; i<=numTypes; i++)
	{
		Get1IndType(&thisType, i);
		if (!TypeIsExcludedQQ(thisType))
			totalSize+=(sizeof(short)+sizeof(long))*Count1Resources(thisType);
	}
	
	totalSize+=sizeof(ResType)+sizeof(short)+sizeof(short)+sizeof(long);	/* for 'Intg' 0 */
	
	return totalSize;
}

static	void FillIntegrityHandle(Handle integrityHandle)
{
	unsigned long	offset;
	short			i, j, numTypes, numOfThisType, resID;
	ResType			thisType, dummyType;
	Str255			dummyName;
	Handle			resHandle;
	Boolean			dontRelease;
	short			resAttrs;
	
	HLock(integrityHandle);
	offset=(unsigned long)*integrityHandle;
	SetResLoad(FALSE);
	*((short*)offset)=CountRelevantTypes()+1;
	offset+=sizeof(short);
	numTypes=Count1Types();
	for (i=1; i<=numTypes; i++)
	{
		Get1IndType(&thisType, i);
		if (!TypeIsExcludedQQ(thisType))
		{
			*((ResType*)offset)=thisType;
			offset+=sizeof(ResType);
			numOfThisType=Count1Resources(thisType);
			*((short*)offset)=numOfThisType;
			offset+=sizeof(short);
			for (j=1; j<=numOfThisType; j++)
			{
				resHandle=Get1IndResource(thisType, j);
				GetResInfo(resHandle, &resID, &dummyType, dummyName);
				dontRelease=(thisType=='CODE');
				resAttrs=GetResAttrs(resHandle);
				dontRelease|=(resAttrs&resPreload);
				*((long*)offset)=GetResourceSizeOnDisk(resHandle);
				offset+=sizeof(long);
				*((short*)offset)=resID;
				offset+=sizeof(short);
				if (!dontRelease)
					ReleaseResource(resHandle);
			}
		}
	}
	
	/* manually include info about integrity resource itself */
	*((ResType*)offset)=kIntegrityType;
	offset+=sizeof(ResType);
	*((short*)offset)=1;
	offset+=sizeof(short);
	*((long*)offset)=GetHandleSize(integrityHandle);
	offset+=sizeof(long);
	*((short*)offset)=0;
	offset+=sizeof(short);
	
	SetResLoad(TRUE);
	HUnlock(integrityHandle);
}

static	OSErr WriteIntegrityHandle(Handle integrityHandle)
{
	OSErr			oe;
	
	AddResource(integrityHandle, kIntegrityType, kIntegrityID, kIntegrityName);
	if ((oe=ResError())!=noErr)
		return oe;
	ChangedResource(integrityHandle);
	WriteResource(integrityHandle);
	if ((oe=ResError())!=noErr)
		return oe;
	UpdateResFile(CurResFile());
	if ((oe=ResError())!=noErr)
		return oe;
	ReleaseResource(integrityHandle);
	
	return noErr;
}

static	Boolean VerifyIntegrity(void)
{
	Handle			integrityHandle, resHandle;
	unsigned long	offset, maxOffset;
	ResType			thisType;
	Boolean			bad, dontRelease;
	long			resSize, realSize;
	short			numOfThisType, realNumOfThisType, resID, resAttrs;
	short			realTypesToWorryAbout, numTypesToWorryAbout;
	
	integrityHandle=Get1Resource(kIntegrityType, kIntegrityID);
	if (integrityHandle==0L)
		return FALSE;
	
	SetResLoad(FALSE);
	HLock(integrityHandle);
	offset=(unsigned long)*integrityHandle;
	maxOffset=offset+SizeResource(integrityHandle);
	bad=FALSE;
	realTypesToWorryAbout=CountRelevantTypes();
	numTypesToWorryAbout=*((short*)offset);
	offset+=sizeof(short);
	bad=((offset>maxOffset) || (numTypesToWorryAbout!=realTypesToWorryAbout));
	while ((offset<maxOffset) && (!bad))
	{
		thisType=*((ResType*)offset);
		offset+=sizeof(ResType);
		bad=(offset>maxOffset);
		if (!bad)
		{
			numOfThisType=*((short*)offset);
			offset+=sizeof(short);
			bad=(offset>maxOffset);
		}
		if (!bad)
		{
			realNumOfThisType=Count1Resources(thisType);
			bad=(numOfThisType!=realNumOfThisType);
		}
		if (!bad)
		{
			while ((numOfThisType>0) && (!bad))
			{
				resSize=*((long*)offset);
				offset+=sizeof(long);
				bad=(offset>maxOffset);
				if (!bad)
				{
					resID=*((short*)offset);
					offset+=sizeof(short);
					bad=(offset>maxOffset);
				}
				if (!bad)
				{
					resHandle=Get1Resource(thisType, resID);
					bad=(resHandle==0L);
				}
				if (!bad)
				{
					realSize=SizeResource(resHandle);
					bad=(realSize!=resSize);
					dontRelease=((thisType=='CODE') || (thisType==kIntegrityType));
					resAttrs=GetResAttrs(resHandle);
					dontRelease|=(resAttrs&resPreload);
					if (!dontRelease)
						ReleaseResource(resHandle);
				}
				numOfThisType--;
			}
		}
	}
	
	HUnlock(integrityHandle);
	ReleaseResource(integrityHandle);
	SetResLoad(TRUE);
	
	return !bad;
}

static	short CountRelevantTypes(void)
{
	short			i;
	short			numTypesToWorryAbout;
	
	numTypesToWorryAbout=Count1Types();
	for (i=0; i<kExcludedTypes; i++)
		if (Count1Resources(gExcludedType[i])>0)
			numTypesToWorryAbout--;
	
	return numTypesToWorryAbout;
}
