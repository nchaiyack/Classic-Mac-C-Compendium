/*
 *  KeMoTest.c
 *
 *  Copyright (c) 1992,1993 Dan Costin.  All rights reserved.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include "KeMo.h"

void AccuracyTest(void);
void ResponseTest(void);
void TimerTest(void);
void ScreenTest(void);
void KeyCodesTest(void);
void ShowHideMBar(void); 

/* Main menu */
main()
{
	char ans[20];
	long err;

	printf("Welcome to KeMo's test suite!\n");

	KeMoInit(KeMoQuiet + KeMoAltKeys);	/* no beeps on errors (but do show alerts) */
										/* also, set keyboard to differentiate between
											left and right modifier keys */
	
	while(1) {
		printf("\nThe following tests are available:\n");
		printf("    [A]ccuracy        - measures keyboard and mouse detection accuracy\n");
		printf("                        (uses KeMoAccuracy)\n");
		printf("    [E]nd Programs    - end all other running programs (uses KeMoQuitAllApps)\n");
		printf("    [K]eys            - reports symbol for keys pressed (uses KeMoCode2Asc)\n");
		printf("    [M]enus           - show/hide menu bar\n");
		printf("    [R]esponses       - checks detection of up and down calls to KeMoWait\n");
		printf("    [S]creen Refresh  - calls KeMoSynch on every screen refresh for 20 seconds\n");
		printf("                        and reports what should be the monitor's refresh rate\n");
		printf("    [T]imer           - checks the accuracy of the timer against the Mac clock\n");
		printf("    [Q]uit\n");
		printf("\nPlease select one of the above: ");
		fflush(stdout);
		fflush(stdin);
		
		fgets(ans, 10, stdin);
		
		switch (toupper(*ans)) {
			case 'A': AccuracyTest(); break;
			case 'E': KeMoQuitAllApps(); break;
			case 'K': KeyCodesTest(); break;
			case 'M': ShowHideMBar(); break;
			case 'R': ResponseTest(); break;
			case 'T': TimerTest(); break;
			case 'S': ScreenTest(); break;
			case 'Q': ExitToShell(); break;
			default: printf("Your choice is not recognized.  Please try again.\n");
					continue;
			}
		}
}


void ShowHideMBar() 
{
	long err;
	
	if ( (err = KeMoShowMBar()) && (err = KeMoHideMBar()) )
		printf("\n*** Call to menu bar operation returned error code %ld.\n", err);
	return;
}

void KeyCodesTest() {
	KeMoParms parms;
	char c;
	long err;

	if (err = KeMoSelect(KeMoOneKey))	{	/* select one keyboard */
		printf("\n*** Couldn't select keyboard only at this time (%ld).\n", err);
		return;
		}

	c = 0;
	printf("\nPress one key at a time.  To end, press 'q' or press no keys for 5 seconds.\n");

	while (c != 'q' && c != '!') {	/* end with q or timeout (shows up as ! returned
										by KeMoCode2Asc) */
										
		KeMoWait(KeMoDown, 5000L, &parms);	/* wait for a kepypress up to 5 seconds */
		
		c = KeMoCode2Asc(parms.key);	/* convert the key code to ASCII representation */
		
		if (c < 10)					/* keypad numbers */
			printf("K%c ", c+'0');
		else if (c < 20)			/* function keys 1-9 */
			printf("F%c ", c-10+'0');
		else if (c < 30)			/* function keys 10-15 */
			printf("F1%c ", c-20+'0');
		else						/* other key */
			printf("%c ", c);
		fflush(stdout);
		}
	printf("\n\n");
	KeMoReset();		/* undo selection of keyboard only */
}

void ScreenTest()
{
	long x;
	long t, i;

	if (x = KeMoSync(1)) {
		printf("Can't do screen sync: error %ld\n", x);
		return;
		}
		
	printf("\nThis test takes 20 seconds.  Please be patient.\n");
	
	i = 0;				/* Time is updated once a second by the Mac
							(it's a low-memory Mac variable) */
	t = Time + 1;
	while (t > Time);	/* wait for a fresh second */
	
	while (t + 20 > Time) {		/* call KeMoSync for exactly 20 seconds */
		KeMoSync(1);
		i++;
		}
		
	printf("This monitor's screen refresh rate is approximately %.1f Hz.\n", i/20.0);
	
	return;
}


void TimerTest()
{
	long t;
	
	printf("\nThis test runs for 10 seconds.  Press any key in about 7 seconds (at beep).\n");
	printf("If you hear a second beep, you did not press a key in time (press a key anyway\n");
	printf("to stop the test).\n");
	
	t = KeMoTimerTest();		/* this function takes care of this test */
								/* it returns microseconds */
	if (t < 0)
		printf("\nThere was an error: %ld.  Perhaps you didn't press the key early enough?\n", t);
	else
		printf("\nTimer accuracy while looking for keyboard: %ld msecs/sec.\n", (long)(t/1000.0 + .5));
	
	printf("\nRunning Delay test (1 second).\n");
	KeMoTimerStart();
	KeMoDelay(1000L);
	t = KeMoTimerStop(KeMoNoCorrection);
	
	printf("A delay of 1000 msecs takes %ld msecs to complete.\n", t);
}

