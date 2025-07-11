/*
	Terminal 2.2
	"MacBinary.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#endif

#include "MacBinary.h"
#include "CRC.h"
#include "Utilities.h"

#if 1

typedef struct {
	Byte b[BinHeaderLength];
} HEADER;
#define HEADER_oldVersion		(header->b)[  0]
#define HEADER_name				(header->b)[  1]
#define HEADER_info1			(header->b)[ 65]
#define HEADER_protected		(header->b)[ 81]
#define HEADER_zero1			(header->b)[ 82]
#define HEADER_dLength			(header->b)[ 83]
#define HEADER_rLength			(header->b)[ 87]
#define HEADER_creation			(header->b)[ 91]
#define HEADER_modification		(header->b)[ 95]
#define HEADER_getInfoLength	(header->b)[ 99]
#define HEADER_info2			(header->b)[101]
#define HEADER_filesLength		(header->b)[116]
#define HEADER_sHeaderLength	(header->b)[120]
#define HEADER_newVersion		(header->b)[122]
#define HEADER_minimumVersion	(header->b)[123]
#define HEADER_crc				(header->b)[124]
#define HEADER_zero3			(header->b)[126]

#else	/* This no longer works correctly for THINK C 5.02 */

typedef struct {
	Byte oldVersion;		/* 000: Must be zero for compatibility */
	Byte name[64];			/* 001: Pascal string */
	Byte info1[16];			/* 065: Original Finder info: */
							/*		File type (long) */
							/*		File creator (long) */
							/*		Flags (word, low byte must be zero) */
							/*		File's location (Point) */
							/*		File's window (short) */
	Byte protected;			/* 081: Low order bit */
	Byte zero1;				/* 082: Must be zero for compatibility */
	Byte dLength[4];		/* 083: Data fork length (in bytes) */
	Byte rLength[4];		/* 087: Resource fork length (in bytes) */
	Byte creation[4];		/* 091: Creation date */
	Byte modification[4];	/* 095: Modification date */
	Byte getInfoLength[2];	/* 099:	Get info length */
	Byte info2;				/* 101: Finder flags, low byte */
	Byte zero2[14];			/* 102: Not used */
	Byte filesLength[4];	/* 116: Length of total unpacked files */
	Byte sHeaderLength[2];	/* 120: Length of secondary header */
	Byte newVersion;		/* 122: Uploading program version number */
	Byte minimumVersion;	/* 123: Minimum version number needed */
	Byte crc[2];			/* 124: CRC of previous 124 bytes */
	Byte zero3[2];			/* 126: Not used */
} HEADER;

#endif

#define DESKTOP		0x0001	/* ---- ---- ---- ---1 */
#define	COLOR		0x000E	/* ---- ---- ---- 111- */
#define INITED		0x0100	/* ---- ---1 ---- ---- */
#define CHANGED		0x0200	/* ---- --1- ---- ---- */
#define BUSY		0x0400	/* ---- -1-- ---- ---- */
#define BOZO		0x0800	/* ---- 1--- ---- ---- */
#define SYSTEM		0x1000	/* ---1 ---- ---- ---- */
#define BUNDLE		0x2000	/* --1- ---- ---- ---- */
#define INVISIBLE	0x4000	/* -1-- ---- ---- ---- */
#define LOCKED		0x8000	/* 1--- ---- ---- ---- */

#define NOMODIFY	(DESKTOP | INITED | CHANGED | BUSY)

typedef struct {
	Byte	name[64];	/* File name */
	short	volume;		/* Volume reference number */
	long	directory;	/* Directory ID */
	short	ref;		/* File reference number */
	long	data;		/* Size of data fork */
	long	resource;	/* Size of resource fork */
	long	mark;		/* Current read/write mark */
	HEADER	header;		/* MacBinary header */
} MBFCB;

static MBFCB **File = 0;		/* Current file control block handle */

/* ----- Minimum ------------------------------------------------------- */

static long Minimum(register long a, register long b)
{
	return (a < b) ? a : b;
}

/* ----- Set up MacBinary header --------------------------------------- */

