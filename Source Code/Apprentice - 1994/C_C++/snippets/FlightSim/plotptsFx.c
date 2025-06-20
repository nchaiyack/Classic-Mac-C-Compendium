#include "flight.h"


FxVector	fcubes[8] = {{-1, -1, -1}, {1, -1, -1}, {1, 1, -1},
{-1, 1, -1}, {-1, 1, 1}, {-1, -1, 1}, {1, -1, 1}, {1, 1, 1}};

Point	Origin = {128, 128};
int		magnify = 0;
FxThreeMatrx	fxRotMatrx;


/**** Matrix to rotate 3-vector on three axes ****/
FxMkRotMat(Xrot, Yrot, Zrot, matrx)
double Xrot, Yrot, Zrot;
register	FxThreeMatrx matrx;
{
	double cXrot , cYrot , cZrot ;
	double sXrot , sYrot , sZrot ;

	cXrot = cos(Xrot);
	cYrot = ldexp(cos(Yrot), magnify);
	cZrot = cos(Zrot);
	sXrot = sin(Xrot);
	sYrot = ldexp(sin(Yrot), magnify);
	sZrot = sin(Zrot);

	matrx[0][0] = DToFix(cYrot * cZrot);
	matrx[0][1] = DToFix(cZrot * sXrot * sYrot - ldexp(sZrot * cXrot, magnify));
	matrx[0][2] = DToFix(-(cXrot * cZrot * sYrot + ldexp(sXrot * sZrot, magnify)));
	matrx[1][0] = DToFix(sZrot * cYrot);
	matrx[1][1] = DToFix(sXrot * sYrot * sZrot + ldexp(cXrot * cZrot, magnify));
	matrx[1][2] = DToFix(-sYrot * sZrot * cXrot + ldexp(sXrot * cZrot, magnify));
	matrx[2][0] = DToFix(sYrot);
	matrx[2][1] = DToFix(-sXrot * cYrot);
	matrx[2][2] = DToFix(cXrot * cYrot);
}
/*	matrx[0][0] = cYrot * cZrot
	matrx[0][1] = cZrot * sXrot * sYrot - sZrot * cXrot
	matrx[0][2] = -(cXrot * cZrot * sYrot + sXrot * sZrot)
	matrx[1][0] = sZrot * cYrot
	matrx[1][1] = sXrot * sYrot * sZrot + cXrot * cZrot
	matrx[1][2] = -sYrot * sZrot * cXrot + sXrot * cZrot
	matrx[2][0] = sYrot
	matrx[2][1] = -sXrot * cYrot
	matrx[2][2] = cXrot * cYrot*/


FracXRotatMatrx(cXrot, sXrot, oMatrx)
Fract	cXrot, sXrot;
register	FracThreeMatrx	oMatrx;
{
	FracThreeMatrx	tMatrx;

	tMatrx[0][0] = oMatrx[0][0];
	tMatrx[0][1] = oMatrx[0][1];
	tMatrx[0][2] = oMatrx[0][2];
	tMatrx[1][0] = FracMul(oMatrx[1][0], cXrot) + FracMul(oMatrx[2][0], sXrot);
	tMatrx[1][1] = FracMul(oMatrx[1][1], cXrot) + FracMul(oMatrx[2][1], sXrot);
	tMatrx[1][2] = FracMul(oMatrx[1][2], cXrot) + FracMul(oMatrx[2][2], sXrot);
	tMatrx[2][0] = FracMul(oMatrx[2][0], cXrot) - FracMul(oMatrx[1][0], sXrot);
	tMatrx[2][1] = FracMul(oMatrx[2][1], cXrot) - FracMul(oMatrx[1][1], sXrot);
	tMatrx[2][2] = FracMul(oMatrx[2][2], cXrot) - FracMul(oMatrx[1][2], sXrot);
	*(FracThreeMatrxStrc *)(oMatrx) = *(FracThreeMatrxStrc *)(tMatrx);
}

