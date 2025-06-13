/*/
     Project Arashi: Numbers.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, April 4, 1993, 2:55
     Created: Sunday, November 15, 1992, 21:06

     Copyright © 1992-1993, Juri Munkki
/*/

/*
**	Some floating point number conversion utilies.
*/

#include "Numbers.h"

long double	StringToLongDouble(
	StringPtr	pstr)
{
#if !__option(mc68881) && __option(native_fp)
	return str2num(pstr);
#else
	extended	temp;
	long double	result;
	
	temp = str2num(pstr);
	x80tox96(&temp, &result);
	return result;
#endif
}

NumberClass	FindNumClass(
	long double	n)
{
#if !__option(mc68881) && __option(native_fp)
	return classextended(n);
#else
	extended	temp;
	
	x96tox80(&n, &temp);
	return classextended(temp);
#endif
}

long double	MakeNaN(
	short	n)
{
#if !__option(mc68881) && __option(native_fp)
	return nan(n);
#else
	extended	temp;
	long double	result;
	
	temp = nan(n);
	x80tox96(&temp, &result);
	return result;
#endif

}

void	NumberToDecimalRecord(
	long double		number,
	decimal			*decimalRecord,
	int				howManyDecimals)
{
	decform		myDecForm;
	extended	temp;
	
	myDecForm.style = 1;
	myDecForm.digits = howManyDecimals;
	decimalRecord->exp = 0;

#if !__option(mc68881) && __option(native_fp)
	num2dec(&myDecForm,number,decimalRecord);
#else
	
	x96tox80(&number, &temp);
	num2dec(&myDecForm, temp, decimalRecord);
#endif
}
