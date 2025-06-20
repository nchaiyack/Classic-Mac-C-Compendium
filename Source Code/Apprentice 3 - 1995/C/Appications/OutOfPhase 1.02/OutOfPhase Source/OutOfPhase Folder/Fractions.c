/* Fractions.c */
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

#include "Fractions.h"
#include "Factoring.h"


/* note: this module does NOT deal with negative numbers!!! */


/* convert a decimal number to a fraction with the specified denominator limit */
void								Double2Fraction(double Value, unsigned long Denominator,
											FractionRec* Fraction)
	{
		ERROR(Value < 0,PRERR(ForceAbort,"Double2Fraction:  value is less than zero"));
		Fraction->Denominator = Denominator;
		Fraction->Integer = Value;
		Fraction->Fraction = (Value - Fraction->Integer) * Denominator + 0.5;
		ReduceFraction(Fraction);
	}


/* convert fraction to a double */
double							Fraction2Double(FractionRec* Fraction)
	{
		ERROR(Fraction->Fraction >= Fraction->Denominator,PRERR(ForceAbort,
			"Fraction2Double:  numerator is larger than denominator"));
		return Fraction->Integer + ((double)Fraction->Fraction / Fraction->Denominator);
	}


/* add fractions.  Destination fraction can be one of the source fractions */
void								AddFractions(FractionRec* Left, FractionRec* Right, FractionRec* Dest)
	{
		unsigned long			FractionTemp;
		unsigned long			DenominatorTemp;
		unsigned long			IntegerTemp;

		ERROR(Left->Fraction >= Left->Denominator,PRERR(ForceAbort,
			"AddFractions:  numerator is larger than denominator"));
		ERROR(Right->Fraction >= Right->Denominator,PRERR(ForceAbort,
			"AddFractions:  numerator is larger than denominator"));
		/* add fractional parts */
#if !DEBUG
		if (Left->Denominator == Right->Denominator)
			{
				/* if the denominators are the same, then adding is really easy */
				DenominatorTemp = Left->Denominator;
				FractionTemp = Left->Fraction + Right->Fraction;
			}
		 else
#endif
			{
				unsigned long			GCF;

				/* if the denominators are not the same, then we need to multiply each */
				/* side by some number so that they will be the same.  finding the greatest */
				/* common factor helps us find the smallest number to multiply by. */
				/* Left->Denominator / GCF = the factors that left has which right needs. */
				/* Right->Denominator / GCF = the factors that right has which left needs. */
				GCF = FindCommonFactors(Left->Denominator,Right->Denominator);
				/* by multiplying the denominators together, then dividing out the extra */
				/* set of common factors, we find the smallest common denominator.  The */
				/* division is performed inside to prevent overflow */
				DenominatorTemp = (Left->Denominator / GCF) * Right->Denominator;
				/* the left and right sides should yield the same denominator */
				ERROR(DenominatorTemp != (Right->Denominator / GCF) * Left->Denominator,
					PRERR(ForceAbort,"AddFractions:  couldn't factor denominators"));
				/* since we are multiplying each fraction by N/N, we need to multiply */
				/* the numerators by the same thing we multiplied the denominators by. */
				FractionTemp = Left->Fraction * (Right->Denominator / GCF)
					+ Right->Fraction * (Left->Denominator / GCF);
			}
		/* add the integer components */
		IntegerTemp = Left->Integer + Right->Integer;
		/* if there was an overflow in the fractional part, carry it to the integer */
		if (FractionTemp >= DenominatorTemp)
			{
				/* since we are adding, the amount of carry should never be more than 1 */
				FractionTemp -= DenominatorTemp;
				IntegerTemp += 1;
			}
		ERROR(FractionTemp >= DenominatorTemp,PRERR(ForceAbort,
			"AddFractions:  numerator is larger than denominator after reduction"));
		/* store result */
		Dest->Integer = IntegerTemp;
		Dest->Fraction = FractionTemp;
		Dest->Denominator = DenominatorTemp;
	}


