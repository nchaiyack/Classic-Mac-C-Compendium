///-------------------------------------------------------------------------------------
//	SpriteCompiler.c
//
//	Created:	1/17/94
//	By:			Tony Myles
//
//	Copyright: © 1994 Tony Myles, All rights reserved worldwide
//
//	Description:	constants, structures, and prototypes for the sprite compiler
///--------------------------------------------------------------------------------------


#ifndef __SWCOMMON__
#include "SWCommonHeaders.h"
#endif

#ifndef __TYPES__
#include <Types.h>
#endif

#ifndef __MEMORY__
#include <Memory.h>
#endif

#ifndef __RESOURCES__
#include <Resources.h>
#endif

#ifndef __QUICKDRAW__
#include <QuickDraw.h>
#endif

#ifndef __DEBUGUTILS__
#include "DebugUtils.h"
#endif

#ifndef __SPRITE__
#include "Sprite.h"
#endif

#ifndef __SPRITEWORLDUTILS__
#include "SpriteWorldUtils.h"
#endif

#ifndef __SPRITECOMPILER__
#include "SpriteCompiler.h"
#endif



OSErr SWCompileColorIconResource(
	short iconResID,
	PixelCodeHdl* newPixCodeH)
{
	OSErr err;
	GrafPtr savePort;
	CGrafPtr newGrafPort;
	CGrafPtr maskGrafPort;
	CIconHandle cIconH;
	Rect cIconRect;

	*newPixCodeH = NULL;
	newGrafPort = NULL;
	maskGrafPort = NULL;
	cIconH = NULL;

	GetPort(&savePort);

	err = SWGetCIcon(&cIconH, iconResID);

	if (err == noErr)
	{
		HNoPurge((Handle)cIconH);

		cIconRect = (**cIconH).iconPMap.bounds;

		err = SWCreateBestCGrafPort(&newGrafPort, &cIconRect);
	}

	if (err == noErr)
	{
		SetPort((GrafPtr)newGrafPort);

		SWPlotCIcon(cIconH, &cIconRect);
	}

	if (err == noErr)
	{
		err = SWCreateCGrafPortFromCIconMask(&maskGrafPort, cIconH);
	}

	if (err == noErr)
	{
		err = SWParsePixels(newGrafPort->portPixMap, maskGrafPort->portPixMap, newPixCodeH);
	}

	if (cIconH != NULL)
	{
		SWDisposeCIcon(cIconH);
	}

	if (newGrafPort != NULL)
	{
		SWDisposeCGrafPort(newGrafPort);
	}

	if (maskGrafPort != NULL)
	{
		SWDisposeCGrafPort(maskGrafPort);
	}

	SetPort(savePort);

	return err;
}


OSErr SWParsePixels(
	PixMapHandle srcPixMapH,
	PixMapHandle maskPixMapH,
	PixelCodeHdl* newPixCodeH)
{
	OSErr err = noErr;
	PixelCodeHdl tempPixCodeH;

	*newPixCodeH = NULL;

	tempPixCodeH = (PixelCodeHdl)NewHandle(0);

	if (tempPixCodeH != NULL)
	{
		long scanLine;
		long numberOfScanLines = (**srcPixMapH).bounds.bottom - (**srcPixMapH).bounds.top;
		long pixelsPerScanLine = (**srcPixMapH).rowBytes & 0x7FFF;
		Ptr srcBaseAddrP, maskBaseAddrP;

		srcBaseAddrP = (**srcPixMapH).baseAddr;
		maskBaseAddrP = (**maskPixMapH).baseAddr;

		err = SWGeneratePreamble(tempPixCodeH);

		for (scanLine = 0; (err == noErr) && (scanLine < numberOfScanLines); scanLine++)
		{
			err = SWCompileMaskScanLine(tempPixCodeH, maskBaseAddrP, pixelsPerScanLine, scanLine == (numberOfScanLines-1));

			if (err == noErr)
			{
				srcBaseAddrP += pixelsPerScanLine;
				maskBaseAddrP += pixelsPerScanLine;
			}
		}

		if (err == noErr)
		{
			err = SWGeneratePostamble(tempPixCodeH);
		}

		if (err == noErr)
			*newPixCodeH = tempPixCodeH;
	}
	else
	{
		err = MemError();
	}

	return err;
}


