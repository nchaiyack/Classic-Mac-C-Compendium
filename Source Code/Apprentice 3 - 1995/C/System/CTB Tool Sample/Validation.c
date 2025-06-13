// ===========================================================================
// "Connection Tool Skeleton in C" for the Communications Toolbox
// 
//	Copyright © 1994 Peter J. Creath
//	All Rights Reserved Worldwide
// ===========================================================================

#include "ConnToolCommon.h"

// ===========================================================================
// Function prototypes
// ===========================================================================
extern pascal long	main(ConnHandle hConn, short msg, ConfigPtr *hConfig, long allocate, long procID);
extern long	DoValidate(ConnHandle hConn);
extern void	DoDefault(ConfigPtr *hConfig, Boolean allocate);

// ===========================================================================
// main()
// 	This function is the entry point for the 'cval' resource.  It passes control to the appropriate
// 	subroutines, depending on the incoming message.  This can probably remain unchanged.
// ===========================================================================

pascal long main(ConnHandle hConn, short msg, ConfigPtr *hConfig, long allocate, long procID)
{
long			rtnValue;

	switch (msg)
		{
			case cmValidateMsg:						// hConn is valid here -- check its configuration
				rtnValue = DoValidate(hConn);
				break;
				
			case cmDefaultMsg:						// hConn is not valid here -- allocate and set up the default configuration
				DoDefault(hConfig, allocate);
				rtnValue = FALSE;						// IM-CTB doesn't mention what this should be...
				break;
			
			default:
				rtnValue = cmNotSupported;
		}

return (rtnValue);
}

// ===========================================================================
// DoValidate()
// 	This function is called in response to a cmValidateMsg.  It should check a currently valid
// 	connection to make sure its configuration is OK.  It returns TRUE if it's OK, and FALSE if not.
// ===========================================================================

long	DoValidate(ConnHandle hConn)
{
ConfigPtr		pConfig;
long				rtnValue = FALSE;

	pConfig = (ConfigPtr)((*hConn)->config);			// Dereference the configuration record
	if (pConfig->foobar == 0)
		rtnValue = FALSE;											// It's OK
	else
		{
			DoDefault(&pConfig, FALSE);						// It's bad -- rebuild it
			rtnValue = TRUE;
		}
}

// ===========================================================================
// DoDefault()
// 	This subroutine is called in response to a cmDefaultMsg or an invalid ConfigRecord.  It should
// 	allocate the memory for the ConfigRecord if requested, and should fill in the configuration with
//	the default data.
// ===========================================================================

void	DoDefault(ConfigPtr *hConfig, Boolean allocate)
{
ConfigPtr	pConfig;

	if (allocate)		// Allocate the memory for our ConfigRecord
		{
			pConfig = (ConfigPtr)NewPtr(sizeof(ConfigRecord));
			*hConfig = pConfig;
			// You should probably check for errors here, but Inside Mac CommToolbox doesn't
			// mention how you should handle them... 
		}
	else					// Use the existing ConfigRecord
		{
			pConfig = *hConfig;
		}

	// Fill in the default data here
	pConfig->foobar = 0;
	// etc.
}
