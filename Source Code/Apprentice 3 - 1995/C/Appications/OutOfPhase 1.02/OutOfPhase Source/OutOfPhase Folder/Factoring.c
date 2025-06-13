/* Factoring.c */
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

#include "Factoring.h"
#include "Memory.h"


struct FactorRec
	{
		FactorRec*					Next;
		unsigned long				Number;
	};


static FactorRec*						PrimeList;
static FactorRec*						PrimeTail;
static MyBoolean						PrimeListFinished;
EXECUTE(static MyBoolean		Initialized = False;)
EXECUTE(static long					ListLength = 0;)


/* initialize internal data structures used by factorer */
MyBoolean							InitializeFactoring(void)
	{
		ERROR(Initialized,PRERR(ForceAbort,"InitializeFactoring:  already initialized"));
		EXECUTE(Initialized = True;)
		EXECUTE(ListLength = 0;)
		PrimeList = NIL;
		PrimeTail = NIL;
		PrimeListFinished = False;
		return True;
	}


/* dispose of factor stuff */
void									ShutdownFactoring(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"ShutdownFactoring:  not initialized"));
		DisposeFactorList(PrimeList);
	}


/* dispose factor list */
void									DisposeFactorList(FactorRec* List)
	{
		while (List != NIL)
			{
				FactorRec*			Temp;

				CheckPtrExistence(List);
				Temp = List;
				List = List->Next;
				ReleasePtr((char*)Temp);
			}
	}


/* see if a number is prime according to the list.  the prime list is assumed to */
/* be complete enough to test the number.  In other words, the prime after the */
/* last prime in the list must be greater than or equal to the number. */
static MyBoolean			PrimeTest(unsigned long Integer)
	{
		FactorRec*					Scan;

		ERROR(!Initialized,PRERR(ForceAbort,"PrimeTest:  not initialized"));
		Scan = PrimeList;
		while ((Scan != NIL) && (Scan->Number < Integer))
			{
				/* divide number by a prime.  if the remainder is 0, then they divided */
				/* evenly and the prime is a factor, so the number isn't prime. */
				if ((Integer % Scan->Number) == 0)
					{
						return False;
					}
				Scan = Scan->Next;
			}
		/* if none of the primes less than the number were a factor of the */
		/* number, then it is prime. */
		return True;
	}


/* extend the prime list by one entry */
static MyBoolean			ExtendPrimeList(void)
	{
		ERROR(!Initialized,PRERR(ForceAbort,"ExtendPrimeList:  not initialized"));
		/* check for special case of putting only even numbered prime (2) on the list */
		if (PrimeList == NIL)
			{
				/* if there is nothing in the prime number list, put 2 (the first prime) */
				/* onto the list. */
				PrimeList = (FactorRec*)AllocPtrCanFail(sizeof(FactorRec),"FactorRec");
				if (PrimeList != NIL)
					{
						PrimeTail = PrimeList;
						PrimeList->Next = NIL;
						PrimeList->Number = 2;
						EXECUTE(ListLength += 1;)
						return True;
					}
				 else
					{
						return False;
					}
			}
		else if (!PrimeListFinished)
			{
				unsigned long				Scan;

				/* looking for the next prime number */
				/* first, check for the only even numbered prime */
				if (PrimeTail->Number == 2)
					{
						Scan = 3;
					}
				 else
					{
						Scan = PrimeTail->Number + 2;
					}
				/* perform scan.  Scan is always an odd number.  So is 0xffffffff, so */
				/* we just make sure Scan is less than that number. */
				while (Scan < 0xffffffff)
					{
						ERROR((Scan & 1) == 0,PRERR(ForceAbort,
							"ExtendPrimeList:  testing even number"));
						/* checking number to see if it is prime */
						if (PrimeTest(Scan))
							{
								FactorRec*			Temp;

								/* if number is prime, then we add it to the list */
								Temp = (FactorRec*)AllocPtrCanFail(sizeof(FactorRec),"FactorRec");
								if (Temp != NIL)
									{
										Temp->Next = NIL;
										Temp->Number = Scan;
										PrimeTail->Next = Temp;
										PrimeTail = Temp;
										EXECUTE(ListLength += 1;)
										return True;
									}
								 else
									{
										return False;
									}
							}
						Scan += 2; /* don't try even numbers */
					}
				/* if we get here, then Scan == 0xffffffff (biggest odd number) */
				/* if our numbers are so big that we hit the limit of our long integer */
				/* then indicate that we can't find any more primes */
				PrimeListFinished = True;
				return False;
			}
		else
			{
				/* prime list is finished, so return False indicating another */
				/* prime couldn't be added to the list */
				return False;
			}
		EXECUTE(PRERR(ForceAbort,"ExtendPrimeList:  illegal exit"));
	}


/* find the common factors of two numbers.  the product of the common factors */
/* is returned (i.e. greatest common factor). */
unsigned long					FindCommonFactors(unsigned long Left, unsigned long Right)
	{
		FactorRec*					Scan;
		unsigned long				Product;

		ERROR(!Initialized,PRERR(ForceAbort,"FindCommonFactors:  not initialized"));
		/* while the end of the prime number list is less than the largest possible */
		/* factor of one of the numbers, and we can make a new prime list entry. */
		/* ideally, we should take square root of our numbers, but that would take */
		/* too long, so division by 2 still saves a lot of time. */
		/* Some day, I should learn Euclid's algorithm. */
	 CheckPrimesAgainPoint:
		if ((PrimeTail == NIL) || (PrimeTail->Number < Left / 2)
			|| (PrimeTail->Number < Right / 2))
			{
				if (!ExtendPrimeList())
					{
						/* failed, so just continue */
						goto DoneWithPrimesPoint;
					}
				goto CheckPrimesAgainPoint;
			}
	 DoneWithPrimesPoint:
		/* initialize list scanning */
		Scan = PrimeList;
		/* product accumulates the common factors */
		Product = 1;
		/* see note above about division by 2 */
		while ((Scan->Number <= Left) && (Scan->Number <= Right))
			{
				if ((Left % Scan->Number == 0) && (Right % Scan->Number == 0))
					{
						/* if the number divides both numbers evenly, then it is */
						/* a common factor, so we accumulate it into Product and */
						/* remove it from both numbers */
						Left = Left / Scan->Number;
						Right = Right / Scan->Number;
						Product = Product * Scan->Number;
					}
				 else
					{
						/* if the number isn't a common factor, then we check the next */
						/* number.  note that we only do this if it isn't a common factor */
						/* since there might be multiple instances of a given common */
						/* factor, such as for 2*2*3 & 2*2*7 (i.e. 12 & 28) */
						Scan = Scan->Next;
					}
			}
		/* return the common factor product */
		return Product;
	}
