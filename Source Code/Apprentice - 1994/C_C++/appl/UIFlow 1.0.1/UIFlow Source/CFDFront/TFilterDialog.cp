char gMachine[80];
char gExuser[80];
char gPasswd[80];
char gAccount[80];
char gCommand[80];

#pragma segment CollabGlobals
#include <sys/errno.h>
#include <socket.ext.h>	// prototypes for s_close and
#include <ris.h>

#define DATAREADYERROR	-1
#define	DTMNODATA		0
#define DTMMESSAGE		1
#define DTMPALETTE		2
#define DTMDATA			3
#define DTMCOLLABORATE	4

#undef NODEBUG_PRINT

#ifdef NODEBUG_PRINT
#define Debug_Print(s)
#endif
//LAM fix this when new version DTM fixed.
#undef DTM1
#ifdef DTM1
#define		dtm_set_class(h, c)		(strcpy((h), (c)), strcat((h), " "))
#define		dtm_compare_class(h, c)	!strncmp((h), (c), strlen(c))
#define tmpDTMmakeInPort(a,b)	DTMmakeInPort((a))
#define tmpDTMmakeOutPort(a,b)	DTMmakeOutPort((a))
#else
#define tmpDTMmakeInPort(a,b)	DTMmakeInPort((a),(b))
#define tmpDTMmakeOutPort(a,b)	DTMmakeOutPort((a),(b))
#endif


Boolean IsCancelEvent();
#pragma segment Main

//-----------------------------------------------------------------------------------------------
//	CancelEvent ()
//				This routine returns true if command-. has been entered.
//-----------------------------------------------------------------------------------------------
Boolean IsCancelEvent()
	{
	EventRecord tmpEvent;
	// look for a command-., this is just a wait
	WaitNextEvent(mUpMask, &tmpEvent, 10, nil);
	if (EventAvail(keyDownMask, &tmpEvent)) 
		if (tmpEvent.what == keyDown || tmpEvent.what == autoKey)
			if ((tmpEvent.modifiers & cmdKey) && ((char)(tmpEvent.message & 0xFF) == '.'))
				{
				GetNextEvent(keyDownMask, &tmpEvent);
				return true;
				}
	
	return false;
	}
//
// this is the socket spin routine. It will simply check for dtm stuff.
//

int MySpin (spin_msg, long)
{
	
	return IsCancelEvent ();
}

#pragma segment	Collab
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	UItemAssign
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
UItemAssign( DialogPtr , int , int (*)())
{
//	Rect		ibox;
//	short		typ;

//	GetDItem( dlog, item, typ, ihndl, ibox);
//	SetDItem( dlog, item,  typ,  (Handle) proc, ibox);
	return true;
}

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	OutlineItem		Outline a dialog item
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
pascal void OutlineItem (DialogPtr theDialog, short theItem)
{
	Rect		ibox;
	Handle	ihndl;
	short		typ;
	
	GetDItem( theDialog, theItem, &typ, &ihndl, &ibox);
	PenSize( 3,3);
	InsetRect( &ibox, -4,-4);
	FrameRoundRect( &ibox, 16,16);
}

/*****************************************************************/
/*   Modalproc for prompting which is tied to exremote()
*    This proc makes sure that key echoes are suppressed for item #6
*/

#define PASSWDITM	(short)6

/* special keyboard stuff. */

#define KBETX	0x03
#define KBBS	0x08
#define KBHT	0x09
#define KBCR	0x0D
#define KBESC	0x1B

