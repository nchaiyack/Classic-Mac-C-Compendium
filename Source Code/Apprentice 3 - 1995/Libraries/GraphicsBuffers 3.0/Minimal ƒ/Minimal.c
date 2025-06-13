/*
	Minimal.c
	by Hiep Dam
	June 1995
	
	This is a simple program intended to show the simplest usage of
	GraphicsBuffers. It allocates some 8-bit GraphicsBuffers, and
	uses the 8-bit blitters.
*/

#include "GraphicsBuffers.h"

// ---------------------------------------------------------------------------

void InitToolBox();

// ---------------------------------------------------------------------------

void main() {
	WindowPtr mainWindow;
	PicHandle theSprite;
	GraphicsBufferPtr bkgndBuffer, dockBuffer, spriteBuffer, maskBuffer, outputBuffer;
	GWorldPtr saveWorld;
	GDHandle saveDev;
	Rect windowBounds, bufferBounds, spriteBufferBounds;

	InitToolBox();
	(void)InitGraphicsBuffers();

	/* This demo will only work for 8-bit depths */
	if ((**(**GetMainDevice()).gdPMap).pixelSize != 8) {
		SysBeep(10);
		return;
	}
	
	/* Make the window */
	SetRect(&bufferBounds, 0, 0, 200, 200);
	windowBounds = bufferBounds;
	OffsetRect(&windowBounds, 40, 40);
	mainWindow = NewCWindow(NULL, &windowBounds, "\pMinimal", true, documentProc,
		(WindowPtr)-1L, false, 0);
	SetPort(mainWindow);
	
	/* Fill in the GraphicsBuffers. If bkgndBuffer was a GraphicsBuffer,
	   we would pass "&bkgndBuffer"; however since bkgndBuffer is a
	   GraphicsBufferPtr (GraphicsBuffer *), we'll just pass "bkgndBuffer" */
	(void)NewGraphicsBuffer(&bkgndBuffer, 0, &bufferBounds, noNewDevice, true);
	(void)NewGraphicsBuffer(&dockBuffer, 0, &bufferBounds, noNewDevice, true);
	
	/* The sprite is only 32x32 pixels. So we'll make the sprite buffer this size */
	SetRect(&spriteBufferBounds, 0, 0, 32, 32);
	(void)NewGraphicsBuffer(&spriteBuffer, 0, &spriteBufferBounds, noNewDevice, true);
	/* We'll be using the deep mask blitter, so setup a deep graphics buffer mask */
	/* If we're to use CopyMask instead, pass 1 as depth and 0 as flags */
	(void)NewGraphicsBuffer(&maskBuffer, 0, &spriteBufferBounds, noNewDevice, true);
	
	/* Make output buffer. Note: it requires a COLOR window */
	(void)Convert2GraphicsBuffer(&outputBuffer, mainWindow, &windowBounds, true);


	/* Fill in the buffers with the appropriate images/sprites */
	GetGWorld(&saveWorld, &saveDev);
	/* Do the sprite first */
	theSprite = GetPicture(128);
	SetGraphicsBuffer(spriteBuffer);
	DrawPicture(theSprite, &(**theSprite).picFrame);
	ReleaseResource((Handle)theSprite);
	/* Now do the sprite mask */
	theSprite = GetPicture(129);
	SetGraphicsBuffer(maskBuffer);
	DrawPicture(theSprite, &(**theSprite).picFrame);
	/* Since this is a deep mask blitter, we'll have to invert the mask */
	/* You can do this beforehand when creating the mask, of course */
	InvertRect(&spriteBufferBounds);
	ReleaseResource((Handle)theSprite);
	
	/* Do the bkgnd buffer */
	SetGraphicsBuffer(bkgndBuffer);
	ForeColor(blueColor);
	FillRect(&bufferBounds, &qd.black);
	/* Do the dock buffer */
	CopyGraphicsBuffer(bkgndBuffer, dockBuffer, &bufferBounds, &bufferBounds);
	
	SetGWorld(saveWorld, saveDev);
	
	/* Copy bkgnd to output (i.e. window) */
	CopyGraphicsBuffer(bkgndBuffer, outputBuffer, &bufferBounds, &bufferBounds);
	
	while (!Button()) {} /* Wait for mouse to start */

	{
		Rect oldSpriteRect, currentSpriteRect, unionSpriteRect;
		Rect spriteSourceRect;
		long i, dummy;
		
		SetRect(&oldSpriteRect, 0, 0, 32, 32);
		spriteSourceRect = oldSpriteRect;
		currentSpriteRect = oldSpriteRect;
		unionSpriteRect = oldSpriteRect;
		
		HideCursor();
		for (i = 0; i < 80; i++) {
			oldSpriteRect = currentSpriteRect;
			OffsetRect(&currentSpriteRect, 2, 2);
			UnionRect(&oldSpriteRect, &currentSpriteRect, &unionSpriteRect);

			/* Erase old sprite image by copying over it with bkgnd */
			BlitGraphicsBuffer_8bit(bkgndBuffer, dockBuffer, &oldSpriteRect,
				&oldSpriteRect);

			/* Draw sprite at new location */
			BlitGraphicsBuffer_Mask8bit(spriteBuffer, dockBuffer,
				&spriteSourceRect, &currentSpriteRect, maskBuffer);

			/* Update to screen; we're blitting directly to the
			   screen. By doing this we'll have to hide the cursor.
			   This is the only time we have to hide the cursor; if
			   we just use copybits at this step no cursor hiding is
			   necessary. */
			BlitGraphicsBuffer_8bit(dockBuffer, outputBuffer,
				&unionSpriteRect, &unionSpriteRect);
				
			/* This may be really fast, so slow it down */
			Delay(1, &dummy);
		}
		ShowCursor();
	}
	
	while (!Button()) {} /* Wait for click to end demo */
	FlushEvents(everyEvent, 0);
} // END main

// ---------------------------------------------------------------------------

void InitToolBox() {
	InitGraf((Ptr) &qd.thePort);
	InitFonts();
	InitWindows();
	InitMenus();
	TEInit();
	InitDialogs(0L);
	InitCursor();
} // END InitToolBox