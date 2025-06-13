/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"

extern	Boolean	useMainFunc;

extern	Real		*funcResults;
extern	Vector		*scalVectResults;
extern	Point		*graphPoints;	/* same elements as funcResults */

extern	Boolean		invldFormla;

extern	Boolean		functCurrent;
extern	Boolean		vectrsCurrent;

extern	Boolean		noRedrawGrph;

extern	Vector		maxVect, minVect;
extern	Real		rotMatrx[3][3];

extern	Boolean		inMultiFndr;

Real	startR, startT;
Real	endR, endT;
Real	deltaR, deltaT;
int		numR, numT;


EvalCylindFunct()
{
	Boolean	AllocateCylndPts();

	invldFormla = FALSE;
	noRedrawGrph = TRUE;

	LoadFunction();

	if (invldFormla)	/* already gave warning message */
		return;

	GetSpecDCont();

	deltaR = (endR - startR) / (numR - 1);
	deltaT = (endT - startT) / (numT - 1);

	if (NOT(AllocateCylndPts()))
		return;

	StoreCylindFunct();

	vectrsCurrent = FALSE;
	functCurrent = TRUE;
	noRedrawGrph = FALSE;
}

static
Boolean
AllocateCylndPts()
{
	if (funcResults NEQ NIL)
		free(funcResults);
	if (scalVectResults NEQ NIL)
		free((void *)scalVectResults);
	if (graphPoints NEQ NIL)
		free((void *)graphPoints);
	funcResults = (Real *)NewPtr((long)sizeof(Real) * (long)numR * numT);
	if (MemErr)
		return(FALSE);
	scalVectResults = (Vector *)NewPtr((long)sizeof(Vector) * (long)numR * numT);
	if (MemErr)
		return(FALSE);
	graphPoints = (Point *)NewPtr((long)sizeof(Point) * (long)numR * numT);
	if (MemErr)
		return(FALSE);

	return(TRUE);
}

static
StoreCylindFunct()
{
	register	int	rCnt, tCnt;
	Real		currR, currT;
	Real		maxFunc, minFunc, funcVal;
	Real		rAtMax, tAtMax;
	Real		rAtMin, tAtMin;
	Real		TheFunction();
	Boolean		CmndPeriod();
	
	ShowMeter((long)numR * numT);

	useMainFunc = TRUE;
	funcVal = TheFunction(startR, startT);
	maxFunc = funcVal;
	minFunc = funcVal;
	rAtMax = startR;
	tAtMax = startT;
	rAtMin = startR;
	tAtMin = startT;

	currR = startR;
	for (rCnt = 0; rCnt < numR; rCnt++)
	{
		currT = startT;
		for (tCnt = 0; tCnt < numT; tCnt++)
		{
			funcVal = TheFunction(currR, currT);
			DisplayMeter();
			funcResults[rCnt * numT + tCnt] = funcVal;
			if (funcVal > maxFunc)
			{
				maxFunc = funcVal;
				rAtMax = currR;
				tAtMax = currT;
			}
			else if (funcVal < minFunc)
			{
				minFunc = funcVal;
				rAtMin = currR;
				tAtMin = currT;
			}
			currT += deltaT;
		}
		if (CmndPeriod())
			break;  /* for */
		currR += deltaR;
	}

done:
	maxVect.x = rAtMax * cos(tAtMax);
	maxVect.y = rAtMax * sin(tAtMax);
	maxVect.z = maxFunc;

	minVect.x = rAtMin * cos(tAtMin);
	minVect.y = rAtMin * sin(tAtMin);
	minVect.z = minFunc;

	RemoveMeter();
}



SetCylindVectrs()
{
register	int	rCnt, tCnt;
register	long	offSet;
	Real		ImposLimits();
register	Real		rValue, tValue;
register	Vector		*scalePtr;
	Real	SdeltaR;
register	Real	SdeltaT;
	Vector	pntVect;

	SdeltaR = deltaR;
	SdeltaT = deltaT;

	scalePtr = scalVectResults;

	rValue = startR;

	for (rCnt = 0; rCnt < numR; rCnt++)
	{
		tValue = startT;
		for (tCnt = 0; tCnt < numT; tCnt++)
		{
			offSet = (long)rCnt * (long)numT + tCnt;
			pntVect.x = rValue * cos(tValue);
			pntVect.y = rValue * sin(tValue);
			pntVect.z = (ImposLimits(funcResults[offSet]));
			MatrxMul(rotMatrx, &pntVect, scalePtr++);
			tValue += SdeltaT;
		}
		rValue += SdeltaR;
	}

	vectrsCurrent = TRUE;
}



/*--------------------------------------------------------------------------------*/






__SetCylindVectrs()
{
register	int	rCnt, tCnt;
register	long	offSet;
	Real		ImposLimits();
register	Real		rValue, tValue;
register	Vector		*scalePtr;
	Real	SdeltaR;
register	Real	SdeltaT;
	Vector	pntVect;

	SdeltaR = deltaR;
	SdeltaT = deltaT;

	scalePtr = scalVectResults;

	rValue = startR;

	for (rCnt = 0; rCnt < numR; rCnt++)
	{
		tValue = startT;
		for (tCnt = 0; tCnt < numT; tCnt++)
		{
			offSet = (long)rCnt * (long)numT + tCnt;
			pntVect.x = ImposLimits(rValue*tValue);
			pntVect.y = ImposLimits(sin(tValue)+cos(rValue));
			pntVect.z = ImposLimits(cos(rValue));
			MatrxMul(rotMatrx, &pntVect, scalePtr++);
			tValue += SdeltaT;
		}
		rValue += SdeltaR;
	}

	vectrsCurrent = TRUE;
}