static short InitMacBinary(
	short volume,
	long directory,
	Byte *name,
	HEADER *header,
	long *data,
	long *resource)
{
	short err;
	unsigned short crc;
	HFileParam param;

	memset(&param, 0, sizeof(param));
	param.ioNamePtr = (StringPtr)name;
	param.ioVRefNum = volume;
	param.ioDirID = directory;
	if (err = PBHGetFInfo((HParmBlkPtr)&param, FALSE))
		return err;

	memset(header, 0, BinHeaderLength);
	memcpy(&HEADER_name, name, name[0] + 1);
	HEADER_info2 = param.ioFlFndrInfo.fdFlags & 0x00FF;
	param.ioFlFndrInfo.fdFlags &= 0xFF00;
	*(long *)&param.ioFlFndrInfo.fdLocation = 0;
	param.ioFlFndrInfo.fdFldr = 0;
	memcpy(&HEADER_info1, (void *)&param.ioFlFndrInfo, 16);
	HEADER_protected = (Byte)(param.ioFlAttrib) & 0x01;
	memcpy(&HEADER_dLength, (void *)&param.ioFlLgLen, 4);
	memcpy(&HEADER_rLength, (void *)&param.ioFlRLgLen, 4);
	memcpy(&HEADER_creation, (void *)&param.ioFlCrDat, 4);
	memcpy(&HEADER_modification, (void *)&param.ioFlMdDat, 4);
	HEADER_newVersion = 129;
	HEADER_minimumVersion = 129;
	crc = CalcCRC((Byte *)header, 124, 0);
	memcpy(&HEADER_crc, (void *)&crc, 2);

	*data = param.ioFlLgLen;
	*resource = param.ioFlRLgLen;
	return 0;
}

/* ----- Verify MacBinary header --------------------------------------- */

short BinCheckHeader(
	register Byte *p,
	Byte *name,				/* File name */
	long *data,				/* Data fork length */
	long *resource)			/* Resource fork length */
{
	register short i, j, version;
	register HEADER *header;
	unsigned short crc;

	*name = 0;
	*data = *resource = 0;
	header = (HEADER *)p;
	if (p[0] || p[74] || p[82] || p[83] || p[87] ||
			(i = HEADER_name) > 63)
		return 0;
	memcpy((void *)&crc, &HEADER_crc, 2);
	if (crc == CalcCRC((Byte *)header, 124, 0) &&
			HEADER_minimumVersion <= 129)
		version = 2;
	else {
		for (j = 101; j < 125; j++)
			if (p[j])
				return 0;
		version = 1;
	}
	memcpy((void *)name, &HEADER_name, i + 1);
	memcpy((void *)data, &HEADER_dLength, 4);
	memcpy((void *)resource, &HEADER_rLength, 4);
	return version;
}

/* ----- Set up file info from MacBinary header ------------------------ */

static short SetupMacBinary(
	HEADER *header,
	short volume,
	long directory,
	Byte *name)
{
	register short err;
	HFileParam param;
	FInfo info;

	memset(&param, 0, sizeof(param));
	param.ioNamePtr = (StringPtr)name;
	param.ioVRefNum = volume;
	param.ioDirID = directory;
	if (err = PBHGetFInfo((HParmBlkPtr)&param, FALSE))
		return err;

	memcpy((void *)&info, &HEADER_info1, sizeof(info));
	info.fdFlags &= 0xFF00;			/* High byte */
	info.fdFlags |= HEADER_info2;	/* Low byte */
	info.fdFlags &= ~NOMODIFY;
	param.ioFlFndrInfo.fdType = info.fdType;
	param.ioFlFndrInfo.fdCreator = info.fdCreator;
	param.ioFlFndrInfo.fdFlags &= NOMODIFY;
	param.ioFlFndrInfo.fdFlags |= info.fdFlags;
	memcpy((void *)&param.ioFlCrDat, &HEADER_creation, 4);
	memcpy((void *)&param.ioFlMdDat, &HEADER_modification, 4);
	param.ioNamePtr = (StringPtr)name;
	param.ioVRefNum = volume;
	param.ioDirID = directory;
	return PBHSetFInfo((HParmBlkPtr)&param, FALSE);
}

/* ----- Open disk file for reading ------------------------------------ */

