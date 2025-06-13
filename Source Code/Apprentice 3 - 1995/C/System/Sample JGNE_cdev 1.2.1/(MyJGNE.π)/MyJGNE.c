/*
	MyJGNE.c
	
	Written by Ken Worley, 06/27/94, using Symantec Think C 7.0 (TPM 6.0.1).
	Copyright 1994. All Rights Reserved.
	AOL KNEworley
	internet KNEworley@aol.com
	
	Feel free to use this code in a project of your own, but give me proper
	credit in your documentation or about box.  Feel free to distribute this
	code in its entirety to anyone, but never do so without the copyright
	notice above nor without its accompanying files.  This code is NOT in
	the public domain.  Use of this code in a commercial product requires
	my permission.  I am not responsible for anything that might happen to
	your Mac or your data as a result of using this code.  I believe this
	code is stable and will not cause any damage to anything, but use it
	at your own risk.
		
	This code incorporates 'ShowIconFamily' by Patrick C. Beard and
	modified by James W. Walker.  This piece of code
	is called to display the extension's icon at startup.  This
	code was based on the original ShowInit by Paul Mercer, Darin Adler, Paul
	Snively and Steve Capps.  These are the guys to blame for our startup icon
	parades.  That piece of code (ShowIconFamily.c) is PUBLIC DOMAIN.  I made
	some modifications to it in order to use it without any global variables.
	
	This file includes the main routine which installs the filter (the main
	routine runs at system startup), and the MyTask routine which is the
	calling point of the filter routine.  Any other routines included in
	this file are called by MyTask.
	
	This code sets a couple of the fields in the global data which are
	accessed and/or modified by other code later on.  The 'CPon' field is set to
	true or false depending on whether or not the preferences resource indicates
	the control panel was set to 'on' or 'off' when last closed.  The
	'taskInstalled' field is true if the task was installed, false otherwise.
	
	The following #defines are used to identify icons that are displayed when
	the extension is run:
	
		kIconFamilyID is the resource number of the icon family our final file will
			be using.  The number is used to send to ShowInit so our icon can be
			shown in the startup icon parade.
		
		kXIconID is the resource number of the icon family to be used when there was
			a problem when loading the extension.  This icon looks Xed out.
		
		kNoActionIconID is the resource number of the icon family to be used when
			the patch is not loaded (when the control panel is set to "off").
	
	SO WHAT'S A JGNE?
	
		The Mac System recognizes a low memory global with the name JGNEFilter.
		This global CAN point to a routine used to "filter" ALL events that are
		retrieved by calling GetNextEvent or WaitNextEvent.  Without any
		modification, this global is normally NULL.  We can, however, fill it
		with the address of a "filter procedure" that examines an event and
		decides whether or not the application gathering it should actually receive
		it or not (and possibly doing some other things along the way).  I also
		refer to this filter procedure as the "task code."
		
	MEMORY SHARING
	
		The JGNE installer code (the main routine) shares memory with the control
		panel code by storing the address of a global structure (myData) in a
		resource.  The cdev accesses the resource and checks a constant field to
		ensure that it has the shared data.
		
		The installer code (and consequently its globals) hangs around even after it
		runs by retrieving a handle to itself, locking itself down, and telling the
		resource manager to forget it.  Since we also include the JGNE filter task
		code in this file, the installer code and the task code are all saved in
		the same chunk of memory along with the globals.  That's why the task code
		can access the same globals as the installer code.
	
	SETUPA4
	
		The THINK C compiler has a nice mechanism that allows global variables in
		code resources.  Normally, global variables in applications are accessed
		through the A5 register.  Since code resources run while an application
		is using the A5 register, THINK lets the code resource use the A4 register
		to access its globals.  The mechanism allows access to the same globals
		by any routine in the same file (compiled into the same resource).  See
		THINK's documentation for further explanation.
		
	WHY IS THE CONTROL PANEL IN A DIFFERENT PROJECT?
	
		The system requires that a control panel reside in its own resource with
		a certain resource type and a certain resource number.  This code is saved
		in a code resource of type 'INIT' with resource id 0.  Code resources of
		type 'INIT' in a file of type 'INIT' that are in the System Folder,
		Extensions Folder, or Control Panels Folder are automatically executed at
		startup time.  This INIT file is also allowed to have a code resource of
		type 'cdev' in it allowing us to combine a control panel (cdev) and
		extension (INIT) into the same finished file.  They only have to be compiled
		separately and put into different code resources.
	
	WHAT IF TWO EXTENSIONS BOTH WANT TO INSTALL JGNE FILTERS?
	
		Any decent JGNE filter saves the address of any filter already installed
		and jumps to that filter after it runs itself.  Like so...
		
			JGNEFilter --> aFilter		(global already points to a filter)
			oldFilter=JGNEFilter		(we save the value in a global of our own)
			JGNEFilter=myFilter			(and install our own filter)
			
			myFilter-->oldFilter		(after myFilter runs, it jumps to the)
										(address stored in oldFilter)
		
		A long chain of JGNE filters might exist, each calling the next filter
		in line until execution is returned to the program that called GetNextEvent
		or WaitNextEvent in the first place.
	
	THE 'sysz' RESOURCE
	
		There is a resource included in the project of type 'sysz.'  This resource
		holds one long value that, ONLY in PRE system 7 machines, acts as a request
		to add that number of bytes to the system heap.  Again, this only has any
		effect if loaded on a system 6 or earlier machine.  The number of bytes
		requested is added to the system heap, but are NOT reserved for any code
		or program in particular.
		
	NOTE
	
		You should not need to make any changes to anything in this file.
		Any allocation of memory or loading of resources should be done in
		MyInitialize.c.  The only other routine you should need to modify is
		the MyEvtHandler routine in the file MyEvtHandler.c.
*/

