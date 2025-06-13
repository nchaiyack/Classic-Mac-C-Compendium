/* Factoring.h */

#ifndef Included_Factoring_h
#define Included_Factoring_h

/* Factoring module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

struct FactorRec;
typedef struct FactorRec FactorRec;

/* initialize internal data structures used by factorer */
MyBoolean							InitializeFactoring(void);

/* dispose of factor stuff */
void									ShutdownFactoring(void);

/* dispose factor list */
void									DisposeFactorList(FactorRec* List);

/* find the common factors of two numbers.  the product of the common factors */
/* is returned (i.e. greatest common factor). */
unsigned long					FindCommonFactors(unsigned long Left, unsigned long Right);

#endif
