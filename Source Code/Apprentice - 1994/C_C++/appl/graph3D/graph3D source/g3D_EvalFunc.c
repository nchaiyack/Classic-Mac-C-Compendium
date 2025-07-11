/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"

#ifdef	_LSC3_
#	include	<QuickDraw.h>
#	include	<TextEdit.h>
#	include	<EventMgr.h>
#	include	<DialogMgr.h>
#	include	<MemoryMgr.h>
#endif


extern	DialogPtr		functDial;

extern	Boolean	useMainFunc;

extern	int		mainFunct[64];
extern	int		numMnOps;
extern	Real	mnConsts[30];
extern	int		numMnConsts;

extern	int		derivFunct[64];
extern	int		numDerivOps;
extern	Real	derivConsts[30];
extern	int		numDerivConsts;

extern	Real		*funcResults;
extern	Vector		*scalVectResults;
extern	Point		*graphPoints;	/* same elements as funcResults */

extern	Real	startX, startY;
extern	Real	endX, endY;
extern	Real	deltaX, deltaY;
extern	int		numX, numY;

extern	Boolean		functCurrent;
extern	Boolean		vectrsCurrent;

extern	Boolean		noRedrawGrph;

extern	int			graphType;

extern	Vector		maxVect, minVect;

extern	Real		rotMatrx[3][3];
extern	Boolean		inMultiFndr;


Real		TheFunction(...);
Boolean		CmndPeriod(...);


/***		Local variables		***/
Real	startX, startY;
Real	endX, endY;
Real	deltaX, deltaY;
int		numX, numY;


Boolean		invldFormla;
static	int		*texasPtr;

static	int lastOp = END_OP;

/***		Local functions		***/

void	ToUprStr(...);


EvalFunction()
{
	Boolean	AllocatePts();

	invldFormla = FALSE;
	noRedrawGrph = TRUE;

	if (graphType EQ FUNCT_GTYPE)
		LoadFunction();
	else
		LoadDeriv();

	if (invldFormla)	/* already gave warning message */
		return;

	GetSpecDCont();

	deltaX = (endX - startX) / (numX - 1);
	deltaY = (endY - startY) / (numY - 1);

	if (NOT(AllocatePts()))
	{
		MemAlert();
		return;
	}

	if (graphType EQ FUNCT_GTYPE)
		StoreFunction();
	else
		StoreDeriv();

	vectrsCurrent = FALSE;
	functCurrent = TRUE;
	noRedrawGrph = FALSE;
}


LoadFunction()
{
	char	contStr[256];
	int		itemType;
	Handle	itemHndle;
	Rect	itemRect;

	useMainFunc = TRUE;
	GetDItem(functDial, DIALI_F_FUNCT, &itemType, &itemHndle, &itemRect);
	GetIText(itemHndle, contStr);
	ptoc(contStr);
	ReadFormula(contStr);
}

static
ReadFormula(argStr)
char	*argStr;
{
	int	op, lenOp;
	Boolean	done = FALSE;
	char	*strPtr;
	int		lenStr;
	int		texas[50];

	ToUprStr(argStr);
	if (useMainFunc)
	{
		numMnOps = 0;
		numMnConsts = 0;
	}
	else
	{
		numDerivOps = 0;
		numDerivConsts = 0;
	}

	texasPtr = texas + 49;
	*texasPtr = END_SYMB;

	lenStr = strlen(argStr);
	strPtr = argStr;

	invldFormla = FALSE;
	lastOp = END_OP;

	while (NOT(done))
	{
		while(isspace(*strPtr))
			++strPtr;

		if (strPtr - argStr EQ lenStr)
		{
			op = END_SYMB;
			lenOp = 0;
		}
		else
			NextOp(strPtr, &op, &lenOp);
		DoAction(op, lenOp, &done, &strPtr);
	}
	if (invldFormla)
	{
		Alert(ALRT_FORM, NIL);
		if (useMainFunc)
			numMnOps = 0;
		else
			numDerivOps = 0;
	}
}

static
void
ToUprStr(str)
register	unsigned char	*str;
{
register	unsigned char c;

	c = *str;
	while (c)
	{
		if ((*str >= 'a') AND (*str <= 'z'))
			*str -= 'a' - 'A';
		c = *(++str);
	}
}

