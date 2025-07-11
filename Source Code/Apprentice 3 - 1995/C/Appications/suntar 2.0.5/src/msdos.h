/*
 * msdos common header file
 */

#define MSECTOR_SIZE	512		/* MSDOS sector size in bytes */
#define MDIR_SIZE	32		/* MSDOS directory size in bytes */
#define MAX_CLUSTER	8192		/* largest cluster size */
#define MAX_PATH	128		/* largest MSDOS path length */
#define MAX_DIR_SECS	64		/* largest directory (in sectors) */

#define NEW		1
#define OLD		0

struct directory {
	unsigned char name[8];		/* file name */
	unsigned char ext[3];		/* file extension */
	unsigned char attr;		/* attribute byte */
	unsigned char reserved[10];	/* ?? */
	unsigned char time[2];		/* time stamp */
	unsigned char date[2];		/* date stamp */
	unsigned char start[2];		/* starting cluster number */
	unsigned char size[4];		/* size of the file */
};

/*#pragma options(!align_arrays) ??? porta la size da 516 a 514, ma ancora non ci siamo */

struct bootsector {
	unsigned char jump[3];		/* Jump to boot code */
	unsigned char banner[8-1];	/* OEM name & version */
	unsigned char lll1;
	unsigned char secsiz[2-1];	/* Bytes per sector hopefully 512 */
	unsigned char lll2;
	unsigned char clsiz;		/* Cluster size in sectors */
	unsigned char nrsvsect[2];	/* Number of reserved (boot) sectors */
	unsigned char nfat;		/* Number of FAT tables hopefully 2 */
	unsigned char dirents[2-1];	/* Number of directory slots */
	unsigned char lll3;
	unsigned char psect[2-1];		/* Total sectors on disk */
	unsigned char lll4;
	unsigned char descr;		/* Media descriptor=first byte of FAT */
	unsigned char fatlen[2];	/* Sectors in FAT */
	unsigned char nsect[2];		/* Sectors/track */
	unsigned char nheads[2];	/* Heads */
	/*unsigned char nhs[2];	*/	/* number of hidden sectors, small disks */
	unsigned char nhs[4];		/* number of hidden sectors, big disks */
	unsigned char bigsect[4];	/* big total sectors, big disks only */
	unsigned char junk[476];	/* who cares? */
};

