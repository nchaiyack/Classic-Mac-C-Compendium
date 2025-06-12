/* $Id: $ */

/*
 * Minimal 'stat' emulation: tells directories from files and
 * gives length and mtime.
 *
 * Largely based on code written by Guido van Rossum, CWI, Amsterdam
 * and placed by him in the public domain --
 * retrieved by anonymous FTP from ftp.cwi.nl
 */

#include "ThinkCPosix.h"

int sys_nerr = 0;
char *sys_errlist[] = {""};
char *myenviron[] = {NULL};
char **environ = myenviron;
extern int __uid, __gid;
int Stat(char*, long, struct stat*);


/* Bits in ioFlAttrib: */
#define LOCKBIT	(1<<0)		/* File locked */
#define DIRBIT	(1<<4)		/* It's a directory */

/*
 * Mac-ky "stat" in which filename is given relative to a directory,
 * specified by long DirID.
 */

int
Stat(name, DirID, buf)
	char *name;
	long DirID;
	struct stat *buf;
{
	CInfoPBRec cipbr;
	HFileInfo *fpb = (HFileInfo*)&cipbr;
	DirInfo *dpb = (DirInfo*)&cipbr;
	unsigned char pname[256];
	short err;

	strcpy((char*)pname, name);
	c2pstr(pname);

	dpb->ioDrDirID = DirID;
	fpb->ioNamePtr = pname;
	fpb->ioVRefNum = 0;
	fpb->ioFDirIndex = 0;
	fpb->ioFVersNum = 0;
	err = PBGetCatInfo(&cipbr, FALSE);
	if (err != noErr) {
		errno = ENOENT;
		return -1;
	}
	if (fpb->ioFlAttrib & LOCKBIT)
		buf->st_mode= 0444;
	else
		buf->st_mode= 0666;
	if (fpb->ioFlAttrib & DIRBIT) {
		buf->st_mode |= 0111 | S_IFDIR;
		buf->st_size= dpb->ioDrNmFls;
		buf->st_rsize= 0;
	}
	else {
		buf->st_mode |= S_IFREG;
		if (fpb->ioFlFndrInfo.fdType == 'APPL')
			buf->st_mode |= 0111;
		buf->st_size= fpb->ioFlLgLen;
		buf->st_rsize= fpb->ioFlRLgLen;
	}
	buf->st_atime= fpb->ioFlCrDat;
	buf->st_mtime= fpb->ioFlMdDat;
	buf->st_ctime= fpb->ioFlCrDat;
	buf->st_ino= (unsigned short)fpb->ioDirID;
	buf->st_uid= __uid;
	buf->st_gid= __gid;
	buf->st_FlFndrInfo= fpb->ioFlFndrInfo;
	return 0;
}

int
stat(path, buf)
	char *path;
	struct stat *buf;
{
	return Stat(path, 0L, buf);
}

int
fstat(fd, buf)
	int fd;
	struct stat *buf;
{
	FCBPBRec fcb;
	unsigned char pname[256];
	long DirID;
	short err;
	
	/*
	 * fdopen() gives FILE entry with name of file,
	 * as well as RefNum of containing directory
	 */
	
	FILE *fp = fdopen(fd, "");
	
	/* 
	 * PBGetFCBInfo() converts short RefNum to long DirID
	 */
	 
	fcb.ioRefNum= fp->refnum;
	fcb.ioVRefNum= 0;
	fcb.ioFCBIndx= 0;
	fcb.ioNamePtr= pname;
	err= PBGetFCBInfo(&fcb, FALSE);
	if (err != noErr) {
		errno = ENOENT;
		return -1;
	}
	DirID = fcb.ioFCBParID;
	
	p2cstr(pname);
	return Stat((char*)pname, DirID, buf);
}