/********************* INCLUDED HEADERS ***********************/

#include <SetUpA4.h>	/* This header file contains code!!  Since			*/
						/* I'm using the standard THINK C header which		*/
						/* automatically jumps to main, I'm all right		*/
						/* including this here.  Using the standard header	*/
						/* is an option in the project type dialog.			*/
						/* The standard header is used when the Custom		*/
						/* header check box is NOT checked.					*/

#include "SharedData.h"	/* includes definitions of data structures			*/
						/* used both here and in the cdev, and some			*/
						/* common defines used by both.						*/

/*********** PROJECT SPECIFIC DEFINES, GLOBALS, ETC. *************/

#include "MyDefines&Globals.h"

/************************** DEFINES ******************************/

/* The following three icons are the ones I defined for this example.  Modify them */
/* for your purposes or come up with some on your own.  The numbers */
/* are resource numbers. */

#define		kIconFamilyID		-4064	/* ICN# rsrc with our icons */
#define		kXIconID			-4033	/* Xed out icon family */
#define		kNoActionIconID		-4034	/* icon family when we're OFF */

#define		kProcessNullEvts	false	/* examine/process null events? */

/************************** MACROS ******************************/

#define		GoodEvt(x)	(x & 0xFF00)	/* test high byte only */

/************************* GLOBALS ****************************/

Ptr		JGNEfilter : 0x29A;	/* low memory global that points to the */
							/* GetNextEvent filter routine */

/* Memory that the control panel needs to access also */

myDataStruct	myData;

/* Whatever other globals we need */

pascal void	(*theNextFilter)( void );	/* the next JGNE filter (if any) */

Boolean			interceptIt;	/* are we intercepting the event? */
short*			myReturnVal;	/* addr of filter's return value on stack */
EventRecord*	myEvtPtr;		/* pointer to the event record */
short			stillHasEvt;	/* do we have an event to work with? */

/************************ PROTOTYPES **************************/

void		main( void );

Boolean		MyInitialize( myDataPtr myData, Boolean *installTask, Boolean *showIcon );

void		ShowIconFamily( short iconId );

pascal void		MyTask( void );

Boolean		ProcessEvt( EventRecord *event );

#include "MyEvtHandler.h"	/* prototypes/defines for routines in MyEvtHandler.c */

/************************ FUNCTIONS ***************************/

/* main */
// This routine is actually the extension or INIT code that is run at startup time.
// Its job is to initialize variables, allocate any memory we'll need later (in the
// filter code), check the preferences resource, and install the filter code.

