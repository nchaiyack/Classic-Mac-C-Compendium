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

// Declare a Guide for Passing the Filter Helper, for Universal Headers
#if !GENERATINGPOWERPC
typedef void (*FilterHelperProcPtr)(EventRecord *theEvent, Ptr helperData);
typedef FilterHelperProcPtr FilterHelperUPP;
enum {
	uppFilterHelperProcInfo = 0
	};
#define NewFilterHelperProc(proc)	(FilterHelperUPP)(proc)
#else
typedef UniversalProcPtr FilterHelperUPP;
enum {
	uppFilterHelperProcInfo = kCStackBased
		 | STACK_ROUTINE_PARAMETER(1, SIZE_CODE(sizeof(EventRecord *)))
		 | STACK_ROUTINE_PARAMETER(2, SIZE_CODE(sizeof(Ptr)))
	};
#define NewFilterHelperProc(proc) (FilterHelperUPP) \
		NewRoutineDescriptor((ProcPtr)(proc), uppFilterHelperProcInfo, GetCurrentISA())
#endif GENERATINGPOWERPC

// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

Ptr InstallEventFilter(FilterHelperUPP helperProc, Ptr helperData);
Ptr ReleaseEventFilter(Ptr filterProc);

void EventFilterHelper(EventRecord *theEvent, Ptr helperData);

#endif  ____FILTER_HEADER____

