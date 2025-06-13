/*
	MyExtension.c
	
	Written by Ken Worley, 10/04/94, using Symantec Think C 7.0.
	Copyright 1994. All Rights Reserved.
	AOL KNEworley
	
	Feel free to use this code in an extension of your own.  Please don't
	publish or distribute this code without giving me proper credit.
	
	This code is set up to use Apple's "Universal Headers."  If you're not
	using those, some slight modifications may be necessary (changing
	pointer types).
	
	This extension is an example.  It shows how to use an extension to install
	a trap patch and shows how to share data between an extension, a control
	panel, and a trap patch.  You can use this as a basis for writing your own
	extension/control panel combination (or just a plain extension).
		
	The method for sharing data between the extension and patch was adapted
	from an extension called 'FlashInit.'  This was an example extension written
	by Richard Harvey in April 1990.  Thanks Richard!!  The method involves
	allocating a chunk of memory, then replacing a dummy reference in the patch
	code with the actual address of the memory (our shared memory structure).
	
	We can't use that method with the control panel code because it gets written
	back to disk, so we also store the address of the shared data in a resource
	that the control panel can access when it is opened.
	
	This extension also incorporates 'ShowIconFamily' by Patrick C. Beard and
	modified by James W. Walker.  This piece of code
	is called to display the extension's icon at startup.  This
	code was based on the original ShowInit by Paul Mercer, Darin Adler, Paul
	Snively and Steve Capps.  These are the guys to blame for our startup icon
	parades.  That piece of code (ShowIconFamily.c) is public domain.  I made
	some modifications to it in order to use it without any global variables.
	
	This extension uses no global or static variables, so it's not necessary
	to set up and restore A4.  All of our 'global' data is in the shared data
	structure (in a locked block in the system heap).
	
	I've also included a 'sysz' resource in the resource file.  Pre-System 7
	Macs will look at this resource when loading the extension and interpret the
	number there as a request to expand the system heap by the specified number
	of bytes.  The expanded space is not reserved for the extension, it is just
	added to the system heap space.  System 7 expands and contracts the system
	heap dynamically and so ignores the 'sysz' resource.  A template is also included
	for easily modifying the sysz resource with ResEdit.
	
	We use what I call a 'preferences' resource to save some settings between
	restarts.  For the most part, this same information is also held in memory
	(in the shared data structure) while the computer is running so that the
	patch code can access the settings.  The resource is normally accessed once
	by the extension code, then accessed and possibly changed later by the control
	panel code.
	
	This code sets a couple of the fields in the shared data structure which are
	accessed and/or modified by other code later on.  The 'CPon' field is set to
	true or false depending on whether or not the preferences resource indicates
	the control panel was set to 'on' or 'off' when last closed.  The 'patched'
	field is set to true if the trap patch was installed and false if it was not.
	
	About the only thing you'll ever need to modify in this file when writing
	your own extension (unless you're making major changes) are some of the #defines.
	
		kTrapToPatch determines which trap the patch is installed for.
		kParamBytes is the number of bytes passed on the stack as parameters for
			the trap routine.  (This would be zero for a register based trap.)
		kReturnBytes is the number of bytes reserved on the stack for the return
			value.  (This would also be zero for a register based trap.)
			
		kTaskRsrcNo is the resource number of the patch code.
		kTaskRsrcType is the resource type of the patch code ('task').
		
		kCDEVRsrcNo is the resource number of the control panel code.  You shouldn't
			need to change this.
			
		kIconFamilyID is the resource number of the icon family our final file will
			be using.  The number is used to send to ShowInit so our icon can be
			shown in the startup icon parade.
		
		kXIconID is the resource number of the icon family to be used when there was
			a problem when loading the extension.  This icon looks Xed out.
		
		kNoPatchIconID is the resource number of the icon family to be used when
			the patch is not loaded (when the control panel is set to "off").
		
		kMemAddrType is the resource type of the resource used to hold the address of
			the shared data structure.
		kMemAddrID is the resource number of the above resource.
	
	BUILDING THE CODE RESOURCE
	
		This file should be included in a project of type 'code resource' along
		with MacTraps. The type should be 'INIT' for the resource and it should
		have the system heap attribute set. (It should NOT be locked).  The resulting
		code resource should be included in the same file with the patch code
		resource and the control panel resource with the overall file type being
		'cdev'. The resource ID can be zero.
*/

/* trap patching defines */

#define		kTrapToPatch		_MenuSelect
#define		kParamBytes			4		/* a Point-in global coordinates */
#define		kReturnBytes		4		/* long int-hiword is menu id-loword is item# */

/* patch code defines */

#define		kTaskRsrcNo			128		/* the patch code's resource number */
#define		kTaskRsrcType		'task'	/* the patch code's resource type */

/* other defines */

