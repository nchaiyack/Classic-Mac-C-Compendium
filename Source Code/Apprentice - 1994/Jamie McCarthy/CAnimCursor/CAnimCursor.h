/*
 * CAnimCursor.h
 * Version 1.0b4, 14 September 1992
 *
 */



/********************************/

#pragma once

/********************************/

#include <oops.h>

/********************************/

#include <Retrace.h>

/********************************/

enum {
	kCACModeInterrupted,
	kCACModeContinuous
} ;

	/*
	 * Go back to the normal cursor each time through the event loop,
	 * by default.
	 */
#define kCACDefaultMode (kCACModeInterrupted)

	/* Do about eight cursors per second, by default. */
#define kCACDefaultTicksBetweenCursors (8)

	/* Wait a half-second before beginning, by default. */
#define kCACDefaultInitialDelayTicks (30)

	/*
	 * Give the "normal" code one tick to sneak in ahead of the interrupt,
	 * by default.
	 */
#define kCACDefaultColorInterruptSlop (1)

/********************************/

class CAnimCursor;
extern CAnimCursor *gAnimCursor;

/********************************/

typedef struct {
	short			nCursors;
	short			cCursor;
	CursHandle	cursor[];
} acurStruct;
typedef acurStruct *acurPtr, **acurHndl;

/********************************/



class CAnimCursor : public indirect {
	
public:
	
	void			IAnimCursor(short rsrcID);
	void			Dispose(void);
	
	
						/*
						 * Be sure to set the resource file appropriately, with
						 * UseResFile(), before you call useAnimCursorID().  Be
						 * aware that IAnimCursor() calls it, too.
						 */
	void			useAnimCursorID(short rsrcID);
	void			stopUsingAnimCursor(void);
	
	
						/* These two methods start and stop the action. */
	void			startAnimating(void);
	void			stopAnimating(void);
						/* This one tells you whether the action's going on. */
	Boolean		getIsAnimating(void);
	
	
						/*
						 * This is called from the interrupt task, as well as by
						 * your code.  You don't have to call it unless you
						 * want color (color can't be used from an interrupt),
						 * or you called setUsingInterrupts(FALSE).
						 */
	void			animateCursor(void);
	
	
						/* The default tick values are defined above. */
	void			setTicksBetweenCursors(short newTicksBetweenCursors);
	void			setInitialDelayTicks(short newInitialDelayTicks);
	
	
						/*
						 * By default, it uses interrupts;  if you prefer a
						 * non-smooth animation, or one that will stop when your
						 * code hits an infinite loop, do setUseInterrupts(FALSE)
						 * and call animateCursor() as often as possible.
						 */
	void			setUsingInterrupts(Boolean newUsingInterrupts);
	Boolean		getUsingInterrupts(void);
	
	
						/*
						 * If you want your color cursor to be used, you've got to
						 * call gAnimCursor->animateCursor() every so often.  But
						 * the interrupt routine will always get to it before you
						 * will, unless you give yourself a few ticks where _you_
						 * can call animateCursor(), but the _interrupt_ can't.
						 * These ticks are called "slop."  (Of course, if you
						 * turn interrupts off, slop doesn't matter.)
						 */
	void			setSlopTicks(short newSlopTicks);
	short			getSlopTicks(void);
	
	
						/*
						 * By default, animation stops each time through the TCL's
						 * event loop:  CAcurSwitchboard calls stopAnimating(), and
						 * CAcurDesktop lets the CView hierarchy determine what the
						 * cursor should be.  If you don't like this, call
						 * setMode(kCACContinuous), and animation will continue
						 * through the event loop.  Of course, your CView hierarchy
						 * will lose control over what the cursor looks like.
						 * (If you're not using the TCL, the mode doesn't matter.)
						 */
	void			setMode(short newMode);
	short			getMode(void);
	
	
						/*
						 * To avoid finding 'crsr' resources in the System file or
						 * somesuch, CAnimCursor grabs the refNum of the top
						 * resource file when IAnimCursor is found, and assumes
						 * all the 'crsr' resources are in that file.
						 */
	void			setTopResFile(short newTopResFile);
	void			useTopResFile(void);
	
	
						/*
						 * If TRUE, the interrupt handler is running.  This must
						 * be public, because the (non-method) callback routine
						 * must access it;  but you shouldn't mess with it.
						 */
	Boolean		isInAnInterrupt;
	
	
protected:
	
	Boolean			isAnimating;
	Boolean			tryToUseColor;
	Boolean			usingColorCursors;
	Boolean			usingInterrupts;
	Boolean			vblTaskIsInstalled;
	Boolean			isInAnimateCursor;		// if TRUE, the interrupt will not re-enter
	
	short				itsTopResFile;
	short				itsAcurID;
	acurHndl			itsAcurHndl;
	
	short				itsTicksBetweenCursors;
	short				itsTicksBetweenCursorsForInterrupt;
	short				itsTicksBetweenCursorsForNormalCode;
	short				itsInitialDelayTicks;
	unsigned long	itsInitialTick;
	unsigned long	itsLastTick;
	
	short				itsMode;
	
							/**********
							 * Note that these two variables _must_ be declared
							 * right next to each other, in this order. */
	long				itsA5;
	VBLTask			itsVBLTask;
							/* If they aren't, the VBL task will choke and die
							 * when it tries to get at global variables.
							 **********/
	
	
	void			determineTryToUseColor(void);
	
	void			loadIndividualCursors(void);
	void			disposeIndividualCursors(void);
	
	OSErr			installVBLTask(void);
	OSErr			removeVBLTask(void);
	
	void			nextCursor(void);
	
} ;