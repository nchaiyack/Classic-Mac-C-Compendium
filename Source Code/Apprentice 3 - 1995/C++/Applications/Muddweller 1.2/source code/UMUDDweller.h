/* UMUDDweller - Implementation of TMUDDwellerApp                             */

#ifndef __UMUDDWELLER__
#define __UMUDDWELLER__


		// ¥ MacApp
#ifndef __UMacApp__
#include "UMacApp.h"
#endif


//------------------------------------------------------------------------------

class TMUDDwellerApp: public TApplication {
public:
	virtual pascal void BuildReserves (TDocument *preserve);
	virtual pascal void Close (void);
	virtual pascal void DispatchEvent (EventInfo *theEventInfo,
		struct TCommand **commandToPerform);
	virtual pascal TCommand *DoCommandKey (short ch, EventInfo *info);
	virtual pascal TDocument *DoMakeDocument (CmdNumber itsCmdNumber);
	virtual pascal void DoSetupMenus (void);
	virtual pascal Boolean DropSome (TDocument *preserve);
	virtual pascal void Idle (IdlePhase phase);
	virtual pascal void IMUDDwellerApp (void);
	virtual pascal struct TCommand *MenuEvent (long menuItem);
	virtual pascal void SFGetParms (CmdNumber itsCmdNumber, short *dlgID,
		Point *where, Ptr *fileFilter, Ptr *dlgHook, Ptr *filterProc,
		TypeListHandle typeList);
	virtual pascal Boolean TrackCursor (void);
};

#endif
