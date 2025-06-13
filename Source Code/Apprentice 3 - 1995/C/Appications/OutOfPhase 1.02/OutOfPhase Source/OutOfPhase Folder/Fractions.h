/* Fractions.h */

#ifndef Included_Fractions_h
#define Included_Fractions_h

/* Fractions module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* Factoring */

/* note: this module does NOT deal with negative numbers!!! */

typedef struct FractionRec
	{
		unsigned long		Integer;
		unsigned long		Fraction;
		unsigned long		Denominator;
	} FractionRec;

/* convert a decimal number to a fraction with the specified denominator limit */
void								Double2Fraction(double Value, unsigned long Denominator,
											FractionRec* Fraction);

/* convert fraction to a double */
double							Fraction2Double(FractionRec* Fraction);

/* add fractions.  Destination fraction can be one of the source fractions */
void								AddFractions(FractionRec* Left, FractionRec* Right, FractionRec* Dest);

/* test to see if the left is greater than the right */
MyBoolean						FracGreaterThan(FractionRec* Left, FractionRec* Right);

/* test to see if the left is greater than or equal to the right */
MyBoolean						FracGreaterEqual(FractionRec* Left, FractionRec* Right);

/* test fractions for equality */
MyBoolean						FractionsEqual(FractionRec* Left, FractionRec* Right);

/* reduce fraction */
void								ReduceFraction(FractionRec* Frac);

/* multiply fractions.  destination can be one of the sources */
/* this function will fail on numbers considerably smaller than the */
/* range of representable fractions. */
void								MultFractions(FractionRec* Left, FractionRec* Right, FractionRec* Dest);

/* subtract second fraction from first.  Destination can be one of the sources */
void								SubFractions(FractionRec* Left, FractionRec* Right, FractionRec* Dest);

#endif