OSErr SWGeneratePreamble(PixelCodeHdl srcPixCodeH)
{
	OSErr err = noErr;
	Size curHandleSize;
	unsigned char* scanCodeP;

	curHandleSize = GetHandleSize((Handle)srcPixCodeH);
	err = MemError();

	if (err == noErr)
	{
		SetHandleSize((Handle)srcPixCodeH, curHandleSize + 10);
		err = MemError();
	}

	if (err == noErr)
	{
		scanCodeP = ((unsigned char*)*srcPixCodeH) + curHandleSize;
		
			//48E7 1F3E          MOVEM.L   D3-D7/A2-A6,-(A7)
		*(unsigned long*)scanCodeP = 0x48E71F3E;
		scanCodeP += 4;

			//4CEF 0303 002C     MOVEM.L   $002C(A7),D0/D1/A0/A1
		*(unsigned long*)scanCodeP = 0x4CEF0303;
		scanCodeP += 4;

		*(unsigned short*)scanCodeP = 0x002C;
		scanCodeP += 2;
	}

	return err;
}

/*
00000000: 48E7 1F3E          MOVEM.L   D3-D7/A2-A6,-(A7)
00000004: 4CEF 0303 002C     MOVEM.L   $002C(A7),D0/D1/A0/A1
0000000A: 4E92               JSR       (A2)
0000000C: 4CDF 7CF8          MOVEM.L   (A7)+,D3-D7/A2-A6
00000010: 4E75               RTS
*/

OSErr SWGeneratePostamble(PixelCodeHdl srcPixCodeH)
{
	OSErr err = noErr;
	Size curHandleSize;
	unsigned char* scanCodeP;

	curHandleSize = GetHandleSize((Handle)srcPixCodeH);
	err = MemError();

	if (err == noErr)
	{
		SetHandleSize((Handle)srcPixCodeH, curHandleSize + 6);
		err = MemError();
	}

	if (err == noErr)
	{
		scanCodeP = ((unsigned char*)*srcPixCodeH) + curHandleSize;
		
			//4CDF 7CF8          MOVEM.L   (A7)+,D3-D7/A2-A6
		*(unsigned long*)scanCodeP = 0x4CDF7CF8;
		scanCodeP += 4;

			//4E75               RTS
		*(unsigned short*)scanCodeP = 0x4E75;
		scanCodeP += 2;
	}

	return err;
}



