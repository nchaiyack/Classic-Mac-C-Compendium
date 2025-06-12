From: Stuart Cheshire <cheshire@cs.stanford.edu>
Subject: Disk Cache performance evaluation test software

The test program writes 512K to disk, using various block sizes, from 1024
FSWrite calls of 512 bytes each to 32 FSWrite of 16K each.

Each test is done three ways, firstly with simple FSWrite, then with FSWrite
alternating with PBFlushFile, and then with FSWriteNoCache.

The first is what you might naively write in a program which is trying to run
cooperatively in the background while transferring a file to disk. It results
in all the writes going to the cache, and then the Mac freezes solid for ten
seconds when the file is closed.

The second is an attempt to fix this by flushing the file to disk after every
write. It achieves the goal of spreading the delay over all the writes,
instead of having a big freeze at the end, but it still takes over 10 seconds.

The third method was pointed out to me by Jim Matthews (author of Fetch). It
uses the little known (but supported -- see IM:Files, page 89) non-caching
option of PBWrite.
Similarly, this achieves the goal of spreading the delay over all the writes,
but it also makes it go up to 25 times faster.

Results:

Tests were performed with a Macintosh Quadra 700, System 7.0.1/Tuneup 1.1.1,
768KB disk cache, 32 bit addressing, no virtual memory, 20MB RAM:

Testing Writing 512K in various block sizes
Testing 0.5K blocks
Standard writes:    Write:  39 (! 0s)  Close: 691 (!11s)  Total: 730 (!12s)
Write and flush:    Write: 689 (!11s)  Close:   1 (! 0s)  Total: 690 (!11s)
No-cache writes:    Write: 689 (!11s)  Close:   1 (! 0s)  Total: 690 (!11s)
Testing   1K blocks
Standard writes:    Write:  26 (! 0s)  Close: 691 (!11s)  Total: 717 (!11s)
Write and flush:    Write: 691 (!11s)  Close:   1 (! 0s)  Total: 692 (!11s)
No-cache writes:    Write: 346 (! 5s)  Close:   0 (! 0s)  Total: 346 (! 5s)
Testing   2K blocks
Standard writes:    Write:  23 (! 0s)  Close: 692 (!11s)  Total: 715 (!11s)
Write and flush:    Write: 691 (!11s)  Close:   0 (! 0s)  Total: 691 (!11s)
No-cache writes:    Write:  85 (! 1s)  Close:   0 (! 0s)  Total:  85 (! 1s)
Testing   4K blocks
Standard writes:    Write:  21 (! 0s)  Close: 692 (!11s)  Total: 713 (!11s)
Write and flush:    Write: 691 (!11s)  Close:   1 (! 0s)  Total: 692 (!11s)
No-cache writes:    Write:  51 (! 0s)  Close:   0 (! 0s)  Total:  51 (! 0s)
Testing   8K blocks
Standard writes:    Write:  21 (! 0s)  Close: 691 (!11s)  Total: 712 (!11s)
Write and flush:    Write: 690 (!11s)  Close:   1 (! 0s)  Total: 691 (!11s)
No-cache writes:    Write:  39 (! 0s)  Close:   0 (! 0s)  Total:  39 (! 0s)
Testing  16K blocks
Standard writes:    Write:  17 (! 0s)  Close: 692 (!11s)  Total: 709 (!11s)
Write and flush:    Write: 694 (!11s)  Close:   0 (! 0s)  Total: 694 (!11s)
No-cache writes:    Write:  26 (! 0s)  Close:   1 (! 0s)  Total:  27 (! 0s)

Notice that:

1. With the simple writes, every block size, even 16K, incurs a Mac-crippling
ten second freeze when the file is closed.

2. Using write and flush takes the same time as simple writes, but spread
over all the Write calls instead of in a single big freeze at the end.

3. In BOTH of the above cases, it takes at best 691 ticks to write 512K,
making a data rate of 44.5K/sec.

4. Using No-cache writes incurs no freeze for the close call, and if you are
writing 4K blocks it achieves 602K/sec, more than ten times faster than the
simple writes. If you are prepared to go to 16K blocks, it exceeds a
megabyte per second -- more than 25 times faster than simple writes with
the same block size.

