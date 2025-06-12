/* StartupOpen.h */

#ifndef Included_StartupOpen_h
#define Included_StartupOpen_h

/* StartupOpen module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* Array */
/* Files */

/* this is so we don't have to drag Files.h in the header. */
struct FileSpec;

/* compile a list of files that should be opened when the program starts up. */
/* The parameters should be exactly the ones passed into main() upon startup. */
/* It is implementation defined as to whether they will be used; the Macintosh */
/* does not use them, but uses Apple Events for opening startup documents instead. */
void					PrepareStartupDocuments(int argc, char* argv[]);

/* Get a startup item.  It will initially return False.  Once the open event is */
/* received, it will return True from then on.  If there is a file specification */
/* to get, it will be returned, otherwise NIL will be returned.  The file */
/* specification should be disposed of with DisposeFileSpec.  Here's how you know if */
/* you should open an untitled document:  the first time it returns True, if it */
/* also returns NIL, then do it. */
MyBoolean			GetStartupObject(struct FileSpec** ReturnStuff);

/* clean up any internal structures allocated by PrepareStartupDocument. */
void					ClearStartupDocuments(void);

/* this returns True if the system would like the program to quit.  The program */
/* should then ask the user if he wants to save all changed documents. */
/* A Quit event on the Macintosh will cause this to return True */
MyBoolean			CheckQuitPending(void);

/* If some other signal besides an implementation defined system quit signal */
/* is received, this can be used to indicate such, and cause a normal shutdown */
/* of the program to occur. */
void					SetQuitPending(void);

/* If the user cancels the quit, this should be used to clear the flag and */
/* allow the program to continue running */
void					AbortQuitInProgress(void);

#endif
