/* ErrorDaemon */

#ifndef Included_ErrorDaemon_h
#define Included_ErrorDaemon_h

/* ErrorDaemon module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

struct ErrorDaemonRec;
typedef struct ErrorDaemonRec ErrorDaemonRec;

/* get a new error daemon */
ErrorDaemonRec*			NewErrorDaemon(void);

/* dispose of the error daemon */
void								DisposeErrorDaemon(ErrorDaemonRec* Daemon);

/* report a new clamping value.  value should be positive and greater than 1 */
void								ErrorDaemonReportClamping(ErrorDaemonRec* Daemon, float ClampedValue);

/* return True if clamping occurred */
MyBoolean						ErrorDaemonDidClampingOccur(ErrorDaemonRec* Daemon);

/* obtain the maximum clamping value */
float								ErrorDaemonGetMaxClamping(ErrorDaemonRec* Daemon);

#endif
