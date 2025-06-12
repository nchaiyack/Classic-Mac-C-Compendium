/* FloatingPoint.c */
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

#pragma options(pack_enums)
#include <SANE.h>
#pragma options(!pack_enums)

#include "FloatingPoint.h"


long double				GetPI(void)
	{
		extended			A;
		long double		B;

		A = pi();
		x80tox96(&A,&B);
		return B;
	}


long double				GetINF(void)
	{
		extended			A;
		long double		B;

		A = inf();
		x80tox96(&A,&B);
		return B;
	}


long double				ATAN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = atan(A);
		x80tox96(&A,&B);
		return B;
	}


long double				COS(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = cos(A);
		x80tox96(&A,&B);
		return B;
	}


long double				EXP(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = exp(A);
		x80tox96(&A,&B);
		return B;
	}


long double				FABS(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = fabs(A);
		x80tox96(&A,&B);
		return B;
	}


long double				LN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = log(A);
		x80tox96(&A,&B);
		return B;
	}


long double				SIN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = sin(A);
		x80tox96(&A,&B);
		return B;
	}


long double				SQRT(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = sqrt(A);
		x80tox96(&A,&B);
		return B;
	}


long double				TAN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = tan(A);
		x80tox96(&A,&B);
		return B;
	}


long double				POWER(long double X, long double Y)
	{
		extended			XX;
		extended			YY;
		extended			ZZ;
		long double		Z;

		x96tox80(&X,&XX);
		x96tox80(&Y,&YY);
		ZZ = power(XX,YY);
		x80tox96(&ZZ,&Z);
		return Z;
	}


long double				ACOS(long double X)
	{
		return 2 * ATAN(SQRT((1 - X) / (1 + X)));
	}


long double				ASIN(long double X)
	{
		long double		Y;

		Y = FABS(X);
		if (Y > 0.5)
			{
				Y = 1 - Y;
				Y = 2 * Y - Y * Y;
			}
		 else
			{
				Y = 1 - Y * Y;
			}
		return ATAN(X / SQRT(Y));
	}
