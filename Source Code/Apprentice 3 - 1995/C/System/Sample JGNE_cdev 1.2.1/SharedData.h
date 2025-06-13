/*	SharedData.h
 *
 *	This file contains some defines and data definitions common to both
 *	the extension code and the control panel code.
 */


/* These are the resource number and resource type of the 'preferences' */
/* resource used to store control panel settings. */

#define kCPprefsRsrcType	'pref'	/* the preferences resource type & ID# */
#define kCPprefsRsrcID		-4048

/* These are the resource number and type of the resource that stores */
/* the address of the data structure that the control panel needs access to */

#define kMemAddrType		'memA'	/* shared memory address resource type & ID# */
#define kMemAddrID			-4048


// This is preferences structure accessed by the INIT and the cdev.  It is
// stored as a resource to hold control panel settings.

typedef struct {
	Boolean		On;			/* Is the extension "on" or "off"? */
	Boolean		ShowIcon;	/* Show icon at startup? */
	short		modifiers;	/* modifier keys */
} CPprefsStruct, *CPprefsPtr, **CPprefsHandle;

/* This is the struct used to access the shared memory address resource */

typedef struct {
	long	theAddr;
} **memAddrHdl;

typedef struct {

	/* Variables used by the Control Panel */
	/* These are declared here because the control panel needs to */
	/* share the values with the INIT and/or task code. */
	
	Boolean			CPon;			/* is the control panel switched on or off? */
	short			CPmodifiers;	/* modifier keys set in control panel */
	CPprefsHandle	CPprefsRsrc;	/* holds the prefs rsrc when loaded */
	DialogPtr		CPdlg;			/* holds the CP dialog ptr */
	
	Boolean		INITrun;		/* was the INIT run at startup? */
	
	long		checkValue;		/* check value set by the extension and */
								/* checked by the cdev */

	Boolean		taskInstalled;	/* was the task installed? */

	Boolean		hasColor;		/* does this machine have color QuickDraw? */
	short		sysVersion;		/* system version:  6, 7, or zero */

} myDataStruct, *myDataPtr;

