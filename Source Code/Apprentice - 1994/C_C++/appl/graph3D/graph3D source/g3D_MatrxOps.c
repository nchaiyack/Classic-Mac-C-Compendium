/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"
#ifndef	_LSC3_
#	include	<QuickDraw.h>
#endif


extern	Real	rotMatrx[3][3];
extern	Real	scale;
extern	Point	Origin;


extern	Real	rotMatrx[3][3];

#ifdef FIXED_MATH

Fixed	_rotMatrx[3][3];

FxMatrxToMatrx(fxMat, matrx)
register	Fixed	fxMat[3][3];
register	Real	matrx[3][3];
{
	register	int	i, j;

	for (i = 0; i < 3; i++)
		for (j = 0; j < 3; j++)
			matrx[i][j] = FixToD(fxMat[i][j]);
}

FxConv3VtoPt(vectP, cnvPntP)
FxVector	*vectP;
register	Point	*cnvPntP;
{
	FxVector result;

	FxMatrxMul(_rotMatrx, vectP, &result);

	cnvPntP->h = FixToi((result.y)) + Origin.h;
	cnvPntP->v = FixToi((-result.z)) + Origin.v;
}

/**** Matrix to rotate 3-vector on three axes ****/
FxMkRotMat(Xrot, Yrot, Zrot, matrx)
Real Xrot, Yrot, Zrot;
register	Fixed	matrx[3][3];
{
	Real cXrot , cYrot , cZrot ;
	Real sXrot , sYrot , sZrot ;

	cXrot = cos(Xrot);
	cYrot = cos(Yrot);
	cZrot = cos(Zrot);
	sXrot = sin(Xrot);
	sYrot = sin(Yrot);
	sZrot = sin(Zrot);


	matrx[0][0] = DToFix(cYrot * cZrot);
	matrx[0][1] = DToFix(cZrot * sXrot * sYrot - sZrot * cXrot);
	matrx[0][2] = DToFix(-(cXrot * cZrot * sYrot + sXrot * sZrot));
	matrx[1][0] = DToFix(sZrot * cYrot);
	matrx[1][1] = DToFix(sXrot * sYrot * sZrot + cXrot * cZrot);
	matrx[1][2] = DToFix(-sYrot * sZrot * cXrot + sXrot * cZrot);
	matrx[2][0] = DToFix(sYrot);
	matrx[2][1] = DToFix(-sXrot * cYrot);
	matrx[2][2] = DToFix(cXrot * cYrot);
}


FxMkXRotMat(Xrot, matrx)
Real		Xrot;
register	Fixed	matrx[3][3];
{
	Fixed	cXrot, sXrot;

	FxMkIDMat(matrx);

	cXrot = DToFix(cos(Xrot));
	sXrot = DToFix(sin(Xrot));

	matrx[1][1] = cXrot;
	matrx[1][2] = sXrot;
	matrx[2][1] = -sXrot;
	matrx[2][2] = cXrot;
}

FxMkYRotMat(Yrot, matrx)
Real		Yrot;
register	Fixed	matrx[3][3];
{
	Fixed	cYrot, sYrot;

	FxMkIDMat(matrx);

	cYrot = DToFix(cos(Yrot));
	sYrot = DToFix(sin(Yrot));

	matrx[0][0] = cYrot;
	matrx[0][2] = -sYrot;
	matrx[2][0] = sYrot;
	matrx[2][2] = cYrot;
}

FxMkZRotMat(Zrot, matrx)
Real		Zrot;
register	Fixed	matrx[3][3];
{
	Fixed	cZrot, sZrot;

	FxMkIDMat(matrx);

	cZrot = DToFix(cos(Zrot));
	sZrot = DToFix(sin(Zrot));

	matrx[0][0] = cZrot;
	matrx[0][1] = -sZrot;
	matrx[1][0] = sZrot;
	matrx[1][1] = cZrot;
}

FxMkIDMat(matrx)
register	Fixed	matrx[3][3];
{
	matrx[0][0] = FIX_1;
	matrx[0][1] = FIX_0;
	matrx[0][2] = FIX_0;
	matrx[1][0] = FIX_0;
	matrx[1][1] = FIX_1;
	matrx[1][2] = FIX_0;
	matrx[2][0] = FIX_0;
	matrx[2][1] = FIX_0;
	matrx[2][2] = FIX_1;
}

FxMatrxMul(mat, vect, resVect)
register	Fixed	mat[3][3];
register	FxVector	*vect;
register	FxVector	*resVect;
{
	resVect->x =
		FixMul(mat[0][0], vect->x) +
		FixMul(mat[0][1], vect->y) +
		FixMul(mat[0][2], vect->z);
	resVect->y =
		FixMul(mat[1][0], vect->x) +
		FixMul(mat[1][1], vect->y) +
		FixMul(mat[1][2], vect->z);
	resVect->z =
		FixMul(mat[2][0], vect->x) +
		FixMul(mat[2][1], vect->y) +
		FixMul(mat[2][2], vect->z);
}

