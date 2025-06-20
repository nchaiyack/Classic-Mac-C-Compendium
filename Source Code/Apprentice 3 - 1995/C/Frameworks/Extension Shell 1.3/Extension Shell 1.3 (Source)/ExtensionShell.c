/*	NAME:
		ExtensionShell.c

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		This file contains the main entry point of Extension Shell. It 
		calls everything else to do the actual work.		

	NOTES:
		�	Compiled with THINK C 6.0.
		
		�	Extension Shell must be compiled with System Heap and Locked attributes.
			We don't lock ourselves down manually, so we implicitly depend on this.

		�	At startup, A0 contains the address of our CODE resource. THINK C CODE
			resources reach their globals from A4, so we save the value of A4
			when we enter and restore it when done. This is THINK C specific.
		
		�	We must also initialise the Toolbox. The only tricky one is QuickDraw,
			since we can't reach a valid thePort. Instead we allocate a record of
			size GrafSize and pass the address of the thePort field to InitGraf.
		
		�	Initialising the Window Manager would erase the screen. This
			means we that we would zap all the previous icons, and feel an
			irresistable urge to throw horrible windows and dialogs up on
			screen at starutp. Don't initialise the Window Manager - see the
			�DeskHook and INIT Evils� Tech Note if you think you're special.

	___________________________________________________________________________


	RELEASE INFORMATION:
		Extension Shell is freeware. Basically, it tries to tie together all the
		code/ideas related to INIT writing that have been floating around for
		years, and combine them in an Extension-independant way. All of the
		techniques it makes use of are, to the best of my knowledge, public
		domain/freeware themselves. However, Extension Shell is still
		Copyrighted � 1993-1994, Dair Grant. You may not redistribute it in any
		modified form.

		You may not charge for Extension Shell itself, or redistribute it as part
		of a commercial package without my prior permission. Shareware houses, BBS
		Sysops, or CD-ROM Distributors may include Extension Shell in their
		collections, on the understanding that they do not charge for access to
		Extension Shell specifically.

		If you use Extension Shell in a commercial Extension, you are obliged to
		display some form of indication that your product uses Extension Shell code.
		This notice must be plainly visible to users (either in the 'vers' resource,
		or in some other form of documentation), and not just those curious enough to
		use ResEdit. Shareware and Freeware authors can use the code without notice.

		End of <legalstuff.h>. I do plan to support this code. If you have any
		problems, or see any errors in the code, let me know, and the source will
		be updated. If you don't like how something is implemented, and can think
		of a better way - again, let me know.

	___________________________________________________________________________
*/
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <GestaltEqu.h>
#include "ESConstants.h"
#include "ExtensionShell.h"
#include "StandaloneCode.h"
#include "NotifyMsg.h"
#include "ShowIcon.h"
#include "ParamBlock.h"
#include "InstallCode.h"
#include "UninstallCode.h"
#include "AddrsTable.h"





//=============================================================================
//		Private function prototypes																	 
//-----------------------------------------------------------------------------
void		main(void);
void		InitToolbox(void);
void		InitParamBlock(void);
void		CallESHandler(short theMsg);
OSErr		InstallForESHandler(void);
void		TryAndUninstallESHandlerResources(void);
void		IndicateError(void);
void		ShowTheIcon(void);
pascal Boolean UserForcedDisable(short keyCode, Boolean checkMouse);
pascal Boolean IsTrapAvailable(int trapNum);





//=============================================================================
//		Global variables																 
//-----------------------------------------------------------------------------
FakeQD			gTheQDGlobals;
ESParamBlock	gTheParamBlock;
AddressTable	*gTheAddressTable;










