/* BinaryCodedDecimal.c */
/*****************************************************************************/
/*                                                                           */
/*    Out Of Phase:  Digital Music Synthesis on General Purpose Computers    */
/*    Copyright (C) 1994  Thomas R. Lawrence                                 */
/*                                                                           */
/*    This program is free software; you can redistribute it and/or modify   */
/*    it under the terms of the GNU General Public License as published by   */
/*    the Free Software Foundation; either version 2 of the License, or      */
/*    (at your option) any later version.                                    */
/*                                                                           */
/*    This program is distributed in the hope that it will be useful,        */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of         */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          */
/*    GNU General Public License for more details.                           */
/*                                                                           */
/*    You should have received a copy of the GNU General Public License      */
/*    along with this program; if not, write to the Free Software            */
/*    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.              */
/*                                                                           */
/*    Thomas R. Lawrence can be reached at tomlaw@world.std.com.             */
/*                                                                           */
/*****************************************************************************/

#include "MiscInfo.h"
#include "Audit.h"
#include "Debug.h"
#include "Definitions.h"

#include "BinaryCodedDecimal.h"
#include "DataMunging.h"
#include "Memory.h"


/* maximum string:  10 digits + 1 sign + 1 decimal */
#define BCDBUFFSIZE (10 + 1 + 1)


/* convert double precision number to small BCD number.  Rounding is towards */
/* nearest. (0.4 ==> 0, 0.6 ==> 1, -0.4 ==> 0, -0.6 ==> -1) */
/* value is constrained if it is out of range */
SmallBCDType			Double2SmallBCD(double Value)
	{
		double					CorrectedFactor;

		CorrectedFactor = (Value * SMALLBCDPRECISION) + ((Value >= 0) ? 0.5 : -0.5);
		if (CorrectedFactor > 29999)
			{
				return 29999;
			}
		else if (CorrectedFactor < -29999)
			{
				return -29999;
			}
		else
			{
				return (SmallBCDType)CorrectedFactor;
			}
	}


/* convert double precision number to large BCD number. */
LargeBCDType			Double2LargeBCD(double Value)
	{
		double					CorrectedFactor;

		CorrectedFactor = (Value * LARGEBCDPRECISION) + ((Value >= 0) ? 0.5 : -0.5);
		if (CorrectedFactor > 999999999)
			{
				return 999999999;
			}
		else if (CorrectedFactor < -999999999)
			{
				return -999999999;
			}
		else
			{
				return (LargeBCDType)CorrectedFactor;
			}
	}


/* convert double precision to small extended BCD number */
SmallExtBCDType		Double2SmallExtBCD(double Value)
	{
		double					CorrectedFactor;

		CorrectedFactor = (Value * SMALLEXTBCDPRECISION) + ((Value >= 0) ? 0.5 : -0.5);
		if (CorrectedFactor > 999999999)
			{
				return 999999999;
			}
		else if (CorrectedFactor < -999999999)
			{
				return -999999999;
			}
		else
			{
				return (SmallExtBCDType)CorrectedFactor;
			}
	}


/* convert small BCD number to string */
char*							SmallBCDToString(SmallBCDType Number)
	{
		char						Buffer[BCDBUFFSIZE];
		int							Index;
		char*						ReturnValue;

		Index = 0;
		/* sign */
		if (Number < 0)
			{
				Buffer[Index++] = '-';
				Number = - Number;
			}
		/* tens */
		if ((Number / 10000) != 0)
			{
				Buffer[Index++] = (Number / 10000) + '0';
				Number -= (Number / 10000) * 10000;
			}
		/* ones */
		Buffer[Index++] = (Number / 1000) + '0';
		Number -= (Number / 1000) * 1000;
		if (Number != 0)
			{
				Buffer[Index++] = '.';
				/* tenths */
				Buffer[Index++] = (Number / 100) + '0';
				Number -= (Number / 100) * 100;
				if (Number != 0)
					{
						/* hundreths */
						Buffer[Index++] = (Number / 10) + '0';
						Number -= (Number / 10) * 10;
						if (Number != 0)
							{
								/* thousandths */
								Buffer[Index++] = Number + '0';
							}
					}
			}
		ReturnValue = BlockFromRaw(Buffer,Index);
		if (ReturnValue != NIL)
			{
				SetTag(ReturnValue,"SmallBCDToString");
			}
		return ReturnValue;
	}


