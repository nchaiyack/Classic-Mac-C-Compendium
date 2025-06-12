/***
 * CTestThread.h
 *
 *  Implement some random tests for this puppy...
 *		Copyright © Gordon Watts 1994 (gwatts@fnal.fnal.gov)
 *
 ***/

#include "CCooperativeThread.h"

class CTestThread : public CCooperativeThread {
protected:
	Boolean			quitNow;

	void			DoTest (short testNum);

public:
					CTestThread (void);

	void			ThreadRoutine (void);
};