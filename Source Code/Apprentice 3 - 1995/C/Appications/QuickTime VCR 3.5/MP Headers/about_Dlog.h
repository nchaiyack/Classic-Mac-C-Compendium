/*---------------------------------------------------------------------
	Program: about_Dlog.h
	By: Geoffrey Slinker
	Date: 12:08:10 PM  5/2/92
	Purpose: contains the dialog subclasses for the tweener application
---------------------------------------------------------------------*/
#pragma once
#include "Dlog.h"

#define ABOUT_DLOG		600

/*....................................................................*/
class about_Dlog : public Dlog {
	public:
	virtual void HandleDialog(EventRecord *aEvent);
};