//=============================================================================
//		main : Entry point to our 'INIT' code resource.																 
//-----------------------------------------------------------------------------
//		Note :	The only thing we have to watch out for is that we have to get
//				access to our globals. We do this in the way mentioned above.
//
//				Unlike most 'INIT' entry routines, we don't call RecoverHandle,
//				HLock, and DetachResource on ourselves. We don't leave *anything*
//				behind apart from the trap-patches. We don't lock ourselves down
//				as the 'INIT' resource was compiled with System Heap & Locked
//				attributes.
//-----------------------------------------------------------------------------
void main(void)
{	THz				oldZone;




	// Set up A4, switch to the System Zone so that any memory we allocate
	// hangs out there, and initialise everything
	GetGlobals();
	oldZone = GetZone();
	SetZone(SystemZone());
	InitToolbox();
	InitParamBlock();
	
	

	// Execute the ES Handler CODE resource
	CallESHandler(kInitialiseParamBlock);



	// Try and install the whatever we have to. If we don't have
	// anything to install, InstallUsersResources() returns noErr.
	if (InstallForESHandler() != noErr)
		{
		// If there's a problem, we have to call the handler again
		CallESHandler(kHandleError);
		
	
	
		// If we're allowed to, try and remove any code that was installed
		if (gTheParamBlock.removeInstalledCode)
			TryAndUninstallESHandlerResources();
		}



	// Handle any errors - this could arise from a failure to install
	// a code resource, or the Extension might have decided it
	// just isn't able to run (e.g., we're under System 6.x.x).
	if (gTheParamBlock.beepNow || gTheParamBlock.postError)
		IndicateError();
	
	
		
	// Show any icons that we're to show
	ShowTheIcon();



	// Return to the previous zone, and restore A4
	SetZone(oldZone);
	UngetGlobals();
}










//=============================================================================
//		InitToolbox : Initialises the Toolbox.																 
//-----------------------------------------------------------------------------
//		Note :	If we initialise all the Toolbox Managers, we will erase the
//				screen. This looks ugly - so all but the minimum is moved inside
//				the __InitAllToolbox__ #define.
//-----------------------------------------------------------------------------
void InitToolbox(void)
{




	// Initialise the bits of the Toolbox that we need
	InitGraf(&gTheQDGlobals.thePort);



#ifdef __InitAllToolbox__
	// This fixes the pre-Mac II bug that can happen if we try and create a window
 	// from within an INIT. See the �DeskHook and INIT Evils� Tech Note.
	*((void**)DragHook) = NULL;
	*((void**)DeskHook) = NULL;



	// Initialise the rest of the Toolbox
	InitFonts();
	InitWindows();
	InitCursor();
	InitMenus();
	InitDialogs(NULL);
	TEInit();
#endif
	
	
	
	// Flush out any events that may be lurking
	FlushEvents(everyEvent, 0);
	FlushEvents(everyEvent, 0);
	FlushEvents(everyEvent, 0);
}










//=============================================================================
//		InitParamBlock : Initialise gTheParamBlock.															 
//-----------------------------------------------------------------------------
//		Note :	We want to minimise the work the user's CODE resource has to
//				do. As such we minimise everything to the 'least harmful'
//				value.
//-----------------------------------------------------------------------------
void InitParamBlock(void)
{	OSErr		myErr;

	

	
	// Initialise the general values
	myErr = Gestalt(gestaltSystemVersion, &gTheParamBlock.systemVersion);
	gTheParamBlock.IsTrapAvailable		= IsTrapAvailable;
	gTheParamBlock.UserForcedDisable	= UserForcedDisable;



	// Initialise the Icon related values
	gTheParamBlock.numIcons				= 0;
	gTheParamBlock.animationDelay		= 3;
	
	
	
	// Initialise the installable code related values
	gTheParamBlock.installAddressTable	= false;
	gTheParamBlock.numCodeResources		= 0;
	gTheParamBlock.errorIndex			= 0;
	gTheParamBlock.theErr				= noErr;

	
		
	// Initialise the error handling related values
	gTheParamBlock.removeInstalledCode	= false;
	gTheParamBlock.beepNow				= false;
	gTheParamBlock.postError			= false;
	gTheParamBlock.errorStringsID		= 0;
	gTheParamBlock.errorStringIndex		= 0;
}