static
NextOp(argStr, op, lenOp)
char	*argStr;
int	*op, *lenOp;
{
	int	FindFunc(), LenNum();

	*lenOp = 1;
	if (isdigit(argStr[0]) OR (argStr[0] EQ '.'))
	{
		*op = PUSH_NUM_OP;
		*lenOp = LenNum(argStr);
		return;
	}

	switch (argStr[0]) {
	case 'X':
		*op = PUSH_X_OP;
		break;
	case 'Y':
		*op = PUSH_Y_OP;
		break;
	case 'Z':
		if (graphType EQ DERIV_GTYPE)
			*op = PUSH_Z_OP;
		else
			invldFormla = TRUE;
		break;
	case 'R':
		if (graphType EQ CYLIND_GTYPE)
			*op = PUSH_X_OP;
		else
			invldFormla = TRUE;
		break;
	case 'P':
		if (graphType EQ CYLIND_GTYPE)
			*op = PUSH_Y_OP;
		else
			invldFormla = TRUE;
		break;
	case '+':
		*op = ADD_OP;
		break;
	case '-':
		*op = SUBT_OP;
		break;
	case '*':
		*op = MULT_OP;
		break;
	case '/':
		*op = DIV_OP;
		break;
	case '^':
		*op = POWER_OP;
		break;
	case '(':
		*op = OP_L_PAREN;
		break;
	case ')':
		*op = OP_R_PAREN;
		break;
	default:
		*op = FindFunc(argStr, lenOp);
	}
}

static
int
FindFunc(funcStr, lenFunc)
register	char	*funcStr;
int			*lenFunc;
{
	if (NOT(strncmp(funcStr, "EXP", 3))) {
		*lenFunc = 3;
		return(EXP_OP);
	}
	if (NOT(strncmp(funcStr, "LOG", 3))) {
		*lenFunc = 3;
		return(LOG_OP);
	}
	if (NOT(strncmp(funcStr, "LOG10", 5))) {
		*lenFunc = 5;
		return(LOG10_OP);
	}
	if (NOT(strncmp(funcStr, "SQRT", 4))) {
		*lenFunc = 4;
		return(SQRT_OP);
	}
	if (NOT(strncmp(funcStr, "SQ", 2))) {
		*lenFunc = 2;
		return(SQ_OP);
	}
	if (NOT(strncmp(funcStr, "SINH", 4))) {
		*lenFunc = 4;
		return(SINH_OP);
	}
	if (NOT(strncmp(funcStr, "COSH", 4))) {
		*lenFunc = 4;
		return(COSH_OP);
	}
	if (NOT(strncmp(funcStr, "TANH", 4))) {
		*lenFunc = 4;
		return(TANH_OP);
	}
	if (NOT(strncmp(funcStr, "SIN", (long)3))) {
		*lenFunc = 3;
		return(SIN_OP);
	}
	if (NOT(strncmp(funcStr, "ASIN", 4))) {
		*lenFunc = 4;
		return(ASIN_OP);
	}
	if (NOT(strncmp(funcStr, "COS", 3))) {
		*lenFunc = 3;
		return(COS_OP);
	}
	if (NOT(strncmp(funcStr, "ACOS", 4))) {
		*lenFunc = 4;
		return(ACOS_OP);
	}
	if (NOT(strncmp(funcStr, "TAN", 3))) {
		*lenFunc = 3;
		return(TAN_OP);
	}
	if (NOT(strncmp(funcStr, "ATAN", 4))) {
		*lenFunc = 4;
		return(ATAN_OP);
	}
	if (NOT(strncmp(funcStr, "COTAN", 5))) {
		*lenFunc = 5;
		return(COTAN_OP);
	}
	invldFormla = TRUE;
}

static
int
LenNum(argStr)
register	char	*argStr;
{
register	char	*tstr;

	tstr = argStr;

	while(isdigit(*tstr))
		++tstr;

	if (*tstr EQ '.')
	{
		++tstr;
		while(isdigit(*tstr))
			++tstr;
	}

	if ((*tstr EQ 'e') OR (*tstr EQ 'E'))
	{
		++tstr;
		if ((*tstr = '+') OR (*tstr = '-'))
			++tstr;
		while(isdigit(*tstr))
			++tstr;
	}
	return((int)(tstr - argStr));
}


