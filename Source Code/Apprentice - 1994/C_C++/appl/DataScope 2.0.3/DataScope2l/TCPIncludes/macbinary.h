#define MB_READ		0
#define MB_WRITE	1
#define MB_DISABLE	0x80

typedef struct MBHead {		/* MacBinary Header */
	char zero1;
	char nlen;
	char name[63];
	char type[4];
	char creator[4];
	char flags;
	char zero2;
	char location[6];
	char protected;
	char zero3;
	char dflen[4];
	char rflen[4];
	char cdate[4];
	char mdate[4];

} MBHead;

typedef struct MBFile {
	char name[64];			/* The 'C' version of the name */
	short
		fd,					/* Current File Descriptor */
		mode,				/* Are we reading or writing */
		vrefnum,			/* The volume reference number */
		binary,				/* MacBinary active ? */
		sector1,			/* Are we at the first sector of the file */
		fork;				/* 0 if we're writing the DF, 1 if were writing the RF */
	long
		bytes,				/* Length of the current fork */
		rlen,				/* Length of Resource Fork (bytes) */
		dlen;				/* Length of Data Fork (bytes) */
	MBHead
		header;				/* the first sector... */
} MBFile;
