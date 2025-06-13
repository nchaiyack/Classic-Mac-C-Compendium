// PlayRiffs 1.0.1
// by Ken Long (kenlong@netcom.com)
// updated for CodeWarrior 6 on 950712

#include "Sound.h"

#define over qd.screenBits.bounds.right
#define down qd.screenBits.bounds.bottom

PicHandle		aboutPict, pattyPict, bluePict;
PixPatHandle	Miss_Patty;

short riff;

Handle			riff_1, riff_2, riff_3, riff_4, riff_5, 
				riff_6, riff_7, riff_8, riff_9;
				
SndChannelPtr	chanPtr = 0L, riff_ptr = 0L, channelPtr;
SndCommand		call_back, flush_out, quiet_down, buffer_it;
Boolean 		Jimi_Hendrix = false, hotLicks = false, 
				quiet = false, tunes = true;

char			var_1, var_2;

WindowPtr		shell_window, splash_window;

RgnHandle		mBarRgn, GrayRgn;
short			*mBarHeightPtr;
short			twenty_pixels;
short 			riffRes;
Rect 			splashBounds, pictRect;
long 			ticks;

//¥ Prototypes:
//¥ Set up...
void Hide_Menu_Bar (void);
void Show_Menu_Bar (void);
void Do_The_Splash (void);
void Create_Our_Screen (void);
Boolean We_Be_Hued (void);
void Wake_The_Managers (void);
void Bag_Some_Riffs (void);
void Setup_Snd_Commands (void);

//¥ ...and run.
void Do_The_Sound (short whichID, Boolean asynch);
void Stop_The_Riffs (void);
void Jam_Session (void);
pascal void Auto_Redial (SndChannelPtr chan, SndCommand order);
void Play_The_Riffs (void);
void Main_Event_Loop (void);
void Handle_Key_Hits (EventRecord *theEvent);
void Put_Things_Back (void);
void main (void);

//¥ --------------------------------------------------------------- ¥//
//¥ ----------------- S E T U P - S E C T I O N ------------------- ¥//
//¥ --------------------------------------------------------------- ¥//

void Hide_Menu_Bar (void) 
{
	Rect	mBarRect;

	GrayRgn = GetGrayRgn ();
	mBarHeightPtr = (short *)  0x0BAA;
	twenty_pixels = *mBarHeightPtr;
	*mBarHeightPtr = 0;
	mBarRect = qd.screenBits.bounds;
	mBarRect.bottom = mBarRect.top + twenty_pixels;
	mBarRgn = NewRgn ();
	RectRgn (mBarRgn, &mBarRect);
	UnionRgn (GrayRgn, mBarRgn, GrayRgn);
	PaintOne (0L, mBarRgn);
}

void Show_Menu_Bar (void) 
{
	*mBarHeightPtr = twenty_pixels;
	DiffRgn (GrayRgn, mBarRgn, GrayRgn);
	DisposeRgn (mBarRgn);
}

void Do_The_Splash ()
{
	
	//¥ A centered, 300w, 250h window.
	SetRect (&splashBounds, over / 2 - 150, 
						    down / 2 - 125, 
						    over / 2 + 150,
						    down / 2 + 125);

	//¥ "C" window so our 'ppat" will draw.
	splash_window = NewCWindow (0L, 			//¥ No storage - LIVE!
							   &splashBounds, 	//¥ As delineated above.
							   "\p", 			//¥ No place for one.
							   true, 			//¥ Yes, it's visible
							   dBoxProc, 		//¥ For dialog border.
							  (WindowPtr) -1L, 	//¥ In front.
							   false, 			//¥ No go away.
							   0);				//¥ Reference value 0.
							  
	SetPort(splash_window);

	//¥ splash_window is our set port, so we go off local coords.
	SetRect (&pictRect, 0, 0, 300, 249);
	
	aboutPict = GetPicture (128);
	
	DrawPicture (aboutPict, &pictRect);
	Delay (360, &ticks);
	HideWindow (splash_window);
}

void Create_Our_Screen ()
{
	Rect windowBounds, blueRect, sisterRect;
	
	windowBounds = qd.screenBits.bounds;
	
	shell_window = NewCWindow(0L, &windowBounds, "\p", true, plainDBox, (WindowPtr) -1L, true, 0);
	SetPort(shell_window);

	Miss_Patty = GetPixPat (128);

	FillCRect (&windowBounds, Miss_Patty);

	SetRect (&blueRect, 73, 217, 73 + 38, 217 + 55);
	bluePict = GetPicture (130);
	DrawPicture (bluePict, &blueRect);
}

Boolean We_Be_Hued ()
{
	SysEnvRec whichMac;
	SysEnvirons (2, &whichMac);
	return (whichMac.hasColorQD);
}

void Wake_The_Managers ()
{
	MaxApplZone ();
	InitGraf (&qd.thePort);
	InitFonts ();
	FlushEvents (everyEvent, 0);
	InitWindows ();
	InitMenus ();
	TEInit ();
	InitDialogs (0L);
	InitCursor ();
	
	MoreMasters ();
	MoreMasters ();
	MoreMasters ();
}