#define	CMP_NONE	0
#define	CMP_SIGN	1
#define	CMP_HALT	2
#define	CMP_ERROR	3


static
DoAction(op, lenOp, done, argStr)
int		op;
int		lenOp;
Boolean *done;
char	**argStr;
{
	int		action, localOp;
	Boolean	popOp;
	int		CheckSymbs();

	localOp = op;

	action = CheckSymbs(&localOp, done);

	switch (action) {
	case ACT_OPtTEX:	/* send it to texas */
		--texasPtr;
		*texasPtr = localOp;
		popOp = TRUE;
		break;
	case ACT_TEXtF:		/* pop from texas to formula */
		PushOpFormula(*texasPtr++);
		popOp = FALSE;
		break;
	case ACT_DELOPT:	/* send it to texas */
		++texasPtr;
		popOp = TRUE;
		break;
	case ACT_STPDN:		/* we're done */
		*done = TRUE;
		popOp = FALSE;
		break;
	case ACT_STPERR:	/* something wrong; stop */
		invldFormla = TRUE;
		*done = TRUE;
		popOp = FALSE;
		break;
	case ACT_OPtF:	/* add it to the formula */
		PushOpFormula(localOp);
		popOp = TRUE;
		if (localOp EQ PUSH_NUM_OP)
			PushConst(*argStr);
		break;
	case ACT_DELOP:	/* get rid of it */
		popOp = TRUE;
		break;
	}

	if (popOp)
		*argStr += lenOp;
}

static
PushOpFormula(newOp)
int	newOp;
{
	if (useMainFunc)
	{
		mainFunct[numMnOps] = newOp;
		++numMnOps;
/*		if (numMnOps > MAX_OPS)
			GeneralAlert("Formula too complex");
*/
	}
	else
	{
		derivFunct[numDerivOps] = newOp;
		++numDerivOps;
	}
}

static
PushConst(constStr)
char	*constStr;
{
	Real	atof();

	if (useMainFunc)
		mnConsts[numMnConsts++] = atof(constStr);
	else
		derivConsts[numDerivConsts++] = atof(constStr);
}

static
int
CheckSymbs(op, done)
int		*op;
Boolean	*done;
{


static	int		actionTable[MAX_SYMBS][MAX_SYMBS] = {
						/* next symb -> */
		/* texas V */
						4,1,1,1,1,1,1,5,1,6,1,
						2,2,2,1,1,1,1,2,1,6,1,
						2,2,2,1,1,1,1,2,1,6,1,
						2,2,2,2,2,1,1,2,1,6,1,
						2,2,2,2,2,1,1,2,1,6,1,
						2,2,2,2,2,2,1,2,1,6,1,
						5,1,1,1,1,1,1,3,1,6,1,
						5,5,5,5,5,5,5,5,1,5,1,
						2,2,2,2,2,2,1,2,5,5,1,
						5,5,5,5,5,5,5,5,5,5,1,
						2,2,2,2,2,2,1,2,1,6,1
						};


static	int		compareOps[MAX_SYMBS][MAX_SYMBS] = {
/*curr op -> */
/*last op V */
CMP_HALT,  CMP_SIGN,  CMP_SIGN,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE,
CMP_ERROR, CMP_SIGN,  CMP_SIGN,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE,
CMP_ERROR, CMP_SIGN,  CMP_SIGN,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE,
CMP_ERROR, CMP_SIGN,  CMP_SIGN,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE,
CMP_ERROR, CMP_SIGN,  CMP_SIGN,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE,
CMP_ERROR, CMP_SIGN,  CMP_SIGN,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE/*CMP_ERROR*/, CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE,
CMP_ERROR, CMP_SIGN,  CMP_SIGN,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE,
CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_ERROR, CMP_NONE,
CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,
CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_ERROR, CMP_NONE,
CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_ERROR, CMP_NONE,  CMP_ERROR, CMP_NONE,  CMP_NONE,  CMP_NONE
};

	int	errCode, symbol, action;


	symbol = SymbFromOp(*op);

	action = actionTable[SymbFromOp(*texasPtr)][symbol];

	errCode = compareOps[SymbFromOp(lastOp)][SymbFromOp(*op)];

	switch (errCode) {
	case CMP_NONE:
		break;
	case CMP_SIGN:
		if (*op EQ SUBT_OP)
			*op = NEG_OP;
		else
			action = ACT_DELOP;
		action = actionTable[SymbFromOp(*texasPtr)][SymbFromOp(*op)];
		break;
	case CMP_HALT:
		*done = TRUE;
		break;
	case CMP_ERROR:
		*done = TRUE;
		invldFormla = TRUE;
		break;
	default:
		*done = TRUE;
	}

	if ((action NEQ ACT_DELOP) AND (action NEQ ACT_TEXtF))
		lastOp = *op;

	return(action);
}