void	main( void )
{
	Boolean				showStartupIcon;	/* show icon at startup? */
	Boolean				installTask;		/* install the task? */
	
	Boolean				stillOK;			/* still OK to proceed? */
	
	Handle				me;					/* a handle to this code */
	
	/* Set up for A4 based global variables (specific to THINK C) */
	
		RememberA0();	/* A0 contains the base address of this code */
		SetUpA4();

	/* Initialize some local variables */
	
		stillOK = true;
		showStartupIcon = true;
		installTask = true;
	
	/* Retrieve a handle to this code and lock it down */
		
		{
			Ptr		ptrToMe;
			
			/* Move the value in register A0 to ptrToMe.  A0 holds the */
			/* direct address of this code (courtesy THINK). */
			
				asm
				{
					MOVE.L	A0,ptrToMe
				}
			
			/* Now recover the Handle to this code using RecoverHandle */
			
				me = RecoverHandle( ptrToMe );
			
			/* Make the Resource Manager forget about us and lock this */
			/* code down so it stays in memory even after the extension */
			/* has run its course.  This allows the globals we've defined */
			/* to hang around (as well as the task code). */
			
				DetachResource( me );
				HLock( me );
		}
		
	/* Initialize some values */

		if ( stillOK )
		{
			/* This check value is checked by the cdev to make sure the */
			/* extension was run at system startup time. */
			
				myData.checkValue = 0x12341234;
			
			/* Set INITrun flag so control panel will know we've run */
			
				myData.INITrun = true;
		}
		
	// Call MyInitialize to initialize variables, allocate memory, load resources,
	// etc.  MyInitialize returns a Boolean value put into stillOK to indicate
	// if all is well, and may also modify the values in installTask and
	// showStartupIcon to affect whether or not the filter code is actually
	// installed and whether or not the startup icon is shown.
	
		if ( stillOK )
			stillOK = MyInitialize( &myData, &installTask, &showStartupIcon );
		
	/*	Store the address of myData in a resource that can be accessed by
	 *	our control panel.  That way, the control panel can use and change
	 *	that data.
	 */
		if ( stillOK )
		{
			memAddrHdl			theMemAddr;		/* resource used to hold the */
												/* addr of the data structure */
												/* shared with the cdev */
	
			// load the existing resource so we can change it
			
			theMemAddr = (memAddrHdl)Get1Resource( kMemAddrType, kMemAddrID );
			if ( theMemAddr )	/* got the resource */
			{
				(*theMemAddr)->theAddr = (long)(&myData); /* addr of myData */
				ChangedResource( (Handle)theMemAddr );
				WriteResource( (Handle)theMemAddr );	/* write rsrc back to file */
			}
			else	// resource not found, so create a new resource
			{
				theMemAddr = (memAddrHdl)NewHandleSysClear( sizeof( long ) );
				if ( theMemAddr )
				{
					(*theMemAddr)->theAddr = (long)(&myData); /* addr of myData */
					AddResource( (Handle)theMemAddr, kMemAddrType, kMemAddrID,
						"\pShared Memory Addr" );		/* add a rsrc to file */
					WriteResource( (Handle)theMemAddr ); /* write it out to the file */
				}
				else
					stillOK = false;	/* unable to allocate memory for new rsrc */
			}
		}

	/*	Store the address of the next JGNE filter for the task code to
	 *	jump to later (after it does its own processing).
	 */
	 
	 	theNextFilter = (void*)JGNEfilter;	/* save old filter address */
	 	
	/*	Now we actually install the filter. */
	
		if ( stillOK && installTask )
		{
			JGNEfilter = (void*)MyTask;

			myData.taskInstalled = true;
		}
		else
		{
			myData.taskInstalled = false;
		}
	
	/*	Show our icon in the startup icon parade.  If there was a problem somewhere
	 *	along the way, we'll show the icon even if the control panel says not to.
	 */

		if ( showStartupIcon || ( !stillOK ) )
		{
			if ( stillOK )
				if ( installTask )
					ShowIconFamily( kIconFamilyID );
				else
					ShowIconFamily( kNoActionIconID );
			else
				ShowIconFamily( kXIconID );
		}
	
	/* If not still OK now, unlock this code and make it purgeable so it doesn't */
	/* continue to take up valuable space in the system heap. */
	
		if ( !stillOK )
		{
			HUnlock( me );
			HPurge( me );
		}
		
	/* Restore old A4 value */
	
		RestoreA4();
}

