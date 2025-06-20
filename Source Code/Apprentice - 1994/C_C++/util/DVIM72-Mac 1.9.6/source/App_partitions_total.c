#include <Processes.h>

long	App_partitions_total( void );
Boolean TrapAvailable( short trapnum );

#ifndef GESTALT
#define GESTALT			0xA1AD
#endif

long	App_partitions_total( void )
{
	ProcessInfoRec			info;
	ProcessSerialNumber		psn;
	long					total;
	long					response;
	
	if ( !TrapAvailable(GESTALT) ||
		(Gestalt(gestaltOSAttr, &response) != noErr) ||
		!(response & (1L << gestaltLaunchControl)) )
		return 0L;
	total = 0L;
	info.processName = NIL;
	info.processAppSpec = NIL;
	info.processInfoLength = sizeof(ProcessInfoRec);
	psn.highLongOfPSN = 0L;
	psn.lowLongOfPSN = kNoProcess;
	while ( GetNextProcess(&psn) == noErr )
	{
		if ( GetProcessInformation(&psn, &info) == noErr)
			total += info.processSize;
	}
	return total;
}