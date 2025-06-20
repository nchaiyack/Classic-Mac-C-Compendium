/*---------------------------------------------------------------------
	Program: Alert.c
	By: Geoffrey Slinker
	Date: 9:12:36 AM  6/9/92
	Purpose: Contains methods for Alert class, see Alert.h
---------------------------------------------------------------------*/
#include <stdio.h>
#include <string.h>

#include "Alert_Class.h"

void CleanUpMyStuff(void);
/**********************************************************************/
Alert_Class::Alert_Class(void)	// constructor code
{	
	rsrcNumber = 0;
}
/*....................................................................*/
Alert_Class::~Alert_Class(void)	// destructor code
{	
}
/*....................................................................*/
void Alert_Class::ChooseResource(int rsrcNumber)
{
	(*this).rsrcNumber = rsrcNumber;
}
/*....................................................................*/
void Alert_Class::AlertNote(Str255 p0, Str255 p1, Str255 p2, Str255 p3)
{
	int rN;
	
	rN = (*this).rsrcNumber;
	if ( rN == 0  ) {
		SysBeep(7);
		return;
	} /* end if */
	
	ParamText(p0,p1,p2,p3);
	
	NoteAlert(rN,(ProcPtr)NULL);
	
	ParamText("\p","\p","\p","\p");
}
/*....................................................................*/
void Alert_Class::AlertCaution(Str255 p0, Str255 p1, Str255 p2, Str255 p3)
{
	int rN;
	
	rN = (*this).rsrcNumber;
	if ( rN == 0  ) {
		SysBeep(7);
		return;
	} /* end if */
	
	ParamText(p0,p1,p2,p3);
	
	CautionAlert(rN,(ProcPtr)NULL);

	ParamText("\p","\p","\p","\p");
}
/*....................................................................*/
void Alert_Class::AlertStop(Str255 p0, Str255 p1, Str255 p2, Str255 p3)
{
	int rN;
	
	rN = (*this).rsrcNumber;
	if ( rN == 0  ) {
		SysBeep(7);
		return;
	} /* end if */
	
	ParamText(p0,p1,p2,p3);
	
	StopAlert(rN,(ProcPtr)NULL);

	ParamText("\p","\p","\p","\p");
	
	CleanUpMyStuff();
	ExitToShell();
}