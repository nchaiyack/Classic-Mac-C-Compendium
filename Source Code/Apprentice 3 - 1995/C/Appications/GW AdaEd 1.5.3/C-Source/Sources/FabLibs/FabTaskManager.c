#include	"FabTaskManager.h"

enum {
fabQueueType = 25
};

//static NMUPP sNMUPP = nil;
static QHdr	sQueueHdr = { 0, nil, nil};

static void myResponse(TMInfoPtr theTaskPtr);

void myResponse(TMInfoPtr theTaskPtr)
{
void (*proc)(long);
long	oldA5, param;

oldA5 = SetA5((long)theTaskPtr->savedA5);
(void) Dequeue((QElemPtr)theTaskPtr, &sQueueHdr);
param = theTaskPtr->parameter;
proc = theTaskPtr->tmProc;
DisposePtr((Ptr)theTaskPtr);
proc(param);
(void) SetA5(oldA5);
}

void InitTaskManager(void)
{
//sNMUPP = NewNMProc(myResponse);
}

void PROCCALLEDATINTERRUPTTIME(TMInfoPtr theTaskPtr, long param)
{
long	oldA5;

theTaskPtr->parameter = param;
oldA5 = SetA5((long)theTaskPtr->savedA5);
(void) Enqueue((QElemPtr)theTaskPtr, &sQueueHdr);
(void) SetA5(oldA5);
}

void InitTaskRecord(void (*theProc)(long), TMInfoPtr theTaskPtr)
{
theTaskPtr->qType = fabQueueType;
theTaskPtr->tmProc = theProc;
//theTaskPtr->respProc = myResponse;
theTaskPtr->savedA5 = LMGetCurrentA5();
}

void CheckCallQueue(void)
{
TMInfoPtr	head;

while (head = (TMInfoPtr)sQueueHdr.qHead)
	myResponse(head);
}
