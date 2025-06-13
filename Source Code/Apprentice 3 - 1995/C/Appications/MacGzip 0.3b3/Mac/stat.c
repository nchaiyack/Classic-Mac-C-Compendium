/* stat.c - poorman implementation of sys/stat.h */

/*
modification history
--------------------
01b,20may95,ejo  some additional tweaks for CW.
01a,07nov94,ejo  written.
*/

/*
About this implementation
-------------------------
The stat information does not fit perfectly good for the Mac, we don't
have things like inodes, links, uid's and guids. This implementation
put the following information in the stat fields:

st_mode		- S_IFDIR | 0777 for directories
			- S_IFDIR | 0666 for regular files
			- the w bit is cleared for locked files
			- the x bit is set for applications and MPW tools
			- S_IFCHR | 0777 for stdin, stdout and stderr

st_ino		- ioDrDir ID for directories, file number for files
			- 0 for stdin, stdout and stderr
st_dev		- the volume reference number, 0 for stdin, stdout and stderr
st_nlink	- always 1
st_uid		- always 0
st_gid		- always 0
st_size		- ioFlLgLen (size of data fork)
			- 0 for directories and stdin, stdout and stderr
st_atime	- ioDrMdDat for dirs, ioFlMdDat for files (modification date)
			- current calendar time for stdin, stdout and stderr
st_mtime	- same as st_atime
st_ctime	- ioDrCrDat for dirs, ioFlCrDat for files (creation date)
			- current calendar time for stdin, stdout and stderr
			
Only fstat, mkdir and stat is actually implemented. chmod, mkfifo and
umask has no meaning on the Mac and are just dummies.

Written 4 Nov 94, Erling Johansen.
*/
	
#include <errno.h>
#ifdef MPW
#include <ioctl.h>
#endif
#include <time.h>

#include "catinfo.h"
#include "sys/stat.h"

#ifndef ENOENT
#define ENOENT		 2 /* No such file or directory */
#endif
#ifndef EACCES
#define EACCES		13 /* Permission denied */
#endif
#ifndef EFAULT
#define EFAULT		14 /* Bad address */
#endif
#ifndef EEXIST
#define EEXIST		17 /* File exists */
#endif

/*******************************************************************************
*
* chmod - change file mode. Dummy on the Mac.
*
* RETURNS: always 0.
*/
int (chmod) (const char *path, mode_t mode)
	{
#ifdef MPW_C
#pragma unused (path, mode)
#endif
	return 0;
	}
	
/*******************************************************************************
*
* CatInfoToStat - fill in stat struct from CInfoPBRec struct.
*
* RETURNS: always zero.
*/
static int CatInfoToStat (CInfoPBRec *info, struct stat *buf)
	{
	buf->st_nlink = 1;
	buf->st_uid = 0;
	buf->st_gid = 0;
	
	if (info->dirInfo.ioFlAttrib & ioDirMask)
		{
		/* a directory */
		buf->st_mode = S_IFDIR | 0777;
		buf->st_ino = info->dirInfo.ioDrDirID;
		buf->st_dev = info->dirInfo.ioVRefNum;
		buf->st_size = 0;
		buf->st_atime = info->dirInfo.ioDrMdDat;
		buf->st_mtime = info->dirInfo.ioDrMdDat;
		buf->st_ctime = info->dirInfo.ioDrCrDat;
		}
	else
		{
		/* a file */
		buf->st_mode = S_IFREG | 0777;
		
		if (info->hFileInfo.ioFlAttrib & 0x01)
			buf->st_mode &=	~0222;

		if (info->hFileInfo.ioFlFndrInfo.fdType != 'APPL'
			&& info->hFileInfo.ioFlFndrInfo.fdType != 'MPST')
			buf->st_mode &=	~0111;
			
		buf->st_ino = info->hFileInfo.ioDirID;
		buf->st_dev = info->hFileInfo.ioVRefNum;
		buf->st_size = info->hFileInfo.ioFlLgLen;
		buf->st_atime = info->hFileInfo.ioFlMdDat;
		buf->st_mtime = info->hFileInfo.ioFlMdDat;
		buf->st_ctime = info->hFileInfo.ioFlCrDat;
		}
			
	return 0;
	}
	