// pascal Boolean
pascal Boolean pfilt(DialogPtr td, EventRecord * tdevent, short * itemhit)
	{
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	char 		s[256];
	long 		c;
	int			i,len;
	short		insertPt = 32767;
	
/*
*  If it is not the password item, let modaldialog handle it.
*/
	
	switch(tdevent->what) {
		case keyDown:
		case autoKey:
			len = strlen(gPasswd);
/*
*  If it is CR, take it as OK button, as per Inside Mac
*/
			c = (tdevent->message & charCodeMask);
			
			switch (c)
			{
				/* return or enter. */
				case KBCR:
				case KBETX:
					*itemhit = 1;
					return true;
					
				/* cancel (ESC) */
				case KBESC:
					*itemhit = 2;
					return true;
					
				/* horizontal tab? */
				case KBHT:
					return false;
					
				/* backspace. */
				case KBBS:
					if (((DialogRecord *)td)->editField+1 != PASSWDITM)	
							return false;	/* only handle passwd field. */
					
					if ( (**((DialogRecord *)td)->textH).selStart != (**((DialogRecord *)td)->textH).selEnd)
						{
						memmove ((void *) &gPasswd[(**((DialogRecord *)td)->textH).selStart],
							(void *) &gPasswd[(**((DialogRecord *)td)->textH).selEnd], 
							(size_t) ((**((DialogRecord *)td)->textH).teLength - (**((DialogRecord *)td)->textH).selEnd));
						len -= (**((DialogRecord *)td)->textH).selEnd - (**((DialogRecord *)td)->textH).selStart;
						insertPt = (**((DialogRecord *)td)->textH).selStart+1;
						}
					else
						if (len)
							len--;
					
					gPasswd[len] = '\0';
					break;
				
				/* a regular character. */
				default:
					if (((DialogRecord *)td)->editField+1 != PASSWDITM)	
							return false;	/* only handle passwd field. */
					
					/* move the data to make room for the new entry. */
					if ( (**((DialogRecord *)td)->textH).selStart != (**((DialogRecord *)td)->textH).selEnd)
						{
						memmove (&gPasswd[(**((DialogRecord *)td)->textH).selStart+1],
							&gPasswd[(**((DialogRecord *)td)->textH).selEnd], 
							(**((DialogRecord *)td)->textH).teLength - (**((DialogRecord *)td)->textH).selEnd);
						len -= (**((DialogRecord *)td)->textH).selEnd - (**((DialogRecord *)td)->textH).selStart;
						}
						
						
					insertPt = (**((DialogRecord *)td)->textH).selStart+1;
					if (c > 31)
					/* legal entry. */
						if (len < 16) 
						{
							gPasswd[insertPt-1] = (char) c;
							gPasswd[++len] = '\0';
						}
					else 
					{	/* illegal entry. */
						SysBeep (3);
						return true;
					}
					break;

			}
										
				
/*
*  Clean up after myself, set the Æ echo to the right length.
*/
			for (i=0; i< len; i++)
				s[i] = '¥';
			s[len] = 0;

			GetDItem(td, PASSWDITM, &itemType, &itemHdl, &itemRect);
			setitext(itemHdl, s);
			SelIText(td, PASSWDITM, insertPt, insertPt);
			return true;

		default:
			break;
	}

	return(false);				/* default return, unknown key or mouse pressed */		

}

//
//	This routine will get the machine name or IP address, the account and password.
//
#define COMMAND_I	11
#define	MACHINE_I	 3
#define ACCOUNT_I	 5
#define PASSWORD_I 	 7

Boolean TDataTransport::GetParameters()
	{
	char		s[256];			// a string for reading from and writing to items.
	DialogPtr	theDialog;		// the dialog box to enter the params into.
	short		itemType;		// for the GetDItem
	Handle		itemHdl;		// dito.
	Rect		itemRect;		// dito.
	short 		itemHit;		// the item hit.

	// get the dialog box from which the machine name, user name and password will be gotten.
	theDialog = GetNewDialog(2002, nil, (WindowPtr) -1);
	FailNILResource ((Handle) theDialog);
	
	// this item will be the password item. Handled differently, the characters
	// are hidden.
	
	// init the other fields, possibly with previously entered name machine
	// and password, if any were entered previously
	GetDItem(theDialog, MACHINE_I, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, gMachine);
	SelIText(theDialog, (short) MACHINE_I, (short) 0, (short) 32767);																	// make the selection
	UItemAssign(theDialog, 9, (int (*)())OutlineItem);
		
	// do the dialog.
	do {
		ModalDialog((ModalFilterProcPtr) pfilt, &itemHit);
		} while (itemHit < 8 || itemHit > 10);

	// if OK hit, then get the new parameters.
	if (itemHit == 8 || itemHit == 10) 																							// OK hit, get the info.
		{
		GetDItem(theDialog, MACHINE_I, &itemType, &itemHdl, &itemRect);
		getitext(itemHdl, gMachine);
		}
	
	// trash the dialog.
	DisposDialog(theDialog);																				// trash the dialog.
	if (itemHit != 1)
		return false;
	else
		return true;
	}


