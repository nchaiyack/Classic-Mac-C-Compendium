/*
	Terminal 2.2
	"Serial.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#define CREFNUM(x)	x.ioCRefNum
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment Main2
#define CREFNUM(x)	x.ioCRefNum
#endif

#include "Port.h"
#include "Text.h"
#include "Main.h"

#ifdef USECTB					/* Communication Tool Box includes */
#include <CommResources.h>
#include <CRMSerialDevices.h>
#endif

#define INPUTLENGTH Config.input		/* Size of input buffer */
#define CURRENTPORT	Settings.portName	/* Current port's name */

typedef	struct {
		Byte *name;				/* Port's name */
		Byte *input;			/* Input driver's name */
		Byte *output;			/* Output driver's name */
} SERIALNAME;

/* ----- Static data --------------------------------------------------- */

static short INPUT = 0;			/* Current input reference number */
static short OUTPUT = 0;		/* Current output reference number */
static short PORTNO = 0;		/* 1 = Modem Port, 2 = Printer Port */
static Ptr inputBuffer = 0;		/* Current input buffer */
static Ptr outputBuffer = 0;	/* Current output buffer */

#define baud38400	1			/* Not definded in SerialDvr.h */

static short Baud[] = {			/* Baud rates we support */
	baud300,					/* 0 */
	baud600,					/* 1 */
	baud1200,					/* 2 */
	baud2400,					/* 3 */
	baud4800,					/* 4 */
	baud9600,					/* 5 */
	baud19200,					/* 6 */
	baud38400,					/* 7 */
	baud57600					/* 8 */
};
static short Data[] = {			/* Data bits we support */
	data7,						/* 0 */
	data8						/* 1 */
};
static short Parity[] = {		/* Parity options we support */
	noParity,					/* 0 */
	evenParity,					/* 1 */
	oddParity					/* 2 */
};
static short StopBits[] = {		/* Stop bits we support */
	stop10,						/* 0 */
	stop20						/* 1 */
};

static SERIALNAME Names[] = {	/* Used without Communication Tool Box */
		{ (Byte *)"\pModem Port", (Byte *)"\p.AIn", (Byte *)"\p.AOut" },
		{ (Byte *)"\pPrinter Port", (Byte *)"\p.BIn", (Byte *)"\p.BOut" }
};

/* ----- Find serial devices ------------------------------------------- */

void SerialDevice(SDPROC callback)
{
	/*	Loop thru all serial devices and call the "callback" function
		with the following arguments:
			1. Index (one based)
			2. Port's name
			3. Input driver's name
			4. Output driver's name
		The "callback" function should return TRUE to stop the search,
		or FALSE to continue. */

#ifdef USECTB
	if (CTB) {			/* Communications Toolbox is available */
		register CRMRec *p;
		register long old;
		register CRMSerialRecord *ser;
		register short index;
		CRMRec r;

		memset(&r, 0, sizeof(r));
		r.qType = crmType;
		for (old = 0, index = 1; ; old = p->crmDeviceID, ++index) {
			r.crmDeviceType = crmSerialDevice;
			r.crmDeviceID = old;
			if (!(p = (CRMRec *)CRMSearch((QElemPtr)(p = &r))))
				break;
			ser = (CRMSerialRecord *)p->crmAttributes;
			if ((* callback)(index, *(ser->name), *(ser->inputDriverName),
					*(ser->outputDriverName)))
				break;	/* Stop search */
		}
	} else
#endif
	{				/* Communications Toolbox not available */
		register short index;
		register SERIALNAME *p;

		for (index = 0, p = Names;
				index < sizeof(Names)/sizeof(SERIALNAME);
				++index, ++p)
			if ((* callback)(index + 1, p->name, p->input, p->output))
				break;	/* Stop search */
	}
}

/* ----- Open serial driver -------------------------------------------- */

static short OpenErr;		/* Driver open error */
static Byte *OpenName;		/* Port's name to find */

static short OpenSerial(	/* "callback" function for "SerialDevice" */
	short index,
	Byte *name,
	Byte *input,
	Byte *output)
{
	if (!EqualString(name, OpenName, FALSE, TRUE))
		return FALSE;		/* Not the name we want, continue */
	if (OpenErr = OpenDriver(input, &INPUT))
		return TRUE;		/* Name ok, but error during open, stop */
	if (OpenErr = OpenDriver(output, &OUTPUT)) {
		CloseDriver(INPUT);	/* Output open error */
		INPUT = OUTPUT = 0;
	} else {
		DTR = TRUE;			/* Input & output open ok, DTR asserted */
		PORTNO = index;
	}
	return TRUE;			/* Stop, port was found */
}