#ifdef MPW
/*******************************************************************************
*
* fstat - get status for an open file.
*
* RETURNS: zero on success, otherwise -1.
*/
int (fstat) (int fildes, struct stat *buf)
	{
	FCBPBRec pb;
	
	/* uninitalized pointers are often zero... */
	if (buf == NULL)
		{
		errno = EFAULT;
		return -1;
		}
		
	/* check check for stdin, stdout and stderr. They don't exist in the */
	/* file system as they do in Unix. We treat them as character special */
	/* files on the non-existent device 0, all with st_ino 0, accessed, */
	/* modified and status changed NOW! */
	if (fildes >= 0 && fildes <= 2)
		{
		buf->st_mode = S_IFCHR | 0666;
		buf->st_ino = 0;
		buf->st_dev = 0;
		buf->st_nlink = 1;
		buf->st_uid = 0;
		buf->st_gid = 0;
		buf->st_size = 0;
		buf->st_atime = time (NULL);
		buf->st_mtime = buf->st_atime;
		buf->st_ctime = buf->st_atime;
		return 0;
		}
	
	if (ioctl (fildes, FIOREFNUM, (long *) &pb.ioRefNum) != -1)
		{
		unsigned char pas_name[MAX_PAS_LEN + 1];

		/* setup to get info about the open file */
		pb.ioFCBIndx = 0;
		pb.ioNamePtr = pas_name;
		
		if (PBGetFCBInfoSync (&pb) == noErr)
			{
			CInfoPBRec info;
			
			/* get cat info based on name, volref and dirID */
			info.hFileInfo.ioNamePtr = pas_name;
			info.hFileInfo.ioVRefNum = pb.ioFCBVRefNum;
			info.hFileInfo.ioDirID = pb.ioFCBParID;
			info.hFileInfo.ioFDirIndex = 0;
	
			if (PBGetCatInfoSync (&info) == noErr)
				return CatInfoToStat (&info, buf);
			}
		}
	
	errno = EBADF;
	return -1;
	}
#endif

/*******************************************************************************
*
* mkdir - make a directory.
*
* RETURNS: zero on success, otherwise -1.
*/
int (mkdir) (const char *path, mode_t mode)
	{
#ifdef MPW_C
#pragma unused (mode)
#endif
	CInfoPBRec info;
	long dirID;
	int err;
	
	if ((err = GetCatInfo (path, &info, &dirID)) == ENOENT)
		{
		HParamBlockRec pb;
			
		pb.fileParam.ioNamePtr = info.dirInfo.ioNamePtr;
		pb.fileParam.ioVRefNum = info.dirInfo.ioVRefNum;
		pb.fileParam.ioDirID = dirID;
		
		if (PBDirCreateSync (&pb) == noErr)
			return 0;
			
		err = EACCES;
		}
	
	errno = err == 0 ? EEXIST : err;	
	return -1;
	}

/*******************************************************************************
*
* mkfifo - make a FIFO special file. No meaning on the Mac.
*
* RETURNS: always -1 (error).
*/
int (mkfifo) (const char *path, mode_t mode)
	{
#ifdef MPW_C
#pragma unused (path, mode)
#endif
	return -1;
	}

/*******************************************************************************
*
* stat - get information about a file (or directory).
*
* RETURNS: zero on success, otherwise -1.
*/
int (stat) (const char *path, struct stat *buf)
	{
	CInfoPBRec info;
	int err;
	
	if ((err = GetCatInfo (path, &info, NULL)) != 0)
		errno = err;
	else if (buf == NULL)
		errno = EFAULT;
	else
		return CatInfoToStat (&info, buf);
		
	return -1;
	}

/*******************************************************************************
*
* umask - sets a file creation mask. No meaning on the Mac.
*
* RETURNS: the previous mask (always 0666).
*/
mode_t (umask) (mode_t cmask)
	{
	return cmask = 0666;
	}