static
int
SymbFromOp(op)
int	op;
{
	if (op < 0) {
		if (op > OP_L_PAREN)
			return(NUMB_SYMB);
		if (op EQ OP_L_PAREN)
			return(L_PAREN_SYMB);
		return(R_PAREN_SYMB);
	}

	if (op EQ END_OP)
		return(END_SYMB);

	if (op <= POWER_OP)
		return(op);

	if (op EQ NEG_OP)
		return(NEG_SYMB);
	else if (op > NEG_OP)
		return(FUNC_SYMB);
	
	return(END_SYMB);
}

static
LoadDeriv()
{
	char	contStr[256];
	int		itemType;
	Handle	itemHndle;
	Rect	itemRect;

	useMainFunc = TRUE;
	GetDItem(functDial, DIALI_D_FUNC1, &itemType, &itemHndle, &itemRect);
	GetIText(itemHndle, contStr);
	ptoc(contStr);
	ReadFormula(contStr);

	useMainFunc = FALSE;
	GetDItem(functDial, DIALI_D_FUNC2, &itemType, &itemHndle, &itemRect);
	GetIText(itemHndle, contStr);
	ptoc(contStr);
	ReadFormula(contStr);
}

Boolean
AllocatePts()
{
	if (funcResults NEQ NIL)
		free(funcResults);
	if (scalVectResults NEQ NIL)
		free((void *)scalVectResults);
	if (graphPoints NEQ NIL)
		free((void *)graphPoints);
	funcResults = (Real *)NewPtr((long)sizeof(Real) * (long)numX * numY);
	if (MemErr)
		goto Fail;
	scalVectResults = (Vector *)NewPtr((long)sizeof(Vector) * (long)numX * numY);
	if (MemErr)
		goto Fail;
	graphPoints = (Point *)NewPtr((long)sizeof(Point) * (long)numX * numY);
	if (MemErr)
		goto Fail;

	return(TRUE);
Fail:
	InitPts();
	return(FALSE);
}

MemAlert()
{
	InitCursor();
	Alert(ALRT_MEMRY, NIL);
}

static
StoreFunction()
{
	register	int	xCnt, yCnt;
	Real		currX, currY;
	Real		maxFunc, minFunc, funcVal;
	Real		xAtMax, yAtMax;
	Real		xAtMin, yAtMin;
	
	ShowMeter((long)numX * numY);

	useMainFunc = TRUE;
	funcVal = TheFunction(startX, startY);
	maxFunc = funcVal;
	minFunc = funcVal;
	xAtMax = startX;
	yAtMax = startY;
	xAtMin = startX;
	yAtMin = startY;

	currX = startX;
	for (xCnt = 0; xCnt < numX; xCnt++)
	{
		currY = startY;
		for (yCnt = 0; yCnt < numY; yCnt++)
		{
			funcVal = TheFunction(currX, currY);
			DisplayMeter();
			funcResults[(long)xCnt * (long)numY + yCnt] = funcVal;
			if (funcVal > maxFunc)
			{
				maxFunc = funcVal;
				xAtMax = currX;
				yAtMax = currY;
			}
			else if (funcVal < minFunc)
			{
				minFunc = funcVal;
				xAtMin = currX;
				yAtMin = currY;
			}
			currY += deltaY;
		}
		if (CmndPeriod())
			break;  /* for */
		currX += deltaX;
	}

done:
	maxVect.x = xAtMax;
	maxVect.y = yAtMax;
	maxVect.z = maxFunc;
	minVect.x = xAtMin;
	minVect.y = yAtMin;
	minVect.z = minFunc;

	RemoveMeter();
}