FracYRotatMatrx(cYrot, sYrot, oMatrx)
Fract	cYrot, sYrot;
register	FracThreeMatrx	oMatrx;
{
	FracThreeMatrx	tMatrx;

	tMatrx[0][0] = FracMul(oMatrx[0][0], cYrot) - FracMul(oMatrx[2][0], sYrot);
	tMatrx[0][1] = FracMul(oMatrx[0][1], cYrot) - FracMul(oMatrx[2][1], sYrot);
	tMatrx[0][2] = FracMul(oMatrx[0][2], cYrot) - FracMul(oMatrx[2][2], sYrot);
	tMatrx[1][0] = oMatrx[1][0];
	tMatrx[1][1] = oMatrx[1][1];
	tMatrx[1][2] = oMatrx[1][2];
	tMatrx[2][0] = FracMul(oMatrx[0][0], sYrot) + FracMul(oMatrx[2][0], cYrot);
	tMatrx[2][1] = FracMul(oMatrx[0][1], sYrot) + FracMul(oMatrx[2][1], cYrot);
	tMatrx[2][2] = FracMul(oMatrx[0][2], sYrot) + FracMul(oMatrx[2][2], cYrot);
	*(FracThreeMatrxStrc *)(oMatrx) = *(FracThreeMatrxStrc *)(tMatrx);
}

FracZRotatMatrx(cZrot, sZrot, oMatrx)
Fract	cZrot, sZrot;
register	FracThreeMatrx	oMatrx;
{
	FracThreeMatrx	tMatrx;

	tMatrx[0][0] = FracMul(oMatrx[0][0], cZrot) - FracMul(oMatrx[1][0], sZrot);
	tMatrx[0][1] = FracMul(oMatrx[0][1], cZrot) - FracMul(oMatrx[1][1], sZrot);
	tMatrx[0][2] = FracMul(oMatrx[0][2], cZrot) - FracMul(oMatrx[1][2], sZrot);
	tMatrx[1][0] = FracMul(oMatrx[1][0], cZrot) + FracMul(oMatrx[0][0], sZrot);
	tMatrx[1][1] = FracMul(oMatrx[1][1], cZrot) + FracMul(oMatrx[0][1], sZrot);
	tMatrx[1][2] = FracMul(oMatrx[1][2], cZrot) + FracMul(oMatrx[0][2], sZrot);
	tMatrx[2][0] = oMatrx[2][0];
	tMatrx[2][1] = oMatrx[2][1];
	tMatrx[2][2] = oMatrx[2][2];
	*(FracThreeMatrxStrc *)(oMatrx) = *(FracThreeMatrxStrc *)(tMatrx);
}

FractToFixMatrx(srcMat, destMat)
register	FracThreeMatrx srcMat;
register	FxThreeMatrx destMat;
{
	register	int	i, j;
	
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			destMat[i][j] = Frac2Fix(srcMat[i][j]);
}

FxTranspMatrx(srcMat, destMat)
register	FracThreeMatrx srcMat;
register	FxThreeMatrx destMat;
{
	register	int	i, j;
	
	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			destMat[i][j] = srcMat[j][i];
}

FxMatrxMul(mat, vect, resVect)
register	FxThreeMatrx mat;
register	FxVector *vect;
register	FxVector *resVect;
{
	resVect->x = _FixMul(mat[0][0], vect->x) + _FixMul(mat[0][1], vect->y) + _FixMul(mat[0][2], vect->z);
	resVect->y = _FixMul(mat[1][0], vect->x) + _FixMul(mat[1][1], vect->y) + _FixMul(mat[1][2], vect->z);
	resVect->z = _FixMul(mat[2][0], vect->x) + _FixMul(mat[2][1], vect->y) + _FixMul(mat[2][2], vect->z);
}

FxMatrxByMatrx(mat1, mat2, resMat)
register	FxThreeMatrx mat1, mat2, resMat;
{
	register	int	i, j;

	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
		{
			resMat[i][j] = _FixMul(mat1[i][0], mat2[0][j])
				+ _FixMul(mat1[i][1], mat2[1][j])
				+ _FixMul(mat1[i][2], mat2[2][j]);
		}
}

FxSubtVector(a, b, result)
register	FxVector	*a, *b, *result;
{
	result->x = a->x - b->x;
	result->y = a->y - b->y;
	result->z = a->z - b->z;
}

