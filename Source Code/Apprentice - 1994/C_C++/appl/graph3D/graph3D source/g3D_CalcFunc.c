/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"

#ifdef	_LSC3_
#	include	<QuickDraw.h>
#	include	<MemoryMgr.h>
#endif


extern	Boolean		useMainFunc;

extern	int		mainFunct[64];
extern	int		numMnOps;
extern	Real	mnConsts[30];

extern	int		derivFunct[64];
extern	int		numDerivOps;
extern	Real	derivConsts[30];


Real
TheFunction(x, y, z)
Real	x, y, z;
{
	Real	stack[50];
register	int		i, currOp;
	Real		StackOp(), FuncEval(), ArithEval();
register	Real	*stackPtr;
	int		currConst = 0;
	int		numOps;

	stackPtr = stack + 49;
	if (useMainFunc)
		if (numMnOps EQ 0)
			return(0.0);
		else
			numOps = numMnOps;
	else
		if (numDerivOps EQ 0)
			return(0.0);
		else
			numOps = numDerivOps;

	*stackPtr = 0.0;
	for (i = 0; i < numOps; i++)
	{
		if (useMainFunc)
			currOp = mainFunct[i];
		else
			currOp = derivFunct[i];
		if (IS_PUSH(currOp))
		{
			--stackPtr;
			switch (currOp) {
			case PUSH_X_OP:
				*stackPtr = x;
				break;
			case PUSH_Y_OP:
				*stackPtr = y;
				break;
			case PUSH_Z_OP:
				*stackPtr = z;
				break;
			case PUSH_NUM_OP:
				if (useMainFunc)
					*stackPtr = mnConsts[currConst++];
				else
					*stackPtr = derivConsts[currConst++];
				break;
			}
		}
		else if (IS_FUNC(currOp))
			*stackPtr = FuncEval(*stackPtr, currOp);
		else
		{
			stackPtr[1] = ArithEval(stackPtr[1], stackPtr[0], currOp);
			++stackPtr;
		}
	}

	return(*stackPtr);
}

static
Real
ArithEval(arg1, arg2, op)
Real	arg1, arg2;
int		op;
{
extern	int	graphType;

	switch (op) {
	case MULT_OP:
		return(arg1 * arg2);
	case DIV_OP:
		if (graphType EQ DERIV_GTYPE)
			if (Abs(arg2) > 1e-50)
				return(arg1 / arg2);
			else
				return(0.0);
		else
			return(arg1 / arg2);
	case ADD_OP:
		return(arg1 + arg2);
	case SUBT_OP:
		return(arg1 - arg2);
	case MOD_OP:
		return(round(arg1) MOD round(arg2));
	case POWER_OP:
		return(pow(arg1, arg2));
	}
}

kw()
{
	if (NOT(OptionDown()))
	{
		while(NOT(ShiftDown()));
		while(ShiftDown());
	}
}

es()
{
	ExitToShell();
}

static
Real
FuncEval(arg, op)
Real	arg;
int		op;
{
	switch (op) {
	case NEG_OP:
		return(-arg);
	case EXP_OP:
		return(exp(arg));
	case LOG_OP:
		return(log(arg));
	case LOG10_OP:
		return(log10(arg));
	case SQ_OP:
		return(SQ(arg));
	case SQRT_OP:
		if (arg > 0)
			return(sqrt(arg));
		else
			return(0.0);
	case SIN_OP:
		return(sin(arg));
	case ASIN_OP:
		return(asin(arg));
	case COS_OP:
		return(cos(arg));
	case ACOS_OP:
		return(acos(arg));
	case TAN_OP:
		return(tan(arg));
	case ATAN_OP:
		return(atan(arg));
	case COTAN_OP:
		return(cotan(arg));
	case SINH_OP:
		return(sinh(arg));
	case COSH_OP:
		return(cosh(arg));
	case TANH_OP:
		return(tanh(arg));
	}
}

/*-------------------------------------------------------------------------*/