static short OpenSerialDriver(
	Byte *port)				/* Port's name (e.g. "\pModem Port") */
{
	OpenName = port;
	OpenErr = 1;			/* If no port with this name is found */
	DTR = FALSE;			/* DTR negated until port is open */
	PORTNO = 0;
	SerialDevice((ProcPtr)OpenSerial);
	return OpenErr;
}

/* ----- Calculate serial duration ------------------------------------- */

/*	Note: using speed changing modems (while using MNP4 or MNP5) this
	leads to wrong results... */

long SerialDuration(
	register short setup,	/* Serial chip setup word */
	register short bytes)	/* Number of bytes in buffer */
{
	register short bits;	/* Bits per bytes */
	register long bps;		/* Bits per second (*100) */
	short baud, data, parity, stop;
	static long b[] = {		/* Baud * 100 */
		30000, 60000, 120000, 240000, 480000,
		960000, 1920000, 3840000, 5760000 };

	SerialGetSetup(setup, &baud, &data, &parity, &stop);
	bits =
		1 +					/* Start bit */
		7 + data +			/* Data bits (7 or 8) */
		(parity ? 1 : 0) +	/* Parity bit (0 or 1) */
		1 + stop;			/* Stop bits (1 or 2) */
	bps = b[baud];
	return ((long)bits * (long)bytes * 6015L) / bps;
}

/* ----- Set serial parameters ----------------------------------------- */

void SerialReset(
	register short setup)	/* Serial chip setup word */
{
	SerReset(OUTPUT, setup);
	SerReset(INPUT, setup);
}

/* ----- Set to 8 data bits and no parity (binary file transfers) ------ */

void SerialBinary(
	register short setup)	/* Serial chip setup word */
{
	/* Parity   : bits 13 - 12 (0, 1, 2, 3 for no, odd, no, even) */
	/* Data bits: bits 11 - 10 (0, 1, 2, 3 for 5, 7, 6, 8) */
	/* --00 11-- ---- ---- */

	SerialReset((setup & 0xC3FF) | 0x0C00);
}

/* ----- See if set to 8 data bits ------------------------------------- */

Boolean Serial8Bits(
	register short setup)	/* Serial chip setup word */
{
	/* Parity   : bits 13 - 12 (0, 1, 2, 3 for no, odd, no, even) */
	/* Data bits: bits 11 - 10 (0, 1, 2, 3 for 5, 7, 6, 8) */
	/* --00 11-- ---- ---- */

	return (setup & 0x0C00) == 0x0C00;
}

/* ----- DTR on exit, drop or no drop ---------------------------------- */

void SerialDropDTR(register Boolean nodrop)
{
	CntrlParam c;

	if (!OUTPUT)
		return;
	CREFNUM(c) = OUTPUT;
	c.csParam[0] = nodrop ? 0x8000 : 0x0000;
	c.csCode = 16;
	PBControl((ParmBlkPtr)&c, FALSE);
}

/* ----- Set serial handshake ------------------------------------------ */

void SerialHandshake(
	register short how)		/* 0=none,1=XON/XOFF,2=CTS,3=DTR,4=CTS/DTR */
{
	CntrlParam c;
	register SerShk *handshake;

	if (!OUTPUT)
		return;
	CREFNUM(c) = OUTPUT;
	c.csCode = 14;				/* SerHShake */
	handshake = (SerShk *)&c.csParam[0];
	handshake->fXOn = 0;		/* no XON/XOFF output flow control */
	handshake->fCTS = 0;		/* no CTS (input) hardware handshake */
	handshake->xOn  = 0x00;		/* XON character */
	handshake->xOff = 0x00;		/* XOFF character */
	handshake->errs = 0;		/* Ignore input errors */
	handshake->evts = 0;		/* No device driver events */
	handshake->fInX = 0;		/* no XON/XOFF input flow control */
	handshake->fDTR = 0;		/* no DTR (output) hardware handshake */
	switch(how) {
		case 1:		/* XON/XOFF */
			handshake->fXOn = 1;	/* XON/XOFF output flow control */
			handshake->xOn  = 0x11;	/* XON character (ctrl-Q) */
			handshake->xOff = 0x13;	/* XOFF character (ctrl-S) */
			break;
		case 2:		/* CTS */
			handshake->fCTS = 1;	/* CTS (input) hardware handshake */
			break;
		case 3:		/* DTR */
			handshake->fDTR = 1;	/* DTR (output) hardware handshake */
			break;
		case 4:		/* CTS/DTR */
			handshake->fCTS = handshake->fDTR = 1;
			break;
	}
	PBControl((ParmBlkPtr)&c, FALSE);
}

