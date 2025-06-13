/*
	Copyright '89	Christopher Moll
	all rights reserved
*/


#include	"graph3D.h"

extern	int		numX, numY;

extern	Real	startX, startY;
extern	Real	endX, endY;
extern	Real	deltaX, deltaY;
extern	int		numX, numY;

extern	Vector		maxVect, minVect;

extern	Real		*funcResults;
extern	Boolean		vectrsCurrent;

OpenMandel()
{
static	Point	where = {100, 100};
	long		theList[4];
	SFReply		theReply;
	FILE		*rdFile;
	Boolean		StoreMandelPts(), GetMandHeader();
	int			maxDwell;

	theList[0] = 'ManD';
	
	SFGetFile(where, "", NIL, 1, theList, NIL, &theReply);
	if (theReply.good)
	{
		SetVol(NIL, theReply.vRefNum);
		rdFile = fopen(ptoc((char *)theReply.fName), "r");
		if (rdFile)
		{
			if (NOT(GetMandHeader(rdFile, &maxDwell)))
				goto Fail;
			if (NOT(StoreMandelPts(rdFile, maxDwell)))
				goto Fail;

Fail:		fclose(rdFile);
			vectrsCurrent = FALSE;
			InvalidGraph();
		}
	}
}

static
Boolean
GetMandHeader(rdFile, maxDwellP)
FILE	*rdFile;
int		*maxDwellP;
{
	FileSection	fileHead;
	CoordBlock	cblock;
	int			rdReslt;
	char		dump[78];
	Boolean		AllocatePts();

	rdReslt = fread(&fileHead, sizeof(FileSection), 1, rdFile);
	rdReslt = fread(&cblock, sizeof(CoordBlock), 1, rdFile);
	rdReslt = fread(&dump, 78, 1, rdFile);

	numX = cblock.height;
	numY = cblock.width;
	deltaX = cblock.maxDwell / 10.0;
	deltaY = cblock.maxDwell / 10.0;
	if (NOT(AllocatePts()))
	{
		MemAlert();
		return(FALSE);
	}
	*maxDwellP = cblock.maxDwell;
	return(TRUE);
}

static
Boolean
StoreMandelPts(rdFile, maxDwell)
FILE	*rdFile;
int		maxDwell;
{
	register	int	xCnt, yCnt, dwellVal;
	Real		currX, currY;
	int			maxFunc, minFunc;
	Real		xAtMax, yAtMax;
	Real		xAtMin, yAtMin;
	int			rdReslt, offSet;
	int			*dwells;
	Boolean		CmndPeriod();

	dwells = (int	*)NewPtr((long)numX * (long)numY * sizeof(int));
	if (MemErr)
		return(FALSE);
	rdReslt = fread(dwells, numX * sizeof(int), numY, rdFile);
	if (rdReslt NEQ numY)
	{
		DisposPtr(dwells);
		return(FALSE);
	}

	maxFunc = dwells[0];
	minFunc = dwells[0];
	xAtMax = startX;
	yAtMax = startY;
	xAtMin = startX;
	yAtMin = startY;

	currX = startX;
{FILE	*wrt;
wrt=fopen("wrt","w");

	for (xCnt = 0; xCnt < numX; xCnt++)
	{
		currY = startY;
		for (yCnt = 0; yCnt < numY; yCnt++)
		{
			offSet = (long)xCnt * numY + (long)yCnt;
			dwellVal = dwells[offSet];
			if (dwellVal > maxDwell)
				dwellVal = maxDwell;
fprintf(wrt, "%d\t", dwellVal);
			funcResults[offSet] = dwellVal;
			if (dwellVal > maxFunc)
			{
				maxFunc = dwellVal;
				xAtMax = currX;
				yAtMax = currY;
			}
			else if (dwellVal < minFunc)
			{
				minFunc = dwellVal;
				xAtMin = currX;
				yAtMin = currY;
			}
			currY += deltaY;
		}
fprintf(wrt, "\n");
		if (CmndPeriod())
			break;  /* for */
		currX += deltaX;
	}
fclose(wrt);
}
	maxVect.x = xAtMax;
	maxVect.y = yAtMax;
	maxVect.z = maxFunc;
	minVect.x = xAtMin;
	minVect.y = yAtMin;
	minVect.z = minFunc;
	DisposPtr(dwells);
	return(TRUE);
}
