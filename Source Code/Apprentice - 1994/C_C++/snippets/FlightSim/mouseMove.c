#include "flight.h"


extern	FxVector	fxGrid[GRID_SZ][GRID_SZ];
extern	FxVector	fxGridRot[GRID_SZ][GRID_SZ];
extern	FxVector	fxOriginV;

extern	FracThreeMatrx	fracRotatMatrx;
extern	FxThreeMatrx	transpCurrFxRotMatrx;

extern	Fract	sinPtOne, cosPtOne, sinPtTwo, cosPtTwo;
extern	Fract	sinNPtOne, cosNPtOne, sinNPtTwo, cosNPtTwo;

extern	WindowPtr		theWind;	/*the main window*/


FxVector	pyramid	= {iToFix(-200), iToFix(-100), iToFix(25)},
			cpyPyramid	= {iToFix(-200), iToFix(-100), iToFix(25)};

DoMouseMove(strtClik)
Point	*strtClik;
{
static	int		lookUp[6][6] = 
	{	11,	11,	16,	16,	15,	15,
		11,	12,	13,	13,	14,	15,
		6,	7,	8,	8,	9,	10,
		6,	7,	8,	8,	9,	10,
		0,	3,	4,	4,	5,	2,
		0,	0,	1,	1,	2,	2};

	int		hCoord, vCoord;
	Point	clikPt;

	clikPt = *strtClik;

	while (StillDown())
	{
		hCoord = clikPt.h / 43;
		vCoord = clikPt.v / 43;

		SetBack();
		DoMove(lookUp[vCoord][hCoord]);
		GetWind(theWind);
		SetPort(theWind);
		GetMouse(&clikPt);
	}
}

DoMove(posCoord)
int		posCoord;
{

	register	int	i, j;
	FxVector	tempV, _fxGrid[GRID_SZ][GRID_SZ];
	Point	clikPt;
	FxThreeMatrx	currFxRotMatrx;
static	FxVector	xIncrmnt = {iToFix(stepSize), 0, 0};

	switch	(posCoord) {
	case 0:
		FracYRotatMatrx(cosPtTwo, sinPtTwo, fracRotatMatrx);
		FracXRotatMatrx(cosPtTwo, sinPtTwo, fracRotatMatrx);
		break;
	case 1:
		FracYRotatMatrx(cosPtTwo, sinPtTwo, fracRotatMatrx);
		break;
	case 2:
		FracYRotatMatrx(cosPtTwo, sinPtTwo, fracRotatMatrx);
		FracXRotatMatrx(cosNPtTwo, sinNPtTwo, fracRotatMatrx);
		break;
	case 3:
		FracYRotatMatrx(cosPtOne, sinPtOne, fracRotatMatrx);
		FracXRotatMatrx(cosPtOne, sinPtOne, fracRotatMatrx);
		break;
	case 4:
		FracYRotatMatrx(cosPtOne, sinPtOne, fracRotatMatrx);
		break;
	case 5:
		FracYRotatMatrx(cosPtOne, sinPtOne, fracRotatMatrx);
		FracXRotatMatrx(cosNPtOne, sinNPtOne, fracRotatMatrx);
		break;
	case 6:
		FracXRotatMatrx(cosPtTwo, sinPtTwo, fracRotatMatrx);
		break;
	case 7:
		FracXRotatMatrx(cosPtOne, sinPtOne, fracRotatMatrx);
		break;
	case 8:
		DoForward();
		break;
	case 9:
		FracXRotatMatrx(cosNPtOne, sinNPtOne, fracRotatMatrx);
		break;
	case 10:
		FracXRotatMatrx(cosNPtTwo, sinNPtTwo, fracRotatMatrx);
		break;
	case 11:
		FracYRotatMatrx(cosNPtTwo, sinNPtTwo, fracRotatMatrx);
		FracXRotatMatrx(cosPtTwo, sinPtTwo, fracRotatMatrx);
		break;
	case 12:
		FracYRotatMatrx(cosNPtOne, sinNPtOne, fracRotatMatrx);
		FracXRotatMatrx(cosPtOne, sinPtOne, fracRotatMatrx);
		break;
	case 13:
		FracYRotatMatrx(cosNPtOne, sinNPtOne, fracRotatMatrx);
		break;
	case 14:
		FracXRotatMatrx(cosNPtOne, sinNPtOne, fracRotatMatrx);
		break;
	case 15:
		FracXRotatMatrx(cosNPtTwo, sinNPtTwo, fracRotatMatrx);
		break;
	case 16:
		FracYRotatMatrx(cosNPtTwo, sinNPtTwo, fracRotatMatrx);
		break;
	}

	if (posCoord NEQ 8)
	{
		FractToFixMatrx(fracRotatMatrx, currFxRotMatrx);
		FxTranspMatrx(currFxRotMatrx, transpCurrFxRotMatrx);
DoStep();

		for (i = 0; i < GRID_SZ; i++)
			for (j = 0; j < GRID_SZ; j++)
			{
				FxSubtVector(&fxGrid[i][j], &fxOriginV, &tempV);
				FxMatrxMul(currFxRotMatrx, &tempV, &_fxGrid[i][j]);
			}
FxSubtVector(&pyramid, &fxOriginV, &tempV);
FxMatrxMul(currFxRotMatrx, &tempV, &cpyPyramid);

		EraseRect(&thePort->portRect);
		for (i = 0; i < GRID_SZ; i++)
			FxPlotLine(&_fxGrid[i][0], &_fxGrid[i][GRID_SZ - 1]);
	
		for (j = 0; j < GRID_SZ; j++)
			FxPlotLine(&_fxGrid[0][j], &_fxGrid[GRID_SZ - 1][j]);
		*(FxGridMatrx *)fxGridRot = *(FxGridMatrx *)_fxGrid;	
DrawPyr();
	}
}

DoStep()
{
static	FxVector	xIncrmnt = {iToFix(stepSize), 0, 0};
	FxVector	tempV;

	FxMatrxMul(transpCurrFxRotMatrx, &xIncrmnt, &tempV);
	FxSubtVector(&fxOriginV, &tempV, &fxOriginV);
}

DoForward()
{
	FxVector	tempV;
	register	int	i, j;
static	FxVector	xIncrmnt = {iToFix(stepSize), 0, 0};

	FxMatrxMul(transpCurrFxRotMatrx, &xIncrmnt, &tempV);
	FxSubtVector(&fxOriginV, &tempV, &fxOriginV);
	for (i = 0; i < GRID_SZ; i++)
		for (j = 0; j < GRID_SZ; j++)
			fxGridRot[i][j].x += iToFix(stepSize);
cpyPyramid.x += iToFix(stepSize);

	EraseRect(&thePort->portRect);
	for (i = 0; i < GRID_SZ; i++)
		FxPlotLine(&fxGridRot[i][0], &fxGridRot[i][GRID_SZ - 1]);

	for (j = 0; j < GRID_SZ; j++)
		FxPlotLine(&fxGridRot[0][j], &fxGridRot[GRID_SZ - 1][j]);
DrawPyr();
}

DrawPyr()
{
	FxPlotLine(&fxGridRot[0][0], &cpyPyramid);
	FxPlotLine(&fxGridRot[1][0], &cpyPyramid);
	FxPlotLine(&fxGridRot[0][1], &cpyPyramid);
	FxPlotLine(&fxGridRot[1][1], &cpyPyramid);
}