void Bag_Some_Riffs ()
{		
// 	riffRes = OpenResFile("\pRiffs");   // Tired this - stack ate heap.
										// It needs more work.
// 	if (riffRes == 0L)
// 		ExitToShell ();
	
	//¥ We're gathering by name.
	riff_1 = GetNamedResource ('snd ', "\pRiff_01");
	riff_2 = GetNamedResource ('snd ', "\pRiff_02");
	riff_3 = GetNamedResource ('snd ', "\pRiff_03");
	riff_4 = GetNamedResource ('snd ', "\pRiff_04");
	riff_5 = GetNamedResource ('snd ', "\pRiff_05");
	riff_6 = GetNamedResource ('snd ', "\pRiff_06");
	riff_7 = GetNamedResource ('snd ', "\pRiff_07");
	riff_8 = GetNamedResource ('snd ', "\pRiff_08");
	riff_9 = GetNamedResource ('snd ', "\pRiff_09");
}

void Setup_Snd_Commands ()
{
	chanPtr = (SndChannelPtr) NewPtrClear (sizeof (SndChannel));
	if (chanPtr != 0L)
	{
		chanPtr->qLength = stdQLength;
	}
	riff_ptr = 0L;
	call_back.cmd = callBackCmd;
	call_back.param1 = 0;
	call_back.param2 = SetCurrentA5 ();
	
	flush_out.cmd = flushCmd;
	flush_out.param1 = 0;
	flush_out.param2 = 0L;
	
	quiet_down.cmd = quietCmd;
	quiet_down.param1 = 0;
	quiet_down.param2 = 0L;
	
	buffer_it.cmd = bufferCmd;
	buffer_it.param1 = 0;
	buffer_it.param2 = 0L;
}

//¥ --------------------------------------------------------------- ¥//
//¥ -------------------- R U N - S E C T I O N -------------------- ¥//
//¥ --------------------------------------------------------------- ¥//

//¥ John Calhoun's sound player, from Stella Obscura (again).
void Do_The_Sound (short whichID, Boolean asynch)
{
	Handle theSnd;
	OSErr err;
	Boolean soundActive;
	
	soundActive = true;	
	
	if ((soundActive))
	{
		theSnd = GetResource ('snd ', whichID);
		
		if ((theSnd != 0L) && (ResError () == noErr))
		{
			if ((channelPtr != 0L))
			{
				err = SndDisposeChannel (channelPtr, true);
				channelPtr = 0L;
			}
			if ((asynch == true) && 
				(SndNewChannel 
				(&channelPtr, 0, initMono, 0L) == noErr)) 
				err = SndPlay(channelPtr, (SndListHandle)theSnd, true); 
			else	
				err = SndPlay(0L, (SndListHandle)theSnd, false);
		}
	}
}

//¥ This gets the ball rolling.  Auto_Redial calls Play_The_Riffs,
//¥ as a loop (calling it back) and the ID number is switched there.
void Jam_Session ()
{
	Boolean formost = true;
	
	if (! tunes)
		return;
		
	if (Jimi_Hendrix)
		return;
	
	//¥ If it's number one, it's a new sound channel.  Otherwise,
	//¥ we just use the one we have.
	if (formost)
		SndNewChannel (&riff_ptr, 0, 0, 
					  (SndCallBackProcPtr) &Auto_Redial);

	//¥ This assignment makes "riff" be riff_01, or ID #9001.
	riff = 1;
	Jimi_Hendrix = true;
	formost = false;
	
	SndPlay (riff_ptr, (SndListHandle)riff_1, true);
	SndDoCommand (riff_ptr, &call_back, true);
}

//¥ Commonly used call back routine.
pascal void Auto_Redial (SndChannelPtr chan, SndCommand order)
{
	long saveA5;
	saveA5 = SetCurrentA5 ();
	SetA5 (order.param2);
	Play_The_Riffs ();
	SetA5 (saveA5);
}

//¥ This is called by Auto_Redial, which is called by SndNewChannel
//¥ (in Jam_Session).
//¥ It starts off playing Riff_01 (ID 9001), 
void Play_The_Riffs ()
{
	Handle which_riff;
	
	//¥ Just a couple toggles to put in more control if needed.
	if (! hotLicks || ! tunes)
		return;	
			
	//¥ We are already playing riff_1 coming into this routine.
	//¥ Format 1 snd resources, please.
	switch (riff)
	{
		//¥ Plays 9002 twice, then bumps up the series value.
		case 1:
		case 2:
			riff++;
			which_riff = riff_2;
		break;

		case 3:
			riff++;
			which_riff = riff_3;
		break;

		case 4:
			riff++;
			which_riff = riff_4;
		break;

		case 5:
		case 6:
			riff++;
			which_riff = riff_2;
		break;

		case 7:
		case 8:
			riff++;
			which_riff = riff_5;
		break;

		case 9:
			riff++;
			which_riff = riff_6;
		break;

		case 10:
			riff++;
			which_riff = riff_7;
		break;

		case 11:
			riff++;
			which_riff = riff_8;
		break;

		//¥ History repeats itself!
		case 12:
		case 13:
			riff++;
			which_riff = riff_2;
		break;

		case 14:
			riff++;
			which_riff = riff_5;
		break;

		case 15:
			riff++;
			which_riff = riff_6;
		break;

		case 16:
			riff = 1;
			which_riff = riff_9;
		break;
	}
	
 	var_1 = *((Ptr) (((long) *which_riff) + 3));
 	var_2 = *((Ptr)  ((long) *which_riff) + 10 * var_1 + 1);
 	
 	buffer_it.param2 = ((long) *which_riff) + 10 * var_1 + 10 * var_2;

	SndDoCommand (riff_ptr, &buffer_it, true);
	SndDoCommand (riff_ptr, &call_back, true);
}

