/* ImportWAVSample.h */

#ifndef Included_ImportWAVSample_h
#define Included_ImportWAVSample_h

/* ImportWAVSample module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* MainWindowStuff */
/* Files */
/* Alert */
/* BufferedFileInput */
/* SampleConsts */
/* Memory */
/* DataMunging */
/* SampleObject */

/* forwards */
struct MainWindowRec;

/* this routine asks for a file and tries to import the contents of that */
/* file as a WAV sample.  it reports any errors to the user. */
void								ImportWAVSample(struct MainWindowRec* MainWindow);

#endif
