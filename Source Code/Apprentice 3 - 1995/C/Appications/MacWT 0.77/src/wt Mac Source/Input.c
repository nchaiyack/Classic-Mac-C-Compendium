/*
** File:		Input.c
**
** Written by:	Bill Hayden
**				Nikol Software
**
** Copyright � 1995 Nikol Software
** All rights reserved.
*/


#include "MacWT.h"
#include "input.h"
#include <string.h>
#include <math.h>

static Intent gIntent;



void init_input_devices(void)
{
	memset(&gIntent, 0, sizeof(gIntent));
}




void end_input_devices(void)
{
	// nada for Mac
}




static void add_special(Intent *intent, int special)
{
	if (intent->n_special < MAX_SPECIAL_INTENTIONS)
		{
		intent->special[intent->n_special] = special;
		intent->n_special++;
		}
}





Intent *read_input_devices(void)
{
	unsigned char kmp[16];
	Boolean	rotating_cw,
			rotating_ccw,
			moving_forward,
			moving_backward,
			running,
			strafing;
	struct {
		short	rightArrow,
				leftArrow,
				upArrow,
				downArrow,
				control,
				escape,
				shift,
				space,
				tab,
				w,
				e,
				r,
				t,
				y;
	} keyboard;


	gIntent.force_x = gIntent.force_y = gIntent.force_z = 0.0;
	gIntent.force_rotate = 0.0;
	gIntent.n_special = 0;

	MacAttractMode();

	do {
		if (gKeyboardControl)
			GetAndProcessEvent();

		GetKeys((long *)kmp);

		if (gPaused)
			{
			gStartTicks = gFrameCount = 0;
			init_input_devices();		// zero out the "intent" structure...
			}

		if (!gGameOn)
			{
			// if the forward key is pressed, start the game immediately!
			if (MacKeyDown(kmp, 0x7e) || MacKeyDown(kmp, 0x5b))
				BeginGame();
			}

	} while (!gGameOn || (gPaused && !quitting));


	keyboard.rightArrow = MacKeyDown(kmp, 0x7C) || MacKeyDown(kmp, 0x58);
	keyboard.leftArrow = MacKeyDown(kmp, 0x7b) || MacKeyDown(kmp, 0x56);
	keyboard.upArrow = MacKeyDown(kmp, 0x7e) || MacKeyDown(kmp, 0x5b);
	keyboard.downArrow = MacKeyDown(kmp, 0x7d) || MacKeyDown(kmp, 0x54);
	keyboard.control = MacKeyDown(kmp, 0x3b) || MacKeyDown(kmp, 0x37);	// Ctrl or Cmd
	keyboard.shift = MacKeyDown(kmp, 0x38) || MacKeyDown(kmp, 0x3c);
	keyboard.space = MacKeyDown(kmp, 0x31);
	keyboard.tab = MacKeyDown(kmp, 0x30);
	keyboard.escape = MacKeyDown(kmp, 0x35) || MacKeyDown(kmp, 0x32);
	keyboard.w = MacKeyDown(kmp, 0x0d);
	keyboard.e = MacKeyDown(kmp, 0x0e);
	keyboard.r = MacKeyDown(kmp, 0x0f);
	keyboard.t = MacKeyDown(kmp, 0x11);
	keyboard.y = MacKeyDown(kmp, 0x10);


	// logic snarfed from "x11input.c" & "linux-mouse.c"

	rotating_ccw = keyboard.leftArrow;
	rotating_cw = keyboard.rightArrow;
	moving_forward = keyboard.upArrow;
	moving_backward = keyboard.downArrow;
	running = keyboard.shift;
	strafing = keyboard.control;

	if (keyboard.space)
		add_special(&gIntent, INTENT_JUMP);

	if (keyboard.w)
		add_special(&gIntent, INTENT_ACTION1);
		
	if (keyboard.e)
		add_special(&gIntent, INTENT_ACTION2);

	if (keyboard.r)
		add_special(&gIntent, INTENT_ACTION3);
		
	if (keyboard.t)
		add_special(&gIntent, INTENT_ACTION4);

	if (keyboard.y)
		{
		gKeyboardControl = !gKeyboardControl;
		if (gKeyboardControl)
			{
			FlushEvents(everyEvent, 0);	// Get rid of all those mouse clicks we might have done
			ShowCursor();
			}
		else
			HideCursor();
		}
		
	if (keyboard.escape || quitting)
		add_special(&gIntent, INTENT_END_GAME);

	if (gKeyboardControl)
		{
		if (rotating_cw)
			{
			if (strafing)
				gIntent.force_y -= MOVE_FORCE;
			else
				gIntent.force_rotate -= TURN_FORCE;
			}
	
		if (rotating_ccw)
			{
			if (strafing)
				gIntent.force_y += MOVE_FORCE;
			else
				gIntent.force_rotate += TURN_FORCE;
			}
		
		if (moving_forward)
			gIntent.force_x += MOVE_FORCE;
		
		if (moving_backward)
			gIntent.force_x -= MOVE_FORCE;
		}
	else
		{
		Point	Pt;


		GetMouse(&Pt);
		LocalToGlobal(&Pt);

		if (Button())
			add_special(&gIntent, INTENT_JUMP);

		if (strafing)
			gIntent.force_y += ((double) (qd.screenBits.bounds.right - Pt.h) - (double) (qd.screenBits.bounds.right >> 1)) / 8;
		else
			gIntent.force_rotate += ((double) (qd.screenBits.bounds.right - Pt.h) - (double) (qd.screenBits.bounds.right >> 1)) / 32;

		gIntent.force_x += ((double) (qd.screenBits.bounds.bottom - Pt.v) - (double) (qd.screenBits.bounds.bottom >> 1)) / 8;
     
		if (fabs(gIntent.force_x) < 0.3)
			gIntent.force_x = 0.0;
		if (fabs(gIntent.force_y) < 0.3)
			gIntent.force_y = 0.0;
		if (fabs(gIntent.force_rotate) < 0.2)
			gIntent.force_rotate = 0.0;
		}
		
	if (running)
		{
		gIntent.force_x *= 2.0;
		gIntent.force_y *= 2.0;
		gIntent.force_z *= 2.0;
		gIntent.force_rotate *= 2.0;
		}

	return &gIntent;
}