extern	Real		*funcResults;
extern	Vector		*scalVectResults;
extern	Point		*graphPoints;	/* same elements as funcResults */


extern	Real	startX, startY;
extern	Real	endX, endY;
extern	Real	deltaX, deltaY;
extern	int		numX, numY;

extern	Real	scale;

extern	Boolean		vectrsCurrent;
extern	Boolean		pntsCurrent;

extern	Vector		maxVect, minVect;
extern	Point	XaxisPt, YaxisPt, ZaxisPt;
extern	Point	Origin;
extern	Real	rotMatrx[3][3];

extern	Rect	graphRect;
extern	Boolean		hideSurface, useHeight;

extern	Real	zScale;
extern	Real	ceilgZ, floorZ;


SRotatePoints(cAng, sAng, plane)
register	Real	cAng, sAng;
int		plane;
{
	register	long	i, numElmnts;
	register	double	temp1, temp2;

	numElmnts = numX * numY;
	if (plane EQ 1)
	{
		for (i = 0; i < numElmnts; i++)
		{
			temp1 = scalVectResults[i].y;
			temp2 = scalVectResults[i].z;
			scalVectResults[i].y = temp1 * cAng + temp2 * sAng;
			scalVectResults[i].z = temp2 * cAng - temp1 * sAng;
		}
	}
	else if (plane EQ 2)
	{
		for (i = 0; i < numElmnts; i++)
		{
			temp1 = scalVectResults[i].x;
			temp2 = scalVectResults[i].z;
			scalVectResults[i].x = temp1 * cAng - temp2 * sAng;
			scalVectResults[i].z = temp2 * cAng + temp1 * sAng;
		}
	}
	else if (plane EQ 3)
	{
		for (i = 0; i < numElmnts; i++)
		{
			temp1 = scalVectResults[i].x;
			temp2 = scalVectResults[i].y;
			scalVectResults[i].x = temp1 * cAng - temp2 * sAng;
			scalVectResults[i].y = temp2 * cAng + temp1 * sAng;
		}
	}
	else
	{
		for (i = 0; i < numElmnts; i++)
		{
			temp1 = scalVectResults[i].z;
			temp2 = scalVectResults[i].y;
			scalVectResults[i].z = temp1 * cAng - temp2 * sAng;
			scalVectResults[i].y = temp2 * cAng + temp1 * sAng;
		}
	}
}

SetPoints()
{
register	Real	_scale;
register	int	xCnt, yCnt;
register	long	offSet;
	
	SetNewScale();

	_scale = scale;
	if (hideSurface)
		CheckHideView();

	for (xCnt = 0; xCnt < numX; xCnt++)
		for (yCnt = 0; yCnt < numY; yCnt++)
		{
			offSet = (long)xCnt * (long)numY + yCnt;
			graphPoints[offSet].h = (int)(scalVectResults[offSet].y * _scale)
							+ Origin.h;
			graphPoints[offSet].v = (int)(-scalVectResults[offSet].z * _scale)
							+ Origin.v;
		}
	GenAxes();
	pntsCurrent = TRUE;
	if (hideSurface AND useHeight)
		SetHeightShades();
	else if (hideSurface)
		SetNormalShades();
}

/*************************/

Uchar		*gridFace = NIL;
extern		Real		cosSrc, sinSrc;
extern		Boolean		scrnColor, colorNorms;


SetNormalShades()
{
register	int		xCnt, yCnt, v;
register	long	offSet;
	Uchar	VectFace();

	colorNorms = scrnColor;
	if (gridFace NEQ NIL)
		free(gridFace);
	gridFace = (Uchar *)NewPtr(sizeof(char) * (long)numX * numY);
	if (MemErr)
		return(FALSE);

	for (xCnt = 0; xCnt < numX-1; xCnt++)
	{
		for (yCnt = 0; yCnt < numY-1; yCnt++)
		{
			offSet = (long)xCnt * (long)numY + (long)yCnt;
			v = VectFace(&scalVectResults[(long)(xCnt + 1) * numY + (long)yCnt],
					&scalVectResults[offSet],
					&scalVectResults[(long)xCnt * numY + (long)(yCnt + 1)]);
			gridFace[offSet] = v;
		}
	}
}

