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

#ifdef THINK_C
	#pragma options(pack_enums)
#endif
#include <SANE.h>
#ifdef THINK_C
	#pragma options(!pack_enums)
#endif

#include "FloatingPoint.h"


#ifdef THINK_C
	#if !__option(mc68881) && __option(native_fp)
		/* native for no FPU = always 10-byte (no convertion necessary) */
		#define x80tox96(X,Y) (*(Y) = *(X))
		#define x96tox80(X,Y) (*(Y) = *(X))
	#else
		/* use normal definition */
	#endif
#endif


long double				XGETPI(void)
	{
		extended			A;
		long double		B;

		A = pi();
		x80tox96(&A,&B);
		return B;
	}


long double				XATAN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = atan(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XCOS(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = cos(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XEXP(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = exp(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XFABS(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = fabs(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XLN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = log(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XSIN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = sin(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XSQRT(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = sqrt(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XTAN(long double X)
	{
		extended			A;
		long double		B;

		x96tox80(&X,&A);
		A = tan(A);
		x80tox96(&A,&B);
		return B;
	}


long double				XPOWER(long double X, long double Y)
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


long double				XACOS(long double X)
	{
		return 2 * XATAN(XSQRT((1 - X) / (1 + X)));
	}


long double				XASIN(long double X)
	{
		long double		Y;

		Y = XFABS(X);
		if (Y > 0.5)
			{
				Y = 1 - Y;
				Y = 2 * Y - Y * Y;
			}
		 else
			{
				Y = 1 - Y * Y;
			}
		return XATAN(X / XSQRT(Y));
	}


MyBoolean					ISNAN(long double X)
	{
		extended			XX;
		numclass			Form;

		x96tox80(&X,&XX);
		Form = classextended(XX);
		return ((Form != NORMALNUM) && (Form != ZERONUM) && (Form != DENORMALNUM));
	}
