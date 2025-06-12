/*
 * common declarations for loading and dumping appleshare protections
 * by Aaron Wohl (aw0g+@andrew.cmu.edu)
 */

#ifndef EEXTERN
#define EEXTERN extern
#endif

#ifdef RUBBISH
typedef struct {
	STANDARD_PBHEADER 
	int				filler3;
	int				ioDenyModes;
	int				filler4;
	SignedByte		filler5;
	SignedByte		ioACUser;
	long			filler6;
	long			ioACOwnerID;
	long			ioACGroupID;
	long			ioACAccess;
	long			ioDirID;
} fixed_AccessParam;

typedef union {
	HIOParam		ioParam;
	HFileParam		fileParam;
	HVolumeParam	volumeParam;
	fixed_AccessParam		accessParam;
	ObjParam		objParam;
	CopyParam		copyParam;
	WDParam			wdParam;
} fixed_HParamBlockRec, *fixed_HParmBlkPtr;
#endif

#define ERR_BUF_SIZE (1024)
int read_line(char *aline,FILE *afile);
#define BAD_VOL_NUM (0)
long get_vol_id(char *in_name);
struct cache_entry_R {
	char text[34];
	long num;
};
typedef struct cache_entry_R cache_entry,*cache_entry_pt;

EEXTERN cache_entry_pt group_cache;
EEXTERN cache_entry_pt user_cache;

void allocate_cache_entries(void);
#define CACHE_SIZE (300)