//=============================================================================
//		CallESHandler : Execute this Extension's ES Handler CODE resource.															 
//-----------------------------------------------------------------------------
//		Note :	This routine can be called several times. We load the CODE
//				resource in once, and save it in a static variable. When
//				the System closes our resource fork, the code will get
//				flushed out (we're not calling DetachResource).
//
//				If we can't find the ES Handler, we die - nothing else for it.
//-----------------------------------------------------------------------------
void CallESHandler(short theMsg)
{	static void		(*theHandler)(short theMsg, ESParamBlock *gTheParamBlock)=nil;
	Handle			theHnd;




	// If we've not already loaded the code, do so, and lock it down
	if (theHandler == nil)
		{
		theHnd = Get1Resource(kESHandlerCodeType, kESHandlerCodeID);
		if (theHnd == nil)
			DebugStr("\pExtension Shell - couldn't find ES Handler");
		else
			HLock(theHnd);
			
		theHandler = (ProcPtr) StripAddress(*theHnd);
		}
	

	
	// Call the code with the message and gTheParamBlock
	(*theHandler)(theMsg, &gTheParamBlock);
}










//=============================================================================
//		InstallForESHandler : Install what the ES Handler wants installed.															 
//-----------------------------------------------------------------------------
//		Note :	This routine attempts to install the routines the ES Handler
//				CODE resource wants installed. If there is a problem, the
//				appropriate fields in gTheParamBlock will be filled in, and
//				an error code returned. If everything installs without a
//				problem, noErr is returned.
//-----------------------------------------------------------------------------
OSErr InstallForESHandler(void)
{	int		i;
	OSErr	theErr=noErr;




	// If we're to install an address table, install it. If there's a problem,
	// it will have to stay locked in the System heap. But we can't set it up
	// after we install everything, because some of the things we install might
	// depend on earlier things being installed correctly. After we've installed
	// it we call the ES Handler again just in case there's something special
	// it wants to do to the address table. There may well be some Extension
	// specific fields tagged onto the end of the address table and the ES
	// Handler will have to initialise them to some values. If there's a problem,
	// the ES Handler can set gTheParamBlock.theErr to some error value (any
	// error value) and we won't install any code. The Handler will then be
	// called later on to handle the error as normal.
	if (gTheParamBlock.installAddressTable)
		{
		InstallAddressTable();
		
		CallESHandler(kInitialiseAddrsTable);
		if (gTheParamBlock.theErr != noErr)
			return(gTheParamBlock.theErr);
		}



	// Install each resource in turn. If there's a problem, we save the
	// error details and break out of the loop.
	for (i = 1; i <= gTheParamBlock.numCodeResources && theErr == noErr; i++)
		{
		theErr = InstallCode(i);
		if (theErr != noErr)
			{
			gTheParamBlock.errorIndex = i;
			gTheParamBlock.theErr     = theErr;
			}
		}
		
		
	
	// Return any error code - gTheParamBlock.TheErr is initialised to
	// noErr, so it always holds the correct value.
	return(gTheParamBlock.theErr);
}










//=============================================================================
//		TryAndUninstallESHandlerResources : Try and uninstall what was
//											previously installed.
//-----------------------------------------------------------------------------
//		Note :	This routine attempts to remove the routines the ES Handler
//				resource tried to install. The error index is assumed to be at
//				the resource that could not be installed, and so only entries
//				before ErrorIndex are removed.
//
//				We make *no* guarantees as to what can be removed. Hopefully,
//				most things can, but you never know.
//-----------------------------------------------------------------------------
void TryAndUninstallESHandlerResources(void)
{	int		i;




	// gTheParamBlock.ErrorIndex contains the index of the item that
	// couldn't be installed. We try and uninstall everything before this.
	// If we bombed on the first item, there's nothing to uninstall.
	if (gTheParamBlock.errorIndex > 1)
		{
		for (i = 1; i < gTheParamBlock.errorIndex; i++)
			UninstallCode(i);
		}
}










