/*
	Sign.h
	
	User-entered text for Graphic Elements demo
	
	Copyright 1993 by Al Evans. All rights reserved.
	
	11/11/93
	
*/

#ifdef applec
#ifndef __cplusplus
#ifndef PRELOAD
#pragma load "::ToolKit.precompile"
#define PRELOAD
#endif
#endif
#endif

#ifndef GRAPHELEMENTS
#include "GraphElements.h"
#endif

//Sign position

#define signLeft	300
#define signTop		35

//Sign plane

#define signPlane	4

//Sign ID
#define signID 'SIGN'

#ifdef __cplusplus
extern "C" {
#endif

Boolean LoadSignScene(GEWorldPtr world);

//Sign autochange proc
pascal void DoSign(GEWorldPtr world, GrafElPtr sign);

//Get current sign text into oldText
pascal void GetSignText(StringPtr oldText);

//Set sign text
pascal void SetSignText(GEWorldPtr world, StringPtr newText);

#ifdef __cplusplus
}
#endif
