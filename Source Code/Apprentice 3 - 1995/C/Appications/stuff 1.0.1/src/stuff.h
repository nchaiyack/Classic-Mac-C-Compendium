/* stuff.h: contains declarations for SIT headers */

#include <string.h>

#include <Files.h>
#include <AppleEvents.h>
#include <Quickdraw.h>
#include <Menus.h>
#include <Windows.h>
#include <ToolUtils.h>
#include <Desk.h>
#include <SegLoad.h>
#include <OSUtils.h>
#include <Memory.h>
#include <Fonts.h>
#include <Events.h>
#include <OSEvents.h>
#include <TextEdit.h>
#include <Dialogs.h>
#include <Aliases.h>
#include <Folders.h>
#include <Errors.h>

typedef unsigned char u_char;
typedef unsigned short u_short;
typedef unsigned int u_int;
typedef unsigned long u_long;
typedef unsigned short ushort;

typedef struct sitHdr {		/* 22 bytes */
	u_char	sig1[4];		/* = 'SIT!' -- for verification */
	u_char	numFiles[2];	/* number of files in archive */
	u_char	arcLen[4];		/* length of entire archive incl. */
	u_char	sig2[4];		/* = 'rLau' -- for verification */
	u_char	version;		/* version number */
	char reserved[7];
};

typedef struct fileHdr {	/* 112 bytes */
	u_char	compRMethod;		/* rsrc fork compression method */
	u_char	compDMethod;		/* data fork compression method */
	u_char	fName[64];			/* a STR63 */
	char	fType[4];			/* file type */
	char	fCreator[4];		/* creator... */
	char	FndrFlags[2];		/* copy of Finder flags */
	char	cDate[4];			/* creation date */
	char	mDate[4];			/* !restored-compat w/backup prgms */
	u_char	rLen[4];			/* decom rsrc length */
	u_char	dLen[4];			/* decomp data length */
	u_char	cRLen[4];			/* compressed lengths */
	u_char	cDLen[4];
	u_char	rsrcCRC[2];			/* crc of rsrc fork */
	u_char	dataCRC[2];			/* crc of data fork */
	char	reserved[6];
	u_char	hdrCRC[2];			/* crc of file header */
};

/* file format is:
	sitArchiveHdr
		file1Hdr
			file1RsrcFork
			file1DataFork
		file2Hdr
			file2RsrcFork
			file2DataFork
		.
		.
		.
		fileNHdr
			fileNRsrcFork
			fileNDataFork
*/



/* compression methods */
#define noComp	0	/* just read each byte and write it to archive */
#define repComp 1	/* RLE compression */
#define lpzComp 2	/* LZW compression */
#define hufComp 3	/* Huffman compression */

/* all other numbers are reserved */


/* comp14.c */
void compress(int, int);
void copy(int, int);
extern long prog_div;
void	UpdateProgress(long progress);
extern unsigned short crc;
unsigned short updcrc(unsigned short icrc, unsigned char *icp, int icnt);

/* progress.c */
OSErr GotRequiredParams(AppleEvent *);
pascal OSErr HandleOapp(AppleEvent *, AppleEvent *, long);
void scanfolder(FSSpec *);
pascal OSErr HandleOdoc(AppleEvent *, AppleEvent *, long);
pascal OSErr HandlePdoc(AppleEvent *, AppleEvent *, long);
pascal OSErr HandleQuit(AppleEvent *, AppleEvent *, long);
pascal OSErr ReturnEventNotHandled(AppleEvent *, AppleEvent *, long);
void main(void);
void SetUpMenus(void);
void AdjustMenus(void);
void HandleMenu(long);
void SetUpWindow(void);
void InitMacintosh(void);
void HandleMouseDown(EventRecord *);
void HandleEvent(void);
void	UpdateProgress(long);
void	UpdateFileName(FSSpec *name);
void BeginProcessing(void);
void EndProcessing(int numDocs);
void ProcessDoc (FSSpec *curDocFSS, char mode);
void HandleMenu (long mSelect);
void SetUpMenus(void);
void InitMacintosh(void);
void HandleMouseDown (EventRecord *theEvent);
void HandleEvent(void);
void Drawprogress (short active);
void SetUpWindow(void);
void ChkOsErr(OSErr);