/* test to see if the left is greater than the right */
MyBoolean						FracGreaterThan(FractionRec* Left, FractionRec* Right)
	{
		ERROR(Left->Fraction >= Left->Denominator,PRERR(ForceAbort,
			"FracGreaterThan:  numerator is larger than denominator"));
		ERROR(Right->Fraction >= Right->Denominator,PRERR(ForceAbort,
			"FracGreaterThan:  numerator is larger than denominator"));
		if (Left->Integer > Right->Integer)
			{
				/* if the integer portion is bigger, then there's no contest */
				return True;
			}
		else if (Left->Integer < Right->Integer)
			{
				/* same as above */
				return False;
			}
		else
			{
				/* if the integer portions are the same, then we have to compare the */
				/* fractional portions */
#if !DEBUG
				if (Left->Denominator == Right->Denominator)
					{
						/* if the denominators are the same, then comparison is easy */
						return Left->Fraction > Right->Fraction;
					}
				 else
#endif
					{
						unsigned long			GCF;

						/* if the denominators are not the same, then they have to be */
						/* made the same.  as before, the GCF is the factors that are */
						/* common to both sides.  Left->Denominator / GCF is the portion of */
						/* the left that right needs and Right->Denominator / GCF is the portion */
						/* of the right that left needs.  We don't care about the new */
						/* denominator, but we will compare the new numerators. */
						GCF = FindCommonFactors(Left->Denominator,Right->Denominator);
						return Left->Fraction * (Right->Denominator / GCF)
							> Right->Fraction * (Left->Denominator / GCF);
					}
			}
	}


/* test to see if the left is greater than or equal to the right */
MyBoolean						FracGreaterEqual(FractionRec* Left, FractionRec* Right)
	{
		ERROR(Left->Fraction >= Left->Denominator,PRERR(ForceAbort,
			"FracGreaterThan:  numerator is larger than denominator"));
		ERROR(Right->Fraction >= Right->Denominator,PRERR(ForceAbort,
			"FracGreaterThan:  numerator is larger than denominator"));
		if (Left->Integer > Right->Integer)
			{
				/* if the integer portion is bigger, then there's no contest */
				return True;
			}
		else if (Left->Integer < Right->Integer)
			{
				/* same as above */
				return False;
			}
		else
			{
				/* if the integer portions are the same, then we have to compare the */
				/* fractional portions */
#if !DEBUG
				if (Left->Denominator == Right->Denominator)
					{
						/* if the denominators are the same, then comparison is easy */
						return Left->Fraction >= Right->Fraction;
					}
				 else
#endif
					{
						unsigned long			GCF;

						/* if the denominators are not the same, then they have to be */
						/* made the same.  as before, the GCF is the factors that are */
						/* common to both sides.  Left->Denominator / GCF is the portion of */
						/* the left that right needs and Right->Denominator / GCF is the portion */
						/* of the right that left needs.  We don't care about the new */
						/* denominator, but we will compare the new numerators. */
						GCF = FindCommonFactors(Left->Denominator,Right->Denominator);
						return Left->Fraction * (Right->Denominator / GCF)
							>= Right->Fraction * (Left->Denominator / GCF);
					}
			}
	}


/* test fractions for equality */
MyBoolean						FractionsEqual(FractionRec* Left, FractionRec* Right)
	{
		ERROR(Left->Fraction >= Left->Denominator,PRERR(ForceAbort,
			"FractionsEqual:  numerator is larger than denominator"));
		ERROR(Right->Fraction >= Right->Denominator,PRERR(ForceAbort,
			"FractionsEqual:  numerator is larger than denominator"));
		if (Left->Integer != Right->Integer)
			{
				/* if the integers aren't equal, then it's easy */
				return False;
			}
		else
			{
#if !DEBUG
				/* if the integer portions are the same, then we have to compare the */
				/* fractional portions */
				if (Left->Denominator == Right->Denominator)
					{
						/* if the denominators are the same, then comparison is easy */
						return Left->Fraction == Right->Fraction;
					}
				 else
#endif
					{
						unsigned long			GCF;

						/* if the denominators are not the same, then they have to be */
						/* made the same.  as before, the GCF is the factors that are */
						/* common to both sides.  Left->Denominator / GCF is the portion of */
						/* the left that right needs and Right->Denominator / GCF is the portion */
						/* of the right that left needs.  We don't care about the new */
						/* denominator, but we will compare the new numerators. */
						GCF = FindCommonFactors(Left->Denominator,Right->Denominator);
						return Left->Fraction * (Right->Denominator / GCF)
							== Right->Fraction * (Left->Denominator / GCF);
					}
			}
	}


