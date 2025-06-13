// File "filter.h" - 

#ifndef ____FILTER_HEADER____
#define ____FILTER_HEADER____

// * ****************************************************************************** *

// Information on the program's precompiled jGNEFilter
#define kJGNEFilterResType		'jGNE'
#define kJGNEFilterResID		128

// Constant offsets into jGNEFilter for inline data
#define kNextFilterOffset		0x02
#define kEventHelperOffset		0x06
#define kEventHelperDataOffset	0x0A

// Declare a Guide for Passing the Filter Helper
typedef void (*FilterHelperProcPtr)(EventRecord *theEvent, Ptr helperData);

pascal ProcPtr GetJGNEFilter(void) = { 0x2EB8, 0x029A };
pascal void SetJGNEFilter(ProcPtr) = { 0x21DF, 0x029A };

// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

Ptr InstallEventFilter(FilterHelperProcPtr helperProc, Ptr helperData);
Ptr ReleaseEventFilter(Ptr filterProc);

void EventFilterHelper(EventRecord *theEvent, Ptr helperData);

#endif  ____FILTER_HEADER____

