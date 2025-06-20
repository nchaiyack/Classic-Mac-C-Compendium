/*
	GraphElements.h
	
	Basic Graphic Elements for release version 1.0b1
	
	Copyright 1994 by Al Evans. All rights reserved.
	
	3/7/94
*/

#ifndef GRAPHELEMENTS
#define GRAPHELEMENTS

#include "DispCtrl.h"
#include "FastBitCopies.h"

#ifdef __cplusplus
extern "C" {
#endif

//-------------------------------------------------------------------------------------
//Simple PICT-based Graphic Element
//-------------------------------------------------------------------------------------

GrafElPtr NewBasicPICT(GEWorldPtr world, OSType id, short plane, short resNum,
							short mode, short xPos, short yPos);
							
//Render proc for basic PICT graphic
pascal void PICTRenderProc(GrafElPtr element, GWorldPtr destGWorld);

//-------------------------------------------------------------------------------------
//Scrolling Graphic Element
//-------------------------------------------------------------------------------------

typedef struct {
	GrafElement		baseGraphic;
	short			hScroll;			//horiz scroll for each changeIntrvl
	short			vScroll;			//vert scroll for each changeIntrvl	
	short			totalHScroll;		//total current horizontal scroll
	short			totalVScroll;		//total current vertical scroll
} ScrollingGraphic, *ScrlGraphicPtr;

//Creation of PICT-based scrolling graphics
GrafElPtr NewScrollingGraphic(GEWorldPtr world, OSType id, short plane, 
						short resNum, short mode, short xPos, short yPos);

//Start or stop (autoHScroll == autoVScroll == 0) auto-scrolling
void AutoScrollGraphic(GEWorldPtr world, OSType elementID, 
					short interval, short autoHScroll, short autoVScroll);
					
//Manually set current scroll position
void SetScroll(GEWorldPtr world, OSType elementID, short hScroll, short vScroll);

//RenderProc for scrolling graphics
pascal void RenderScrollingGraphic(GrafElPtr graphic, GWorldPtr destGWorld);

//AutoChangeProc for scrolling graphics
pascal void ScrollGraphic(GEWorldPtr world, GrafElPtr graphic);

//-------------------------------------------------------------------------------------
//Animated (frame-sequence) Graphic Element
//-------------------------------------------------------------------------------------

typedef enum {singleframe=0, reciprocating, loop, oneshot} AnimSequence;

typedef struct {
	GrafElement	baseGraphic;
	short			currentFrame;		// number of frame now displayed
	short			nFrames;			// number of frames available
	AnimSequence	seq;				// type of animation
} FrameSeqGraphic, *SeqGraphicPtr;


//Creation of PICT-based multiframe graphics
GrafElPtr NewAnimatedGraphic(GEWorldPtr world, OSType id, short plane, 
					short resNum, short mode, short xPos, short yPos, short nFrames);
					
//Activate or deactivate (interval == 0) automatic animation 
void AnimateGraphic(GEWorldPtr world, OSType elementID, 
					short interval, AnimSequence sequence);

//Manually set current frame					
void SetFrame(GEWorldPtr world, OSType elementID, short newFrame);

//Frame sequence forwards or backwards
void SetAnimDirection(GEWorldPtr world, OSType elementID, Boolean forward);

//Increment or decrement current frame according to frame sequence
void BumpFrame(GEWorldPtr world, OSType elementID);

//Alternate interface to BumpFrame: avoid lookup when GrafElPtr available.
//Also used as AutoChangeProc for frame-based graphics
pascal void PtrBumpFrame(GEWorldPtr world, GrafElPtr graphic);

//Set mirroring -- only horizontal mirroring presently supported
//NOTE!! Only works for transparent FrameSeqGraphics using default BitCopyProc
//and srcCopy FrameSeqGraphics which have been masked with MakeMask() (see FastBitCopies.h)
void SetMirroring(GEWorldPtr world, OSType elementID, Boolean mirrorH, Boolean mirrorV);

//RenderProc for frame-changing graphics
pascal void RenderFrameGraphic(GrafElPtr graphic, GWorldPtr destGWorld);


//-------------------------------------------------------------------------------------
//Simple one-line text graphic
//-------------------------------------------------------------------------------------

typedef struct {
	GrafElement		baseGraphic;
	StringPtr		tgText;				//pointer to text
	FontInfo		tgFInfo;			//FontInfo rec for text
	short			tgFontNum;			//etc.
	short			tgFontSize;
	short			tgFontFace;
	RGBColor		tgColor;			//Color for text
} TextGraphic, *TextGraphicPtr;

//Create new text graphic
GrafElPtr NewTextGraphic(GEWorldPtr world, OSType id, short plane,
					short xPos, short yPos, short mode,
					short fontNum, short txStyle, short size, 
					RGBColor color, StringPtr text);

//Change text of existing text graphic
void SetTextGraphicText(GEWorldPtr world, OSType elementID, StringPtr newText);

//RenderProc for text graphics
pascal void RenderTextGraphic(GrafElPtr graphic, GWorldPtr destGWorld);

//-------------------------------------------------------------------------------------
//This Graphic Element tiles a single PICT into its (larger) animationRect
//-------------------------------------------------------------------------------------

GrafElPtr NewTiledGraphic(GEWorldPtr world, OSType id, short plane, 
						short resNum, short mode, Rect destRect);
						
//RenderProc for tiled graphics
pascal void RenderTiledGraphic(GrafElPtr graphic, GWorldPtr destGWorld);

#ifdef __cplusplus
}
#endif


#endif