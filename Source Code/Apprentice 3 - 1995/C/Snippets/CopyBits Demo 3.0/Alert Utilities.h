//
// Alert Utilities.h v1.0		requires System 7 or later
//
// by Kenneth Worley
// Public Domain
//
// Contact me at:   America Online: KNEworley
// 			internet: KNEworley@aol.com or kworley@fascination.com
//
// Routines that make it easier to show a quick error or
// informational dialog.
//

#ifndef Alert_Utilities_h
#define Alert_Utilities_h

#ifdef __cplusplus
extern "C" {
#endif

// DoAlert shows the dialog specified and waits for the user to press
// the OK button (item #1). Then the dialog goes away. If playAlert is
// true, DoAlert plays the system beep.
void	DoAlert( short dialogID, Boolean playAlert );

// DoAlertStrID shows the dialog specified just like DoAlert, but
// also puts the string stored in STR resource errStrID into the dialog's
// text field (item #2).
void	DoAlertStrID( short dialogID, Boolean playAlert, short errStrID );

// DoAlertStrIndexID does the same thing as DoAlertStrID but gets its
// string from a resource of type STR# with an index number.
void	DoAlertStrIndexID( short dialogID, Boolean playAlert,
			short errStrID, short index );

// DoAlertStr shows a dialog just like DoAlert but puts the string
// specified into the dialog's text field (item #2).
void	DoAlertStr( short dialogID, Boolean playAlert, Str255 errString );

#ifdef __cplusplus
}
#endif

#endif // Alert_Utilities_h