/* convert large BCD number to string */
char*							LargeBCDToString(LargeBCDType Number)
	{
		char						Buffer[BCDBUFFSIZE];
		int							Index;
		char*						ReturnValue;
		MyBoolean				PrintedBigDigit;

		Index = 0;
		PrintedBigDigit = False;
		/* sign */
		if (Number < 0)
			{
				Buffer[Index++] = '-';
				Number = - Number;
			}
		/* thousands digit */
		if (Number / 1000000000 != 0)
			{
				Buffer[Index++] = (Number / 1000000000) + '0';
				Number -= (Number / 1000000000) * 1000000000;
				PrintedBigDigit = True;
			}
		/* hundreds digit */
		if (PrintedBigDigit || (Number / 100000000 != 0))
			{
				Buffer[Index++] = (Number / 100000000) + '0';
				Number -= (Number / 100000000) * 100000000;
				PrintedBigDigit = 0;
			}
		/* tens digit */
		if (PrintedBigDigit || (Number / 10000000 != 0))
			{
				Buffer[Index++] = (Number / 10000000) + '0';
				Number -= (Number / 10000000) * 10000000;
				PrintedBigDigit = True;
			}
		/* ones digit */
		Buffer[Index++] = (Number / 1000000) + '0';
		Number -= (Number / 1000000) * 1000000;
		if (Number != 0)
			{
				Buffer[Index++] = '.';
				/* tenths */
				Buffer[Index++] = (Number / 100000) + '0';
				Number -= (Number / 100000) * 100000;
				if (Number != 0)
					{
						/* hundreths */
						Buffer[Index++] = (Number / 10000) + '0';
						Number -= (Number / 10000) * 10000;
						if (Number != 0)
							{
								/* thousandths */
								Buffer[Index++] = (Number / 1000) + '0';
								Number -= (Number / 1000) * 1000;
								if (Number != 0)
									{
										/* ten-thousandths */
										Buffer[Index++] = (Number / 100) + '0';
										Number -= (Number / 100) * 100;
										if (Number != 0)
											{
												/* hundred-thousandths */
												Buffer[Index++] = (Number / 10) + '0';
												Number -= (Number / 10) * 10;
												if (Number != 0)
													{
														/* millionths */
														Buffer[Index++] = Number + '0';
													}
											}
									}
							}
					}
			}
		ReturnValue = BlockFromRaw(Buffer,Index);
		if (ReturnValue != NIL)
			{
				SetTag(ReturnValue,"LargeBCDToString");
			}
		return ReturnValue;
	}


/* convert small extended BCD number to string */
char*							SmallExtBCDToString(SmallExtBCDType Number)
	{
		char						Buffer[BCDBUFFSIZE];
		int							Index;
		char*						ReturnValue;
		MyBoolean				PrintedBigDigit;

		Index = 0;
		PrintedBigDigit = False;
		/* sign */
		if (Number < 0)
			{
				Buffer[Index++] = '-';
				Number = - Number;
			}
		/* millions digit */
		if (Number / 1000000000 != 0)
			{
				Buffer[Index++] = (Number / 1000000000) + '0';
				Number -= (Number / 1000000000) * 1000000000;
				PrintedBigDigit = True;
			}
		/* hundred-thousands digit */
		if (PrintedBigDigit || (Number / 100000000 != 0))
			{
				Buffer[Index++] = (Number / 100000000) + '0';
				Number -= (Number / 100000000) * 100000000;
				PrintedBigDigit = 0;
			}
		/* ten-thousands digit */
		if (PrintedBigDigit || (Number / 10000000 != 0))
			{
				Buffer[Index++] = (Number / 10000000) + '0';
				Number -= (Number / 10000000) * 10000000;
				PrintedBigDigit = True;
			}
		/* thousands digit */
		if (PrintedBigDigit || (Number / 1000000 != 0))
			{
				Buffer[Index++] = (Number / 1000000) + '0';
				Number -= (Number / 1000000) * 1000000;
				PrintedBigDigit = True;
			}
		/* hundreds digit */
		if (PrintedBigDigit || (Number / 100000 != 0))
			{
				Buffer[Index++] = (Number / 100000) + '0';
				Number -= (Number / 100000) * 100000;
				PrintedBigDigit = True;
			}
		/* tens digit */
		if (PrintedBigDigit || (Number / 10000 != 0))
			{
				Buffer[Index++] = (Number / 10000) + '0';
				Number -= (Number / 10000) * 10000;
				PrintedBigDigit = True;
			}
		/* ones digit */
		Buffer[Index++] = (Number / 1000) + '0';
		Number -= (Number / 1000) * 1000;
		if (Number != 0)
			{
				Buffer[Index++] = '.';
				/* tenths */
				Buffer[Index++] = (Number / 100) + '0';
				Number -= (Number / 100) * 100;
				if (Number != 0)
					{
						/* hundreths */
						Buffer[Index++] = (Number / 10) + '0';
						Number -= (Number / 10) * 10;
						if (Number != 0)
							{
								/* thousandths */
								Buffer[Index++] = Number + '0';
							}
					}
			}
		ReturnValue = BlockFromRaw(Buffer,Index);
		if (ReturnValue != NIL)
			{
				SetTag(ReturnValue,"SmallExtBCDToString");
			}
		return ReturnValue;
	}
