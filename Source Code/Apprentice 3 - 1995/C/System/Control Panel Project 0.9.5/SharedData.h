/*
	SharedData.h
	
	Data structure shared by the extension and the patch code.
*/

/* This is a struct that the shared data holds a handle to */

typedef struct {
	Boolean		On;			/* Is the extension "on" or "off"? */
	Boolean		ShowIcon;	/* Show icon at startup? */
} CPprefsStruct, *CPprefsPtr, **CPprefsHandle;

/* This is the actual shared data structure */

typedef struct {

	Boolean		inUse;			/* This should be set to true when a piece of code */
								/* is using the data structure, then back to false */
								/* when it's through.  Each piece of code that     */
								/* accesses the structure should wait until the    */
								/* flag is false before using the data structure.  */

	/* Variables used by the Trap Patch */
	
	UniversalProcPtr	oldTrap;/* addr of old trap routine (before our patch) */
	long		paramBytes;		/* how many bytes taken up by parameters */
	long		returnBytes;	/* how many bytes taken by return value (can be 0) */
	
	/* Variables used by the Extension and Control Panel */
	
	long		checkValue;		/* set by extension, then checked by cdev to make */
								/* sure extension loaded at startup time. */
	Boolean		patched;		/* was the patch installed? */
	
	/* Variables used by the Control Panel */
	
	DialogPtr	CPdialogPtr;	/* a pointer to the control panel dialog */
	short		CPitems;		/* number of items in the dialog */
	CPprefsHandle	CPprefsRsrc;	/* handle to a prefs struct (saved as a resource) */
	Boolean		CPon;			/* is the control panel function on or off? */
	
	/* Memory used for notifications */
	
	NMRec			nm;				/* a pointer to a Notification Manager record */
	Str255			str;			/* a Pascal string */
	
	/* Whatever other variables we need to share */
	
		
} myDataStruct, *myDataPtr, **myDataHandle;

/* This is the value the extension puts into the checkValue field of the shared
 * data structure and that the control panel checks for when it is opened.
 */

#define kCheckValue 0x12341234