/* ----- Get serial port setup ----------------------------------------- */

short SerialGetSetup(
	register short setup,	/* Serial chip setup word */
	short *baud,			/* 0=300 Baud ... */
	short *data,			/* 0=7 bits ... */
	short *parity,			/* 0=none ... */
	short *stop)			/* 0=1 stop bits ... */
{
	register short i;
	register short v;

	*baud = *data = *parity = *stop = 0;

	v = setup & 0x03FF;					/* Baud rate: bits 9 - 0 */
	for (i = 0; i < sizeof(Baud)/sizeof(short); i++)
		if (v == Baud[i]) {
			*baud = i;
			break;
		}
	if (i >= sizeof(Baud)/sizeof(short))
		return 1;

	v = setup & 0x0C00;					/* Data bits: bits 11 - 10 */
	for (i = 0; i < sizeof(Data)/sizeof(short); i++)
		if (v == Data[i]) {
			*data = i;
			break;
		}
	if (i >= sizeof(Data)/sizeof(short))
		return 2;

	v = setup & 0x3000;					/* Parity: bits 13 - 12 */
	for (i = 0; i < sizeof(Parity)/sizeof(short); i++)
		if (v == Parity[i]) {
			*parity = i;
			break;
		}
	if (i >= sizeof(Parity)/sizeof(short))
		return 3;

	v = setup & 0xC000;					/* Stop bits: bits 15 - 14 */
	for (i = 0; i < sizeof(StopBits)/sizeof(short); i++)
		if (v == StopBits[i]) {
			*stop = i;
			break;
		}
	if (i >= sizeof(StopBits)/sizeof(short))
		return 4;

	return 0;
}

/* ----- Set serial port setup ----------------------------------------- */

short SerialSetSetup(
	register short baud,	/* 0=300 Baud ... */
	register short data,	/* 0=7 bits ... */
	register short parity,	/* 0=none ... */
	register short stop,	/* 0=1 stop bits ... */
	register short *setup)	/* Serial chip setup word */
{
	*setup = Baud[0] | Data[0] | Parity[0] | StopBits[0];
	if (baud < 0 || baud >= sizeof(Baud)/sizeof(short))
		return 1;
	if (data < 0 || data >= sizeof(Data)/sizeof(short))
		return 2;
	if (parity < 0 || parity >= sizeof(Parity)/sizeof(short))
		return 3;
	if (stop < 0 || stop >= sizeof(StopBits)/sizeof(short))
		return 4;
	*setup = Baud[baud] | Data[data] | Parity[parity] | StopBits[stop];
	return 0;
}

/* ----- Close serial port --------------------------------------------- */

void SerialClose(void)
{
	/*	Warning! VERY BAD THINGS CAN HAPPEN (Perhaps even loss of data
		on your hard disk!) long after your program quits if you forget
		to do this, or forget to close the driver properly!
		It is essential that you do not ever leave a custom input buffer
		dangling, EVEN if you abort your program for some other reason!
		The OS will NOT fix this up for you! You have been warned! */

	if (INPUT) {
		SerSetBuf(INPUT, inputBuffer, 0);
		CloseDriver(INPUT);
		INPUT = 0;
	}
	if (OUTPUT) {
		SerSetBuf(OUTPUT, outputBuffer, 0);
		CloseDriver(OUTPUT);
		OUTPUT = 0;
	}
}

/* ----- Open serial port and set parameters --------------------------- */

short SerialOpen(
	register Byte *port,		/* Port's name (e.g. "\pModem Port") */
	register short setup,		/* Serial chip setup word */
	register short handshake)	/* 0=none ... */
{
	register short err;

	memcpy(CURRENTPORT, port, *port + 1);
	if (port[0] == 0)			/* It's ok to use the "none" port */
		return 0;
	if (err = OpenSerialDriver(port))
		return err;
	/* Keep input buffer if already allocated */
	if (!inputBuffer && !(inputBuffer = NewPtr(INPUTLENGTH))) {
		SerialClose();
		return memFullErr;
	}
	SerSetBuf(INPUT, inputBuffer, INPUTLENGTH);
	SerialHandshake(handshake);
	SerialReset(setup);
	return 0;					/* Ok */
}

/* ----- Read characters from serial port ------------------------------ */

void SerialFastRead(
	register Byte *buffer,
	register long count)
{
	IOParam b;

	if (!INPUT)
		return;
	b.ioRefNum = INPUT;
	b.ioBuffer = (Ptr)buffer;
	b.ioReqCount = count;
	PBRead((ParmBlkPtr)&b, FALSE);
}