void Stop_The_Riffs ()
{
	//¥ Don't stop it if it's already stopped.
	if (! Jimi_Hendrix)
		return;
		
	//¥ Boolean meets Jimi, in a negative context.
	Jimi_Hendrix = false;
	
	//¥ Plays a "sound of silence."
	SndDoImmediate (riff_ptr, &quiet_down);
	
	//¥ You know what happens when you flush!
	SndDoCommand (riff_ptr, &flush_out, false);
}

void Main_Event_Loop ()
{
	short			ok;
	EventRecord		theEvent;
	short 			key;
	Rect 			pattyRect;
	
	HiliteMenu (0);		//¥ What menu?
	WaitNextEvent (everyEvent, &theEvent, 0L, 0L);

	//¥ Puts in "Patty with the blue dress, blue dress on..."
	SetRect (&pattyRect, 109, 132, 109 + 97, 132 +72);
	pattyPict = GetPicture (129);

	hotLicks = true;	
	Jam_Session ();
	
	switch (theEvent.what)
	{
		//¥ We do this, instead of a mouse handling routine.
		case mouseDown:
			DrawPicture (pattyPict, &pattyRect);
			Do_The_Sound (10111, true);
		break;
		
		//¥ More bull.
		case mouseUp:
			Do_The_Sound (10110, true);
			FillCRect (&pattyRect, Miss_Patty);
		break;
		
		case keyDown:
		case autoKey:
			Handle_Key_Hits (&theEvent);
		break;
	
		case keyUp:
		break;
	
		//¥ We don't use this because we hog the whole screen.
		case updateEvt:
			//¥ Where'd this come from?
//			BeginUpdate (shell_window);
//			DrawBullseye (((WindowPeek) shell_window)->hilited);
//			EndUpdate (shell_window);
		break;
	
		//¥ Same here.
		case activateEvt:
		break;
	}
}

//¥ Key hits ala "CheeseToast" and 100 others.
void Handle_Key_Hits (EventRecord *theEvent)
{
	Rect aboutBounds;
	short	chCode;
	long ticks;
	
	chCode = theEvent->message & charCodeMask;

	switch (chCode) 
	{
		case '1':  //¥ User hits the 1 key.
		   	Do_The_Sound (10001, true);
		break;

		case '2':
		   	Do_The_Sound (10002, true);
		break;

		case '3':
		   	Do_The_Sound (10003, true);
		break;

		case '4':
		   	Do_The_Sound (10004, true);
		break;

		case '5':
		   	Do_The_Sound (10005, true);
		break;

		case '6':
		   	Do_The_Sound (10006, true);
		break;

		case '7':
		   	Do_The_Sound (10007, true);
		break;

		case '8':
		   	Do_The_Sound (10008, true);
		break;

		case '9':
		   	Do_The_Sound (10009, true);
		break;

		case '0':
		   	Do_The_Sound (10009, true);
		break;

		case '*':
			quiet = true;		//¥ Give the quiet command.
			Stop_The_Riffs ();
		break;

		case 'q':
			Put_Things_Back ();
		break;
	}
}

void Put_Things_Back ()
{
	//¥ Get rid of the key and mouse hit sound channel.
	if (chanPtr != 0L)
	{
		SndDoImmediate (chanPtr, &flush_out); 	//¥ flush.
		SndDisposeChannel (chanPtr, true);
	}
	//¥ Get rid of the riff sound channel.
	if (riff_ptr != 0L)
	{
		SndDoImmediate (riff_ptr, &flush_out); 	//¥ flush.
		SndDisposeChannel (riff_ptr, true);
	}
	//¥ Back to the bar (for drinks and socializing).
	Show_Menu_Bar ();
	ExitToShell ();		//¥ Back to Findersville.
}

void main ()
{
	Rect r1;
	short i;
	SysEnvRec whichMac;
	
	Wake_The_Managers ();
		
	SetEventMask (everyEvent);

	Bag_Some_Riffs ();
	Do_The_Splash ();

	//¥ We do this here because we still look like a desktop.
	Hide_Menu_Bar ();

	SysEnvirons (2, &whichMac);
	
	//¥ We don't mess with no antiques!
	if (whichMac.processor == env68000) 
	{
		tunes = false;
		ExitToShell ();
	}
	Create_Our_Screen ();
	Setup_Snd_Commands ();
	
	for (;;)
		Main_Event_Loop ();
//	CloseResFile(riffRes);  //¥ Remember?
}

