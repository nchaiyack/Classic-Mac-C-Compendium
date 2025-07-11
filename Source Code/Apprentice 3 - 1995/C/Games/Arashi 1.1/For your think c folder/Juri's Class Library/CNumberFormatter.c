/*/
     Project Arashi: CNumberFormatter.c
     Major release: Version 1.1, 7/22/92

     Last modification: Sunday, May 2, 1993, 14:05
     Created: Monday, November 30, 1992, 0:22

     Copyright � 1992-1993, Juri Munkki
/*/

#include "CNumberFormatter.h"
#include "Numbers.h"

void	CNumberFormatter :: INumberFormatter()
{
	unsigned char	emptyString[] = {0};

	IBaseObject();
	
	SetDecimals(20,2,2);

	SetFormatString("\p$-1.0E)�");

	SetNegativeStrings("\p-",emptyString);
	SetPositiveStrings("\p+",emptyString);
	SetCurrencyStrings("\p",emptyString);
	SetSeparatorStrings("\p","\p.");
	
	setround(TONEAREST);
}

void	CNumberFormatter :: SetDecimals(
	short	iPart,
	short	min,
	short	max)
{
	integerPart = iPart;
	if(min < 0) min = 0;

	if(max > 19) max = 19;
	if(min > 19) min = 19;

	if(min > max) max = min;
	
	maxDecimals = max;
	minDecimals = min;
}
void	CNumberFormatter :: SetFormatString(
	StringPtr	format)
{
	int		len;
	
	len = *(unsigned char *)format;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(format, formatString, len+1);
}

void	CNumberFormatter :: SetNegativeStrings(
	StringPtr	pre,
	StringPtr	post)
{
	int		len;
	
	len = *(unsigned char *)pre;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(pre, negativePrefix, len+1);
	
	len = *(unsigned char *)post;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(post, negativePostfix, len+1);

}
void	CNumberFormatter :: SetPositiveStrings(
	StringPtr	pre,
	StringPtr	post)
{
	int		len;
	
	len = *(unsigned char *)pre;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(pre, positivePrefix, len+1);
	
	len = *(unsigned char *)post;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(post, positivePostfix, len+1);

}
void	CNumberFormatter :: SetCurrencyStrings(
	StringPtr	pre,
	StringPtr	post)
{
	int		len;
	
	len = *(unsigned char *)pre;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(pre, currencyPrefix, len+1);
	
	len = *(unsigned char *)post;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(post, currencyPostfix, len+1);
}

void	CNumberFormatter :: SetSeparatorStrings(
	StringPtr	thousands,
	StringPtr	decimals)
{
	int		len;
	
	len = *(unsigned char *)thousands;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(thousands, thousandsSeparator, len+1);
	
	len = *(unsigned char *)decimals;
	if(len >= SUBFORMATLEN)
		len = SUBFORMATLEN-1;

	BlockMove(decimals, decimalSign, len+1);
}

void	AppendBytes(
	StringPtr		dest,
	int				count,
	void			*bytes)
{
	if(count + dest[0] > 255)
	{	count = 255 - dest[0];
	}
	
	if(count > 0)
	{	BlockMove(bytes, dest+dest[0]+1, count);
		dest[0]+= count;
	}
}
	

void	CNumberFormatter::FormatNumber(
	long double		n,
	StringPtr	 	dest)
{
	decimal		myDecimals;
	int			fmIndex;
	int			i,j;
	short		numDecimals;
	short		realExp;
	char		decimals[32];
	int			lastNonZero;
	int			deciCount;
	int			actDecimals,firstDecimals;
	char		*sourceDecimals;
	char		theDec;
		

	NumberToDecimalRecord(n,&myDecimals, maxDecimals);
	
	if(myDecimals.exp+myDecimals.sig.length > integerPart)
	{	
		realExp = myDecimals.exp + myDecimals.sig.length - 1;
		myDecimals.exp -= realExp;
	}
	else
	{	realExp = 0;
	}
		
	dest[0] = 0; // Clear the string;

	sourceDecimals  = (char *)(myDecimals.sig.text +
								myDecimals.sig.length +
							   	myDecimals.exp);
	firstDecimals = -myDecimals.sig.length - myDecimals.exp;
	actDecimals = -myDecimals.exp;
	if(actDecimals < 0)
		actDecimals = 0;

	lastNonZero = minDecimals;
	
	for(deciCount = 0; deciCount < maxDecimals; deciCount++)
	{
		if(deciCount >= actDecimals || deciCount < firstDecimals)
		{	decimals[deciCount] = '0';
		}
		else
		{	theDec = sourceDecimals[deciCount];
			decimals[deciCount] = theDec;
			if(deciCount >= lastNonZero && theDec != '0')
			{	lastNonZero = deciCount+1;
			}
		}
	}

	for(fmIndex = 1; fmIndex <= formatString[0]; fmIndex++)
	{	switch(formatString[fmIndex])
		{
			case '-':	//	sign prefix
				if(myDecimals.sgn)
					AppendBytes(dest, negativePrefix[0],negativePrefix+1);
				else
					AppendBytes(dest, positivePrefix[0],positivePrefix+1);
				break;
			case ')':	//	sign postfix
				if(myDecimals.sgn)
					AppendBytes(dest, negativePostfix[0],negativePostfix+1);
				else
					AppendBytes(dest, positivePostfix[0],positivePostfix+1);
				break;
			case '.':
				if(lastNonZero != 0)
					AppendBytes(dest, decimalSign[0],decimalSign+1);
				break;
			case '$':
				AppendBytes(dest, currencyPrefix[0],currencyPrefix+1);
				break;
			case '�':
				AppendBytes(dest, currencyPostfix[0],currencyPostfix+1);
				break;
			case '1':
				j = myDecimals.sig.length + myDecimals.exp;
				i = 0;
				
				if(j<=0)
				{	char	zero = '0';
				
					AppendBytes(dest, 1, &zero);
				}
				else
				{
					while(j > 0)
					{	int		chunk;
					
						chunk = j % 3;
						if(chunk == 0) chunk = 3;
						
						if(i+chunk < myDecimals.sig.length)
						{	AppendBytes(dest, chunk, myDecimals.sig.text+i);
						}
						else
						{	int		partOne;
							long	zeros='0000';
							
							partOne = myDecimals.sig.length - i;
							if(partOne < 0) partOne = 0;
							
							if(partOne)
								AppendBytes(dest, chunk, myDecimals.sig.text+i);

							AppendBytes(dest, chunk-partOne, (char *)&zeros);
						}
						
						i += chunk;
						j -= chunk;
						
						if(j)
							AppendBytes(dest, thousandsSeparator[0], thousandsSeparator+1);
					}
				}
				break;
			case '0':
				AppendBytes(dest, lastNonZero, decimals);
				break;
			case 'E':
			case 'e':
				if(realExp != 0)
				{	char	expon[32];
					int		len;
				
					NumToString(realExp,expon);
					len = expon[0] +1;
					expon[0] = formatString[fmIndex];
					AppendBytes(dest, len, expon);
				}	
				break;
		}
	}
}