#include "MyInitialize.c"

/* myTask */
//
// This is the "filter" or "task" code that gets called every time an event is
// returned to an application.  This routine is mainly "glue" code that extracts
// the information we need from the stack and registers, calls other filters that
// are installed, and saves the registers.  Most of our actual work is done
// elsewhere in the ProcessEvt routine.

pascal void MyTask( void )
{
	// At this point:
	//		-> no stack frame (LINK) is generated because there are no local vars
	//		-> there is a return value (short, 2 bytes) on the stack that tells
	//			the system if it should accept the event (nonzero yes/zero no).
	//			The routine is declared with a void return value because we're
	//			going to set the return value manually when we return.
	//		-> on top of the return value is the return address (4 bytes).  This
	//			is the "top" of the stack right now.
	//		-> on entry, register A1 points to the event record
	//		-> register D0 contains the "return value" that tells us whether
	//			or not we actually have an event. This is indicated by the
	//			high byte of the low word of that register.
	//
	//		| prev stack contents		|
	//		|---------------------------|
	//		| short ret value 2 bytes	|
	//		|---------------------------|
	//		| return address			|
	//		|		4 bytes				|
	//		|---------------------------|	<- stack pointer (SP)
	//
	
	asm
	{
		MOVEM.L	A0-A6/D1-D7,-(SP)		;SAVE REGISTERS
		MOVE.L	SP,A2					;COPY STACK PTR TO REG A2
		ADD.L	#60,A2					;ADD 60 TO A2 TO GET ADDR OF RETURN
										;VALUE ON THE STACK
		MOVE.L	A1,A3					;COPY REG A1 TO REG A3
	}

	//		| prev stack contents		|
	//		|---------------------------|
	//		| short ret value 2 bytes	|	A1 -> Event Record
	//		|---------------------------|	A3 -> Event Record
	//		| return address			|
	//		|		4 bytes				|	A2 -> Return value
	//		|---------------------------|
	//		| 							|
	//		| saved registers			|
	//		| 		56 bytes			|
	//		| 		4 bytes per			|
	//		| 		register saved		|
	//		| 							|
	//		|---------------------------|	<- stack pointer (SP)
	//
	
	// Set up for access to global variables through register A4.
	// This process uses register A1 and sets register A4 to point
	// to our global variables area.
	
		SetUpA4();
	
	asm
	{
		MOVE.L	A3,A1					;RESTORE REG A1 FROM REG A3 SO IT
										;AGAIN POINTS TO EVENT RECORD
		MOVE.L	A1,myEvtPtr				;PUT EVENT PTR INTO GLOBAL myEvtPtr
		MOVE.L	A2,myReturnVal			;STORE ADDR OF RETURN VALUE IN THE
										;GLOBAL myReturnVal
	}

	//		| prev stack contents		|
	//		|---------------------------|
	//		| short ret value 2 bytes	|	A1 -> Event Record
	//		|---------------------------|	A3 -> Event Record
	//		| return address			|
	//		|		4 bytes				|	A2 -> Return value
	//		|---------------------------|
	//		| 							|
	//		| saved registers			|
	//		| 		56 bytes			|	myEvtPtr -> Event Record
	//		| 		4 bytes per			|	myReturnVal -> Return value
	//		| 		register saved		|
	//		| 							|
	//		|---------------------------|
	//		| old reg A4 value			|
	//		| 		4 bytes				|
	//		|---------------------------|	<- stack pointer (SP)
	//
	
	// When we initially installed this routine as a filter (in the INIT), we
	// checked to see if one or more filters was already installed.  If there
	// was, the address of the next filter is stored in the global theNextFilter.
	//
	// If there is a next filter, call it now.
	//
	// It is the high byte of the value in D0 that is important.
	
		asm
		{
			MOVE.W	D0,stillHasEvt		;move D0 to global stillHasEvt
		}
		
		if ( theNextFilter )
		{
			asm
			{
				MOVE.W	stillHasEvt,D0		;make sure D0 holds the correct value
				MOVE.W	stillHasEvt,-(SP)	;also copy the value to the top of
											;the stack
			}
			
			(*theNextFilter)();
			
			asm
			{
				MOVE.W	(SP)+,stillHasEvt		;get the new return value and
				MOVE.L	myReturnVal,A0			;copy it to where the old return
				MOVE.W	stillHasEvt,(A0)		;value was on the stack
			}
		}
	
	// At this point, stillHasEvt represents either the same return value we
	// initially had when we started, or the return value from the next filter
	// if one was called.
	//
	// If we are still working with a valid event then we'll
	// now process the event ourselves and decide if we want to intercept it.
	
		if ( GoodEvt(stillHasEvt) || kProcessNullEvts )
			interceptIt = ProcessEvt( myEvtPtr );
		else
			interceptIt = false;
	
	// After the next filter was called, the return value on the stack was
	// changed to reflect the new result.  We'll only need to change that
	// result now if we're going to intercept the event.  We do that by
	// making the return value ZERO.
	
		if ( interceptIt )
		{
			myEvtPtr->what = nullEvent;
			(*myReturnVal) = 0;
		}
	
	// Copy the return value to register D0.  D0 and the return value on
	// the stack should mirror each other when we exit.
	
		asm
		{
			MOVE.L	myReturnVal,A2		;get return value addr on stack
			CLR.L	D0					;clear out register D0 first
			MOVE.W	(A2),D0				;copy the return value to D0
		}
		
	// Now restore the previous value of the A4 register.  We'll no longer
	// have access to our globals.
	
		RestoreA4();

	//		| prev stack contents		|
	//		|---------------------------|
	//		| short ret value 2 bytes	|
	//		|---------------------------|
	//		| return address			|
	//		|		4 bytes				|
	//		|---------------------------|
	//		| 							|
	//		| saved registers			|
	//		| 		56 bytes			|
	//		| 		4 bytes per			|
	//		| 		register saved		|
	//		| 							|
	//		|---------------------------|	<- stack pointer (SP)
	//
	
	asm
	{
		MOVEM.L		(SP)+, A0-A6/D1-D7		;RESTORE SAVED REGISTERS
	}

	//		| prev stack contents		|
	//		|---------------------------|
	//		| short ret value 2 bytes	|
	//		|---------------------------|
	//		| return address			|
	//		|		4 bytes				|
	//		|---------------------------|	<- stack pointer (SP)
	//
	// At the end of this routine, the compiler automatically inserts an RTS
	// command which takes the return address off the stack and jumps to it
	// leaving only the return value on the stack (plus the previous contents).
}


