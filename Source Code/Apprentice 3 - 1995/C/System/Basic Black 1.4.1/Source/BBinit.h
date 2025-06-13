#define DELAYTICKS	45		/* = three-quarters of a second */

/*********************************************************************
 *
 * Screen saver commands passed to the SAVC selector
 *
 *********************************************************************/
enum SaverCommand {
	eSaverWakeUp,		/*  Make Basic Black wake up. */
	eSaverSleep,		/*  Make Basic Black go to sleep. */
	eSaverOn,			/*  Turn Basic Black on.  */
	eSaverOff,			/*  Turn Basic Black off. */
	eForceShort = 257	/* force enums to be shorts instead of bytes */
};



/*********************************************************************
 *
 * Function Prototypes
 *
 *********************************************************************/
void main(void);
void StartPatchCode(void);
pascal short MySystemEvent(EventRecord *);
pascal void MyInitCursor(void);
pascal void MyDrawMenuBar(void);
pascal void MyEraseRect(Rect *);
pascal void MyEraseOval(Rect *);
pascal void MyEraseRgn(RgnHandle);
pascal void MySystemTask(void);
pascal void BBlkSelector(OSType, long *);
pascal void SAVRSelector(OSType, long *);
pascal void SAVCSelector(OSType, long *);
pascal OSErr SaverControl(short);
void FallAsleep(void);
void WakeUp(void);
short abs(short);
void DrawClock(GrafPtr);
pascal void RemoveICPatch(void);
void EndPatchCode(void);