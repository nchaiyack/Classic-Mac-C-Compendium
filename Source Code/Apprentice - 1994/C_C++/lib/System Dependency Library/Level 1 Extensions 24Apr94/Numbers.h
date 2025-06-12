/* Numbers.h */

#ifndef Included_Numbers_h
#define Included_Numbers_h

/* Numbers module depends on: */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Memory */

/* these routines work on NON null terminated strings */

/* convert a string to a long double.  string is not null terminated, that's why */
/* the length value is required. */
long double			StringToLongDouble(char* String, long Length);

/* convert a long double to a string.  If the absolute value of Value is outside */
/* of LowBound and HighBound, then it will be in scientific notation */
/* a heap block is allocated, so NIL is returned if it fails due to lack of memory. */
char*						LongDoubleToString(long double Value, int NumDigits,
									long double LowBound, long double HighBound);

/* convert a string to an integer.  the string is not null terminated, so */
/* the length value is required */
long						StringToInteger(char* String, long Length);

/* convert an integer to a string */
/* a heap block is allocated, so NIL is returned if it fails due to lack of memory. */
char*						IntegerToString(long Integer);

#endif