FxAddVector(a, b, result)
register	FxVector	*a, *b, *result;
{
	result->x = a->x + b->x;
	result->y = a->y + b->y;
	result->z = a->z + b->z;
}

FxPlotLine(strt, end)
FxVector	*strt, *end;
{
	FxVector 	delta;
	FxVector	resStrt, resEnd;
	Boolean	strtFlgs[5], endFlgs[5], done, clipped;
register	int		i;
register	Boolean		noneSet;

	delta.x = end->x - strt->x;
	delta.y = end->y - strt->y;
	delta.z = end->z - strt->z;
	resStrt = *strt;
	resEnd = *end;

	FxEvalPoint(&resStrt, strtFlgs);
	FxEvalPoint(&resEnd, endFlgs);

	done = clipped = FALSE;
	while (NOT(done))
	{
		for (i = 0; i < 5; i++)
			if (strtFlgs[i] AND endFlgs[i])
				return;
		i = 0;
		while ((i < 4) AND NOT(strtFlgs[i]))
			i++;
		if (i < 4)
		{
			FxPush(&resStrt, &delta, i);
			clipped = TRUE;
			FxEvalPoint(&resStrt, strtFlgs);
		}
		else
		{
			i = 0;
			while ((i < 4) AND NOT(endFlgs[i]))
				i++;
			if (i < 4)
			{
				FxNegatVect(&delta);
				FxPush(&resEnd, &delta, i);
				FxNegatVect(&delta);
				clipped = TRUE;
				FxEvalPoint(&resEnd, endFlgs);
			}
			else
			{
				FxDraw(&resStrt, &resEnd, clipped);
				done = TRUE;
			}
		}
	}
}

FxNegatVect(theVect)
register	FxVector	*theVect;
{
	theVect->x = -theVect->x;
	theVect->y = -theVect->y;
	theVect->z = -theVect->z;
}

FxEvalPoint(theVect, flgArr)
register	FxVector	*theVect;
register	Boolean	flgArr[5];
{
	flgArr[0] = theVect->x > theVect->z;
	flgArr[1] = theVect->z > -theVect->x;
	flgArr[2] = theVect->y > -theVect->x;
	flgArr[3] = theVect->x > theVect->y;
	flgArr[4] = theVect->x >= 0;
}

FxPush(theVect, delta, plane)
register	FxVector	*theVect, *delta;
int		plane;
{
	Fixed	t;

	switch (plane) {
	case 0:
		t = FixDiv(theVect->z - theVect->x, delta->x - delta->z);
		theVect->x = _FixMul(delta->x, t) + theVect->x;
		theVect->y = _FixMul(delta->y, t) + theVect->y;
		theVect->z = theVect->x;
		break;
	case 1:
		t = FixDiv(-(theVect->x + theVect->z), delta->x + delta->z);
		theVect->x = _FixMul(delta->x, t) + theVect->x;
		theVect->y = _FixMul(delta->y, t) + theVect->y;
		theVect->z = -theVect->x;
		break;
	case 2:
		t = FixDiv(-(theVect->x + theVect->y), delta->x + delta->y);
		theVect->x = _FixMul(delta->x, t) + theVect->x;
		theVect->y = -theVect->x;
		theVect->z = _FixMul(delta->z, t) + theVect->z;
		break;
	case 3:
		t = FixDiv(theVect->x - theVect->y, delta->y - delta->x);
		theVect->x = _FixMul(delta->x, t) + theVect->x;
		theVect->y = theVect->x;
		theVect->z = _FixMul(delta->z, t) + theVect->z;
		break;
	}
}		
	
FxDraw(strt, end, clipped)
register	FxVector		*strt, *end;
Boolean		clipped;
{
	int		strtH, strtV, endH, endV;

	strtH = FixToi(FixDiv(strt->y, -strt->x) << 7) + Origin.h;
	strtV = FixToi(FixDiv(-strt->z, -strt->x) << 7) + Origin.v;
	endH = FixToi(FixDiv(end->y, -end->x) << 7) + Origin.h;
	endV = FixToi(FixDiv(-end->z, -end->x) << 7) + Origin.v;

	MoveTo(strtH, strtV);
	LineTo(endH, endV);
}