short BinOpenRead(
	register short volume,
	register long directory,
	register Byte *name)
{
	register MBFCB **f;
	register MBFCB *p;
	register short err;

	if (File)
		return 1;		/* Is already open! */
	if (!(f = (MBFCB **)NewHandle(sizeof(MBFCB))))
		return 2;		/* Memory problem! */

	HLock((Handle)f);
	p = *f;
	if (!(err = InitMacBinary(volume, directory, name,
			&p->header, &p->data, &p->resource))) {
		memcpy((void *)p->name, (void *)name, name[0] + 1);
		p->volume = volume;
		p->directory = directory;
		p->mark = 0;
	}
	HUnlock((Handle)f);
	if (err)
		DisposHandle((Handle)f);
	else
		File = f;		/* Now is open */
	return err;
}

/* ----- Read from disk file ------------------------------------------- */

short BinRead(
	long *count,
	Byte *buffer)
{
	register MBFCB **f;
	register MBFCB *p;
	register short err;
	register long n;
	long b, c, d, e;
	long i;

	if (!(f = File))
		return 1;		/* Is not open! */
	HLock((Handle)f);
	p = *f;

	b = sizeof(HEADER) + p->data;
	c = b + Filler(BinHeaderLength, p->data);
	d = c + p->resource;
	e = d + Filler(BinHeaderLength, p->resource);

	/* Open data fork of file if necessary */

	if (p->mark == sizeof(HEADER) &&
			(err = OpenFile(p->volume,p->directory,p->name,&p->ref)))
		goto done;

	/* Close data fork of file if necessary */

	if (p->mark == b) {
		FSClose(p->ref);
		p->ref = 0;
	}

	/* Open resource fork of file if necessary */

	if (p->mark == c &&
			(err = OpenResource(p->volume,p->directory,p->name,&p->ref)))
		goto done;

	/* Close resource fork of file if necessary */

	if (p->mark == d) {
		FSClose(p->ref);
		p->ref = 0;
	}

	/* Copy from header */

	if (p->mark < sizeof(HEADER)) {
		n = Minimum(*count, sizeof(HEADER) - p->mark);
		memcpy((void *)buffer, (Byte *)&p->header + p->mark, n);
		goto allright;
	}

	/* Read data fork */

	if (p->mark < b) {
		i = Minimum(*count, b - p->mark);
		err = FSRead(p->ref, &i, buffer);
		n = i;
		goto done;
	}

	/* Clear buffer (fill up to entire multiple of 128 bytes) */

	if (p->mark < c) {
		n = Minimum(*count, c - p->mark);
		memset(buffer, 0, n);
		goto allright;
	}

	/* Read resource fork */

	if (p->mark < d) {
		i = Minimum(*count, d - p->mark);
		err = FSRead(p->ref, &i, buffer);
		n = i;
		goto done;
	}

	/* Clear buffer (fill up to entire multiple of 128 bytes) */

	if (p->mark < e) {
		n = Minimum(*count, e - p->mark);
		memset(buffer, 0, n);
		goto allright;
	}

	/* End of file reached */

	err = eofErr;
	n = 0;
	goto done;

allright:
	err = 0;		/* No error */
done:
	if (!err) {
		p->mark += n;
		if (c = *count - n)
			err = BinRead(&c, buffer + n);
		*count = n + c;
	} else {
		if (err == eofErr) {
			p->mark += n;
			*count = n;
		}
	}

	HUnlock((Handle)f);
	return err;
}

/* ----- Close disk file after reading --------------------------------- */

short BinCloseRead(void)
{
	register short ref;

	if (!File)
		return 1;				/* Is not open! */
	if (ref = (**File).ref)
		FSClose(ref);			/* Data or resource fork */
	DisposHandle((Handle)File);
	File = 0;					/* Now file is closed */
	return 0;
}

/* ----- Open disk file for writing ------------------------------------ */

