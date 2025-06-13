/*
** File:		Blit.h
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright © 1995 Nikol Software
** All rights reserved.
**
** Based on 'BlitCmp' code from Apple Computer, available on the develop 18 CD.
*/


void InitBlit(PixMapPtr srcPM, PixMapPtr dstPM, Rect *srcRect, Rect *dstRect);

pascal void CopyBlit(PixMapPtr srcPM, PixMapPtr dstPM, Rect *srcRect, Rect *dstRect);
