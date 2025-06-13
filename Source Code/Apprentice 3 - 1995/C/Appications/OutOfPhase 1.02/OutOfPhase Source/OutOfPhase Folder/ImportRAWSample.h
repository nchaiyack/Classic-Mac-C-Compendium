/* ImportRAWSample.h */

#ifndef Included_ImportRAWSample_h
#define Included_ImportRAWSample_h

/* ImportRAWSample module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* MainWindowStuff */
/* Files */
/* Alert */
/* Memory */
/* Screen */
/* TextEdit */
/* RadioButton */
/* SimpleButton */
/* Numbers */
/* Main */
/* Menus */
/* EventLoop */
/* SampleConsts */
/* BufferedFileInput */
/* SampleObject */

/* forwards */
struct MainWindowRec;

/* this routine gets a file and imports it according to some generic parameters */
/* that the user can specify.  any errors are handled by the routine. */
void									ImportRAWSample(struct MainWindowRec* MainWindow);

#endif
