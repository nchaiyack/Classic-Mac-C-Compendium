//
// kForeSleepValue or kBackSleepValue are passed to WaitNextEvent as the numTicks
// parameter. They determine how 'friendly' the dropbox is to other applications.
//
// kFirstSleepInterval, kForeSleepInterval and kBackSleepInterval are used during
// directory traversal (i.e. during the handling of ODOC events)
// They are the minimum intervals (in ticks) between successive calls of
// WaitNextEvent during such a traversal.
// kFirstSleepInterval determines the number of ticks to wait before the first
// call of WaitNextEvent during a directory traversal. The idea here is to never
// call WaitNextEvent during 'short' traversals to obtain maximum speed, and to call
// it every now and then during 'longer' traversals for added user-friendliness.
//
// We have two values for both of these parameters, one to use when we are the
// foreground process, one to use when we are not the foreground process.
//
// If no directory traversal is in progress WaitNextEvent is simply called as
// often as possible in an attempt to give as much time to other apps as is
// possible.
//
// Note: if your 'OpenDoc' member function takes a long time to process individual
// files it probably is a good idea to call 'TimeForTea' every now and then
// during the processing. See 'boxmaker.h' for details.
//
#define kForeSleepValue		10
#define kBackSleepValue		60

#define kFirstSleepInterval	60
#define kForeSleepInterval	60
#define kBackSleepInterval	12
//
// miscellaneous constants
// (must be usable for both C++ and Rez sources, so no 'enums' are used)
//
#define kMBarID				128

#define kAppleMenuID		128
#define kFileMenuID			129

#define kSelectFileItem		  1
#define kPrefsItem			  2
#define kQuitItem			  4

#define kErrorAlertID		200
#define kAboutAlertID		256
#define kSettingsDialogID	128
//
// STR# resource related:
//
#define kErrStringID		128

#define kCantRunErr			  1
#define kAEVTErr			  2
