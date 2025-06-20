#include	"Independents.h"


StringPtr GetPtrIndHString(Handle resH, unsigned short index)
{
unsigned short	*compPtr = (unsigned short *)*resH;
unsigned char	*spanPtr;
unsigned short	i = index;

if (*compPtr++ <= i)
	spanPtr = nil;
else {
	spanPtr = (unsigned char *)compPtr;
	while (i) {
		spanPtr += *spanPtr++;
		i--;
		}
	}

return (StringPtr)spanPtr;
}

void GetIndHString(StringPtr dest, Handle resH, unsigned short index)
{
unsigned short	*compPtr = (unsigned short *)*resH;
unsigned char	*spanPtr;
unsigned short	i = index;

if (*compPtr++ <= i)
	StrLength(dest) = 0;
else {
	spanPtr = (unsigned char *)compPtr;
	while (i) {
		spanPtr += *spanPtr++;
		i--;
		}
	BlockMoveData(spanPtr, dest, *spanPtr + 1L);
	}

return;
}
/*
short KeyState(unsigned short k )
// k =  any keyboard scan code, 0-127
{
KeyMap	km;

GetKeys(km);
return ( ( *((unsigned char *)km + (k>>3)) >> (k & 7) ) & 1);
}
*/

Boolean ModifiersState(short mask)
{
EventRecord	ev;

(void) EventAvail(0, &ev);
return (ev.modifiers & mask) != 0;
}

OSType Str2OSType(ConstStr255Param theStr)
{
OSType result;
Ptr	source, dest;
unsigned char i = StrLength(theStr);

source = (Ptr)theStr + 1;
dest = (Ptr)&result;
*dest++ = *source++;
*dest++ = *source++;
*dest++ = *source++;
*dest++ = *source++;

if (i < 4)
	while (i < 4) {
		*--dest = 0x20;
		i++;
		}

return result;
}

void OSType2Str(OSType typ, StringPtr theStr)
{
register StringPtr	tempPtr = theStr;
register StringPtr	sourcePtr = (StringPtr)&typ;

*tempPtr++ = 4;
*tempPtr++ = *sourcePtr++;
*tempPtr++ = *sourcePtr++;
*tempPtr++ = *sourcePtr++;
*tempPtr = *sourcePtr;
}

Handle GetFullPath(const FSSpecPtr fss, Boolean AUXpresent)
{
CInfoPBRec	mypb;
Str63	dirName;
Handle	fullPath;
Size	hCurSize;
OSErr	err = noErr;

fullPath = NewHandle(StrLength(fss->name));
if (fullPath) {
	BlockMoveData(fss->name + 1, *fullPath, StrLength(fss->name));
	if (fss->parID != fsRtParID) {
		mypb.dirInfo.ioNamePtr = dirName;
		mypb.dirInfo.ioVRefNum = fss->vRefNum;
		mypb.dirInfo.ioDrParID = fss->parID;
		mypb.dirInfo.ioFDirIndex = -1;
		do {
			mypb.dirInfo.ioDrDirID = mypb.dirInfo.ioDrParID;
			err = PBGetCatInfoSync(&mypb);
			if (err)
				break;
			else {
				if (AUXpresent) {
					if (dirName[1] != '/') {
						StrLength(dirName) += 1;
						dirName[StrLength(dirName)] = '/';
						}
					}
				else {
					StrLength(dirName) += 1;
					dirName[StrLength(dirName)] = ':';
					}
				hCurSize = InlineGetHandleSize(fullPath);
				SetHandleSize(fullPath, hCurSize + StrLength(dirName));
				err = MemError();
				if (err)
					break;
				else {
					BlockMoveData(*fullPath, *fullPath + StrLength(dirName), hCurSize);
					BlockMoveData(dirName + 1, *fullPath, StrLength(dirName));
					}
				}
			}
		while (mypb.dirInfo.ioDrDirID != fsRtDirID);
		}
	}
if (err) {
	DisposeHandle(fullPath);
	fullPath = nil;
	}
return fullPath;
}

void fabc2pstr(unsigned char *cs, StringPtr destps)
{
unsigned char *scanPtr = cs;
unsigned char *destPtr = destps + 1;
unsigned short count = 0;

while ((*destPtr++ = *scanPtr++) && count < UCHAR_MAX)
	count++;
StrLength(destps) = count;
}

Boolean IsOnScreen(const RectPtr r)
{
Point	topRight;

topRight.h = r->right;
topRight.v = r->top;
return (PtInRgn(topLeft(*r), GetGrayRgn()) || (PtInRgn(topRight, GetGrayRgn())));
}

Boolean IsOnScreenWeak(Point pt)
{
Rect	box;

topLeft(box) = pt;
box.bottom = box.top + qd.thePort->portRect.bottom - qd.thePort->portRect.top;
box.right = box.left + qd.thePort->portRect.right - qd.thePort->portRect.left;
return IsOnScreen(&box);
}

