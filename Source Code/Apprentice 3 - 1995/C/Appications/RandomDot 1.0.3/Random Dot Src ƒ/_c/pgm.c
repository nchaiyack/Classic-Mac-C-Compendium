/* pgm.c - read Portable Graymap Files
	by David Phillip Oster October 1994 oster@netcom.com
	for:
	Stuart Inglis singlis@waikato.ac.nz
	Department of Computer Science
	University of Waikato, Hamilton, New Zealand
 */
#include "RandomDotMain.h"
#include "RandomDotRes.h"
#include "pgm.h"
#include "Error.h"
#include "RandomDotWin.h"
#include "Progress.h"
#include "Utils.h"

/* IsPGMFile - return TRUE if this is a pgm File.
	for now, a pgm file ends in ".pgm" (any case) and starts with
	"P2" or "P5"
 */
Boolean IsPGMFile(ParmBlkPtr io){
	Str255	s;
#if 0
	OSErr	errCode;
	Str255	buf;
	FSSpec	fs;
	Integer	ref;
	LongInt	len;
#endif

	if(Length(io->fileParam.ioNamePtr) < 5){
		return FALSE;
	}
	s[0] = 4;
	BlockMove(&io->fileParam.ioNamePtr[Length(io->fileParam.ioNamePtr) - 3], &s[1], 4);
	if(EqualString(s, "\p.pgm", FALSE, TRUE)){
		return TRUE;
	}
#if 0
	buf[0] = 2;
	len = 2;
	ref = -1;
	/* this line always fails */
	errCode = FSMakeFSSpec(io->fileParam.ioVRefNum, io->fileParam.ioFlNum, io->fileParam.ioNamePtr, &fs);
	if(noErr == errCode){ errCode = FSpOpenDF(&fs, fsCurPerm, &ref); }
	if(noErr == errCode){ errCode = FSRead(ref, &len, &buf[1]); }
	if(noErr == errCode &&
		'P' == buf[1] &&
		('2' == buf[2] || '5' == buf[2])){

		FSClose(ref);
		return TRUE;
	}
	if(ref != -1){
		FSClose(ref);
	}
#endif
	return FALSE;
}

/* GetInt - read an int into *np, return io error code.
 */
static OSErr GetInt(Integer ref, LongInt *np){
	OSErr	errCode;
	LongInt	len;
	LongInt	n;
	char	ch;

	n = 0;
	len = 1;
	do{
		if(noErr != (errCode = FSRead(ref, &len, &ch))){
			return errCode;
		}
	}while(NOT ('0' <= ch && ch <= '9'));

	do{
		n = n * 10 + ch - '0';
		if(noErr != (errCode = FSRead(ref, &len, &ch))){
			return errCode;
		}
	}while('0' <= ch && ch <= '9');
	*np = n;
	return errCode;
}

typedef struct BufferedIO{
	Ptr	pStart;
	Ptr	p;
	Ptr	pEnd;
	Integer	ref;
}BufferedIO, *BufferedIOPtr;

/* OpenBuffer - open ref for buffered reads.
 */
static OSErr OpenBuffer(Integer ref, Ptr *tempp){
	OSErr			errCode;
	BufferedIOPtr	bp;

	bp = (BufferedIOPtr) NewPtr(sizeof(BufferedIO));
	if(noErr == (errCode = MemError())){
		bp->pStart = NewPtr(10000L);
	}
	if(noErr == (errCode = MemError())){
		bp->p =
		bp->pEnd = bp->pStart;
		bp->ref = ref;
		*tempp = (Ptr) bp;
	}
	return errCode;
}

/* BufferedGetChar - 
 */
static OSErr BufferedGetChar(Ptr temp, char *cp){
	OSErr			errCode;
	BufferedIOPtr	bp;
	LongInt			len;

	bp = (BufferedIOPtr) temp;
	if(bp->p < bp->pEnd){
		*cp = *bp->p++;
		return noErr;
	}
	len = 10000L;
	errCode = FSRead(bp->ref, &len, bp->pStart);
	if(eofErr == errCode && len > 0){
		errCode = noErr;
	}
	bp->pEnd = bp->pStart + len;
	bp->p = bp->pStart;
	return noErr == errCode ? BufferedGetChar(temp, cp) : errCode;
}

/* BufferedGetInt - read an int into *np, return io error code.
 */
static OSErr BufferedGetInt(Ptr temp, LongInt *np){
	OSErr	errCode;
	LongInt	len;
	LongInt	n;
	char	ch;

	n = 0;
	len = 1;
	do{
		if(noErr != (errCode = BufferedGetChar(temp, &ch))){
			return errCode;
		}
	}while(NOT ('0' <= ch && ch <= '9'));

	do{
		n = n * 10 + ch - '0';
		if(noErr != (errCode = BufferedGetChar(temp, &ch))){
			return errCode;
		}
	}while('0' <= ch && ch <= '9');
	*np = n;
	return errCode;
}