#define		kCDEVRsrcNo			-4064	/* rsrc no of cdev code (rsrc type 'cdev') */

/* The following three icons are the ones I defined for this example.  Modify them */
/* for your purposes or come up with some on your own.  */

#define		kIconFamilyID		-4064	/* rsrc no of ICN# rsrc with our icons */
#define		kXIconID			-4033	/* rsrc no of Xed out icon family */
#define		kNoPatchIconID		-4034	/* rsrc no of icon family when patch not inst. */

/* control panel preferences resource defines */

#define kCPprefsRsrcType	'pref'
#define	kCPprefsRsrcID		-4048	/* careful - this number is also used in the cdev */

/* shared memory address resource defines */

#define kMemAddrType		'memA'
#define kMemAddrID			-4048	/* careful - this number is also used in the cdev */

typedef struct {
	long	theAddr;
} **memAddrHdl;

#include	<Traps.h>

#include "SharedData.h"			/* The definition of our shared data structure. */
								/* Includes definitions of myDataStruct, myDataPtr, */
								/* and myDataHandle to refer to this data */

/* Prototypes */

void	main( void );
void	ShowIconFamily(short iconId);

/* Functions */

void	main( void )
{
	myDataPtr			myData;		/* pointer to shared data structure */
	Handle				myTask;		/* handle to my trap patch code */
	memAddrHdl			addrHandle;	/* handle to memory address resource */
	UniversalProcPtr	myTaskAddr;	/* used to get addr of the task code */
	THz					saveZone;	/* used to save memory zone */
	long				toReplace,	/* these two items are used to replace the bogus */
						replaceWith;	/* addr in the task code with the */
										/* addr of the shared data structure */
										
	CPprefsHandle		prefsHandle;	/* Handle to the Control Panels prefs rsrc */
	Boolean				showStartupIcon;	/* show icon at startup? */
	Boolean				installPatch;		/* install trap patch? */
	
	Boolean				stillOK;		/* still OK to proceed? */
	Boolean				sharedDataOK;	/* was shared data struct allocated OK? */
	
	stillOK = true;
	showStartupIcon = true;
	installPatch = true;
	
	myData = NULL;
	myTask = NULL;
	addrHandle = NULL;
										
	/*	First save the current memory zone, then switch to the System zone so
	 *	we're positive that all memory allocation occurs in the System Heap.
	 */
	
		saveZone = GetZone();
		SetZone( SystemZone() );
		
	/*	Allocate memory (a pointer) for our data structure so that both this code
	 *	and the patch code will have access to the data.  (We'll see about
	 *	giving access to this data to the patch code later.)  We use a pointer
	 *	rather than a handle because the data has to be locked down anyway.
	 */
		
		myData = (myDataPtr)NewPtrSysClear(sizeof(myDataStruct));
		
		sharedDataOK = true;
		
		if ( !myData )
		{
			stillOK = false;
			sharedDataOK = false;
		}
	
	/*	Mark the data as in use until we're through with it.  Since we just
	 *	created it, we know no one is using it now.  Also initialize some
	 *	values in the shared data structure and set the check value so the
	 *	control panel can tell the extension actually ran.
	 */
		if ( sharedDataOK )
		{
			myData->inUse = true;
			myData->oldTrap = 0L;
			myData->CPprefsRsrc = NULL;
			myData->checkValue = kCheckValue;
		}

	/*	Load the control panel's preferences resource to see if we should show
	 *	the icon at startup and if we should patch the trap.  After we've read it,
	 *	release it.  The control panel will reload it.  If we cannot read the
	 *	resource from the file (probably because it doesn't yet exist), just assume
	 *	true/yes for both questions.
	 */
	 
	 	myData->CPon = true;		/* preset to 'on' unless we find otherwise */
	 								/* installPatch was also preset to true above */
	 	
	 	prefsHandle = (CPprefsHandle)Get1Resource( kCPprefsRsrcType, kCPprefsRsrcID );
	 	if ( prefsHandle )
	 	{
	 		if ( !(*prefsHandle)->On )
	 		{
	 			installPatch = false;
	 			myData->CPon = false;
	 		}
	 		
	 		if ( !(*prefsHandle)->ShowIcon )
	 			showStartupIcon = false;
	 			
	 		ReleaseResource( (Handle)prefsHandle );
	 	}
	
	/*	Get the address of the trap we're going to patch. */
		if ( sharedDataOK )
			myData->oldTrap = NGetTrapAddress( kTrapToPatch, ToolTrap );
	
	/*	Put the size of the trap's parameters and the size of the return value
	 *	(can be zero) in the shared data structure so that the patch code has
	 *	access to it.  This is used by the patch code to handle parameters that
	 *	are passed on the stack (same for the return value).  Note that with
	 *	register based routines (like a lot of the memory routines), the patch
	 *	has to know which registers are used and what their sizes are itself.
	 *	With stack based routines, the patch does not necessarily need to know
	 *	anything else about the parameters unless it wants to do something with
	 *	them.
	 */
	 	if ( sharedDataOK )
	 	{
	 		myData->paramBytes = kParamBytes;
	 		myData->returnBytes = kReturnBytes;
	 	}
		
	/*	Now load the code resource that comprises the actual 'patch.'  This code
	 *	will execute (instead of the original code) when the trap is executed.
	 *	The patch itself determines whether or not to execute (jump to) the original
	 *	trap code once it's done its thing.  The actual patch code is in its own
	 *	resource of type 'task'.
	 */
	 	if ( installPatch && stillOK )
	 	{
			myTask = Get1Resource( kTaskRsrcType, kTaskRsrcNo );
		
			if ( !myTask )
				stillOK = false;
		}
		
	/*	Here's an example of a clever technique used in Harvey's code.  We use the
	 *	Munger routine to replace a predetermined pattern in the task code with
	 *	the handle of our data structure so that the patch code has access to
	 *	the data.  The patch code assigns its data structure handle to 0x12345678
	 *	(which is a bogus address).  We use Munger to find this pattern
	 *	and replace it with the real address of our data structure handle.
	 */

		replaceWith = (long)( myData );
		toReplace = 0x12345678;
		
		if ( stillOK && installPatch )
			Munger( myTask, 0L, &toReplace, 4, &replaceWith, 4 );

	/*	The original (Harvey) code also used Munger to force the patch to jump to
	 *	the original trap address after it executed.  I'm leaving that up to the
	 *	patch code to do by itself (if it wants to).  The original (old) trap
	 *	address is stored in the oldTrap field in the shared data structure.
	 *	
	 *	Now we actually patch the trap.  We've waited until now to detach and
	 *	lock the task (patch) code because Munger expects a movable/resizeable
	 *	Handle. (Actually, this is just for form. Since we replaced a pattern
	 *	with something the same size, there was no need to resize or move the
	 *	memory block, so the locked/unlocked status wasn't that important.)
	 */
		if ( stillOK && installPatch )
		{
			DetachResource( myTask );
			HLock( myTask );
		
			myTaskAddr = (UniversalProcPtr)(StripAddress( (Ptr)(*myTask) ) );
			NSetTrapAddress( myTaskAddr, kTrapToPatch, ToolTrap );
			
			myData->patched = true;
		}
		else
		{
			myData->patched = false;
		}
	
	/*	Store the address of our shared data structure in a resource that
	 *	can be accessed by our control panel.  That way, the control panel 
	 *	can use and change our shared data.
	 */
		if ( stillOK )
		{
			addrHandle = (memAddrHdl)Get1Resource( kMemAddrType, kMemAddrID );
			if ( addrHandle )	/* got the resource */
			{
				(*addrHandle)->theAddr = replaceWith;	/* store value of myData */
				ChangedResource( (Handle)addrHandle );
				WriteResource( (Handle)addrHandle );	/* write rsrc back to file */
			}
			else	/* create a new resource */
			{
				addrHandle = (memAddrHdl)NewHandleClear( sizeof( long ) );
				if ( addrHandle )
				{
					(*addrHandle)->theAddr = replaceWith;	/* store value of myData */
					AddResource( (Handle)addrHandle, kMemAddrType, kMemAddrID,
						"\pShared Memory Addr" );		/* add a rsrc to file */
					WriteResource( (Handle)addrHandle ); /* write it out to the file */
				}
				else
				{
					stillOK = false;	/* unable to allocate memory for new rsrc */
				}
			}
		}
		
	/*	If there was a problem and it looks like the trap was still patched,
	 *	go ahead and remove the patch (restoring the original trap address).
	 */
	 
	 	if ( ( !stillOK ) && sharedDataOK )
	 	{
	 		if ( myTask )	/* if the task code was loaded, it was probably installed */
	 		{
	 			NSetTrapAddress( myData->oldTrap, kTrapToPatch, ToolTrap );
	 			DisposeHandle( myTask );
	 			myTask = NULL;
	 			
	 			myData->patched = false;
	 		}
	 	}
	
	/*	Show our icon in the startup icon parade.  If there was a problem somewhere
	 *	along the way, we'll show the icon even if the control panel says not to.
	 */

		if ( showStartupIcon || ( !stillOK ) )
		{
			if ( stillOK )
				if ( installPatch )
					ShowIconFamily( kIconFamilyID );
				else
					ShowIconFamily( kNoPatchIconID );
			else
				ShowIconFamily( kXIconID );
		}

	/*	Restore the memory zone to what it was when we began. */
	
		SetZone( saveZone );
	
	/*	Mark the data structure as no longer in use - we're through with it. */
	
		if ( sharedDataOK )
			myData->inUse = false;
}

#include "ShowIconFamily.c"