short BinOpenWrite(
	register short volume,
	register long directory,
	register Byte *name,
	register Byte *header)
{
	register MBFCB **f;
	register MBFCB *p;
	Byte name2[64];

	if (File)
		return 1;		/* Is already open! */

	if (!(f = (MBFCB **)NewHandle(sizeof(MBFCB))))
		return 2;		/* Memory problem! */

	HLock((Handle)f);
	p = *f;

	BinCheckHeader(header, name2, &p->data, &p->resource);
	memcpy((void *)&p->header, (void *)header, sizeof(HEADER));
	memcpy((void *)p->name, (void *)name, name[0] + 1);
	p->volume = volume;
	p->directory = directory;
	p->mark = sizeof(HEADER);

	HUnlock((Handle)f);
	File = f;			/* Now is open */
	return 0;
}

/* ----- Write to disk file -------------------------------------------- */

short BinWrite(
	long *count,
	Byte *buffer)
{
	register MBFCB **f;
	register MBFCB *p;
	register short err;
	register long n;
	long b, c, d, e;
	long i;

	if (!(f = File))
		return 1;		/* Is not open! */
	HLock((Handle)f);
	p = *f;

	b = sizeof(HEADER) + p->data;
	c = b + Filler(BinHeaderLength, p->data);
	d = c + p->resource;
	e = d + Filler(BinHeaderLength, p->resource);

	/* Open data fork of file if necessary */

	if (p->mark == sizeof(HEADER) &&
			(err = OpenFile(p->volume,p->directory,p->name,&p->ref)))
		goto done;

	/* Close data fork of file if necessary */

	if (p->mark == b) {
		FSClose(p->ref);
		p->ref = 0;
	}

	/* Open resource fork of file if necessary */

	if (p->mark == c &&
			(err = OpenResource(p->volume,p->directory,p->name,&p->ref)))
		goto done;

	/* Close resource fork of file if necessary */

	if (p->mark == d) {
		FSClose(p->ref);
		p->ref = 0;
	}

	/* Ignore header */

	if (p->mark < sizeof(HEADER)) {
		n = Minimum(*count, sizeof(HEADER) - p->mark);
		goto allright;
	}

	/* Write data fork */

	if (p->mark < b) {
		i = Minimum(*count, b - p->mark);
		err = FSWrite(p->ref, &i, buffer);
		n = i;
		goto done;
	}

	/* Ignore buffer (filled up to entire multiple of 128 bytes) */

	if (p->mark < c) {
		n = Minimum(*count, c - p->mark);
		goto allright;
	}

	/* Write resource fork */

	if (p->mark < d) {
		i = Minimum(*count, d - p->mark);
		err = FSWrite(p->ref, &i, buffer);
		n = i;
		goto done;
	}

	/* Ignore buffer (filled up to entire multiple of 128 bytes) */

	if (p->mark < e) {
		n = Minimum(*count, e - p->mark);
		goto allright;
	}

	/* End of file reached (can happen if using XModem 1K) */

	err = eofErr;
	n = 0;
	goto done;

allright:
	err = 0;		/* No error */
done:
	if (!err) {
		p->mark += n;
		if (c = *count - n)
			err = BinWrite(&c, buffer + n);
		*count = n + c;
	} else {
		if (err == eofErr) {
			p->mark += n;
			*count = n;
			err = noErr;
		}
	}

	HUnlock((Handle)f);
	return err;
}

/* ----- Close disk file after writing --------------------------------- */

short BinCloseWrite(void)
{
	register short err;
	register MBFCB **f;
	register MBFCB *p;

	if (!(f = File))
		return 1;				/* Is not open! */
	HLock((Handle)f);
	p = *f;
	err = SetupMacBinary((HEADER *)&p->header, p->volume, p->directory,
		p->name);
	if (p->ref)
		FSClose(p->ref);		/* Data or resource fork */
	FlushVol(0, p->volume);
	HUnlock((Handle)f);
	DisposHandle((Handle)f);
	File = 0;					/* Now file is closed */
	return err;
}

/* ----- Get length of complete MacBinary file ------------------------- */

short BinGetEOF(register long *eof)
{
	register MBFCB **f;
	register MBFCB *p;

	*eof = 0;
	if (!(f = File))
		return 1;				/* Is not open! */
	p = *f;
	*eof = BinHeaderLength +
		p->data + Filler(BinHeaderLength, p->data) +
		p->resource + Filler(BinHeaderLength, p->resource);
	return 0;
}
