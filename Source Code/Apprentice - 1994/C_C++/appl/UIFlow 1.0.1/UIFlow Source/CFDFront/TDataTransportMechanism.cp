#include "dtm.h"
#include "sds.h"

// **********************************************************************
//	raster image data set. All we will do with it for now is write it.
// **********************************************************************
class TRasterData : public TObject
	{
	private:
		char **		fRstr;
		int				fDims[2];
		char 			fTitle [128];
	
	public:
		// This routine will write the scientific data out to an
		// hdf file.
		WriteRIS ();
		
	};
	
// **********************************************************************
//	this is the data transport mechanism. 
// **********************************************************************
class TDataTransport : public TObject
	{
	private:
		TDocument	*	fDocument;
		short		fInport;
		short		fOutport;
		short		fRexecSocket;
		char			fHeader[DTM_MAX_HEADER];
		char			fExpass[80];
		char			fExuser[80];
		char			fExmachine[80];
		
	public:
// 	This routine will launch a remote program and establish input and output connections with it
		int LaunchRemote (char * command);
		
// 	This routine determines if there is data coming in. If there is it will return it's type. 
//	If there is not data comming in,  it will return 0. If the connection is broken it will return -1.
		int DataReady ();
		
// This routine will read an incoming message and return a pointer to the message.
		int ReceiveMessage (char * message);
		
// This routine will send a command or message to the remote program.
		int SendMessage (char * message);
			
		Boolean GetMachineName(void);
		void IDataTransport(TDocument *);														// initializes data transport
		int ReceiveRasterData (TRasterData *);												// reads a raster image group.
		int EndDTM (void);																				// kill the DTM ports. 
	};
	
pascal Boolean pfilt(DialogPtr td,EventRecord * tdevent,short itemhit);
void UItemAssign( DialogPtr dlog, int item, pascal void (*proc)(DialogPtr,short));
Boolean IsCancelEvent();
pascal void OutlineItem(DialogPtr theDialog, short theItem);

// --------------------------------------------------------------------------------------------------
//	CancelEvent ()
//				This routine returns true if command-. has been entered.
// --------------------------------------------------------------------------------------------------
Boolean IsCancelEvent()
	{
	EventRecord tmpEvent;
	
	// look for a command-.
	if (EventAvail(keyDownMask, &tmpEvent)) 
		{
		GetNextEvent(keyDownMask, &tmpEvent);
		if (tmpEvent.what == keyDown || tmpEvent.what == autoKey)
			if ((tmpEvent.modifiers & cmdKey) && ((char)(tmpEvent.message & 0xFF) == '.'))
				return true;
		}
	
	return false;
	}

// --------------------------------------------------------------------------------------------------
//	UItemAssign
// --------------------------------------------------------------------------------------------------
void UItemAssign( DialogPtr dlog, int item, pascal void (*proc)())
	{
	Rect		ibox;
	char		**ihndl;
	short		typ;

	GetDItem( dlog, item, &typ, &ihndl, &ibox);
	SetDItem( dlog, item,  typ,  (Handle) proc, &ibox);
	}

// --------------------------------------------------------------------------------------------------
//	OutlineItem		Outline a dialog item
// --------------------------------------------------------------------------------------------------
pascal void OutlineItem(DialogPtr theDialog, short theItem)
	{
	Rect		ibox;
	char		**ihndl;
	short		typ;
	
	GetDItem( theDialog, theItem, &typ, &ihndl, &ibox);
	PenSize( 3,3);
	InsetRect( &ibox, -4,-4);
	FrameRoundRect( &ibox, 16,16);
	}

