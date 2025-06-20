#ifndef GRAPHICSBUFFERS_H_
	#include "GraphicsBuffers.h"
#endif

typedef struct {
	Rect offBounds;
	Rect curRect;
	Rect oldRect;
	Rect unionRect;
	
	GraphicsBufferPtr buffer;
	GraphicsBufferPtr maskBuffer;
	RgnHandle region;
	Ptr mask;
	
	short hVel;
	short vVel;
} BasicSprite;

/* Some globals */

typedef struct {
	BasicSprite masterSprite;

	GraphicsBufferPtr outputBuffer;
	GraphicsBufferPtr bkgndBuffer;
	GraphicsBufferPtr dockBuffer;
	GraphicsBufferPtr spriteBuffer;
	GraphicsBufferPtr spriteMaskBuffer;
	GraphicsBufferPtr spriteMaskDeepBuffer;

	ImageEraser eraser;
	ImageDrawer drawer;
	ImageRefresher refresher;

	short bufferDepth;
	short origMonitorDepth;
	short animationMethod;

	short numSpritesAvail;
	short curSprite;
	short numSpritesInDemo;
	short useBkgndSound;
	short _notUsed;

	DialogPtr testDlog;
	WindowPtr demoWindow;
	GDHandle primaryDevice;

	Rect monitorBounds;
	Rect windowBounds;
	Rect bufferBounds;
	Rect spriteBufferBounds;
	Rect curSpriteBox;
} TestBed;

extern TestBed gTestBed;