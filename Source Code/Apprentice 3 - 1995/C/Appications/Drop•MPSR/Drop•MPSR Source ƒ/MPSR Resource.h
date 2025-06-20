/*
	MPSR Resource.h
	
	Describes the MPSR resource.
	
		From: rollin@newton.apple.com (Keith Rollin)
		Subject: Re: MPSR Resource format?
		
		In article <phixusCvwM0H.Lus@netcom.com>, phixus@netcom.com (Chris
		DeSalvo) wrote:
		
		>Anyone know the format of the MPSR resource that is created by MPW and
		>other products?
		
		When reading this, keep in mind that it's from MPW code, which use 4-byte
		integers.

		----------------------------------------------------------------------------
		Keith Rollin --- Phantom Programmer --- Apple Computer, Inc. --- Team Newton
*/

#pragma once

#ifndef __H_MPSR_Resource__
#define __H_MPSR_Resource__

#define editResType		'MPSR'
#define markResType	'MPSR'

// special resource ids
#define editResID		1005
#define markResID		1007

// Edit resource structures
typedef struct FONTRSRC{
	short size;					// font size
	char name[32];					// font name
} FontRSRC;

typedef struct TABRSRC{
	short width;					// width of space in points
	short count;					// tab width in spaces
} TabRSRC;

typedef struct WINDRSRC{
	Rect openrect;					// rectangle of window zoomed out
	Rect closerect;					// rectangle of window zoomed in
} WindRSRC;

typedef struct SELRSRC{
	unsigned long date;				// time of last edit
	long start;					// offset to start of selection
	long end;						// offset to end of selection
	long pos;						// offset to first char in window
} SelRSRC;

typedef struct EDITRSRC{
	FontRSRC fontrsrc;
	TabRSRC tabrsrc;
	WindRSRC windrsrc;
	SelRSRC selrsrc;
	char autoindent;				// true if auto indent is on
	char invisibles;					// true if show invisibles is on
} EditRSRC,* EditRSRCPtr,** EditRSRCHdl;

// Mark resource structures
typedef struct MarkElement {
	long start;					// start position of mark
	long end;						// end position
	unsigned char charCount;			// number of chars in mark name
	char name[1];					// first char of mark name
} MarkElement;								// (name is null terminated)

typedef struct MARKRSRC {
	short count;					// number of marks in the resource
	MarkElement mark[1];			// zero or more mark elements
} MarkRSRC;

#endif
