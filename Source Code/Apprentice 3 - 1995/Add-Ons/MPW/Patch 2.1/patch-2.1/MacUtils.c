/***
 *
 * MacUtils.c
 *
 *	Copyright (c) 1995, Christopher E. Hyde.  All rights reserved.
 *
 *	Updated: 95-06-30
 *
 ***/

#include	<stdio.h>
#include	<stdlib.h>
#include	<fcntl.h>
#include	<ioctl.h>
#include	<Files.h>
#include	<Errors.h>

#pragma segment Utils

//typedef bool Boolean;


int
isatty (int fd)
{
	return ioctl(fd, FIOINTERACTIVE, NULL) == 0;
}


// Create a temporary file name
char*
MakeFileName (size_t size, const char* dir, const char* name)
{
	char* fileName = (char*) malloc(size);
	strcpy(fileName, dir);
	strcat(fileName, name);
	Mktemp(fileName);
	return fileName;
}

#if 0
pascal OSErr
GetDirID (short vRefNum, long dirID, StringPtr name,
		  long* theDirID, bool* isDirectory)
{
	CInfoPBRec pb;
	OSErr error;

	pb.hFileInfo.ioNamePtr = name;
	pb.hFileInfo.ioVRefNum = vRefNum;
	pb.hFileInfo.ioDirID = dirID;
	pb.hFileInfo.ioFDirIndex = 0;		// use ioNamePtr and ioDirID
	error = PBGetCatInfoSync(&pb);
	*theDirID = pb.hFileInfo.ioDirID;
	*isDirectory = (pb.hFileInfo.ioFlAttrib & ioDirMask) != 0;
	return error;
}


pascal OSErr
SetDirectory (short vRefNum, long dirID, StringPtr name,
			  long* theDirID, bool* isDirectory)
{
}
#endif

OSErr
ChangeDirectory (const char* path)
{
	FSSpec		spec;
	OSErr		err;
//	StringPtr	s = C2PStr(path);
	Str255		s;
	int			len = strlen(path);

	if (len > 253)
		return bdNamErr;
	BlockMoveData(path, &s[1], *s = len);

//	FailOSErr(FSMakeFSSpec(0, 0, C2PStr(optarg), &spec));
//	    if (HSetVol(volName, vRefNum, dirID) != noErr)

	if (s[*s] != ':')
		s[++*s] = ':';
	s[++*s] = '\1';			// Very unlikely file name
	err = FSMakeFSSpec(0, 0, s, &spec);
	if (err == fnfErr)
		err = noErr;
	if (err == noErr)
		err = HSetVol(nil, spec.vRefNum, spec.parID);
	return err;
}


#if 0
// Translate ':'s to ';'s and translate '/'s to ':'s
char*
UnixToMac (char* path)
{
	char* s;

	for (s = path; *s; ++s) {
		if (*s == ':')
			*s = ';';
		else if (*s == '/')
			*s = ':';
	}

	return path;
}
#endif


// Translate a unix file path into a Mac file path.
// Translates ':'s to ';'s and '/'s to ':'s
char*
UnixToMac (const char* path)
{
	char* d = xmalloc(strlen(path) + 2);	// In case needs leading ':'
	char* result = d, c;

	if (*path == '/')
		++path;
	else
		*d++ = ':';
	do {
		c = *path++;
		if (c == ':')
			c = ';';
		else if (c == '/')
			c = ':';

		*d++ = c;
	} while (c);

	return result;
}


// Make a new/unique FSSpec based on the name in the FSSpec
void
FSpMakeUnique (const FSSpec* orig, FSSpec* spec)
{
	int len = orig->name[0] + 1;
	StringPtr s /*= &spec->name[len]*/;
	unsigned n = 0;
	Str63 name;
#ifdef DEBUGGING
	OSErr err;
#endif

	BlockMoveData(orig->name, name, len);
	s = &name[len];
	*s++ = '•';					// Append '•'

	do {
		int xLen = 0;
		unsigned tmp = n++;
		do {
			unsigned m = tmp % 36;
			tmp /= 36;
			m += (m < 26) ? 'A' : '0' - 26;		// Try A…Z and 0…9 suffixes
			s[xLen++] = m;
		} while (tmp);

			// Make sure name len <= kMaxFileNameLen
		for ( ; len + xLen > kMaxFileNameLen; --len)
			BlockMoveData(&name[2], &name[1], 36);

		name[0] = len + xLen;
#ifdef DEBUGGING
		err = FSMakeFSSpec(orig->vRefNum, orig->parID, name, spec);
	} while (err == noErr);

	if (err != fnfErr) {
		say("FSpMakeUnique -> %d\n", err);
		my_exit(1);
	}
#else
	} while (FSMakeFSSpec(orig->vRefNum, orig->parID, name, spec) == noErr);
#endif
}


