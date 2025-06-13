/* FloatingPoint.h */

#ifndef Included_FloatingPoint_h
#define Included_FloatingPoint_h

/* FloatingPoint module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */
/* SANE */

long double				XGETPI(void);

long double				XATAN(long double X);
long double				XCOS(long double X);
long double				XEXP(long double X);
long double				XFABS(long double X);
long double				XLN(long double X);
long double				XSIN(long double X);
long double				XSQRT(long double X);
long double				XTAN(long double X);
long double				XPOWER(long double X, long double Y);
long double				XACOS(long double X);
long double				XASIN(long double X);

MyBoolean					ISNAN(long double X);


#define DGETPI() ((double)XGETPI())

#define DATAN(x) ((double)XATAN((x)))
#define DCOS(x) ((double)XCOS((x)))
#define DEXP(x) ((double)XEXP((x)))
#define DFABS(x) ((double)XFABS((x)))
#define DLN(x) ((double)XLN((x)))
#define DSIN(x) ((double)XSIN((x)))
#define DSQRT(x) ((double)XSQRT((x)))
#define DTAN(x) ((double)XTAN((x)))
#define DPOWER(x,y) ((double)XPOWER((x),(y)))
#define DACOS(x) ((double)XACOS((x)))
#define DASIN(x) ((double)XASIN((x)))


#define FGETPI() ((float)XGETPI())

#define FATAN(x) ((float)XATAN((x)))
#define FCOS(x) ((float)XCOS((x)))
#define FEXP(x) ((float)XEXP((x)))
#define FFABS(x) ((float)XFABS((x)))
#define FLN(x) ((float)XLN((x)))
#define FSIN(x) ((float)XSIN((x)))
#define FSQRT(x) ((float)XSQRT((x)))
#define FTAN(x) ((float)XTAN((x)))
#define FPOWER(x,y) ((float)XPOWER((x),(y)))
#define FACOS(x) ((float)XACOS((x)))
#define FASIN(x) ((float)XASIN((x)))

#endif