static
StoreDeriv()
{
	register	int	xCnt, yCnt;
	Real		currFValX, currFValY;
	Real		currX, currY, midXsqr, midYsqr;
	Real		HlfDeltaX, HlfDeltaY;

	Real		maxFunc, minFunc;
	Real		xAtMax, yAtMax;
	Real		xAtMin, yAtMin;
	Real		TheFunction(), GetInitVal();
	Boolean		CmndPeriod();
	
	currFValX = GetInitVal();
	ShowMeter((long)numX * numY);

	HlfDeltaX = deltaX / 2.0;
	HlfDeltaY = deltaY / 2.0;

	maxFunc = currFValX;
	minFunc = currFValX;
	xAtMax = startX;
	yAtMax = startY;
	xAtMin = startX;
	yAtMin = startY;

	currX = startX;
	midXsqr = currX + HlfDeltaX;
	for (xCnt = 0; xCnt < numX; xCnt++)
	{
		currFValY = currFValX;
		currY = startY;
		midYsqr = currY + HlfDeltaY;
		useMainFunc = FALSE;	/* calculate dz/dy */
		for (yCnt = 0; yCnt < numY; yCnt++)
		{
			DisplayMeter();
			funcResults[xCnt * numY + yCnt] = currFValY;
			if (currFValY > maxFunc)
			{
				maxFunc = currFValY;
				xAtMax = currX;
				yAtMax = currY;
			}
			else if (currFValY < minFunc)
			{
				minFunc = currFValY;
				xAtMin = currX;
				yAtMin = currY;
			}
			currFValY = currFValY +
					(TheFunction(currX, midYsqr, currFValY) * deltaY);
			currY += deltaY;
			midYsqr = currY + HlfDeltaY;
		}
		if (CmndPeriod())
			break;  /* for */
		useMainFunc = TRUE;	/* calculate dz/dx */
		currFValX = currFValX +
					(TheFunction(midXsqr, startY, currFValX) * deltaX);
		currX += deltaX;
		midXsqr = currX + HlfDeltaX;
	}

	maxVect.x = xAtMax;
	maxVect.y = yAtMax;
	maxVect.z = maxFunc;
	minVect.x = xAtMin;
	minVect.y = yAtMin;
	minVect.z = minFunc;

	RemoveMeter();
}

static
Real
GetInitVal()
{
	DialogRecord dStorage;
	DialogPtr	initValDial;
	int		itemSelected;
	char	numStrX[100], numStrY[100];
	Handle	itmHand, DialHandle();
	double	atof();
static	double	lastInitVal = 0.0;

	sprintf(numStrX, "%f", startX);
	sprintf(numStrY, "%f", startY);
	ctop(numStrX);
	ctop(numStrY);
	ParamText(numStrX, numStrY, "", "");

	initValDial = GetNewDialog(DIAL_INITVL, &dStorage, (DialogPtr)-1L);
	itmHand = DialHandle(initValDial, DIALI_I_VALUE);
	sprintf(numStrX, "%f", lastInitVal);
	ctop(numStrX);
	SetIText(itmHand, numStrX);

	ShowWindow(initValDial);

	InitCursor();
	do {
		ModalDialog(NIL, &itemSelected);
	} while (itemSelected NEQ OK);
	Clock();

	GetIText(itmHand, numStrX);
	ptoc(numStrX);
	CloseDialog(initValDial);

	lastInitVal = atof(numStrX);
	return(lastInitVal);
}

Handle
DialHandle(theDial, theItem)
DialogPtr	theDial;
int		theItem;
{
	int		itemTyp;
	Handle	itmHand;
	Rect	disp;

	GetDItem(theDial, theItem, &itemTyp, &itmHand, &disp);
	return(itmHand);
}
