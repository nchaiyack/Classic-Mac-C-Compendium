/* ImportAIFFSample.h */

#ifndef Included_ImportAIFFSample_h
#define Included_ImportAIFFSample_h

/* ImportAIFFSample module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */
/* MainWindowStuff */
/* Files */
/* Alert */
/* BufferedFileInput */
/* SampleConsts */
/* DataMunging */
/* SampleObject */

/* forwards */
struct MainWindowRec;

/* this routine asks for a file and tries to import the contents of that */
/* file as an AIFF sample.  it reports any errors to the user. */
void								ImportAIFFSample(struct MainWindowRec* MainWindow);

#endif
