/*	NAME:
		ParamBlock.h

	WRITTEN BY:
		Dair Grant
				
	DESCRIPTION:
		Definition of the ParamBlock that Extension Shell uses for communication
		with your code.

	***************************************************************
	IMPORTANT
	***************************************************************
		This file is probably *the* most important file for Extension Shell. Read
		it carefully - it defines *exactly* how your ES Handler communicates with
		Extension Shell, and how it describes the code resources it wants
		Extension Shell to load.

		Read this file through, and print it out.
		

	___________________________________________________________________________
*/
#ifndef __ESPARAMBLOCK__
#define __ESPARAMBLOCK__
//=============================================================================
//		Include files																	 
//-----------------------------------------------------------------------------
#include <Retrace.h>
#include <Shutdown.h>
#include "ESConstants.h"





//=============================================================================
//		Structures																 
//-----------------------------------------------------------------------------
// Six types of code resources are supported right now. Defines to pick them out
// of the union are defined here, with their structures.
#define kTrapPatchType				1						// A Trap Patch
#define kGestaltSelectorType		2						// A Gestalt Selector
#define kShutdownTaskType			3						// A Shutdown Task
#define kVBLTaskType				4						// A VBL Task
#define kLowMemFilterType			5						// A low-mem filter
#define kCodeBlockType				6						// A block of code
#define kTimeManagerTaskType		7						// A Time Manager Task




// TrapPatch information. Trap Patches need to hold the number of the
// trap to patch. Your ES Handler is responsible for ensuring that the
// trap is implemented. If you're patching something really new, you
// need to be aware of this. The IsTrapAvailable routine is provided
// for you to test for the presence of a trap.
typedef struct {
	int				trapNum;
} ATrapPatch;



// Gestalt selectors may want to override existing selectors. Then again,
// they may not - set the OverwriteExistingSelector field accordingly.
// theSelector is the Gestalt type for the selector.
typedef struct {
	OSType			theSelector;
	Boolean			overwriteExistingSelector;
} AGestaltSelector;



// Shutdown tasks can pass in some flags - defined in Shutdown.h
typedef struct {
	short			theFlags;
} AShutdownTask;



// Information for a VBL task.
typedef struct {
    short			vblCount;
    short			vblPhase;
} AVBLTask;



// Information for a low-mem filter. We need to hold the address of the
// filter to hook ourselves into.
typedef struct {
    long			theEntryPoint;
} ALowMemFilter;



// Information for a block of code. We don't actually need anything,
// but we can't compile it without allocating some members.
typedef struct {
	short			reserved;
} ACodeBlock;



// Information for a Time Manager task. We need to hold the time (in
// milliseconds) before the task is first executed.
typedef struct {
	long			theDelay;
} ATimeManagerTask;




// Each installable 'thing' is specified via a CodeInfo structure. This holds
// the resource type and id of the code resource (e.g. 'CODE', and 1000),
// as well as a CodeType field. To save space, the code-specific details are
// merged in a union type, with the CodeType field being used to specify the
// contents of TheCodeThing. TheAddress is an internal Extension Shell field,
// used to hold the address of the thing installed/the thing it replaced.
// This field is reserved for use by Extension Shell.
typedef struct {
	OSType			resType;									// Resource type of thing
	int				resID;										// Resource ID of thing
	int				codeType;									// Type of this thing
	union			{
					ATrapPatch			theTrapPatch;
					AGestaltSelector	theGestaltSelector;
					AShutdownTask		theShutdownTask;
					AVBLTask			theVBLTask;
					ALowMemFilter		theLowMemFilter;
					ACodeBlock			theCodeBlock;
					ATimeManagerTask	theTimeManagerTask;
					} theCodeThing;								// Specific details for this thing
	Ptr				theAddress;									// RESERVED for Extension Shell
} CodeInfo;




// The ParamBlock. All communication between your code and Extension Shell is done
// through a pointer to one of these structures. Relevent sections are grouped
// together.
typedef struct {
	// General variables. The System Version holds 7.0.0 as 0x0700, 7.0.1 as 0x0701,
	// etc. IsTrapAvailable is provided so that you can test for the presence of
	// traps before attempting to patch them out. UserForcedDisable returns true if
	// the key corresponding to KeyCode (0x38 for Shift, etc) is pressed. If CheckMouse
	// is true, it will also return true if the mouse button is being held down.
	long			systemVersion;
	pascal Boolean	(*IsTrapAvailable)(int trapNum);
	pascal Boolean	(*UserForcedDisable)(short keyCode, Boolean checkMouse);
	
	
	
	// NumIcons contains the amount of valid entries in TheIcons. Set it to 0 if
	// there are no icons, and a value from 1..kMaxNumIcons otherwise. The value
	// in the array corresponds to the resource ID of an Icon Family ('ICN#',
	// 'icl8', and 'icl4'). If the icon can't be found, it won't be plotted.
	// AnimationDelay is the number of Ticks to delay between showing each
	// icon. If you are only showing one icon, set this to 0. By default,
	// Extension Shell initialises NumIcons to 0 and AnimationDelay to 3.
	int				numIcons;
	int				animationDelay;
	int				theIcons[kMaxNumIcons+1];
	
	
	
	// The code that is to be installed is described in TheCodeResources.
	// NumCodeResources contains the amount of valid entries in TheCodeResources,
	// as in NumIcons and TheIcons[]. If an AddressTable is to be installed, set
	// InstallAddressTable to true, and provide the desired Gestalt Selector
	// in AddressTableSelector. In the event of an error, ErrorIndex will contain
	// the entry in TheCodeResources that could not be installed. TheErr will be
	// set to the reason why. By default, Extension Shell initialises
	// numCodeResources to 0.
	Boolean			installAddressTable;
	OSType			addressTableSelector;
	int				numCodeResources;
	CodeInfo		theCodeResources[kMaxNumCodeResources+1];
	int				errorIndex;
	OSErr			theErr;
	
	
	
	// In the event of an error, these variables are used to handle things. If
	// RemoveInstalledCode is set to true, ExtensionShell will attempt to uninstall
	// the code it has already installed. No gaurantee is made that all, if any,
	// of your code will be removed.
	//
	// Setting BeepNow to true will cause ExtensionShell to call SysBeep(30).
	// If PostError is true, a Notification Manager note will be posted.
	// ErrorStringsID  should be set to the id of the 'STR#' resource containing
	// your error strings, and ErrorStringIndex the specific string within this
	// 'STR#' that is to be displayed. The user will get to see the message when
	// the Finder starts handling events.
	Boolean			removeInstalledCode;
	Boolean			beepNow;
	Boolean			postError;
	int				errorStringsID;
	int				errorStringIndex;
	
} ESParamBlock;


#endif