Uchar
VectFace(a, b, c)
Vector	*a, *b, *c;
{
	Vector	side1, side2, reslt;
	Real	length, zCoor;
	Real	VectMag();
register	Real	temp1, temp2;

	SubtVector(a, b, &side1);
	SubtVector(c, b, &side2);
	CrossProduct(&side1, &side2, &reslt);

			temp1 = reslt.y;
			temp2 = reslt.z;
			reslt.y = temp1 * cosSrc + temp2 * sinSrc;
			reslt.z = temp2 * cosSrc - temp1 * sinSrc;

/*if (reslt.x < 0)
	return(0);
*/
	length = VectMag(&reslt);
	zCoor = reslt.z/length;
	if (zCoor < 0)
		zCoor = 0;
	if (scrnColor)
		zCoor = zCoor * 255.0;
	else
		zCoor = zCoor * 63.0;

	return((Uchar)round(zCoor));
}


/* cross-product of two vectors */
CrossProduct(a, b, reslt)
register	Vector	*a, *b, *reslt;
{
	reslt->x = a->y * b->z - a->z * b->y;
	reslt->y = a->z * b->x - a->x * b->z;
	reslt->z = a->x * b->y - a->y * b->x;
}

/*** Difference and sum of two vectors;
			result vector can be same as a or b ***/
SubtVector(a, b, result)
register	Vector	*a, *b, *result;
{
	result->x = a->x - b->x;
	result->y = a->y - b->y;
	result->z = a->z - b->z;
}

Real
VectMag(vectP)
register	Vector	*vectP;
{
	return( sqrt( SQ(vectP->x) + SQ(vectP->y) + SQ(vectP->z) ) );
}


SetHeightShades()
{
register	int		xCnt, yCnt, v;
register	long	offSet;
	Real	min, max, sc;

	colorNorms = scrnColor;
	if (gridFace NEQ NIL)
		free(gridFace);
	gridFace = (Uchar *)NewPtr(sizeof(char) * (long)numX * numY);
	if (MemErr)
		return(FALSE);

	FindFuncMaxMin(funcResults, &min, &max);
	if (scrnColor)
		sc = 255.0 / (max - min);
	else
		sc = 63.0 / (max - min);

	for (xCnt = 0; xCnt < numX-1; xCnt++)
	{
		for (yCnt = 0; yCnt < numY-1; yCnt++)
		{
			offSet = (long)xCnt * (long)numY + (long)yCnt;
			gridFace[offSet] = (funcResults[offSet] - min) * sc;
		}
	}
}

static
FindFuncMaxMin(funcList, minPtr, maxPtr)
register	Real	*funcList;
Real		*minPtr, *maxPtr;
{
register	long	i, numElems;
register	Real	max, min, funcVal;

	max = min = funcList[0];

	numElems = (long)numX * (long)numY;
	for (i = 1; i < numElems; i++)
	{
		funcVal = funcList[i];
		if (funcVal > max)
			max = funcVal;
		else if (funcVal < min)
			min = funcVal;
	}

	*minPtr = min;
	*maxPtr = max;
}


/*************************/

SetVectrs()
{
register	int	xCnt, yCnt;
register	long	offSet;
	Real		ImposLimits();
register	Real		xValue, yValue;
register	Vector		*scalePtr;
	Real	SdeltaX;
register	Real	SdeltaY;
	Vector	pntVect;

	SdeltaX = deltaX;
	SdeltaY = deltaY;

	scalePtr = scalVectResults;

	xValue = startX;

	for (xCnt = 0; xCnt < numX; xCnt++)
	{
		yValue = startY;
		pntVect.x = xValue;
		for (yCnt = 0; yCnt < numY; yCnt++)
		{
			offSet = (long)xCnt * (long)numY + yCnt;
			pntVect.y = yValue;
			pntVect.z = (ImposLimits(funcResults[offSet]));
			MatrxMul(rotMatrx, &pntVect, scalePtr++);
			yValue += SdeltaY;
		}
		xValue += SdeltaX;
	}
	vectrsCurrent = TRUE;
}


