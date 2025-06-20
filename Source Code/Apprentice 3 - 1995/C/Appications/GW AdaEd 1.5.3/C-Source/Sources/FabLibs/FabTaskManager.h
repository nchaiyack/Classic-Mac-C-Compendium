typedef struct TMInfo TMInfo, *TMInfoPtr;

struct TMInfo {
	QElemPtr	link;
	short	qType;
#ifndef powerc
	short	align;
#endif
	long	parameter;
	void (*tmProc)(long);
//	void (*respProc)(TMInfoPtr);
	Ptr		savedA5;
//	NMRec	tmNotification;
	};

/*
theProc is the procedure you want to schedule for execution.
It must be in a LOCKED segment (it should not move, guys).
theProc CAN move memory; if it does not, call it directly!
theProc must be a "pascal void theProc(long)".
The long passed is the refCon field in the Notification Manager record.
You have to supply a task record previously allocated with _NewPtr.
InstallProc can (and should) be called at interrupt time.
Pass NIL in theProc or theTaskPtr, and the Mac will almost surely HANG.
*/
void InitTaskManager(void);
void PROCCALLEDATINTERRUPTTIME(TMInfoPtr theTaskPtr, long param);
void InitTaskRecord(void (*theProc)(long), TMInfoPtr theTaskPtr);
void CheckCallQueue(void);
