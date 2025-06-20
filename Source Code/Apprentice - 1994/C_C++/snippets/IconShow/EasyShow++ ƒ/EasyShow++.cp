/* 
    Copyright � General Consulting & Research Inc. 1993-1994 All rights reserved.
  	Author: 	Peter H. Teeson 
    CIS:		72647,3674
    AppleLink:	CDA0197
  	Date:		1 June 1993 
    Path:IconShow:EasyShow++:EasyShow++.cp
   	
  	This project tests IconShow++ and demonstrates how to use it.	
 */


// Assumes inclusion of <MacHeaders++>
#ifdef	__A4_GLOBALS__
#include <SetUpA4.h>
#endif
// ---------- Prototypes ------------ 
void		main(void);
pascal	void	IconShow(short iconId, Boolean advance);
// ----------- Code ----------------- 
void main(void)
{ 
#ifndef __A4_GLOBALS__			// if we are debugging
	FlushEvents(everyEvent,0);
	InitGraf(&thePort);				
	InitFonts();
	InitWindows();	
	InitMenus();
	TEInit();
	InitDialogs(nil);
	InitCursor();
#else						// else we're called from INIT31	
	RememberA0();
	SetUpA4();
#endif
	KeyMap keys;
	GetKeys(keys);			// get keyboard state
	/*
		typedef long KeyMap[4]<->16 byte binary map of key states.
		The 16 keys bytes are in left-to-right order but the bits are
		in right-to-left order in each byte.
	
		The Shift key is 0x38 which is decimal 56 and dividing by 8 
		(56/8=7) means it's bit is in the 7th byte which is the rightmost 
		byte of keys[1] and is the rightmost bit in that byte 
		(56/8=7 remainder 0).
	 */ 
		
	if (!Button() && !(1 & keys[1]))	// Normal case 
		{	long x=30,y=0;				// 1/2 second delay between icon frames
			short loopcount=9;			// ** Change this to alter number displayed
			for (short i=0;i<loopcount;i++)
			  { // Draw an animated version of the "good" icons 
				Delay ( x, &y );
				IconShow(1000,FALSE);	
  				Delay ( x, &y );
				IconShow(1010,FALSE);
  				Delay ( x, &y );
				IconShow(1020,TRUE);
				// Draw all three "frames" separately of the "good" icons
				Delay ( x, &y );
				IconShow(1000,TRUE);	
  				Delay ( x, &y );
				IconShow(1010,TRUE);
  				Delay ( x, &y );
				IconShow(1020,TRUE); }			
		}
	else 		// If the mouse button or shift key is down
		{	IconShow(1030,TRUE);	// Call IconShow(), to show "bad" icon		
		}
#ifndef __A4_GLOBALS__			// if we are debugging
	return;
#else
 	RestoreA4();
#endif
}

