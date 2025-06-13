/* Numbers.c */
/*****************************************************************************/
/*                                                                           */
/*    System Dependency Library for Building Portable Software               */
/*    Macintosh Version                                                      */
/*    Written by Thomas R. Lawrence, 1993 - 1994.                            */
/*                                                                           */
/*    This file is Public Domain; it may be used for any purpose whatsoever  */
/*    without restriction.                                                   */
/*                                                                           */
/*    This package is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.                   */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Debug.h"
#include "Audit.h"
#include "Definitions.h"

#include "Numbers.h"
#include "Memory.h"
#include "FloatingPoint.h"


#define FLOATTOSTRINGBUFSIZE (128)


long						StringToInteger(char* String, long Length)
	{
		return (long)StringToLongDouble(String,Length);
	}


char*						IntegerToString(long Integer)
	{
		char*					Temp;

		Temp = LongDoubleToString(Integer,37,1e-37,1e37); /* these bounds should be enough */
		if (Temp != NIL)
			{
				SetTag(Temp,"IntegerToString");
			}
		return Temp;
	}


long double			StringToLongDouble(char* String, long Length)
	{
		long double		Accumulator;
		MyBoolean			Negative;
		MyBoolean			Decimal;
		long double		DecimalScan;
		long					Exponent;
		long					Scan;

		Negative = False;
		Decimal = False;
		Scan = 0;
		while (Scan < Length)
			{
				switch (String[Scan])
					{
						case '+':
						case ' ':
							break;
						case '-':
							/* this might not be a good idea... what does the */
							/* average user think? */
							Negative = !Negative;
							break;
						default:
							goto ContinuePoint;
					}
				Scan += 1;
			}
	 ContinuePoint:
		Accumulator = 0;
		while (Scan != Length)
			{
				switch (String[Scan])
					{
						default:
							goto ExitPoint;
						case '.':
							DecimalScan = 0.1;
							Decimal = True;
							break;
						case '0': case '1': case '2': case '3': case '4':
						case '5': case '6': case '7': case '8': case '9':
							if (!Decimal)
								{
									Accumulator = (10 * Accumulator) + (String[Scan] - '0');
								}
							 else
								{
									Accumulator = Accumulator + (DecimalScan * (String[Scan] - '0'));
									DecimalScan = DecimalScan / 10;
								}
							break;
						case 'e': case 'E':
							Exponent = StringToInteger(&(String[Scan + 1]),Length - Scan - 1);
							while (Exponent < 0)
								{
									Exponent += 1;
									Accumulator /= 10;
								}
							while (Exponent > 0)
								{
									Exponent -= 1;
									Accumulator *= 10;
								}
							goto ExitPoint;
					}
				Scan += 1;
			}
	 ExitPoint:
		if (Negative)
			{
				Accumulator = - Accumulator;
			}
		return Accumulator;
	}