FxMatrxByMatrx(mat1, mat2, resMat)
register	Fixed	mat1[3][3], mat2[3][3], resMat[3][3];
{
	register	int	i, j;

	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			resMat[i][j] =	FixMul(mat1[i][0], mat2[0][j]) +
							FixMul(mat1[i][1], mat2[1][j]) +
							FixMul(mat1[i][2], mat2[2][j]);
}

FxCopyMat(sorc, dest)
register	cpyFx3Matrx	*sorc, *dest;
{
	*dest = *sorc;
}

#endif


/*---------------------------------------------------------------------------*/




Conv3VtoPt(vectP, cnvPntP)
Vector	*vectP;
register	Point	*cnvPntP;
{
	Vector result;

	MatrxMul(rotMatrx, vectP, &result);

	cnvPntP->h = result.y + Origin.h;
	cnvPntP->v = (-result.z) + Origin.v;
}

MatrxMul(mat, vect, resVect)
register	Real	mat[3][3];
register	Vector	*vect;
register	Vector	*resVect;
{
	resVect->x =
		mat[0][0] * vect->x + mat[0][1] * vect->y + mat[0][2] * vect->z;
	resVect->y =
		mat[1][0] * vect->x + mat[1][1] * vect->y + mat[1][2] * vect->z;
	resVect->z =
		mat[2][0] * vect->x + mat[2][1] * vect->y + mat[2][2] * vect->z;
}

matrxByMatrx(mat1, mat2, resMat)
register	Real	mat1[3][3], mat2[3][3], resMat[3][3];
{
	register	int	i, j;

	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			resMat[i][j] =	mat1[i][0] * mat2[0][j] +
							mat1[i][1] * mat2[1][j] +
							mat1[i][2] * mat2[2][j];
}
typedef Real Matrix[3][3];

/**** Matrix to rotate 3-vector on three axes ****/
MkRotMat(Xrot, Yrot, Zrot, matrx)
Real Xrot, Yrot, Zrot;
register	Matrix	matrx;
{
	Real cXrot , cYrot , cZrot ;
	Real sXrot , sYrot , sZrot ;

	cXrot = cos(Xrot);
	cYrot = cos(Yrot);
	cZrot = cos(Zrot);
	sXrot = sin(Xrot);
	sYrot = sin(Yrot);
	sZrot = sin(Zrot);

	matrx[0][0] = cYrot * cZrot;
	matrx[0][1] = cZrot * sXrot * sYrot - sZrot * cXrot;
	matrx[0][2] = -(cXrot * cZrot * sYrot + sXrot * sZrot);
	matrx[1][0] = sZrot * cYrot;
	matrx[1][1] = sXrot * sYrot * sZrot + cXrot * cZrot;
	matrx[1][2] = -sYrot * sZrot * cXrot + sXrot * cZrot;
	matrx[2][0] = sYrot;
	matrx[2][1] = -sXrot * cYrot;
	matrx[2][2] = cXrot * cYrot;
}


MatrxByMatrx(mat1, mat2, resMat)
register	Real	mat1[3][3], mat2[3][3], resMat[3][3];
{
	register	int	i, j;

	for(i = 0; i < 3; i++)
		for(j = 0; j < 3; j++)
			resMat[i][j] =	mat1[i][0] * mat2[0][j] +
							mat1[i][1] * mat2[1][j] +
							mat1[i][2] * mat2[2][j];
}


CopyMat(sorc, dest)
register	cpy3Matrx	*sorc, *dest;
{
	*dest = *sorc;
}

MkIDMat(matrx)
register	Real	matrx[3][3];
{
	matrx[0][0] = 1;
	matrx[0][1] = 0;
	matrx[0][2] = 0;
	matrx[1][0] = 0;
	matrx[1][1] = 1;
	matrx[1][2] = 0;
	matrx[2][0] = 0;
	matrx[2][1] = 0;
	matrx[2][2] = 1;
}



MkXRotMat(Xrot, matrx)
Real		Xrot;
register	Real	matrx[3][3];
{
	Real	cXrot, sXrot;

	MkIDMat(matrx);

	cXrot = cos(Xrot);
	sXrot = sin(Xrot);

	matrx[1][1] = cXrot;
	matrx[1][2] = sXrot;
	matrx[2][1] = -sXrot;
	matrx[2][2] = cXrot;
}

MkYRotMat(Yrot, matrx)
Real		Yrot;
register	Real	matrx[3][3];
{
	Real	cYrot, sYrot;

	MkIDMat(matrx);

	cYrot = cos(Yrot);
	sYrot = sin(Yrot);

	matrx[0][0] = cYrot;
	matrx[0][2] = -sYrot;
	matrx[2][0] = sYrot;
	matrx[2][2] = cYrot;
}

MkZRotMat(Zrot, matrx)
Real		Zrot;
register	Real	matrx[3][3];
{
	Real	cZrot, sZrot;

	MkIDMat(matrx);

	cZrot = cos(Zrot);
	sZrot = sin(Zrot);

	matrx[0][0] = cZrot;
	matrx[0][1] = -sZrot;
	matrx[1][0] = sZrot;
	matrx[1][1] = cZrot;
}
