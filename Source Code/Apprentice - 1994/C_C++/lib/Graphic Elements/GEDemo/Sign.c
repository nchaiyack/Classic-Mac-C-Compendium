/*
	Sign.c
	
	User-entered text for Graphic Elements demo
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/11/93
	
*/

#include "Sign.h"

Str255	signText = "\pMessage Here!";
typedef struct {
			Boolean signOn;
			RGBColor color[2];
} BlinkRec, *BlinkRecPtr;

BlinkRec	signBlink;

Boolean LoadSignScene(GEWorldPtr world)
{
	GrafElPtr		thisElement;
	short			fontNum;
	
	GetFNum("\pChicago", &fontNum);
	
	signBlink.signOn = false;
	signBlink.color[0].red = 194 << 8;
	signBlink.color[0].green = 194 << 8;
	signBlink.color[0].blue = 0;
	signBlink.color[1].red = 240 << 8;
	signBlink.color[1].green = 240 << 8;
	signBlink.color[1].blue = 46 << 8;
	
	//Create sign
	thisElement = NewTextGraphic(world, signID, signPlane, signLeft, signTop, srcOr,
							fontNum, bold, 18, signBlink.color[signBlink.signOn], signText);
	if (!thisElement) return false;
	
	//Initialize blinking action
	SetAutoChange(world, signID, DoSign, (Ptr) &signBlink, 750);
	
	return true;
					
}

pascal void DoSign(GEWorldPtr world, GrafElPtr sign)
{
#pragma unused (world)
	BlinkRecPtr	blink;
	
	blink = (BlinkRecPtr) sign->changeData;
	
	blink->signOn = !blink->signOn;
	((TextGraphicPtr) sign)->tgColor = blink->color[blink->signOn];
	ChangedRect(world, &sign->animationRect);
}

pascal void GetSignText(StringPtr oldText)
{
	BlockMove((Ptr) signText, (Ptr) oldText, (long) *signText + 1);
}

//Change sign text
pascal void SetSignText(GEWorldPtr world, StringPtr newText)
{
	BlockMove((Ptr) newText, (Ptr) signText, (long) *newText + 1);
	SetTextGraphicText(world, signID, signText);
}

