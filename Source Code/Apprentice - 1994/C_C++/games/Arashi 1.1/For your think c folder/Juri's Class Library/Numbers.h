/*/
     Project Arashi: Numbers.h
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, November 30, 1992, 2:16
     Created: Sunday, November 15, 1992, 21:13

     Copyright � 1992, Juri Munkki
/*/

#pragma once
#include <SANE.h>

typedef enum { kSNaN, kQNaN, kInfinite, kZero, kNormalNum, kDenormal } NumberClass;

long double		StringToLongDouble(StringPtr pstr);
NumberClass		FindNumClass(long double n);
long double		MakeNaN(short n);
void			NumberToDecimalRecord(	long	double num,
										decimal	*decimalRecord,
										int 	howManyDecimals);
