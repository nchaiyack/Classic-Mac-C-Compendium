/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"
#ifdef	_LSC3_
#	include	<FileMgr.h>
#	include	<EventMgr.h>
#	include	<StdFilePkg.h>
#endif


extern	int		numX, numY;

extern	Real	startX, startY;
extern	Real	endX, endY;
extern	Real	deltaX, deltaY;
extern	int		numX, numY;

extern	Vector		maxVect, minVect;

extern	Real		*funcResults;
extern	Point		*graphPoints;	/* same elements as funcResults */

extern	Boolean		vectrsCurrent;

int		ReadLine(...);


Boolean
ReadData()
{
unsigned	char	rdBuffer[256], *inLine;
	int		theErr;
	int		ReadLine();
	Real	atof();
	Boolean		DoGetFile(), AllocatePts();

	if (NOT(DoGetFile()))
		return(FALSE);

	theErr = ReadLine(256, rdBuffer);
	if (theErr) {
		NumToString((long)theErr, rdBuffer);
		ParamText(rdBuffer, "\0", "\0", "\0");
		Alert(ALRT_FILE, NIL);
		return;
	}

	inLine = rdBuffer;
	
	numX = atof(inLine);
	while(NOT(isspace(*inLine)))
		++inLine;
	while(isspace(*inLine))
		++inLine;
	numY = atof(inLine);
	startX = 0.0;
	startY = 0.0;
	endX = numX;
	endY = numY;

	deltaX = 1.0;
	deltaY = 1.0;

	if (NOT(AllocatePts()))
	{
		MemAlert();
		return;
	}

	StoreDataPts();
	CloseFile();

	vectrsCurrent = FALSE;
	return(TRUE);
}

static
Boolean
DoGetFile()
{
static	Point	where = {100, 100};
	long		theList[4];
	SFReply		theReply;
	
	theList[0] = 'TEXT';
	
	SFGetFile(where, "", NIL, 1, theList, NIL, &theReply);
	
	SetVol(NIL, theReply.vRefNum);
	OpenFile(theReply.fName);
	return(theReply.good);
}

static	IOParam		dataFile;

static
OpenFile(fileName)
unsigned	char	*fileName;
{
	dataFile.ioCompletion = NIL;
	dataFile.ioNamePtr = fileName;
	dataFile.ioVRefNum = 0;
	dataFile.ioPermssn = fsRdPerm;

	PBOpen(&dataFile, FALSE);
}

static
CloseFile()
{
	PBClose(&dataFile, FALSE);
}

#define		newLnStop	0x80


static
int
ReadLine(maxRead, rdBuffer)
int	maxRead;
char	*rdBuffer;
{
	dataFile.ioBuffer = rdBuffer;
	dataFile.ioReqCount = maxRead;
	dataFile.ioPosMode = fsAtMark + newLnStop + ('\r' << 8);

	PBRead(&dataFile, FALSE);
	rdBuffer[dataFile.ioActCount - 1] = '\0';
	return(dataFile.ioResult);
}

#define	LN_LEN	1000

static
StoreDataPts()
{
	register	int	xCnt, yCnt;
	Real		currX, currY;
	Real		maxFunc, minFunc, funcVal;
	Real		xAtMax, yAtMax;
	Real		xAtMin, yAtMin;
	unsigned	char	rdBuffer[LN_LEN], *inLine;
	int			theErr;
	Real		atof(), NextVal();
	Boolean		CmndPeriod();

	theErr = ReadLine(LN_LEN, rdBuffer);
	inLine = rdBuffer;
	funcVal = atof(inLine);
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
			funcVal = NextVal(&inLine);
			funcResults[(long)xCnt * numY + (long)yCnt] = funcVal;
			if (funcVal > maxFunc) {
				maxFunc = funcVal;
				xAtMax = currX;
				yAtMax = currY;
			}
			else if (funcVal < minFunc) {
				minFunc = funcVal;
				xAtMin = currX;
				yAtMin = currY;
			}
			currY += deltaY;
		}
		if (CmndPeriod())
			break;  /* for */
		currX += deltaX;
		theErr = ReadLine(LN_LEN, rdBuffer);
		inLine = rdBuffer;
	}

	maxVect.x = xAtMax;
	maxVect.y = yAtMax;
	maxVect.z = maxFunc;
	minVect.x = xAtMin;
	minVect.y = yAtMin;
	minVect.z = minFunc;
}

static
Real
NextVal(str)
unsigned	char	**str;
{
	Real	result, atof();

	result = atof(*str);

	while(NOT(isspace(**str)))
		(*str)++;
	while(isspace(**str))
		(*str)++;

	return(result);
}
