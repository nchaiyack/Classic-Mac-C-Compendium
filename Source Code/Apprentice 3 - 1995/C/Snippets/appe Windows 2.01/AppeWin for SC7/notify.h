// File "notify.h" - 

#ifndef ____NOTIFY_HEADER____
#define ____NOTIFY_HEADER____

// * ****************************************************************************** *
// * ****************************************************************************** *

#define kNotifyStringList		128
#define kSys71NotifyString		  1
#define kTSMFixNotifyString		  2
#define kJGNENotifyString		  3

// * ****************************************************************************** *
// * ****************************************************************************** *
// Function Prototypes

void UserNotify(short notifyList, short notifyIndex, Boolean exit);
pascal void CompleteNotify(NMRecPtr notify);

#endif  ____NOTIFY_HEADER____

