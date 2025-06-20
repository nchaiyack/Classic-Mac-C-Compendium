/* CheckNameUniqueness.h */

#ifndef Included_CheckNameUniqueness_h
#define Included_CheckNameUniqueness_h

/* CheckNameUniqueness module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* MainWindowStuff */
/* Memory */
/* SampleList */
/* AlgoSampList */
/* WaveTableList */
/* AlgoWaveTableList */
/* InstrList */
/* SampleObject */
/* AlgoSampObject */
/* WaveTableObject */
/* AlgoWaveTableObject */
/* InstrObject */
/* Array */
/* DataMunging */
/* Alert */

/* forwards */
struct MainWindowRec;

/* this checks all of the objects to make sure that names are unique */
/* it returns True if they are, or returns False and presents a warning dialog */
/* if they aren't unique */
MyBoolean						CheckNameUniqueness(struct MainWindowRec* MainWindow);

#endif
