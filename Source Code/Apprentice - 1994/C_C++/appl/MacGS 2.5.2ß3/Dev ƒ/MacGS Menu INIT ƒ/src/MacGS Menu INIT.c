#include <SetUpA4.h>
#include <Traps.h>


static pascal long (*saveMenuSelect) (Point startPt);
static pascal long menuSelectPatch (Point startPt);


	pascal void
main (void)

{
	Handle	h;


	//	Make our INIT code persistent

	RememberA0 ();
	SetUpA4 ();

	asm
	{
		dc.w	_RecoverHandle
		move.l	A0, h
	}
	HLock (h);
	DetachResource (h);

	//	Patch MenuChoice's code

	saveMenuSelect = (void *) NGetTrapAddress (_MenuSelect, ToolTrap);
	NSetTrapAddress ((long) menuSelectPatch, _MenuSelect, ToolTrap);

	RestoreA4 ();
}


	static pascal long
menuSelectPatch (Point startPt)

{
	short	saveResFile;
	long	retVal;


	SetUpA4 ();

	saveResFile = CurResFile ();			//	save the current resource file
	retVal = (*saveMenuSelect) (startPt);	//	do the old patch code
	UseResFile (saveResFile);				//	restore the old value

	RestoreA4 ();

	return retVal;
}
