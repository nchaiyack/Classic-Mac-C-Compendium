/* 64BitMath.h */

#ifndef Included_64BitMath_h
#define Included_64BitMath_h

/* 64BitMath module depends on */
/* MiscInfo.h */
/* Audit */
/* Debug */
/* Definitions */


#if defined(__GNUC__) /* || sizeof(long) == 8 */


#if defined(__GNUC__)
	/* this is the GNU stuff */
	typedef long long int LongLongRec;
#else
	/* if sizeof(long) == 8 then you can enable this */
	typedef long int LongLongRec;
#endif

/* initialize a longlong with a double precision floating point value */
#define Double2LongLong(dbl,lnglng) ((void)((lnglng)\
					= (LongLongRec)((dbl) * 4294967296.0L)))

/* convert a longlong to a double precision floating point value */
#define LongLong2Double(lnglng) ((double)(lnglng) / 4294967296.0L)

/* get the low order unsigned 32 bits from the longlong */
#define LongLongLowHalf(lnglng) ((unsigned long)((lnglng) & 0x00000000ffffffff))

/* get the high order signed 32 bits from the longlong */
#define LongLongHighHalf(lnglng) ((signed long)((lnglng) >> 32))

/* mask the upper word */
#define LongLongMaskHighHalf(lnglng,mask32) ((void)((lnglng) = ((((LongLongRec)(mask32))\
					<< 32) | 0x00000000ffffffff) & (lnglng)))

/* store a new value into the upper word */
#define LongLongSetHighHalf(lnglng,newhi) ((void)((lnglng) = (((LongLongRec)(newhi))\
					<< 32) | ((lnglng) & 0x00000000ffffffff)))

/* add two longlongs:  *L = *L + *R */
#define LongLongAdd(L,R) ((void)(*(L) = *(L) + *(R)))


#else


/* stuff for systems with only 32 bit ints */

/* set this to 1 to always use the generic add */
#define FORCEGENERIC64BITADD (0)

/* this is publicly visible for speed */
struct LongLongRec
	{
		unsigned long				Low32Bits;
		signed long					High32Bits;
	};

typedef struct LongLongRec LongLongRec;

/* initialize a longlong with a double precision floating point value */
#define Double2LongLong(dbl,lnglng) (void)((lnglng).High32Bits = (dbl),\
					(lnglng).Low32Bits = ((dbl) - ((lnglng).High32Bits)) * 4294967296.0L)

/* convert a longlong to a double precision floating point value */
#define LongLong2Double(lnglng) ((lnglng).High32Bits\
					+ (lnglng).Low32Bits / 4294967296.0L)

/* get the low order unsigned 32 bits from the longlong */
#define LongLongLowHalf(lnglng) ((lnglng).Low32Bits)

/* get the high order signed 32 bits from the longlong */
#define LongLongHighHalf(lnglng) ((lnglng).High32Bits)

/* mask the upper word */
#define LongLongMaskHighHalf(lnglng,mask32) ((void)((lnglng).High32Bits &= (mask32)))

/* store a new value into the upper word */
#define LongLongSetHighHalf(lnglng,newhi) ((void)((lnglng).High32Bits = (newhi)))

/* add two longlongs:  *L = *L + *R */
#if ((PROC68020 == CURRENTPROCTYPE) || (PROC68000 == CURRENTPROCTYPE)) && defined(THINK_C) && !FORCEGENERIC64BITADD
	/* 00000000: 2019               MOVE.L    (A1)+,D0 */
	/* 00000002: D198               ADD.L     D0,(A0)+ */
	/* 00000004: 2211               MOVE.L    (A1),D1 */
	/* 00000006: 2010               MOVE.L    (A0),D0 */
	/* 00000008: D181               ADDX.L    D1,D0 */
	/* 0000000A: 2080               MOVE.L    D0,(A0) */
	#pragma parameter LongLongAdd(__A0,__A1)
	void							LongLongAdd(LongLongRec* L, LongLongRec* R)
		= {0x2019, 0xD198, 0x2211, 0x2010, 0xD181, 0x2080};
#else
	void							LongLongAdd(LongLongRec* L, LongLongRec* R);
	#define ENABLEGENERICLONGLONG
#endif


#endif


#endif