StringPtr
CToPStr (const char* s)
{
	static Str255	pStr;

	BlockMoveData(s, &pStr[1], *pStr = (unsigned char) strlen(s));
	return pStr;
}


enum {
	kMaxPathLength		=	1024
};


// Return the full path as a C string given an FSSpec.
char*
FSFullPathName (const FSSpec* spec)
{
	char*		fullPathName = malloc(kMaxPathLength);
	Str63		dirName;
	CInfoPBRec	pb;
	size_t		len = StrLength(spec->name),
				size = len + 1,
				pathLength = kMaxPathLength - size;

	fullPathName[kMaxPathLength - 1] = '\0';
	BlockMoveData(&spec->name[1], &fullPathName[pathLength], len);
//fprintf(stderr, "FullPathName>>File: “%P”\n", spec->name);

	pb.dirInfo.ioNamePtr = dirName;
	pb.dirInfo.ioDrParID = spec->parID;

	do {
		pb.dirInfo.ioVRefNum   = spec->vRefNum;
		pb.dirInfo.ioFDirIndex = -1;				// -1 means get info about ioDrDirID
		pb.dirInfo.ioDrDirID   = pb.dirInfo.ioDrParID;

		if (PBGetCatInfoSync(&pb) != noErr) break;
		len = StrLength(dirName);
		dirName[++len] = ':';
		size += len;								// Move dirName to start of fullPathName
		pathLength -= len;
		BlockMoveData(&dirName[1], &fullPathName[pathLength], len);
//fprintf(stderr, "FullPathName>>Dir: “%P”\n", dirName);
	} while (pb.dirInfo.ioDrDirID != fsRtDirID);

		// Move data down by size bytes
	BlockMoveData(&fullPathName[pathLength], fullPathName, size);
	fullPathName = realloc(fullPathName, size);	// Remove extra space

//fprintf(stderr, "FullPathName: “%s”\n", fullPathName);
	return fullPathName;
}


/*
 *  Macintosh Version of Unix Stat and FStat Functions
 *
 */


// Bits in ioFlAttrib:
enum {
	kLockBit	= 1 << 0,		// File locked
	kDirBit		= 1 << 4,		// It's a directory
	S_IEXEC		= 0
};


static int
__stat (StringPtr fileName, long dirId, struct stat* sr)
{
	CInfoPBRec	pb;
	HFileInfo*	hpb = (HFileInfo*) &pb;
	
	// setup the parameter block and make a synchronous PB call
//	hpb->ioCompletion = nil;
	hpb->ioNamePtr    = fileName;
	hpb->ioVRefNum    =
	hpb->ioFDirIndex  = 0;
	hpb->ioDirID      = dirId;

	if (PBGetCatInfoSync(&pb) != noErr) {
		errno = ENOENT;
		return -1;
	}

	// fill in the 'stat' structure
//	sr->st_mode = (hpb->ioFlAttrib & kDirBit) ? S_IFDIR : S_IFREG;
	if (hpb->ioFlAttrib & kDirBit) {
		sr->st_mode = S_IEXEC | S_IFDIR;
		sr->st_size  = pb.dirInfo.ioDrNmFls;		// Num of files in dir
	} else {
		sr->st_mode = (hpb->ioFlFndrInfo.fdType == 'APPL')
						? (S_IFREG | S_IEXEC) : S_IFREG;
		sr->st_size  = hpb->ioFlLgLen;				// logical data fork length
	}
//	sr->st_mode |= (hpb->ioFlAttrib & kLockBit) ? S_IREAD : S_IRW;
	sr->st_dev   = 0;								// who cares?
	sr->st_ino   = hpb->ioDirID;					// hard file number
	sr->st_nlink = 0;								// no links
	sr->st_uid   = getuid();						// use default uid and
	sr->st_gid   = getgid();						// gid
//	sr->st_size  = hpb->ioFlLgLen;					// logical data fork length
	sr->st_atime =
	sr->st_mtime = hpb->ioFlMdDat;					// access/mod dates the same
	sr->st_ctime = hpb->ioFlCrDat;					// the creation
	return 0;
}


int
stat (const char* filename, struct stat* sr)
{
	Str255	pName;

	BlockMoveData(filename, &pName[1], pName[0] = strlen(filename));

	return __stat(pName, 0, sr);
}


int
fstat (int fd, struct stat* sr)
{
	FCBPBRec	pb;
	Str255		pName;

	// PBGetFCBInfo() gives name of file and dirId

//	pb.ioCompletion = nil;
	pb.ioFCBIndx    = 0;
	pb.ioVRefNum    = 0;
	pb.ioNamePtr    = pName;

	if (ioctl(fd, FIOREFNUM, (long*) &pb.ioRefNum) == -1
					|| PBGetFCBInfoSync(&pb) != noErr) {
		errno = ENOENT;
		return -1;
	}

	return __stat(pName, pb.ioFCBParID, sr);
}