/*	The ProcessEvt routine:
 *		¥ checks the control panel to see if it is "on"
 *		¥ calls MyEvtHandler to process the event and see if it should be
 *			intercepted.
 *		¥ if the event is to be intercepted, ProcessEvt returns true.
 *			Otherwise, it returns false.
 */
Boolean ProcessEvt( EventRecord *event )
{
	Boolean			intercept;
									
	/*	If the control panel setting is not ON, we take no further action.
	 *	CPon was set initially by the extension based on a value in the
	 *	preferences resource.  It may later have been modified by the on/off
	 *	setting in the control panel.
	 *
	 *	Here, we process the event and check to see if it will be
	 *	intercepted by calling the MyEvtHandler routine.  Our return value is based
	 *	on the return value of MyEvtHandler.
	 */
	
		if ( myData.CPon )
		{
			long	oldA5;
			
			/* Make sure the A5 register holds the current app's correct */
			/* A5 world value, and save the previous value. */
			
				oldA5 = SetCurrentA5();
				
			// Call MyEvtHandler to process the event.  If MyEvtHandler returns
			// true, then we'll return true so that the event is
			// "intercepted."  This causes WNE/GNE to return false (basically
			// a NULL event.  If MyEvtHandler returns false, then we'll also
			// return false and the event is returned to the calling
			// application as a real event.
			
				intercept = MyEvtHandler( event );
			
			/* Restore the previous A5 value */
			
				SetA5( oldA5 );
	 	}
	 return intercept;
}

#include "ShowIconFamily.c"

#include "MyEvtHandler.c"
