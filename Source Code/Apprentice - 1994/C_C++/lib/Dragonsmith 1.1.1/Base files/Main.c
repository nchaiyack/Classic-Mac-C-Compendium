/*
	Main.c
	
	Main file for Dragon projects
	
	Copyright � 1992 by Paul M. Hoffman
	Send comments or suggestions to paul.hoffman@umich.edu -or- dragonsmith@umich.edu
	
	This source code may be freely used, altered, and distributed in any way as long as:
		1.	It is GIVEN away rather than sold (except as expressly permitted by the author)
		2.	This statement and the above copyright notice are left intact.
	
	Created	24 Apr 1992	Extracted from Dragon.c
	Modified	01 May 1992	No longer calls Test � Dragon::Run will do so when debugging
			24 Aug 1992	We now correctly delete gDragon at the end (to make sure that its destructor method(s) get called!)
	
*/

#include	"Dragon.h"

Dragon	*gDragon;		// This is it � the global dragon!

void main (void);
Dragon *CreateGDragon (void);

void main (void)
{
	gDragon = CreateGDragon ();
	if (gDragon != NULL) {
		gDragon->Start ();
		gDragon->Run ();
		gDragon->Finish ();
	}
	delete gDragon;
}