/* reduce fraction */
void								ReduceFraction(FractionRec* Frac)
	{
		unsigned long			GCF;

		ERROR(Frac->Fraction >= Frac->Denominator,PRERR(ForceAbort,
			"ReduceFraction:  numerator is larger than denominator"));
		GCF = FindCommonFactors(Frac->Fraction,Frac->Denominator);
		Frac->Fraction = Frac->Fraction / GCF;
		Frac->Denominator = Frac->Denominator / GCF;
	}


/* multiply fractions.  destination can be one of the sources */
/* this function will fail on numbers considerably smaller than the */
/* range of representable fractions. */
void								MultFractions(FractionRec* Left, FractionRec* Right, FractionRec* Dest)
	{
		unsigned long			Numerator;
		unsigned long			Denominator;

		ERROR(Left->Fraction >= Left->Denominator,PRERR(ForceAbort,
			"MultFractions:  numerator is larger than denominator"));
		ERROR(Right->Fraction >= Right->Denominator,PRERR(ForceAbort,
			"MultFractions:  numerator is larger than denominator"));
		/* the product of two fractions: A/B * C/D == AC/BD */
		/* here we multiply the denominators */
		Denominator = Left->Denominator * Right->Denominator;
		/* here we multiply the numerators and convert the integer parts into */
		/* part of the numerator */
		Numerator = (Left->Integer + Right->Integer) * Denominator
			+ Left->Fraction * Right->Denominator
			+ Right->Fraction * Left->Denominator;
		/* division gives us the integer part back and the remainder is the numerator */
		Dest->Integer = Numerator / Denominator;
		Dest->Fraction = Numerator % Denominator;
		Dest->Denominator = Denominator;
		/* since we multiplied, reduce so that denominators don't get out of hand */
		ReduceFraction(Dest);
	}


/* subtract second fraction from first.  Destination can be one of the sources */
void								SubFractions(FractionRec* Left, FractionRec* Right, FractionRec* Dest)
	{
		long							FractionTemp;
		long							DenominatorTemp;
		long							IntegerTemp;

		ERROR(Left->Fraction >= Left->Denominator,PRERR(ForceAbort,
			"AddFractions:  numerator is larger than denominator"));
		ERROR(Right->Fraction >= Right->Denominator,PRERR(ForceAbort,
			"AddFractions:  numerator is larger than denominator"));
		/* add fractional parts */
#if !DEBUG
		if (Left->Denominator == Right->Denominator)
			{
				/* if the denominators are the same, then adding is really easy */
				DenominatorTemp = Left->Denominator;
				FractionTemp = Left->Fraction - Right->Fraction;
			}
		 else
#endif
			{
				unsigned long			GCF;

				/* if the denominators are not the same, then we need to multiply each */
				/* side by some number so that they will be the same.  finding the greatest */
				/* common factor helps us find the smallest number to multiply by. */
				/* Left->Denominator / GCF = the factors that left has which right needs. */
				/* Right->Denominator / GCF = the factors that right has which left needs. */
				GCF = FindCommonFactors(Left->Denominator,Right->Denominator);
				/* by multiplying the denominators together, then dividing out the extra */
				/* set of common factors, we find the smallest common denominator.  The */
				/* division is performed inside to prevent overflow */
				DenominatorTemp = (Left->Denominator / GCF) * Right->Denominator;
				/* the left and right sides should yield the same denominator */
				ERROR(DenominatorTemp != (Right->Denominator / GCF) * Left->Denominator,
					PRERR(ForceAbort,"AddFractions:  couldn't factor denominators"));
				/* since we are multiplying each fraction by N/N, we need to multiply */
				/* the numerators by the same thing we multiplied the denominators by. */
				FractionTemp = Left->Fraction * (Right->Denominator / GCF)
					- Right->Fraction * (Left->Denominator / GCF);
			}
		/* add the integer components */
		IntegerTemp = Left->Integer - Right->Integer;
		/* if there was an overflow in the fractional part, carry it to the integer */
		ERROR(FractionTemp >= DenominatorTemp,
			PRERR(AllowResume,"SubFractions:  overflow occurred when it shouldn't"));
		if (FractionTemp < 0)
			{
				/* since we are adding, the amount of carry should never be more than 1 */
				FractionTemp += DenominatorTemp;
				IntegerTemp -= 1;
			}
		ERROR(FractionTemp < 0,PRERR(ForceAbort,
			"SubFractions:  numerator is way too small"));
		/* store result */
		Dest->Integer = IntegerTemp;
		Dest->Fraction = FractionTemp;
		Dest->Denominator = DenominatorTemp;
	}