//=============================================================================
//		IndicateError : Provide some error handling capabilities.																 
//-----------------------------------------------------------------------------
//		Note :	This routine is how Extension Shell communicates errors to the
//				user. The two functions it offers are to beep, and to post a 
//				Notification Manager dialog to the user.
//-----------------------------------------------------------------------------
void IndicateError(void)
{




	// If we're to beep, do so now
	if (gTheParamBlock.beepNow)
		SysBeep(30);
	
	
	
	// If we're to post a NM message, do so
	if (gTheParamBlock.postError)
		NotificationMessage(gTheParamBlock.errorStringsID, gTheParamBlock.errorStringIndex);
}










//=============================================================================
//		ShowTheIcon : Call PlotINITIcon to display the icons.															 
//-----------------------------------------------------------------------------
//		Note :	We pull the values out of gTheParamBlock, and let PlotINITIcon
//				do all the work.
//-----------------------------------------------------------------------------
void ShowTheIcon(void)
{



	// Call PlotINITIcon to do the work
	PlotINITIcon(gTheParamBlock.systemVersion >= 0x0700,
				 gTheParamBlock.animationDelay,
				 gTheParamBlock.numIcons,
				 &gTheParamBlock.theIcons);
}










//=============================================================================
//		UserForcedDisable : Called by the user's CODE to scan for disables.															 
//-----------------------------------------------------------------------------
//		Note :	We return true if the key corresponding to KeyCode is pressed,
//				and false otherwise.
//				If CheckMouse is true, we also return false if the mouse is not
//				pressed, and true if it is.
//-----------------------------------------------------------------------------
pascal Boolean UserForcedDisable(short keyCode, Boolean checkMouse)
{	unsigned char	theKeys[16];




	// First check the keyboard
    GetKeys((void *) theKeys);
    if ((theKeys[keyCode >> 3] >> (keyCode & 7)) & 1)
    	return(true);
    
    
    
	// No? OK, check if the mouse button is down
	if (checkMouse)
		return(Button());
	
	
	
	// If we've got this far, nothing is being pressed
	return(false);
}










//=============================================================================
//		GetTrapType : Is a given trap an OS trap or a Toolbox trap?															 
//-----------------------------------------------------------------------------
TrapType GetTrapType(int trapNum)
{	TrapType tType;
 
 
 
	tType = (trapNum & 0x0800) ? ToolTrap : OSTrap;
	return(tType);
}  










//=============================================================================
//		IsTrapAvailable : Determines if a given trap number is implemented.															 
//-----------------------------------------------------------------------------
//		Note :	This function is based on Inside Mac VI, 3-8. It comes from
//				Eric Shapiro's July BYTE '93 article, and the sample INITs
//				he provides - on ftp.uu.net in /published/byte.
//-----------------------------------------------------------------------------
pascal Boolean IsTrapAvailable(int trapNum)
{	TrapType	tType;
	short		numToolboxTraps;
 
 
 
 
	// Check trap word for Toolbox bit
	tType = (trapNum & 0x0800) ? ToolTrap : OSTrap;
	
	
	
	// If it's a Toolbox trap...
	if (tType == ToolTrap)
		{
		// Mask through largest # of traps available
		trapNum &= 0x07FF;
		
		
		// Is _InitGraf at address of 0xAA6E?
		if (NGetTrapAddress(kInitGrafTrap,ToolTrap) == NGetTrapAddress(0xAA6E, ToolTrap))
			numToolboxTraps = 0x0200;				// Yes, only this many entries in dispatch table
		else
			numToolboxTraps = 0x0400;				// No, dispatch table is larger
		
		
		// Trap # bigger than dispatch table? If so it's a bogus trap, so abort
		if (trapNum > numToolboxTraps)
			return(FALSE);
		} 
 
 
 
	// Return trap address if it's not an unimplemented trap
	return(NGetTrapAddress(trapNum, tType) != NGetTrapAddress(kUnimplementedTrap, ToolTrap));
}
