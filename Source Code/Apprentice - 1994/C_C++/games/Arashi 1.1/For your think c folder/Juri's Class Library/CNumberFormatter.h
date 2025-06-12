/*/
     Project Arashi: CNumberFormatter.h
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, November 30, 1992, 2:27
     Created: Monday, November 30, 1992, 0:18

     Copyright © 1992, Juri Munkki
/*/

#pragma once
#include <CBaseObject.h>

#define	SUBFORMATLEN	16

class	CNumberFormatter : public CBaseObject
{
public:
	char	formatString[SUBFORMATLEN];
	
	char	negativePrefix[SUBFORMATLEN];
	char	negativePostfix[SUBFORMATLEN];
	
	char	positivePrefix[SUBFORMATLEN];
	char	positivePostfix[SUBFORMATLEN];

	char	currencyPrefix[SUBFORMATLEN];
	char	currencyPostfix[SUBFORMATLEN];

	char	thousandsSeparator[SUBFORMATLEN];
	char	decimalSign[SUBFORMATLEN];
	
	short	integerPart;
	short	minDecimals;
	short	maxDecimals;
	
	void	INumberFormatter();
	void	SetDecimals(short iPart, short min, short max);
	void	SetFormatString(StringPtr format);
	void	SetNegativeStrings(StringPtr pre, StringPtr post);
	void	SetPositiveStrings(StringPtr pre, StringPtr post);
	void	SetCurrencyStrings(StringPtr pre, StringPtr post);
	void	SetSeparatorStrings(StringPtr thousands, StringPtr decimals);
	void	FormatNumber(long double n, StringPtr dest);
};