If anyone wishes to run the test program and give me results for other
configurations, why not post those results here.

// Copyright (C) 23rd November 1993
// Stuart Cheshire <cheshire@cs.stanford.edu>

#include <stdio.h>
#include <stdlib.h>

#define FILE_SIZE      0x80000
#define MAX_BLOCK_SIZE 0x4000
static char *buffer;
static SysEnvRec sysenvirons;
static const unsigned char filename[] = "\pFlusherTestFile";
static IOParam fileflusher;

static OSErr FSWriteNoCache(short refnum, long *count_p, const Ptr buffer_p)
	{
	OSErr retcode;
	ParamBlockRec pb;
	pb.ioParam.ioCompletion = 0;
	pb.ioParam.ioRefNum     = refnum;
	pb.ioParam.ioBuffer     = buffer_p;
	pb.ioParam.ioReqCount   = *count_p;
	pb.ioParam.ioPosMode    = fsAtMark | 0x20; /* don't cache */
	pb.ioParam.ioPosOffset  = 0;
	retcode = PBWrite(&pb, false);
	*count_p = pb.ioParam.ioActCount;
	return(retcode);
	}

static void filetest(unsigned long block_size, short testcase)
	{
	long inOutCount, t1, t2, t3;
	short fRefNum, i, num_blocks = FILE_SIZE / block_size;
	FSDelete(filename, sysenvirons.sysVRefNum);
	if (Create(filename, sysenvirons.sysVRefNum, '????', '????'))
		{ printf("Create failed\n"); exit(1); }
	if (FSOpen(filename, sysenvirons.sysVRefNum, &fRefNum))
		{ printf("FSOpen failed\n"); exit(1); }
	fileflusher.ioCompletion = NULL;
	fileflusher.ioResult     = noErr;
	fileflusher.ioRefNum     = fRefNum;
	t1 = TickCount();
	for (i=0; i<num_blocks; i++)
		{
		inOutCount = block_size;
		switch (testcase)
			{
			case 0: FSWrite(fRefNum, &inOutCount, buffer);
					break;
			case 1: FSWrite(fRefNum, &inOutCount, buffer);
					if (fileflusher.ioResult == noErr)
						PBFlushFile((ParmBlkPtr)&fileflusher, TRUE);
					break;
			case 2: FSWriteNoCache(fRefNum, &inOutCount, buffer);
					break;
			}
		}
	t2 = TickCount();
	if (FSClose(fRefNum)) { printf("FSClose failed\n"); exit(1); }
	t3 = TickCount();
	FSDelete(filename, sysenvirons.sysVRefNum);
	printf("Write:%4ld (!%2lds)  ", t2-t1, (t2-t1)/60);
	printf("Close:%4ld (!%2lds)  ", t3-t2, (t3-t2)/60);
	printf("Total:%4ld (!%2lds)\n", t3-t1, (t3-t1)/60);
	}

static void blocktest(unsigned long block_size)
	{
	printf("Standard writes:    "); filetest(block_size, 0);
	printf("Write and flush:    "); filetest(block_size, 1);
	printf("No-cache writes:    "); filetest(block_size, 2);
	}

void main(void)
	{
	buffer = NewPtr(MAX_BLOCK_SIZE);
	if (!buffer) { printf("Not enough memory\n"); exit(1); }
	SysEnvirons(curSysEnvVers, &sysenvirons);
	printf("Testing Writing %ldK in various block sizes\n", FILE_SIZE / 1024);
	printf("Testing 0.5K blocks\n"); blocktest( 0x200);
	printf("Testing   1K blocks\n"); blocktest( 0x400);
	printf("Testing   2K blocks\n"); blocktest( 0x800);
	printf("Testing   4K blocks\n"); blocktest(0x1000);
	printf("Testing   8K blocks\n"); blocktest(0x2000);
	printf("Testing  16K blocks\n"); blocktest(0x4000);
	}


Stuart Cheshire <cheshire@cs.stanford.edu>
 * <A HREF="file://brubeck.stanford.edu/www/cheshire-bio.html">WWW</A>
 * Stanford Distributed Systems Group Research Assistant
 * Escondido Village Resident Computer Coordinator
 * Macintosh Programmer