static
SetNewScale()
{
	DoubPoint	minPnt, maxPnt;
	Real		scaleX, scaleY;
	int			graphWidth, graphHeight;
	DoubPoint	doubOrigin;

	FindMaxMin(scalVectResults, &minPnt, &maxPnt);

	graphWidth = graphRect.right - graphRect.left;
	graphHeight = graphRect.bottom - graphRect.top;

	scaleX = (Real)(graphWidth) / (maxPnt.h - minPnt.h);
	scaleY = (Real)(graphHeight) / (maxPnt.v - minPnt.v);
	scale = MIN(scaleX, scaleY);

	maxPnt.h = maxPnt.h * scale;
	maxPnt.v = maxPnt.v * scale;
	minPnt.h = minPnt.h * scale;
	minPnt.v = minPnt.v * scale;

	doubOrigin.h = ((graphWidth >> 1) + graphRect.left)
				- ( ((maxPnt.h - minPnt.h) / 2) + minPnt.h );
	doubOrigin.v = ((graphHeight >> 1) + graphRect.top)
			- ( ((maxPnt.v - minPnt.v) / 2) + minPnt.v );
	Origin.h = round(doubOrigin.h);
	Origin.v = round(doubOrigin.v);
}

static
FindMaxMin(graphVects, minPnt, maxPnt)
register	Vector	*graphVects;
DoubPoint	*minPnt, *maxPnt;
{
register	long	i, numElems;
register	Real	max_h, max_v;
register	Real	min_h, min_v;
	Real	vertVal;

	max_h = min_h = graphVects[0].y;
	max_v = min_v = -graphVects[0].z;

	numElems = (long)numX * (long)numY;
	for (i = 1; i < numElems; i++)
	{
		if (graphVects[i].y > max_h)
			max_h = graphVects[i].y;
		else if (graphVects[i].y < min_h)
			min_h = graphVects[i].y;
		vertVal = -graphVects[i].z;
		if (vertVal > max_v)
			max_v = vertVal;
		else if (vertVal < min_v)
			min_v = vertVal;
	}

	minPnt->h = min_h;
	minPnt->v = min_v;
	maxPnt->h = max_h;
	maxPnt->v = max_v;
}

Real
ImposLimits(funcVal)
Real	funcVal;
{
	Real	temp;
	
	temp = funcVal;
	
	if (temp > ceilgZ)
		temp = ceilgZ;
	else if (temp < floorZ)
		temp = floorZ;

	return(temp * zScale);
}

static
ExpandRect(theRect, thePoint)
register	DoubRect	*theRect;
register	DoubPoint	*thePoint;
{
	if (thePoint->h < theRect->left)
		theRect->left = thePoint->h;
	else if (thePoint->h > theRect->right)
		theRect->right = thePoint->h;

	if (thePoint->v < theRect->top)
		theRect->top = thePoint->v;
	else if (thePoint->v > theRect->bottom)
		theRect->bottom = thePoint->v;
}


#ifdef	_MC68881_	/*** This function will be a macro calling ldexp() otherwise ***/

Real
mldexp(a, n)
Real	a;
register	int		n;
{
	if (n EQ 0)
		return(a);
	else if (n > 0)
		return(a * (Real)((long)2 << (n - 1)));
	else
		return(a / (Real)((long)2 << -(n + 1)));
}

#endif

static
GenAxes()
{
	Vector		axisVect;

	axisVect.x = 1000;
	axisVect.y = 0;
	axisVect.z = 0;
	Conv3VtoPt(&axisVect, &XaxisPt);

	axisVect.x = 0;
	axisVect.y = 1000;
	Conv3VtoPt(&axisVect, &YaxisPt);

	axisVect.y = 0;
	axisVect.z = 1000;
	Conv3VtoPt(&axisVect, &ZaxisPt);
}