// **********************************************************************
//	TDataTransport METHODS
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	GetMachineInfo
//	Displays a dialog box which collects host name, logon name & password from the user
// --------------------------------------------------------------------------------------------------
void TDataTransport::IDataTransport (void)
{
	fExuser[0] = fExpass[0] = fExmachine[0] = fExexec[0] = NULL;
	HLock ((Handle) this);
	fHeader = NewPtr (DTM_MAX_HEADER);
	HUnlock ((Handle) this);
	FailNIL (fHeader);
	fInport = fOutport = fRexecSocket = -1;
	fLocked = false;
	s_setspin ((SpinFn) MySpin);
}
	
// 
// This constructor is called when a new user is being added to a a collaborative session.
//	The outport address of the new user will be passed in in the string anOutport.
//
TDataTransport::TDataTransport ()
{
	char aHeader [DTM_MAX_HEADER];
	IDataTransport ();
		
	// Get the machine address and port.
	GetParameters();
	
	// make an outport to send our inport address on.
	fOutport = tmpDTMmakeOutPort (gMachine, DTM_SYNC);
	if (fOutport == -1)
		{
		DTMdestroyPort (fInport);			// destroy the inport.
		Failure (0, 0);
		}
	
}


TDataTransport::~TDataTransport ()
{
	EndDTM ();
}

//
// this routine is here so that you may add your own spin routine to timeout
// on a write.
//
#include <ToolUtils.h>
int TDataTransport::MyBeginWrite(char * aHeader)
{
#ifdef DTM1
	return DTMbeginWrite (fOutport, aHeader, strlen (aHeader)+1);
#else
	long ticks = TickCount() + 480;
	
	while (ticks > TickCount())
		if (DTMavailWrite (fOutport))
			return DTMbeginWrite (fOutport, aHeader, strlen (aHeader)+1);

	return DTMERROR;
#endif
}

//
// this routine is here so that you may add your own spin routine to timeout
// on a write.
//

int TDataTransport::MyEndWrite(void)
{
	int err = DTMendWrite (fOutport);
	return err;
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&& Send DTM datasets &&&&&&&&&&&&&&&&&&&&&&&&&
	
//
// write data for an 8 bit raster to the net.
//

Boolean TDataTransport::Send2DByteData (char * data, int xdim, int ydim, char * title)
{
	int rank = 2;
	int dims[2];
	char aHeader[DTM_MAX_HEADER];
	
	// set up the header.
	dims[0] = xdim;
	dims[1] = ydim;
	
	// Create a header describing the data set 
	SDSsetClass(aHeader);
	SDSsetTitle (aHeader, title);
	SDSsetDimensions(aHeader, rank, dims);
	SDSsetType(aHeader, DTM_CHAR);
	Str255 aStr;
	if (MyBeginWrite(aHeader)==DTMERROR)
		{
		return false;
		}
		
	// send the dataset.
	if (DTMwriteDataset(fOutport, data, xdim*ydim, DTM_CHAR)==DTMERROR)
		{
		MyEndWrite();
		return false ;
		}
		
	// successful
	MyEndWrite();
	return true;
}

//&&&&&&&&&&&&&&&&&&&&&&&&&&&&& Done. &&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&

// --------------------------------------------------------------------------------------------------
//		EndDTM
//		Trash the DTM connections.
// --------------------------------------------------------------------------------------------------
int TDataTransport::EndDTM ()	
{
		
	if (fInport != -1)
		DTMdestroyPort(fInport);
		
	if (fOutport != -1)
		DTMdestroyPort(fOutport);

	fInport = -1;
	fOutport = -1;
	
	// kill the rexec port.
	if (fRexecSocket > -1)
		s_close (fRexecSocket);
	
	// trash the cohandler.
		
	return 0;
}

// **********************************************************************
// These are the methods for TRasterData
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	WriteRIS
// 	This routine will write the scientific data out to an
// 		hdf file. The name of the file is contained in the RstrData
// 		field fFileName. The caller must fill this value in.
// --------------------------------------------------------------------------------------------------

int TRasterData::WriteRIS ()
	{
//	int lastref;																				// contains the last reference number to identify the RI8
//	char fFileName[256];
	
// if there is floating point data write it out to the file too.
//	HLock ((Handle) fRstr);
//	DFR8addimage (fFileName, *fRstr, fDims[0], fDims[1], DFTAG_RLE);
//	HUnlock ((Handle) fRstr);
	
// do the annotation, the label for the file.
//	lastref = DFR8lastref();
//	DFANputlabel (fFileName, DFTAG_RI8, lastref, fTitle);
	
// set up the file type.
//	SetCreatorType (fFileName, '????', '_HDF');
	return 1;
	}
