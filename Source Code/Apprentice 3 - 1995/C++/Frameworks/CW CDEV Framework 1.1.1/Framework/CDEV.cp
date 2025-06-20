#include "CDEV.h"

// prototypes for 2 required procedures
long runable(void);
TControlPanel *makeCDEV(short numItems,DialogPtr cp);

// return 1 if we can run, otherwise 0
long runable(void)
{
	return 1;
}

// code that allocates our cdev
TControlPanel *makeCDEV(short numItems,DialogPtr cp)
{
	return new myCDEV(numItems,cp);
}

// constructor for our cdev
myCDEV::myCDEV(short numItems,DialogPtr cp) : TControlPanel(numItems,cp)
{
}

// destructor for our cdev
myCDEV::~myCDEV(void)
{
}

