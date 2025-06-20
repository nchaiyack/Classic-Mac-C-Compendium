#include "flight.h"


FxVector	fxGrid[GRID_SZ][GRID_SZ];
FxVector	fxGridRot[GRID_SZ][GRID_SZ];
FxVector	fxOriginV = {0, 0, 0};

FracThreeMatrx	fracRotatMatrx;
FxThreeMatrx	transpCurrFxRotMatrx;

Fract	sinPtOne, cosPtOne, sinPtTwo, cosPtTwo;
Fract	sinNPtOne, cosNPtOne, sinNPtTwo, cosNPtTwo;

extern	WindowPtr		theWind;	/*the main window*/


MakeGrid()
{
	int	i, j;
	Fixed	xVal, yVal, zVal;

	MkPort(theWind);
	InitVars();

	yVal = iToFix(-100);
	zVal = iToFix(-startHeight);

	for (i = 0; i < GRID_SZ; i++)
	{
		xVal = iToFix(-200);
		for (j = 0; j < GRID_SZ; j++)
		{
			fxGrid[i][j].x = xVal;
			fxGrid[i][j].y = yVal;
			fxGrid[i][j].z = zVal;
			xVal += iToFix(25);
		}
		yVal += iToFix(25);
	}
	BlockMove(fxGrid, fxGridRot, (long)64 * sizeof(FxVector));

	for (i = 0; i < GRID_SZ; i++)
		FxPlotLine(&fxGrid[i][0], &fxGrid[i][GRID_SZ - 1]);

	for (j = 0; j < GRID_SZ; j++)
		FxPlotLine(&fxGrid[0][j], &fxGrid[GRID_SZ - 1][j]);
}

InitVars()
{
	int		i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			if (i EQ j)
				fracRotatMatrx[i][j] = FRAC_ONE;
			else
				fracRotatMatrx[i][j] = 0;
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			if (i EQ j)
				transpCurrFxRotMatrx[i][j] = FIX_ONE;
			else
				transpCurrFxRotMatrx[i][j] = 0;

/*	sinPtOne = FracSin(DToFix(.1));*/
{double	l;
l = (long)
		ldexp(0.1,16);
}

sinPtOne = FracSin(
		(long)
		ldexp(0.1,16)
		);


	cosPtOne = FracCos(DToFix(.1));
	sinPtTwo = FracSin(DToFix(.2));
	cosPtTwo = FracCos(DToFix(.2));
	sinNPtOne = FracSin(DToFix(-.1));
	cosNPtOne = FracCos(DToFix(-.1));
	sinNPtTwo = FracSin(DToFix(-.2));
	cosNPtTwo = FracCos(DToFix(-.2));
}

	
typedef Fixed (*FProcPtr)() ;

Fixed	(*FixMAddrs)(), (*FixDAddrs)();

AddrsSet()
{
	FixMAddrs = (void *)GetTrapAddress(0x68);
	FixDAddrs = (void *)GetTrapAddress(0x00);	/* I don't know what this should be */
}

long
_FixMul(a, b)
long	a, b;
{
asm{
	subq.l	#4,sp
	move.l	a,-(sp)
	move.l	b,-(sp)
	move.l	FixMAddrs,a0
	jsr		(a0)
	move.l	(sp)+,d0
	}
}

long
_FixDiv(a, b)
long	a, b;
{
asm{
	subq.l	#4,sp
	move.l	a,-(sp)
	move.l	b,-(sp)
	move.l	FixDAddrs,a0
	jsr		(a0)
	move.l	(sp)+,d0
	}
}