// **********************************************************************
//	TDataTransport METHODS
// **********************************************************************
// --------------------------------------------------------------------------------------------------
//	GetMachineInfo
//	Displays a dialog box which collects host name, logon name & password from the user
// --------------------------------------------------------------------------------------------------
void TDataTransport::IDataTransport(TDocument * doc)
	{
	fDocument = doc;
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

//pascal Boolean
//int DTM::pfilt(td,tdevent,itemhit)
pascal Boolean pfilt(DialogPtr td,EventRecord * tdevent,short itemhit)
	{
	short		itemType;
	Handle		itemHdl;
	Rect		itemRect;
	char 		s[256];
	int 		c,i,len;
	short		insertPt = 32767;
	
/*
*  If it is not the password item, let modaldialog handle it.
*/
	
	switch(tdevent->what) {
		case keyDown:
		case autoKey:
			len = strlen(fExpass);
/*
*  If it is CR, take it as OK button, as per Inside Mac
*/
			c = (tdevent->message & charCodeMask);
			
			switch (c)
			{
				/* return or enter. */
				case KBCR:
				case KBETX:
					itemhit = 1;
					return true;
					
				/* cancel (ESC) */
				case KBESC:
					itemhit = 2;
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
						memmove (&fExpass[(**((DialogRecord *)td)->textH).selStart],
							&fExpass[(**((DialogRecord *)td)->textH).selEnd], 
							(**((DialogRecord *)td)->textH).teLength - (**((DialogRecord *)td)->textH).selEnd);
						len -= (**((DialogRecord *)td)->textH).selEnd - (**((DialogRecord *)td)->textH).selStart;
						insertPt = (**((DialogRecord *)td)->textH).selStart+1;
						}
					else
						if (len)
							len--;
					
					fExpass[len] = '\0';
					break;
				
				/* a regular character. */
				default:
					if (((DialogRecord *)td)->editField+1 != PASSWDITM)	
							return false;	/* only handle passwd field. */
					
					/* move the data to make room for the new entry. */
					if ( (**((DialogRecord *)td)->textH).selStart != (**((DialogRecord *)td)->textH).selEnd)
						{
						memmove (&fExpass[(**((DialogRecord *)td)->textH).selStart+1],
							&fExpass[(**((DialogRecord *)td)->textH).selEnd], 
							(**((DialogRecord *)td)->textH).teLength - (**((DialogRecord *)td)->textH).selEnd);
						len -= (**((DialogRecord *)td)->textH).selEnd - (**((DialogRecord *)td)->textH).selStart;
						}
						
						
					insertPt = (**((DialogRecord *)td)->textH).selStart+1;
					if (c > 31)
					/* legal entry. */
						if (len < 16) 
						{
							fExpass[insertPt-1] = c;
							fExpass[++len] = '\0';
						}
					else 
					{	/* illegal entry. */
						SysBeep (3);
						return true;
					}
					break;

			}
										
				
/*
*  Clean up after myself, set the � echo to the right length.
*/
			for (i=0; i< len; i++)
				s[i] = '�';
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

Boolean TDataTransport::GetMachineName()
	{
	char			s[256];
	DialogPtr	theDialog;
	short		itemType;
	Handle		itemHdl;
	Rect			itemRect;
	short 		itemHit,i;
	
	SetCursor(&qd.arrow);

// get the dialog box from which the machine name, user name and password will be gotten.
	theDialog = GetNewDialog(138, nil, (WindowPtr) -1);
//	CenterWindow(theDialog);																			// center the dialog.
	
	// this item will be the password item. Handled differently, the characters
	// are hidden.
	GetDItem(theDialog, 6, &itemType, &itemHdl, &itemRect);
	for (i=0; i< strlen(fExpass); i++)																	// echo � instead of chars
		s[i] = '�';
	s[i] = 0;
	setitext(itemHdl, s);
	
	// init the other fields, possibly with previously entered name machine
	// and password, if any were entered previously
	GetDItem(theDialog, 5, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, fExuser);
	GetDItem(theDialog, 4, &itemType, &itemHdl, &itemRect);
	setitext(itemHdl, fExmachine);
	
	SelIText(theDialog, 4, 0,32767);																	// make the selection
	UItemAssign( theDialog, 3, OutlineItem);
		
	do {
		ModalDialog(pfilt, &itemHit);
		} while (itemHit < 1 || itemHit > 2);

	if (itemHit == 1) 																							// OK hit, get the info.
		{
		GetDItem(theDialog, 5, &itemType, &itemHdl, &itemRect);
		getitext(itemHdl, fExuser);
		GetDItem(theDialog, 4, &itemType, &itemHdl, &itemRect);
		getitext(itemHdl, fExmachine);
		}
	
	DisposDialog(theDialog);																				// trash the dialog.
	if (itemHit != 1)
		return false;
	else
		return true;
	}

// --------------------------------------------------------------------------------------------------
//	LaunchRemote
//	This routine will launch a remote program and establish a connection with it.
// --------------------------------------------------------------------------------------------------

// These are made global so that they can be saved.
char hostMachine[256], hostAccount[80],hostPasswd[80];

int TDataTransport::LaunchRemote (char * command)
{
	char remoteAddress [512], exec [512];
	int count;
	int currentTick = 0;
	char * pHost;
	
	// get the machine name, login name and passwd.
//	if (!this->GetMachineName ())
		return false;	// user canceled
	
	// get the fInport. This will allow us to dynamically determine what the port
	//	number should be.
	fInport = DTMmakeInPort (":0");

	// did we make the inport properly?
	if (fInport == -1)
		return -1;
	
	// get the formal internet address and port number of our inport.
	// the remote will connect to this address for writing.
	DTMgetPortAddr (fInport, remoteAddress, sizeof (remoteAddress));
	sprintf (exec, "%s -DTM %s",command,remoteAddress);	// construct the execution line.
	
	// launch the remote
	if ((fRexecSocket = rexec (&fExmachine, (short) 512, fExuser, fExpass,
				exec, NULL)) < 0)
		return -1;
		
	count = TickCount ();	// counter
		
	// wait for a response
	count += 3600;									// counter
	while (!DTMavailRead(fInport))				// anything there?
		{
		currentTick = TickCount();					// current tick.
		if (TickCount () > count) 
			return -1;
		else
			{
			if (IsCancelEvent()) 
				return -1;
			//mNextCursor (count+currentTick);
			}
		}
	
	// read the address. 
	if (DTMbeginRead (fInport, exec, 256) == DTMERROR)
		{
		DTMendRead (fInport);
		return -1;
		}
		
	DTMendRead (fInport);
	sscanf (exec, "DTM %s", exec);
	fOutport = DTMmakeOutPort (exec);
	
	if (fOutport == -1)
		// couldn't get the remote port.
		return -1;
}

// --------------------------------------------------------------------------------------------------
//	DataReady
// 	is there data to receive.
// --------------------------------------------------------------------------------------------------
int TDataTransport::DataReady()
{
	
	if (fInport != -1)
		if (DTMavailRead(fInport))
			{
			// attempt to read the header of the DTM message 
			if (DTMbeginRead(fInport, fHeader, DTM_MAX_HEADER) == DTMERROR)
				return -1;
				
			if (DTMcompareClass (fHeader, "MSG"))
				return 1;
			else 
				if (DTMcompareClass (fHeader, SDSclass))
					return 2;
			}
		else
			return 0;
}

// --------------------------------------------------------------------------------------------------
//	RecieveMessage
//	This routine will read an incoming message and return a pointer to the message.
// --------------------------------------------------------------------------------------------------
int TDataTransport::ReceiveMessage (char * message)
	{
	strcpy (message, fHeader+3);
	while (DTMrecvDataset(fInport, fHeader, DTM_MAX_HEADER, DTM_CHAR) > 0);
	DTMendRead(fInport);
	return 1;
	}

// --------------------------------------------------------------------------------------------------
//	SendDTMMessage
//			This routine will send a DTM message on a remote machine.
// --------------------------------------------------------------------------------------------------
int TDataTransport::SendMessage(char *command)
	{
	char  h[DTM_MAX_HEADER];
	
	// initialize the DTM ports listed on the command line.
	strcpy(h, "MSG ");
	strcat(h, command);
	
	if (DTMbeginWrite(fOutport, h, strlen(h)+1) == DTMERROR)
		return 0;
		
	DTMendWrite(fOutport);
	return 1;
	}

// --------------------------------------------------------------------------------------------------
//	RecieveRasterData
// 	this routine will read a raster image group. The TRstrData is
// 		expected to be allocated beforehand. The storage for the data
// 		however is allocated by this routine.
// --------------------------------------------------------------------------------------------------
int TDataTransport::ReceiveRasterData (TRasterData * RstrData)
	{
//	int32	dims[3] = {1, 1, 1};
//	int		type, rank = 3;
	
//	SDSgetType (fHeader, &type);
//	SDSgetDimensions(fHeader, &rank, fDims);
//	SDSgetTitle (fHeader, fTitle, sizeof(fTitle));
	
	/* read dataset */
	
//	if (type == DTM_CHAR)
//		{
//		fRstr = NewHandle (fDims[0] * fDims[1]);
//		if (fRstr)
//			return 0;
			
		/* read the data. */
	
//		HLock (fRstr);
//		DTMrecvDataset(fInport, *fRstr, dims[0] * dims[1], type);
//		HUnlock (fRstr);
//		return 1;
//		}
	return 0;
	}

// --------------------------------------------------------------------------------------------------
//		EndDTM
//		Trash the DTM connections.
// --------------------------------------------------------------------------------------------------
int TDataTransport::EndDTM ()															/* kill the DTM ports. */
	{
	if (fInport != -1)
		DTMdestroyPort(fInport);
		
	if (fOutport != -1)
		DTMdestroyPort(fOutport);

	fInport = -1;
	fOutport = -1;
	
	/* kill the rexec port. */
	
//	if (fRexecSocket > -1)
//		s_close (fRexecSocket);
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
//	int lastref;	// contains the last reference number to identify the RI8
	
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


	