/* ----- Read characters from serial port ------------------------------ */

long SerialRead(
	register Byte *buffer,
	register long max)
{
	register long n;
	CntrlParam c;
	IOParam b;

	if (!INPUT)
		return 0;
	CREFNUM(c) = INPUT;
	c.csCode = 2;
	if (PBStatus((ParmBlkPtr)&c, FALSE))
		return 0;
	if (n = *(long *)&c.csParam[0]) {
		b.ioRefNum = INPUT;
		b.ioBuffer = (Ptr)buffer;
		b.ioReqCount = (max > n) ? n : max;
		return PBRead((ParmBlkPtr)&b, FALSE) ? 0 : b.ioReqCount;
	}
	return 0;	/* Nothing available */
}

/* ----- Flush serial input buffer ------------------------------------- */

void SerialFlush(void)
{
	CntrlParam b;

	if (!INPUT)
		return;
	CREFNUM(b) = INPUT;
	b.ioCompletion = 0;
	PBKillIO((ParmBlkPtr)&b, FALSE);
}

/* ----- Abort serial send --------------------------------------------- */

void SerialAbort(void)
{
	CntrlParam b;

	if (!OUTPUT)
		return;
	CREFNUM(b) = OUTPUT;
	b.ioCompletion = 0;
	PBKillIO((ParmBlkPtr)&b, FALSE);
}

/* ----- Send buffer --------------------------------------------------- */

static pascal void SendComplete(void);

#ifdef THINK_C
static pascal void SendComplete()
{
	/* A0 has address of IOParam */
	asm {
		MOVE.L	-4(A0),A0	/* Get busy flag address */
		CLR.B	0(A0)		/* Clear the busy flag (FALSE) */
	}
}
#endif

void SerialSend(
	register Byte *buffer,
	register long count,
	register Boolean *busy)
{
	static struct {		/* Must be static! */
		Boolean *busy;	/* TRUE while sending */
		IOParam b;
	} param;

	if (!OUTPUT) {
		*busy = FALSE;
		return;
	}
	*busy = TRUE;
	param.busy = busy;
	param.b.ioCompletion = (ProcPtr)SendComplete;
	param.b.ioRefNum = OUTPUT;
	param.b.ioBuffer = (Ptr)buffer;
	param.b.ioReqCount = count;
	PBWrite((ParmBlkPtr)&param.b, TRUE);		/* Asynchrone */
}

/* ----- See how many bytes are in input buffer ------------------------ */

long SerialCheck(void)
{
	CntrlParam c;

	if (!INPUT)
		return 0;
	CREFNUM(c) = INPUT;
	c.csCode = 2;
	if (!PBStatus((ParmBlkPtr)&c, FALSE))
		return *(long *)&c.csParam[0];
	return 0;
}

/* ----- Negate or assert DTR ------------------------------------------ */

void SerialDTR(
	register Boolean how)	/* FALSE: negate, TRUE:assert */
{
	CntrlParam c;

	if (!OUTPUT)
		return;
	CREFNUM(c) = OUTPUT;
	c.csCode = how ? 17 : 18;
	PBControl((ParmBlkPtr)&c, FALSE);
	DTR = how;
}

/* ----- Check CTS/DCD ------------------------------------------------- */

/*
	Pin number	Signal name		Signal description

		1			HSKo		Output Handshake (SCC DTR output)
		2			HSKi		Input Handshake (SCC CTS input)
		7			GPi			General-purpose input (SCC DCD input)
*/

#ifdef QUICK_AND_DIRTY

Boolean SerialCTS(void)
{
	register Byte r;

	if (!INPUT || PORTNO < 1 || PORTNO > 2)
		return FALSE;
	r = *(SCCRd + ((PORTNO == 2) ? 0 : 2));	/* SCC channel A/B control */
	return !(r & 0x20);						/* CTS is bit 5 */
}

#else

Boolean SerialCTS(void)
{
	CntrlParam c;

	if (!INPUT)
		return 0;
	CREFNUM(c) = INPUT;
	c.csCode = 8;
	if (!PBStatus((ParmBlkPtr)&c, FALSE))
		return !((SerStaRec *)&c.csParam[0])->ctsHold;
	return 0;
}

#endif

Boolean SerialDCD(void)
{
	register Byte r;

	if (!INPUT || PORTNO < 1 || PORTNO > 2)
		return FALSE;
	r = *(SCCRd + ((PORTNO == 2) ? 0 : 2));	/* SCC channel A/B control */
	return !(r & 0x08);						/* DCD is bit 3 */
}
