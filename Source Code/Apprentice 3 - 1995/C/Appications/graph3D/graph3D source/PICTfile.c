/*
	Copyright '89	Christopher Moll
	all etceteras reserved
*/


#include	"graph3D.h"

extern	EventRecord		theEvent;
extern	Boolean		grphOnScrn;
extern	WindowPtr		graphWind;

Boolean	GetFileID(...);


SavePICT(useObjects)
Boolean	useObjects;
{
	PicHandle	drawPict;
	char	fileName[100];
	int		volRef, fileRef, err;
	long	byteCount;
	Boolean		GetFileID();
	PicHandle	GetGraphPICT();

	if (NOT(GetFileID(fileName, "\PSave PICT:", "", &volRef)))
		return;
	drawPict = GetGraphPICT(useObjects);

	if (err = Create(fileName, volRef, 'MDPL', 'PICT'))
		goto Fail;
	if (err = FSOpen(fileName, volRef, &fileRef))
		goto Fail;
	if (err = SetEOF(fileRef, 512L))
		goto Fail;
	if (err = SetFPos(fileRef,fsFromStart, 512L))	/* skip header */
		goto Fail;

	HLock(drawPict);
	byteCount = GetHandleSize(drawPict);
	if (err = FSWrite(fileRef, &byteCount, *drawPict))
		goto Fail;
	if (err = FSClose(fileRef))
		goto Fail;
	HUnlock(drawPict);
	KillPicture(drawPict);
	return;

Fail:
	sprintf(fileName, "Error saving PICT: %d", err);
	GenralAlert(ctop(fileName));
}



/*** act on any update or activate events pending ***/
AllowUpdat()
{
	if (GetNextEvent(updateMask, &theEvent))	/* ROM */
		DoEvent();
	if (GetNextEvent(updateMask, &theEvent))	/* ROM */
		DoEvent();
}


PicHandle
GetGraphPICT(useObjects)
Boolean	useObjects;
{
	PicHandle	drawPict;
	Boolean		svGraphSt;

	if (useObjects)
	{
		svGraphSt = grphOnScrn;
		grphOnScrn = FALSE;
		drawPict = OpenPicture(&graphWind->portRect);
			DrawGraph();
		ClosePicture();
		grphOnScrn = svGraphSt;
	}
	else
	{
		SelectWindow(graphWind);
		AllowUpdat();

		RmvGraphGrow();
		DrawRotHndls();

		drawPict = OpenPicture(&graphWind->portRect);
			CopyBits(&graphWind->portBits, &graphWind->portBits,
						&graphWind->portRect, &graphWind->portRect,
						srcCopy, NIL);
		ClosePicture();
		PutGraphGrow();
		DrawRotHndls();
	}
	return(drawPict);
}

static
RmvGraphGrow()
{
	Rect	iconR;

	iconR.top = graphWind->portRect.bottom - 16;
	iconR.left = graphWind->portRect.right - 16;
	iconR.bottom = graphWind->portRect.bottom;
	iconR.right = graphWind->portRect.right;
	EraseRect(&iconR);
}

Boolean
GetFileID(newfName, prompt, defName, volRefP)
Uchar	*newfName, *prompt, *defName;	/* pascal strings */
int		*volRefP;
{
static    Point	where = {80, 100};
	SFReply 	reply;

	SFPutFile(where, prompt, defName, NIL, &reply);

	if (reply.good)
	{
		*volRefP = reply.vRefNum;
		Pstrcpy(reply.fName, newfName);
		return(TRUE);
	}
	else
		return(FALSE);
}