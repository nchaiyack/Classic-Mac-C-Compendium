/*---------------------------------------------------------------------
	Program: Alert.h
	By: Geoffrey Slinker
	Date: 11:57:23 AM  5/2/92
	Purpose: Contains definitions of the superclass Alert
---------------------------------------------------------------------*/
#pragma once

/**********************************************************************/
/*-------------------------
Class Definition for my Alert
which is a Alert Class.
-------------------------*/
class Alert_Class {
	protected:
		int rsrcNumber;
		
	public:
		Alert_Class(void);	// constructor
		~Alert_Class(void);
		virtual void ChooseResource(int rsrcNumber);
		virtual void AlertNote(Str255 p0, Str255 p1, Str255 p2, Str255 p3);
		virtual void AlertCaution(Str255 p0, Str255 p1, Str255 p2, Str255 p3);
		virtual void AlertStop(Str255 p0, Str255 p1, Str255 p2, Str255 p3);
}; // class Alert_Class