OSErr SWCompileMaskScanLine(
	PixelCodeHdl newPixCodeH,
	Ptr maskPixelP,
	long numberOfPixels,
	Boolean isLastScanLine)
{
	OSErr err = noErr;
	Size curHandleSize;
	unsigned char* scanCodeP;
	unsigned char* maskScanPixelP = (unsigned char*)maskPixelP;
	Size codeSize;
	long pixelsToScan;
	unsigned short numPixelsToBlit;
	unsigned short pixelOffset;

	curHandleSize = GetHandleSize((Handle)newPixCodeH);
	err = MemError();

	if (err == noErr)
	{
		SetHandleSize((Handle)newPixCodeH, curHandleSize + (numberOfPixels * kMaxInstructionSize) + 4);
		err = MemError();
	}

	if (err == noErr)
	{
		scanCodeP = ((unsigned char*)*newPixCodeH) + curHandleSize;
		codeSize = curHandleSize;
		pixelOffset = 0;
		pixelsToScan = numberOfPixels;

		while (pixelsToScan)
		{
			numPixelsToBlit = 0;

				// scan for pixels to blit
			while (pixelsToScan && *maskScanPixelP)
			{
				maskScanPixelP++;
				numPixelsToBlit++;
				pixelsToScan--;
			}

				// generate code for the pixel run
			while (numPixelsToBlit)
			{
				if (numPixelsToBlit >= 44)
				{
					if (pixelOffset)
					{
							//4CE8 7CFC 0042     MOVEM.L   $0042(A0),D2-D7/A2-A6
							//48E9 7CFC 0042     MOVEM.L   D2-D7/A2-A6,$0042(A1)

						*(unsigned long*)scanCodeP = 0x4CE87CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E97CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 7CFC          MOVEM.L   (A0),D2-D7/A2-A6
							//48D1 7CFC          MOVEM.L   D2-D7/A2-A6,(A1)

						*(unsigned long*)scanCodeP = 0x4CD07CFC;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D17CFC;
						scanCodeP += 4;

						codeSize += 8;
					}

					pixelOffset += 44;
					numPixelsToBlit -= 44;
				}
				else if (numPixelsToBlit >= 40)
				{
					if (pixelOffset)
					{
							//4CD0 3CFC 0042     MOVEM.L   $0042(A0),D2-D7/A2-A5
							//48D1 3CFC 0042     MOVEM.L   D2-D7/A2-A5,$0042(A1)
						*(unsigned long*)scanCodeP = 0x4CE83CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E93CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 3CFC     MOVEM.L   (A0),D2-D7/A2-A5
							//48D1 3CFC     MOVEM.L   D2-D7/A2-A5,(A1)
	
						*(unsigned long*)scanCodeP = 0x4CD03CFC;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D13CFC;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 40;
					numPixelsToBlit -= 40;
				}
				else if (numPixelsToBlit >= 36)
				{
					if (pixelOffset)
					{
							//4CE8 1CFC 0042     MOVEM.L   $0042(A0),D2-D7/A2-A4
							//48E9 1CFC 0042     MOVEM.L   D2-D7/A2-A4,$0042(A1)
	
						*(unsigned long*)scanCodeP = 0x4CE81CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E91CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 1CFC     MOVEM.L   (A0),D2-D7/A2-A4
							//48D1 1CFC     MOVEM.L   D2-D7/A2-A4,(A1)
	
						*(unsigned long*)scanCodeP = 0x4CD01CFC;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D11CFC;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 36;
					numPixelsToBlit -= 36;
				}
				else if (numPixelsToBlit >= 32)
				{
					if (pixelOffset)
					{
							//4CD0 0CFC 0042     MOVEM.L   $0042(A0),D2-D7/A2/A3
							//48D1 0CFC 0042     MOVEM.L   D2-D7/A2/A3,$0042(A1)

						*(unsigned long*)scanCodeP = 0x4CE80CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E90CFC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 0CFC     MOVEM.L   (A0),D2-D7/A2/A3
							//48D1 0CFC     MOVEM.L   D2-D7/A2/A3,(A1)
	
						*(unsigned long*)scanCodeP = 0x4CD00CFC;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D10CFC;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 32;
					numPixelsToBlit -= 32;
				}
				else if (numPixelsToBlit >= 28)
				{
					if (pixelOffset)
					{
							//4CE8 04FC 0042     MOVEM.L   $0042(A0),D2-D7/A2
							//48E9 04FC 0042     MOVEM.L   D2-D7/A2,$0042(A1)
	
						*(unsigned long*)scanCodeP = 0x4CE804FC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E904FC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 04FC     MOVEM.L   (A0),D2-D7/A2
							//48D1 04FC     MOVEM.L   D2-D7/A2,(A1)
	
						*(unsigned long*)scanCodeP = 0x4CD004FC;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D104FC;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 28;
					numPixelsToBlit -= 28;
				}
				else if (numPixelsToBlit >= 24)
				{
					if (pixelOffset)
					{
							//4CE8 00FC 0042     MOVEM.L   $0042(A0),D2-D7
							//48E9 00FC 0042     MOVEM.L   D2-D7,$0042(A1)
						*(unsigned long*)scanCodeP = 0x4CE800FC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E900FC;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 00FC          MOVEM.L   (A0),D2-D7
							//48D1 00FC          MOVEM.L   D2-D7,(A1)
						*(unsigned long*)scanCodeP = 0x4CD000FC;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D100FC;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 24;
					numPixelsToBlit -= 24;
				}
				else if (numPixelsToBlit >= 20)
				{
					if (pixelOffset)
					{
							//4CE8 007C 0042     MOVEM.L   $0042(A0),D2-D6
							//48E9 007C 0042     MOVEM.L   D2-D6,$0042(A1)
						*(unsigned long*)scanCodeP = 0x4CE8007C;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E9007C;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 007C          MOVEM.L   (A0),D2-D6
							//48D1 007C          MOVEM.L   D2-D6,(A1)
						*(unsigned long*)scanCodeP = 0x4CD0007C;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D1007C;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 20;
					numPixelsToBlit -= 20;
				}
				else if (numPixelsToBlit >= 16)
				{
					if (pixelOffset)
					{
							//4CE8 003C 0042     MOVEM.L   $0042(A0),D2-D5
							//48E9 003C 0042     MOVEM.L   D2-D5,$0042(A1)
						*(unsigned long*)scanCodeP = 0x4CE8003C;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E9003C;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 003C     MOVEM.L   (A0),D2-D5
							//48D1 003C     MOVEM.L   D2-D5,(A1)
						*(unsigned long*)scanCodeP = 0x4CD0003C;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D1003C;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 16;
					numPixelsToBlit -= 16;
				}
				else if (numPixelsToBlit >= 12)
				{
					if (pixelOffset)
					{
							//4CE8 001C 0042     MOVEM.L   $0042(A0),D2-D4
							//48E9 001C 0042     MOVEM.L   D2-D4,$0042(A1)
						*(unsigned long*)scanCodeP = 0x4CE8001C;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned long*)scanCodeP = 0x48E9001C;
						scanCodeP += 4;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 12;
					}
					else
					{
							//4CD0 001C     MOVEM.L   (A0),D2-D4
							//48D1 001C     MOVEM.L   D2-D4,(A1)
						*(unsigned long*)scanCodeP = 0x4CD0001C;
						scanCodeP += 4;
	
						*(unsigned long*)scanCodeP = 0x48D1001C;
						scanCodeP += 4;
	
						codeSize += 8;
					}

					pixelOffset += 12;
					numPixelsToBlit -= 12;
				}
				else if (numPixelsToBlit >= 4)
				{
					if (pixelOffset)
					{
							//2368 0066 0066     MOVE.L    $0066(A0),$0066(A1)
	
						*(unsigned short*)scanCodeP = 0x2368;
						scanCodeP += 2;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 6;
					}
					else
					{
							//2290               MOVE.L    (A0),(A1)

						*(unsigned short*)scanCodeP = 0x2290;
						scanCodeP += 2;

						codeSize += 2;
					}

					pixelOffset += 4;
					numPixelsToBlit -= 4;
				}
				else if (numPixelsToBlit >= 2)
				{
					if (pixelOffset)
					{
							//3368 0066 0066     MOVE.W    $0066(A0),$0066(A1)
	
						*(unsigned short*)scanCodeP = 0x3368;
						scanCodeP += 2;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 6;
					}
					else
					{
							//3290               MOVE.W    (A0),(A1)

						*(unsigned short*)scanCodeP = 0x3290;
						scanCodeP += 2;

						codeSize += 2;
					}

					pixelOffset += 2;
					numPixelsToBlit -= 2;
				}
				else if (numPixelsToBlit == 1)
				{
					if (pixelOffset)
					{
							//1368 0066 0066     MOVE.B    $0066(A0),$0066(A1)
	
						*(unsigned short*)scanCodeP = 0x1368;
						scanCodeP += 2;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;
	
						*(unsigned short*)scanCodeP = pixelOffset;
						scanCodeP += 2;

						codeSize += 6;
					}
					else
					{
							//1290               MOVE.B    (A0),(A1)

						*(unsigned short*)scanCodeP = 0x1290;
						scanCodeP += 2;

						codeSize += 2;
					}

					pixelOffset++;
					numPixelsToBlit--;
				}
			}

				// scan for pixels to skip
			while (pixelsToScan && !*maskScanPixelP)
			{
				pixelOffset++;
				maskScanPixelP++;
				pixelsToScan--;
			}
		}

		if (!isLastScanLine)
		{
				//D1C0               ADDA.L    D0,A0
			*(unsigned short*)scanCodeP = 0xD1C0;
			scanCodeP += 2;

				//D3C1               ADDA.L    D1,A1
			*(unsigned short*)scanCodeP = 0xD3C1;
			scanCodeP += 2;

			codeSize += 4;
		}

		SetHandleSize((Handle)newPixCodeH, codeSize);
		err = MemError();
	}

	return err;
}


OSErr SWSavePixelCodeResource(
	PixelCodeHdl newPixCodeH,
	short pixCodeResID)
{
	OSErr err = noErr;
	Handle curPixCodeH;
	Str255 resName = "\p";

	curPixCodeH = GetResource(kPixelCodeResType, pixCodeResID);
	if (curPixCodeH == NULL)
	{
		curPixCodeH = (Handle)newPixCodeH;

		AddResource(curPixCodeH, kPixelCodeResType, pixCodeResID, resName);
		err = ResError();

		if (err == noErr)
		{
			WriteResource(curPixCodeH);
			err = ResError();
		}

		if (err == noErr)
		{
			DetachResource(curPixCodeH);
			err = ResError();
		}
	}
	else
	{
		Size newHandleSize;

		newHandleSize = GetHandleSize((Handle)newPixCodeH);
		err = MemError();

		if (err == noErr)
		{
			err = PtrToXHand((Ptr)*newPixCodeH, curPixCodeH, newHandleSize);
		}

		if (err == noErr)
		{
			ChangedResource(curPixCodeH);
			err = ResError();
		}

		if (err == noErr)
		{
			WriteResource(curPixCodeH);
			err = ResError();
		}

		ReleaseResource(curPixCodeH);
	}

	return err;
}


#if 0

void x(void);

SW_ASM_FUNC void x(void)
{
		SW_ASM_BEGIN

		movem.l	d3-d7/a2-a6,-(sp)		// save off regs
		movem.l 44(sp),d0/d1/a0-a1		// blast the parameters into registers (cool!)
		jsr		(a2)					// jump into the blitter
		movem.l	(sp)+,d3-d7/a2-a6		// restore regs
		rts

			// move 44 bytes
		movem.l	66(a0), d2-d7/a2-a6
		movem.l d2-d7/a2-a6, 66(a1)

			// move 40 bytes
		movem.l	66(a0), d2-d7/a2-a5
		movem.l d2-d7/a2-a5, 66(a1)

			// move 36 bytes
		movem.l	66(a0), d2-d7/a2-a4
		movem.l d2-d7/a2-a4, 66(a1)

			// move 32 bytes
		movem.l	66(a0), d2-d7/a2/a3
		movem.l d2-d7/a2/a3, 66(a1)

			// move 28 bytes
		movem.l	66(a0), d2-d7/a2
		movem.l d2-d7/a2, 66(a1)

			// move 24 bytes
		movem.l	66(a0), d2-d7
		movem.l d2-d7, 66(a1)

			// move 20 bytes
		movem.l	66(a0), d2-d6
		movem.l d2-d6, 66(a1)

			// move 16 bytes
		movem.l	66(a0), d2-d5
		movem.l d2-d5, 66(a1)

			// move 12 bytes
		movem.l	66(a0), d2-d4
		movem.l d2-d4, 66(a1)



			// move 44 bytes (no offset)
		movem.l (a0), d2-d7/a2-a6
		movem.l d2-d7/a2-a6, (a1)

			// move 40 bytes
		movem.l	(a0), d2-d7/a2-a5
		movem.l d2-d7/a2-a5, (a1)

			// move 36 bytes
		movem.l	(a0), d2-d7/a2-a4
		movem.l d2-d7/a2-a4, (a1)

			// move 32 bytes
		movem.l	(a0), d2-d7/a2/a3
		movem.l d2-d7/a2/a3, (a1)

			// move 28 bytes
		movem.l	(a0), d2-d7/a2
		movem.l d2-d7/a2, (a1)

			// move 24 bytes
		movem.l	(a0), d2-d7
		movem.l d2-d7, (a1)

			// move 20 bytes
		movem.l	(a0), d2-d6
		movem.l d2-d6, (a1)

			// move 16 bytes
		movem.l	(a0), d2-d5
		movem.l d2-d5, (a1)

			// move 12 bytes
		movem.l	(a0), d2-d4
		movem.l d2-d4, (a1)



		move.l	0x66(a0),0x66(a1)
		move.w	0x66(a0),0x66(a1)
		move.b	0x66(a0),0x66(a1)

		move.l	(a0),(a1)
		move.w	(a0),(a1)
		move.b	(a0),(a1)

		move.l	(a0)+,0x66(a1)
		move.w	(a0)+,0x66(a1)
		move.b	(a0)+,0x66(a1)

		move.l	(a0)+,(a1)+
		move.w	(a0)+,(a1)+
		move.b	(a0)+,(a1)+

		adda.l	d0, a0
		adda.l	d1, a1

		adda.l	a2, a0
		adda.l	a3, a1


		add.l	#9,a1
		addq.l	#8,a1
		lea		6666(a0),a0
		lea		6666(a1),a1
		lea		6(a1),a1

		SW_ASM_END
}

#endif



