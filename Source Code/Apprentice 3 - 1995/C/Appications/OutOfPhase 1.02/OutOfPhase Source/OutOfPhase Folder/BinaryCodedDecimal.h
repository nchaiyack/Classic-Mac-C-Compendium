/* BinaryCodedDecimal.h */

#ifndef Included_BinaryCodedDecimal_h
#define Included_BinaryCodedDecimal_h

/* BinaryCodedDecimal module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* DataMunging */
/* Memory */

/* small BCD format is 4 digits of bcd: [+-]xX.XXX, represented as an integer value */
/* that is 1000 (one thousand) times the decimal value (milli-unit precision). */
/* It's range is -29.999 to 29.999 */
typedef short SmallBCDType;
#define SMALLBCDPRECISION (1000)

/* small BCD format is 9 digits of bcd: [+-]xXXX.XXXXXX, represented as an integer */
/* value that is 1 000 000 (one million) times the decimal value (micro-unit precision) */
/* It's range is -1999.999999 to 1999.999999 */
typedef long LargeBCDType;
#define LARGEBCDPRECISION (1000000)

/* small extended BCD format is 8 digits: [+-]xXXXXXX.XXX, represented as an integer */
/* value that is 1000 (one thousand) times the decimal value. */
/* It's range is -1999999.999 to 1999999.999 */
typedef long SmallExtBCDType;
#define SMALLEXTBCDPRECISION (1000)

/* convert small BCD number to double precision floating point number. */
#define SmallBCD2Double(x) ((double)(x) / SMALLBCDPRECISION)
#define SmallBCD2Single(x) ((float)(x) / SMALLBCDPRECISION)

/* convert double precision number to small BCD number.  Rounding is towards */
/* nearest. (0.4 ==> 0, 0.6 ==> 1, -0.4 ==> 0, -0.6 ==> -1) */
/* value is constrained if it is out of range */
SmallBCDType			Double2SmallBCD(double Value);

/* convert large BCD number to double precision */
#define LargeBCD2Double(x) ((double)(x) / LARGEBCDPRECISION)
#define LargeBCD2Single(x) ((float)(x) / LARGEBCDPRECISION)

/* convert double precision number to large BCD number. */
LargeBCDType			Double2LargeBCD(double Value);

/* convert small extended BCD number to double precision */
#define SmallExtBCD2Double(x) ((double)(x) / SMALLEXTBCDPRECISION)
#define SmallExtBCD2Single(x) ((float)(x) / SMALLEXTBCDPRECISION)

/* convert double precision to small extended BCD number */
SmallExtBCDType		Double2SmallExtBCD(double Value);

/* convert small BCD number to string */
char*							SmallBCDToString(SmallBCDType Number);

/* convert large BCD number to string */
char*							LargeBCDToString(LargeBCDType Number);

/* convert small extended BCD number to string */
char*							SmallExtBCDToString(SmallExtBCDType Number);

/* more conversions */
#define SmallExtBCD2LargeBCD(x) ((x) * (LARGEBCDPRECISION / SMALLEXTBCDPRECISION))

#endif
