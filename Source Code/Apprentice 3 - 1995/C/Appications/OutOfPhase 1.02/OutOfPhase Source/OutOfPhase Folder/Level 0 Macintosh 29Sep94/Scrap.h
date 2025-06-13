/* Scrap.h */

#ifndef Included_Scrap_h
#define Included_Scrap_h

/* Scrap module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

/* scrap handling routines only work with text.  Arbitrary blocks of data are */
/* treated as text. */

/* initialize or shutdown any stuff needed to handle systemwide data scraps */
/* these are internal routines used only by the Screen module */
MyBoolean		Eep_InitializeScrapHandler(void);
void				Eep_ShutdownScrapHandler(void);

/* get a block containing a copy of the scrap */
/* if a block couldn't be allocated, then it returns NIL */
char*				GetCopyOfScrap(void);

/* make a copy of the block and put the data into the scrap */
/* returns True if successful */
MyBoolean		SetScrapToThis(char* DataToCopy);

#endif
