/***
 * CCooperativeThead.h
 *
 *  Impelement a cooperative thread.  One that must give up the CPU of its own accord.
 *		Copyright � Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/
#pragma once

#include "CThread.h"

class CCooperativeThread : public CThread {
public:
	void		Start (void);						// Start the thread on its way

};