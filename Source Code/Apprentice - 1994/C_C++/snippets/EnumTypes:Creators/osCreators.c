#include <stdio.h>
#include <stdlib.h>

//¥ for each unique file creator, an entry in the queue handle
struct creatorX {
	OSType theCreator;		// this particular file creator
	long count;			// number of instances counted
};

typedef struct creatorX creatorX;
typedef creatorX *creatorPtr;
typedef creatorPtr *CreatorHand;
	
#define kFolderBit 0x10

void main (void);
static void enumerate (CreatorHand theCreators, long *total, long dirid);
static Boolean	diffCreator (CreatorHand theCreators, OSType theCreator, long *count);
static void sortdata (CreatorHand theCreators);
static int compare (void *p1, void *p2);
static void writedata (CreatorHand theCreators, FILE *outfile);

void main ()
{
	long total;
	CreatorHand theCreators;
	
	total = 0;										//¥ init total file count to 0
	theCreators = (CreatorHand)NewHandle (0);				//¥ init the structure queue handle
	SetCursor (*GetCursor (watchCursor));			//¥ we'll be waiting for a little bit
	enumerate (theCreators, &total, fsRtDirID);		//¥ start the recursive file lookup
	sortdata (theCreators);							//¥ qsort the whole queue
	writedata (theCreators, fopen ("CreatorEnum", "w"));	//¥ write it all out to a file
	DisposHandle ((Handle)theCreators);				//¥ get rid of the queue handle
}

//¥ called recursively to look up all files
static void enumerate(CreatorHand theCreators, long *total, long dirid)
{
	OSErr err;
	long entry;
	CInfoPBRec cpb;
	OSType thisCreator;
	register short count, index;
	
	cpb.hFileInfo.ioNamePtr = nil;
	cpb.hFileInfo.ioVRefNum = 0;
	index = 1;
	do 
	{
		cpb.hFileInfo.ioFDirIndex = index;
		cpb.dirInfo.ioDrDirID = dirid;
		cpb.hFileInfo.filler2 = 0;
		if ((err = PBGetCatInfo (&cpb, false)) == noErr) 
		{
			if (cpb.hFileInfo.ioFlAttrib & kFolderBit) 
			{
				//¥ we have a directory - call this function again
				enumerate (theCreators, total, cpb.dirInfo.ioDrDirID);
				err = noErr;
			}
			else 
				{
					//¥ we have a file
					thisCreator = cpb.hFileInfo.ioFlFndrInfo.fdCreator;
					if (diffCreator (theCreators, thisCreator, &entry)) 
					{
						//¥ this file creator in not in our queue yet, so tack it on
						(*total)++;
						SetHandleSize ((Handle)theCreators, (*total) * sizeof (creatorX));
						HLock ((Handle)theCreators);
							(*theCreators)[(*total) - 1].theCreator = thisCreator;	//¥ feed in the creator
							(*theCreators) [(*total) - 1].count = 1;			//¥ start the count at 1
						HUnlock ((Handle)theCreators);
					}
					else 
						{
							//¥ this file creator is already in the queue
							//¥ diffCreator returned its queue entry index, so increment its count
							HLock ((Handle)theCreators);
								(*theCreators)[entry].count ++;
							HUnlock ((Handle)theCreators);
					}
			}
		}
		index ++;
	} while (err == noErr);
}

//¥ searches queue handle for duplicates
static Boolean	diffCreator (CreatorHand theCreators, OSType theCreator, long *count)
{
	register creatorPtr thePtr;
	long numCreators;

	if ((numCreators = GetHandleSize ((Handle)theCreators) / sizeof (creatorX)) != 0) {
		HLock ((Handle)theCreators);
		thePtr = *theCreators;
		for (*count = 0; *count < numCreators; (*count)++) 
		{
			if ((*thePtr).theCreator == theCreator) 
			{
				HUnlock ((Handle)theCreators);
				return (false);
			}
			thePtr ++;
		}
		HUnlock ((Handle)theCreators);
	}
	return (true);
}

//¥ the qsort stub
static void sortdata (CreatorHand theCreators)
{
	HLock ((Handle)theCreators);
	qsort ((unsigned char *) *theCreators, GetHandleSize ((Handle)theCreators) / sizeof (creatorX), sizeof (creatorX), (__cmp_func)compare);
	HUnlock ((Handle)theCreators);
}

//¥ the qsort comparison routine
static int compare (void *p1, void *p2)
{
	register short i;
	register long n1, n2;
	unsigned char c1, c2;
	
	//¥ sorts by number of instances of a given creator
	//¥ and if count is equal, sorts alphanumeric
	n1 = (*(creatorPtr)p1).count;
	n2 = (*(creatorPtr)p2).count;
	if (n1 == n2) 
	{
		for (i = 0; i < sizeof (OSType); i ++) 
		{
			c1 = ((unsigned char *) & (*(creatorPtr)p1).theCreator)[i];
			c2 = ((unsigned char *) & (*(creatorPtr)p2).theCreator)[i];
			if (c1 != c2)
				return ((c1 < c2) ? -1 : 1);
		}
		return (0);
	}
	return ((n1 > n2) ? -1 : 1);
}

//¥ writes the contents of the queue handle
static void writedata (CreatorHand theCreators, FILE *outfile)
{
	register creatorPtr thePtr;
	long numCreators, count;

	if ((numCreators = GetHandleSize ((Handle) theCreators) / sizeof (creatorX)) != 0) {
		HLock ((Handle)theCreators);
		thePtr = *theCreators;
		for (count = 0; count < numCreators; count ++) 
		{
			fprintf (outfile, "'%c%c%c%c'  %ld\n", 
				 ((unsigned char *) & (*thePtr).theCreator)[0],
				 ((unsigned char *) & (*thePtr).theCreator)[1],
				 ((unsigned char *) & (*thePtr).theCreator)[2],
				 ((unsigned char *) & (*thePtr).theCreator)[3],
				 (*thePtr).count);
			thePtr ++;
		}
		HUnlock ((Handle)theCreators);
	}
	fclose (outfile);
}