void ResponseTest()
{
	long err, i, flags, done;
	KeMoParms parms;
	char ans[20];

	flags = 0; done = 0;
	
	while (!done) {
		if (!flags)
			printf("\nPlease select a device to be tested from the following list:\n\n");
		else
			printf("\nPlease select additional devices to be tested from the following list:\n\n");
						/* go through devices 2-4,8-15 */
		for (i = 2; i < 16; i == 4 ? i = 8 : i++) {
			if (KeMoDevArray[i] && !(flags & (1 << i))) {
				printf("\t%2ld: KeMoDev%1lX (%s)\n", i, i, 
				i == 2 && (KeMoDevType[i] == 2 || KeMoDevType[i] == 3) ? "keyboard" : (
				i == 2 || KeMoDevType[i] == 2 || KeMoDevType[i] == 3 ? "keyboard?" : "mouse/other"));
				}
			}
			
		printf("\n\t 0) Cancel\n");
		if (flags)
			printf("\t D) Done - use devices selected so far\n");	
		
		printf("\nChoice: ");
		fflush(stdout);
		
		fgets(ans, 10, stdin);
		
		if (*ans == 'd' || *ans == 'D') break;
		
		i = atoi(ans);
		
		if (i == 0) return;
		
		if (KeMoDevArray[i] && !(flags & (1 << i))) {
			flags |= 1 << i;
			continue;
			}
		else {
			printf("\n*** INVALID SELECTION (%ld). ***\n", i);
			continue;
			}
		}
		
	if (err = KeMoSelect(flags))		/* select the keyboard */
		printf("\n*** Couldn't select desired device at this time (err = %ld).\n", err);
	else {
		printf("\nThis test calls KeMoWait repeatedly with a 2 second timeout.\n");
		printf("Looking for 5 down events: "); fflush(stdout);
		for (i = 0; i < 5; i++) {
			KeMoWait(KeMoDown, 2000L, &parms);
			printf("%c-%s ", KeMoCode2Asc(parms.key), (parms.updown == KeMoDown ? "down" : 
				(parms.updown == KeMoUp ? "up" : 
				(parms.updown == KeMoTimedOut ? "timeout" : "???"))));
			fflush(stdout);
			}
		printf("\n");

		printf("Looking for 5 up events: "); fflush(stdout);
		for (i = 0; i < 5; i++) {
			KeMoWait(KeMoUp, 2000L, &parms);
			printf("%c-%s ", KeMoCode2Asc(parms.key), (parms.updown == KeMoDown ? "down" : 
				(parms.updown == KeMoUp ? "up" : 
				(parms.updown == KeMoTimedOut ? "timeout" : "???"))));
			fflush(stdout);
			}
		printf("\n");

		printf("Looking for 5 up or down events: "); fflush(stdout);
		for (i = 0; i < 5; i++) {
			KeMoWait(KeMoUpDown, 2000L, &parms);
			printf("%c-%s ", KeMoCode2Asc(parms.key), (parms.updown == KeMoDown ? "down" : 
				(parms.updown == KeMoUp ? "up" : 
				(parms.updown == KeMoTimedOut ? "timeout" : "???"))));
			fflush(stdout);
			}
		printf("\n");
		}
				
	KeMoReset();
	return;
}

void AccuracyTest()
{
	float t;
	long err, i, flags, done;
	KeMoParms parms;
	char ans[20];
	short x;

	flags = 0; done = 0;
	
	while (!done) {
		if (!flags)
			printf("\nPlease select a device to be tested from the following list:\n\n");
		else
			printf("\nPlease select additional devices to be tested from the following list:\n\n");
						/* go through devices 2-4,8-15 */
		for (i = 2; i < 16; i == 4 ? i = 8 : i++) {
			if (KeMoDevArray[i] && !(flags & (1 << i))) {
				printf("\t%2ld: KeMoDev%1lX (%s)\n", i, i, 
				i == 2 && (KeMoDevType[i] == 2 || KeMoDevType[i] == 3) ? "keyboard" : (
				i == 2 || KeMoDevType[i] == 2 || KeMoDevType[i] == 3 ? "keyboard?" : "mouse/other"));
				}
			}
			
		printf("\n\t 0) Cancel\n");
		if (flags)
			printf("\t D) Done - use devices selected so far\n");	
		
		printf("\nChoice: ");
		fflush(stdout);
		
		fgets(ans, 10, stdin);
		
		if (*ans == 'd' || *ans == 'D') break;
		
		i = atoi(ans);
		
		if (i == 0) return;
		
		if (KeMoDevArray[i] && !(flags & (1 << i))) {
			flags |= 1 << i;
			continue;
			}
		else {
			printf("\n*** INVALID SELECTION (%ld). ***\n", i);
			continue;
			}
		}		
	
	if (err = KeMoSelect(flags)) {
		printf("\n*** Oops! That selection cannot be made at this time (%ld).\n", err);
		return;
		}
	
	Delay(60L, 0);
	
	printf("End the test by pressing the appropriate device(s).\n");
	printf("The test will be automatically stopped after about 10 seconds.\n");
	
	t = KeMoAccuracy();	/* accuracy is returned in microseconds */
	
	KeMoReset();
	
	if (t < 0)
		printf("\nThe desired accuracy test could not be performed: error %ld.\n", (long)t);
	else
		printf("\nThe timing accuracy is approximately +/- %0.1f msec.\n\n", 
					t/1000.0/2.0);

	return;
}
