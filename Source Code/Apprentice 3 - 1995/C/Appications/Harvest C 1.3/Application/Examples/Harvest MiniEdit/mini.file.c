/*********************************************************************

	mini.file.c
	
	file functions for Miniedit
	
*********************************************************************/
	
#include <TextEdit.h>
#include <Errors.h>
#include <Files.h>
#include <Windows.h>
#include <StandardFile.h>
#include "MiniEdit.h"
#include "mini.file.h"
#include "mini.windows.h"
#include "mini.print.h"

Str255 		theFileName;
static short	theVRefNum;

extern TEHandle  TEH;
extern WindowPtr myWindow;
extern char		 dirty;


	/**
	 **		Prototypes for private functions.
	 **		(They really should be static.)
	 **
	 **/
 
int pStrCopy(StringPtr a, StringPtr b);
int OldFile (Str255 fn, short *vRef);
int ReadFile (short refNum, TEHandle textH);
int FileError(Str255 s, Str255 f);
int SaveAs (Str255 fn, short *vRef);
int SaveFile (Str255 fn, short vrn);
int NewFile (Str255 fn, short *vRef);
int OldFile (Str255 fn, short *vRef);
int CreateFile (Str255 fn, short *vRef, short *theRef);
int WriteFile (short refNum, char *p, long num);



int SetUpFiles(void)
{
	pStrCopy("\p", theFileName);
	theVRefNum = 0;
}

int DoFile (int item)

{
	short 	vRef, refNum, io;
	Str255	fn;

	switch (item) {
		case fmNew: 
			SetWTitle(myWindow, "\pUntitled");
			ShowWindow(myWindow);
			dirty = 0;
			break;

		case fmOpen:
			if (OldFile(fn, &vRef ))
				if (FSOpen(fn, vRef, &refNum)==noErr) {
					if (ReadFile(refNum, TEH)) {
						pStrCopy(fn, theFileName);
						theVRefNum = vRef;
						SetWTitle(myWindow, theFileName);
						dirty = 0;
					}
					if (FSClose(refNum)==noErr) ;
					ShowWindow(myWindow);
					TESetSelect(0, 0, TEH);
					ShowSelect();
				}
				else FileError("\pError opening ", fn);
			break;

		case fmClose:
			if (dirty) {
				ParamText("\pSave changes for Ò",
						(theFileName[0]==0) ? "\pUntitled" : theFileName,
						"\pÓ?", "\p");
				switch (Alert(AdviseAlert, 0L)) {
				case aaSave:
					if (theFileName[0]==0) {
						fn[0] = 0;
						if (!SaveAs(fn, &vRef)) return(0);
					}
		 			else if (!SaveFile(theFileName, theVRefNum)) return(0);
		 			break;
		 		case aaCancel: return(0);
		 		case aaDiscard: dirty = 0;
		 		}
		 	}
			CloseMyWindow();
			break;
		case fmSave:
			if (theFileName[0]==0) goto saveas;
			SaveFile(theFileName, theVRefNum);
			break;
		case fmSaveAs:
	saveas:
			fn[0] = 0;
			if (SaveAs(fn, &vRef )) {
				pStrCopy(fn, theFileName);
				theVRefNum = vRef;
				SetWTitle(myWindow, theFileName);
			}
			break;
		case fmRevert:
			ParamText("\pRevert to last saved version of Ò",
					theFileName, "\pÓ?", "\p");
			switch (Alert(AdviseAlert, 0L)) {
			case aaSave:
				HidePen();
				TESetSelect(0, (**TEH).teLength, TEH);
				ShowPen();
				TEDelete(TEH);
				if ((theFileName[0]!=0) &&
					(FSOpen(theFileName, theVRefNum, &refNum)==noErr)) {
					dirty = !ReadFile(refNum, TEH); 
					/* I don't check for bad read! */
					if (FSClose(refNum)==noErr) ;
				}
				ShowWindow(myWindow);
				UpdateWindow(myWindow);
	 		case aaCancel:
	 		case aaDiscard: return(0);;
	 		}
	
			break;
		case fmPageSetUp:
			DoPageSetUp();
			break;
		case fmPrint:
			PrintText( (**TEH).hText, (long)(**TEH).teLength, (GrafPtr)myWindow,
							StringWidth("\pmmmm"));
			break;
		case fmQuit: 
			if (DoFile(fmClose))
				ExitToShell();
	}
	return(1);
}

static Point SFGwhere = { 90, 82 };
static Point SFPwhere = { 106, 104 };
static SFReply reply;

int SaveAs (Str255 fn, short  *vRef)

{
	short refNum;
	
	if (NewFile(fn, vRef)) 
		if (!CreateFile(fn, vRef, &refNum)) {
			FileError("\pError creating file ", fn);
			return (0);
		} else {
			WriteFile(refNum, (*(**TEH).hText), (long)(**TEH).teLength);
			FSClose(refNum);
			dirty = 0;
			return(1);
		}
}

int SaveFile (Str255 fn, short vrn)

{
	short refNum;
	
	if (FSOpen(fn, vrn, &refNum) != noErr) {
		FileError("\pError opening file ", fn);
		return (1);
	} else {
		WriteFile(refNum, (*(**TEH).hText), (long)(**TEH).teLength);
		dirty = 0;
		FSClose(refNum);
		return(1);
	}
}

int NewFile (Str255 fn, short *vRef)

{
	SFPutFile(SFPwhere, "\pSave file as", fn, 0L, &reply);
	if (!reply.good)
		return (0);
	else {
		pStrCopy(reply.fName, fn);
		*vRef = reply.vRefNum;
		return(1);
	}
}

int OldFile (Str255 fn, short *vRef)

{
	SFTypeList	myTypes;
	
	myTypes[0]='TEXT';

	SFGetFile(SFGwhere, "\p", 0L, 1, myTypes, 0L, &reply );

	if (!reply.good)
		return (0);
	else {
		pStrCopy(reply.fName, fn);
		*vRef = reply.vRefNum;
		return(1);
	}
}

int CreateFile (Str255 fn, short *vRef, short *theRef)

{
	int io;
	
	io=Create(fn, *vRef, 'CEM8', 'TEXT');
	if ((io == noErr) || (io == dupFNErr))
		io = FSOpen(fn, *vRef, theRef );

	return ((io == noErr) || (io == dupFNErr));
}

int WriteFile (short refNum, char *p, long num)

{
	short io;			/* 	a real application would want to check 
						this return code for failures */
	
	io=FSWrite(refNum, &num, p);
}

int ReadFile (short refNum, TEHandle textH)

{
	char	buffer[256];
	long	count;
	short		io;
	
	TESetSelect(0, (**textH).teLength, textH);
	TEDelete(textH);
	do {
		count = 256;
		io = FSRead(refNum, &count, &buffer);
		TEInsert(&buffer, count, textH);
	} while (io==noErr);
	
	return (io == eofErr);
}

int pStrCopy (StringPtr p1, StringPtr p2)

/* copies a pascal string from p1 to p2 */
{
	register int len;
	
	len = *p2++ = *p1++;
	while (--len>=0) *p2++=*p1++;
}


int FileError(Str255 s, Str255 f)

{
	ParamText(s, f,"\p", "\p");
	Alert(ErrorAlert, 0L);
}
