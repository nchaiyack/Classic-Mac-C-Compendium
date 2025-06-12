#include <stdio.h>
#include <stdlib.h>

//¥ for each unique file type, an entry in the queue handle
struct typeX {
	OSType theType;		//¥ this particular file type
	long count;			//¥ number of instances counted
};

typedef struct typeX typeX;
typedef typeX *typePtr;
typedef typePtr *TypeHand;
	
#define kFolderBit 0x10

void main (void);
static void enumerate (TypeHand theTypes, long *total, long dirid);
static Boolean difftype (TypeHand theTypes, OSType theType, long *count);
static void sortdata (TypeHand theTypes);
static int compare (void *p1, void *p2);
static void writedata (TypeHand theTypes, FILE *outfile);

void main ()
{
	long total;
	TypeHand theTypes;
	
	total = 0;										//¥ init total file count to 0
	theTypes = (TypeHand) NewHandle (0);				//¥ init the structure queue handle
	SetCursor (*GetCursor (watchCursor));				//¥ we'll be waiting for a little bit
	enumerate (theTypes, &total, fsRtDirID);			//¥ start the recursive file lookup
	sortdata (theTypes);								//¥ qsort the whole queue
	writedata(theTypes, fopen("TypeEnum", "w"));	//¥ write it all out to a file
	DisposHandle ((Handle)theTypes);					//¥ get rid of the queue handle
}

//¥ called recursively to look up all files
static void enumerate (TypeHand theTypes, long *total, long dirid)
{
	OSErr err;
	long entry;
	CInfoPBRec cpb;
	OSType thistype;
	register short count, index;
	
	cpb.hFileInfo.ioNamePtr = nil;
	cpb.hFileInfo.ioVRefNum = 0;
	index = 1;
	do {
		cpb.hFileInfo.ioFDirIndex = index;
		cpb.dirInfo.ioDrDirID = dirid;
		cpb.hFileInfo.filler2 = 0;
		if ((err = PBGetCatInfo (&cpb, false)) == noErr) 
		{
			if (cpb.hFileInfo.ioFlAttrib & kFolderBit) 
			{
				//¥ we have a directory - call this function again
				enumerate (theTypes, total, cpb.dirInfo.ioDrDirID);
				err = noErr;
			}
			else 
				{
					//¥ we have a file
					thistype = cpb.hFileInfo.ioFlFndrInfo.fdType;
					if (difftype (theTypes, thistype, &entry)) 
					{
						//¥ this file type in not in our queue yet, so tack it on
						(*total)++;
						SetHandleSize ((Handle)theTypes, (*total) * sizeof (typeX));
						HLock ((Handle)theTypes);
							(*theTypes)[ (*total) - 1].theType = thistype;	//¥ feed in the type
							(*theTypes)[ (*total) - 1].count = 1;			//¥ start the count at 1
						HUnlock ((Handle)theTypes);
					}
					else 
						{
							//¥ this file type is already in the queue
							//¥ difftype returned its queue entry index, so increment its count
							HLock ((Handle)theTypes);
								(*theTypes)[entry].count ++;
							HUnlock ((Handle)theTypes);
					}
			}
		}
		index ++;
	} while (err == noErr);
}

//¥ searches queue handle for duplicates
static Boolean	difftype(TypeHand theTypes, OSType theType, long *count)
{
	register typePtr thePtr;
	long numTypes;

	if ((numTypes = GetHandleSize ((Handle) theTypes) / sizeof (typeX)) != 0) {
		HLock ((Handle) theTypes);
		thePtr = *theTypes;
		for (*count = 0; *count < numTypes; (*count)++) 
		{
			if ((*thePtr).theType == theType) 
			{
				HUnlock ((Handle) theTypes);
				return (false);
			}
			thePtr ++;
		}
		HUnlock ((Handle) theTypes);
	}
	return (true);
}

//¥ the qsort stub
static void sortdata (TypeHand theTypes)
{
	HLock ((Handle) theTypes);
	qsort ((unsigned char *)*theTypes, GetHandleSize ((Handle) theTypes) / sizeof (typeX), sizeof (typeX), (__cmp_func)compare);
	HUnlock ((Handle) theTypes);
}

//¥ the qsort comparison routine
static int compare (void *p1, void *p2)
{
	register short i;
	register long n1, n2;
	unsigned char c1, c2;
	
	//¥ sorts by number of instances of a given type
	//¥ and if count is equal, sorts alphanumeric
	n1 = (*(typePtr)p1).count;
	n2 = (*(typePtr)p2).count;
	if (n1 == n2) 
	{
		for (i = 0; i < sizeof (OSType); i ++) 
		{
			c1 = ((unsigned char *) & (*(typePtr)p1).theType)[i];
			c2 = ((unsigned char *) & (*(typePtr)p2).theType)[i];
			if (c1 != c2)
				return ((c1 < c2) ? -1 : 1);
		}
		return (0);
	}
	return ((n1 > n2) ? -1 : 1);
}

//¥ writes the contents of the queue handle
static void writedata (TypeHand theTypes, FILE *outfile)
{
	register typePtr thePtr;
	long numTypes, count;

	if ((numTypes = GetHandleSize ((Handle) theTypes) / sizeof (typeX)) != 0) 
	{
		HLock ((Handle) theTypes);
		thePtr = *theTypes;
		for (count = 0; count < numTypes; count ++) 
		{
			fprintf(outfile, "'%c%c%c%c'  %ld\n", 
				((unsigned char *) & (*thePtr).theType)[0],
				((unsigned char *) & (*thePtr).theType)[1],
				((unsigned char *) & (*thePtr).theType)[2],
				((unsigned char *) & (*thePtr).theType)[3],
				(*thePtr).count);
			thePtr ++;
		}
		HUnlock((Handle)theTypes);
	}
	fclose(outfile);
}