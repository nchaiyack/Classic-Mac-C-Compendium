/***
 * CThreadManager.h
 *
 *  Thread manager interface
 *		Copyright � Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#pragma once

#include "CGWObject.h"

class CMainThread;

class CThreadManager : public CGWObject {
protected:
	CMainThread	*theMainThread;

public:

	/** COnstructors/ destructors **/
	
	CThreadManager (void);
	~CThreadManager (void);
};