/* CloseBuffer - 
 */
static OSErr CloseBuffer(Ptr temp){
	BufferedIOPtr	bp;

	bp = (BufferedIOPtr) temp;
	DisposePtr(bp->pStart);
	DisposePtr((Ptr) bp);
	return noErr;
}

/* PGMFileToGWorld - read a pgm file into a gray scale gworld

	Note: there is a bug in System 7.1 with the LaserWriter driver such that if
		you CopyBits() to the printer, with an 8-bit indexed offscreen gworld
		as the source, the first and last entries of the color table of the
		offscreen gworld get set so the first is white and the last is black.
		So, we follow that scheme in this program: for an 8-bit pixmap index,
		the gray scale value is the complement of the index.
		Watch out, expressions evaluate to ints in C, so ~(unsigned char) 1
		if 0xFFFE not 0xFE
 */
OSErr PGMFileToGWorld(FSSpecPtr fs, ScriptCode code, GWorldPtr *gworld){
	OSErr			errCode;
	CGrafPtr		savePort;
	GDHandle		saveGD;
	Integer			ref;
	LongInt			len;
	Str255			buf, buf2;
	char			magic;
	LongInt			width;
	LongInt			height;
	LongInt			maxval;
	LongInt			x, y;
	Rect			frame;
	unsigned char	ch;
	GWorldPtr		grayImage;
	PixMapHandle	pm;
	Ptr				p;
	unsigned char *pp, *pEnd;
	Ptr				temp;

	GetIndString(buf, kMainStrs, kOpening1S);
	Concat(buf, fs->name);
	GetIndString(buf2, kMainStrs, kOpening2S);
	Concat(buf, buf2);
	SetProgressLabelText(buf);
	grayImage = NIL;
	ref = -1;
	errCode = FSpOpenDF(fs, fsCurPerm, &ref);
	len = 2;
	temp = NIL;
	if(noErr == errCode){ errCode = FSRead(ref, &len, &buf[1]); }
	if(noErr == errCode && 'P' != buf[1]){ errCode = eBadPGMMagicNumber; }
	if(noErr == errCode){ errCode = GetInt(ref, &width); }
	if(noErr == errCode){ errCode = GetInt(ref, &height); }
	if(noErr == errCode){ errCode = GetInt(ref, &maxval); }
	if(noErr == errCode){ 
		magic = buf[2];
		frame.left = 0;
		frame.top = 0;
		frame.right = width;
		frame.bottom = height;
	}
	GetGWorld(&savePort, &saveGD);
	if(noErr == errCode){ errCode = NewGWorld(&grayImage, 8, &frame, grayCTab, NIL, 0); }
	if(noErr == errCode){
		SetGWorld(grayImage, NIL);
		ClipRect(&qd.thePort->portRect);
		pm = GetGWorldPixMap(grayImage);
		LockPixels(pm);
		switch (magic){
		case '2' :
			if(noErr != (errCode = OpenBuffer(ref, &temp))){ goto afterSwitch; }
			for (y = 0; y < height; y++ ){
				if(0 == (y & 15)){
					SetGWorld(savePort, saveGD);
					if(noErr != (errCode = ShowProgress(y, height))){
						goto afterSwitch;
					}
					SetGWorld(grayImage, NIL);
				}
				p = GetPixBaseAddr(pm) + y * ((**pm).rowBytes & 0x7FFF);
				for(x = 0; x < width; x++, p++)	{
					if(noErr != (errCode = BufferedGetInt(temp, &len))){
						goto afterSwitch;
					}
					ch = len;
					*p = ~ch;
				}
			}
			break;
		case '5' :
			len = width;
			for (y = 0; y < height; y++ ){
				if(0 == (y & 15)){
					SetGWorld(savePort, saveGD);
					if(noErr != (errCode = ShowProgress(y, height))){
						goto afterSwitch;
					}
					SetGWorld(grayImage, NIL);
				}
				p = GetPixBaseAddr(pm) + y * ((**pm).rowBytes & 0x7FFF);
				if(noErr != (errCode = FSRead(ref, &len, p))){
					goto afterSwitch;
				}
				pEnd = (unsigned char *)  (p + len);
				for(pp = (unsigned char *) p ; pp < pEnd ; pp++){
					*pp = ~*pp;
				}
			}
			break;
		default : errCode = eBadPGMMagicNumber;
			break; 
		}
afterSwitch:
		SetGWorld(savePort, saveGD);
		ShowProgress(height, height);
		UnlockPixels(pm);
	}
	if(-1 != ref){
		FSClose(ref);
	}
	if(NIL != temp){
		CloseBuffer(temp);
		temp = NIL;
	}
	SetGWorld(savePort, saveGD);
	if(noErr == errCode){
		*gworld = grayImage;
	}else if(NIL != grayImage){
		DisposeGWorld(grayImage);
	}
	return errCode;
}