char*						LongDoubleToString(long double Value, int NumDigits,
									long double LowBound, long double HighBound)
	{
		char					Buffer[FLOATTOSTRINGBUFSIZE];
		int						Index;
		MyBoolean			ScientificNotation;
		MyBoolean			Negative;
		long					Exponent;
		long double		Checker;
		MyBoolean			DecimalEmitted;
		char*					Thang;

		/* rounding */
		Checker = Value;
		for (Index = 0; Index < NumDigits; Index += 1)
			{
				Checker = Checker / 10;
			}
		Value += Checker / 2;

		/* initialize buffer pointer */
		Index = 0;
		if (NumDigits > FLOATTOSTRINGBUFSIZE - 16)
			{
				NumDigits = FLOATTOSTRINGBUFSIZE - 16;
			}

		/* NaN check */
		if (ISNAN(Value))
			{
				Buffer[Index++] = 'N';
				Buffer[Index++] = 'a';
				Buffer[Index++] = 'N';
				goto AllDonePoint;
			}

		/* handle negative values right now */
		Negative = (Value < 0);
		if (Negative)
			{
				Value = - Value;
				ERROR(Index >= FLOATTOSTRINGBUFSIZE,PRERR(ForceAbort,
					"LongDoubleToString:  buffer overflow"));
				Buffer[Index++] = '-';
			}

		/* if value is equal to zero, return zero right now */
		if (Value == 0)
			{
				ERROR(Index >= FLOATTOSTRINGBUFSIZE,PRERR(ForceAbort,
					"LongDoubleToString:  buffer overflow"));
				Buffer[Index++] = '0';
				goto AllDonePoint;
			}

		/* determine if we need to use scientific notation by */
		/* comparing against the bounds passed in */
		ScientificNotation = ((Value < LowBound) || (Value >= HighBound));
		if (ScientificNotation)
			{
				/* if it is scientific notation, we want to normalize the number */
				/* and remember what exponent we should output */
				Exponent = 0;
				while (Value < 1)
					{
						Value = Value * 10;
						Exponent -= 1;
					}
				while (Value >= 10)
					{
						Value = Value / 10;
						Exponent += 1;
					}
			}

		/* increment the checker thing until it is high enough */
		Checker = 1;
		while (Checker * 10 <= Value)
			{
				Checker = Checker * 10;
			}

		/* perform the actual scan */
		DecimalEmitted = False;
		while ((Checker >= 1) || ((NumDigits > 0) && (Value != 0)))
			{
				int					Count;

				/* find out what digit to put */
				Count = 0;
				while (Value >= ((Count + 1) * Checker))
					{
						Count += 1;
					}
				ERROR(Index >= FLOATTOSTRINGBUFSIZE,PRERR(ForceAbort,
					"LongDoubleToString:  buffer overflow"));
				Buffer[Index++] = '0' + Count;
				Value -= Count * Checker;
				/* decrement digit count */
				NumDigits -= 1;
				/* put a decimal if appropriate */
				if (!DecimalEmitted && (Checker == 1) && (NumDigits > 0) && (Value != 0))
					{
						DecimalEmitted = True;
						ERROR(Index >= FLOATTOSTRINGBUFSIZE,PRERR(ForceAbort,
							"LongDoubleToString:  buffer overflow"));
						Buffer[Index++] = '.';
					}
				/* decrement digit position */
				Checker /= 10;
			}

		/* drop trailing zeros */
		if (DecimalEmitted)
			{
				while (Buffer[Index - 1] == '0')
					{
						Index -= 1;
					}
				if (Buffer[Index - 1] == '.')
					{
						Index -= 1;
						DecimalEmitted = False;
					}
			}

		/* if it was scientific notation, then put the exponent on */
		if (ScientificNotation)
			{
				long					IntChecker;

				/* put the 'e' */
				ERROR(Index >= FLOATTOSTRINGBUFSIZE,PRERR(ForceAbort,
					"LongDoubleToString:  buffer overflow"));
				Buffer[Index++] = 'e';
				/* check for negative exponent */
				if (Exponent < 0)
					{
						Exponent = - Exponent;
						ERROR(Index >= FLOATTOSTRINGBUFSIZE,PRERR(ForceAbort,
							"LongDoubleToString:  buffer overflow"));
						Buffer[Index++] = '-';
					}
				/* initialize the integer thing */
				IntChecker = 1;
				while ((IntChecker * 10) <= Exponent)
					{
						/* we don't have to worry about integer overflow because even */
						/* a long double's exponent doesn't approach the integer's limit */
						IntChecker *= 10;
					}
				/* perform the integer conversion scan */
				while (IntChecker > 0)
					{
						int					Count;

						/* figure out the digit's value */
						Count = 0;
						while (Exponent >= ((Count + 1) * IntChecker))
							{
								Count += 1;
							}
						/* emit the digit */
						ERROR(Index >= FLOATTOSTRINGBUFSIZE,PRERR(ForceAbort,
							"LongDoubleToString:  buffer overflow"));
						Buffer[Index++] = '0' + Count;
						/* update the values */
						Exponent -= Count * IntChecker;
						IntChecker /= 10;
					}
			}

		/* finally, create the thang so we can output it */
	 AllDonePoint:
		Thang = AllocPtrCanFail(Index,"LongDoubleToString");
		if (Thang != NIL)
			{
				CopyData(&(Buffer[0]),&(Thang[0]),Index);
			}
		return Thang;
	}
