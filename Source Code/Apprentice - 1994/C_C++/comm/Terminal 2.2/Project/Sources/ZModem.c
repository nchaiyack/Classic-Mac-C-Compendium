/*
	Terminal 2.2
	"ZModem.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment ZModem
#endif

#include "Text.h"
#include "Main.h"
#include "Port.h"
#include "CancelDialog.h"
#include "Crc.h"
#include "Utilities.h"
#include "Strings.h"
#include "FormatStr.h"
#include "File.h"
#include "MacBinary.h"

#include "ZModem.h"

/* #define MONITOR */				/* Debugging */

#ifdef MONITOR
#include "Monitor.h"
#endif

/* ----- Special characters -------------------------------------------- */

#define BS			0x08	/* Backspace */
#define	LF			0x0A	/* Linefeed */
#define CR			0x0D	/* Carriage return */
#define DLE			0x10	/* Data link escape */
#define XON			0x11	/* Flow control */
#define XOFF		0x13	/* Flow control */
#define CAN			0x18	/* Cancel */
#define DEL			0x7F	/* Delete */

#define ZPAD		'*'		/* Padding character begins frames */
#define ZDLE		0x18	/* Zmodem escape character ctrl-X */
#define ZBIN		'A'		/* Binary header indicator (CRC-16) */
#define ZHEX		'B'		/* HEX header indicator */

/* ----- Frame types --------------------------------------------------- */

#define ZRQINIT		0		/* Request receive init */
#define ZRINIT		1		/* Receive init */
#define ZSINIT		2		/* Send init sequence (optional) */
#define ZACK		3		/* ACK to above */
#define ZFILE		4		/* File name from sender */
#define ZSKIP		5		/* To sender: skip this file */
#define ZNAK		6		/* Last packet was garbled */
#define ZABORT		7		/* Abort batch transfers */
#define ZFIN		8		/* Finish session */
#define ZRPOS		9		/* Resume data trans at this position */
#define ZDATA		10		/* Data packet(s) follow */
#define ZEOF		11		/* End of file */
#define ZFERR		12		/* Fatal Read or Write error Detected */
#define ZCRC		13		/* Request for file CRC and response */
#define ZCHALLENGE	14		/* Receiver's Challenge */
#define ZCOMPL		15		/* Request is complete */
#define ZCAN		16		/* Other end canned session with 5 CANs */
#define ZFREECNT	17		/* Request for free bytes on filesystem */
#define ZCOMMAND	18		/* Command from sending program */
#define ZSTDERR		19		/* Output to standard error, data follows */

/* ----- ZDLE sequences ------------------------------------------------ */

#define ZCRCE		'h'		/* CRC next, frame ends, header follows */
#define ZCRCG		'i'		/* CRC next, frame continues nonstop */
#define ZCRCQ		'j'		/* CRC next, frame continues, ZACK expected */
#define ZCRCW		'k'		/* CRC next, ZACK expected, end of frame */
#define ZRUB0		'l'		/* Translate to DEL 0x7F */
#define ZRUB1		'm'		/* Translate to DEL 0xFF */

/* ----- Header structures --------------------------------------------- */

typedef struct {
	unsigned long: 24;
	unsigned long command: 8;	/* 0 or ZCOMMAND */
} ZRQINITflags;

typedef struct {
	unsigned long bufsize: 16;	/* Receiver's buffer size (bytes swapped) */
	unsigned long: 7;
	unsigned long canvhdr: 1;	/* Variable headers OK */
	unsigned long esc8: 1;		/* Rx expects 8th bit to be escaped */
	unsigned long escctl: 1;	/* Rx expects ctl chars to be escaped */
	unsigned long canfc32: 1;	/* Rx can use 32 bit CRC */
	unsigned long canlzw: 1;	/* Rx can uncompress */
	unsigned long canrle: 1;	/* Rx can decode RLE */
	unsigned long canbrk: 1;	/* Rx can send a break signal */
	unsigned long canovio: 1;	/* Rx can receive data during disk I/O */
	unsigned long canfdx: 1;	/* Rx can send and receive true FDX */
} ZRINITflags;

typedef struct {
	unsigned long: 24;
	unsigned long esc8: 1;		/* Tx expects 8th bit to be escaped */
	unsigned long escctl: 1;	/* Tx expects ctl chars to be escaped */
	unsigned long: 6;
} ZSINITflags;

typedef struct {
	unsigned long: 1;
	unsigned long sparse: 1;	/* Encoding for sparse file operations */
	unsigned long: 5;
	unsigned long canvhdr: 1;	/* Variable headers OK */
	unsigned long transport: 8;	/* Transport options */
	unsigned long skip: 1;		/* Skip file if not present at rx */
	unsigned long: 2;
	unsigned long manage: 5;	/* Management options */
	unsigned long conv: 8;		/* Conversion options */
} ZFILEflags;

typedef struct {
	unsigned long: 31;
	unsigned long ack: 1;		/* Acknowledge, then do command */
} ZCOMMANDflags;

typedef union {
	unsigned char b[4];
	unsigned long position;		/* File position (low byte first) */
	ZRQINITflags zrqinit;
	ZRINITflags zrinit;
	ZSINITflags zsinit;
	ZFILEflags zfile;
	ZCOMMANDflags zcommand;
} HEADER;

/* ----- Conversion options, ZFILE frame ------------------------------- */

#define ZCBIN		1		/* Binary transfer - inhibit conversion */
#define ZCNL		2		/* Convert NL to local eol convention */
#define ZCRESUM		3		/* Resume interrupted file transfer */

/* ----- Management options, ZFILE frame ------------------------------- */

#define ZMNEWL		1		/* Transfer if source newer or longer */
#define ZMCRC		2		/* Transfer if different file CRC or length */
#define ZMAPND		3		/* Append contents to existing file (if any) */
#define ZMCLOB		4		/* Replace existing file */
#define ZMNEW		5		/* Transfer if source newer */
#define ZMDIFF		6		/* Transfer if dates or lengths different */
#define ZMPROT		7		/* Protect destination file */

/* ----- Transport options, ZFILE frame -------------------------------- */

#define ZTLZW		1		/* Lempel-Ziv compression */
#define ZTRLE		3		/* Run Length encoding */

/* ----- Other constants ----------------------------------------------- */

#define ZATTNLEN	32		/* Max length of attention string */
#define ZMAXSPLEN	1024	/* Max subpacket length */

/* ----- Return values (internal) -------------------------------------- */

#define GOTOR	0x0100
#define GOTCRCE (GOTOR | ZCRCE)	/* ZDLE-ZCRCE received */
#define GOTCRCG (GOTOR | ZCRCG)	/* ZDLE-ZCRCG received */
#define GOTCRCQ (GOTOR | ZCRCQ)	/* ZDLE-ZCRCQ received */
#define GOTCRCW (GOTOR | ZCRCW)	/* ZDLE-ZCRCW received */

/* ----- Globals ------------------------------------------------------- */

Byte ZAutoReceiveString[] = {
	ZPAD, ZDLE, ZHEX,
	'0', '0',									/* Type: ZRQINIT */
	'0', '0', '0', '0', '0', '0', '0', '0',
	'0', '0', '0', '0',							/* CRC1 CRC2 */
	CR,
	0
};
Byte ZLastRx[] = { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };

#define GARBAGE 1400+2400		/* Garbage count before header */
static jmp_buf CancelEnv;		/* Long jump if cancel or abort */
static Boolean Zctlesc;			/* TRUE: escape all control characters */
static Byte	*TxBuffer = 0;		/* Transmit buffer */
static Byte	*TxPtr;				/* Pointer into transmit buffer */

/* Shared by transmit and receive */
static short RefNum;			/* File reference number */
static unsigned long Mark;		/* Current file position */
static unsigned long Length;	/* Length of file */
static unsigned long StartPos;	/* Start file position */
static unsigned long StartTime;	/* Start time */
static short Vers;				/* MacBinary version */
static Byte *Buffer;			/* Block buffer + attention string + ... */

/* Transmit only */
static unsigned long Txwindow;	/* Size of the transmitted window */
static unsigned long Txwspac;	/* Spacing between ZCRCQ requests */
static unsigned short Rxbuflen;	/* Receiver's max buffer length */
static unsigned long Lastsync;	/* Last offset to which we got a ZRPOS */
static unsigned long Lrxpos;	/* Receiver's last reported offset */
static short BeenHereB4;		/* How many times ZRPOS same place */

/* Receive only */
static Byte TryZhdrType;		/* Header type to send */
static unsigned short Count;	/* Bytes in block buffer */
#define Attn (Buffer+ZMAXSPLEN)	/* Sender's attention string */
static unsigned short AttnLen;	/* Bytes in attention string */
#define BinHdr (Attn+ZATTNLEN)	/* MacBinary header received */
#define BUFFERSIZE (ZMAXSPLEN+ZATTNLEN+BinHeaderLength)
static Boolean ConvertNL;		/* Convert newline to local eol */

/* ===== COMMON FUNCTIONS ============================================== */

#ifdef MONITOR
/* ----- Translate ZModem constants ------------------------------------ */

static char *FrameTypes[] = {
	"\pZRQINIT",
	"\pZRINIT",
	"\pZSINIT",
	"\pZACK",
	"\pZFILE",
	"\pZSKIP",
	"\pZNAK",
	"\pZABORT",
	"\pZFIN",
	"\pZRPOS",
	"\pZDATA",
	"\pZEOF",
	"\pZFERR",
	"\pZCRC",
	"\pZCHALLENGE",
	"\pZCOMPL",
	"\pZCAN",
	"\pZFREECNT",
	"\pZCOMMAND",
	"\pZSTDERR"
};

static char *ZFrameType (register short x)
{
	static char s[10];

	if (x < sizeof(FrameTypes)/sizeof(char *))
		return FrameTypes[x];
	NumToString(x, s);
	return s;
}

typedef struct {
	Byte	c;
	char	*s;
} FRAMEND;

static FRAMEND ZFrameEnds[] = {
	ZCRCE, "\pZCRCE",
	ZCRCG, "\pZCRCG",
	ZCRCQ, "\pZCRCQ",
	ZCRCW, "\pZCRCW",
	ZRUB0, "\pZRUB0",
	ZRUB1, "\pZRUB1",
	0, 0
};

static char *ZFrameEnd (register Byte x)
{
	register FRAMEND *p = ZFrameEnds;
	static char s[10];
	
	while (p->s) {
		if (p->c == x)
			return p->s;
		++p;
	}
	NumToString(x, s);
	return s;
}
#endif

/* ----- Should we escape control characters? -------------------------- */

static Boolean Escape(void)
{
	return Settings.ZEscapeCtl || !Serial8Bits(Settings.portSetup);
}

/* ----- Allocate memory for transmit buffer --------------------------- */

static short NewTxBuffer (register Boolean block)
{
	if (TxBuffer)
		return 1;				/* Error: already allocated */
	TxBuffer = (Byte *)NewPtr(block ?
		2*(ZMAXSPLEN + 6) :		/* Worst case for transmitting block */
		32);					/* Worst case for transmitting header */
	return TxBuffer ? 0 : memFullErr;
}

/* ----- Dispose of transmit buffer ------------------------------------ */

static void DisposeTxBuffer (void)
{
	while (Busy)
		;
	if (TxBuffer)
		DisposPtr((Ptr)TxBuffer);
	TxBuffer = 0;
}

/* ----- Prepare new transmission -------------------------------------- */

static void InitTxBuffer (void)
{
	while (Busy)	/* Wait until previous transmission finished */
		if (CheckCancel())
			longjmp(CancelEnv, CANCEL);
	TxPtr = TxBuffer;
}

/* ----- Start transmission, don't wait -------------------------------- */

static void TransmitTxBuffer (void)
{
	SerialSend(TxBuffer, TxPtr - TxBuffer, &Busy);
}

/* ----- Send more bytes ----------------------------------------------- */

static void SendString (register Byte *s, register unsigned short n)
{
	InitTxBuffer();
	memcpy(TxPtr, s, n);
	TxPtr += n;
	TransmitTxBuffer();
}

/* ----- Send cancel string -------------------------------------------- */

static void SendCancel(void)
{
	static Byte cs[] = {
		CAN, CAN, CAN, CAN, CAN, CAN, CAN, CAN,
		BS, BS, BS, BS, BS, BS, BS, BS, BS, BS
	};

	SerialSend(cs, sizeof(cs), &Busy);
	while (Busy)
		;
}

/* ----- Sleep a while ------------------------------------------------- */

static void Sleep (register unsigned long ticks)
{
	register unsigned long start = Ticks;

	while((Ticks - start) < ticks)
		if (CheckCancel())
			longjmp(CancelEnv, CANCEL);
}

/* ----- Send a byte --------------------------------------------------- */

static Byte SendByte (register Byte c)
{
	*TxPtr++ = c;
	return c;
}

/* ----- Send attention string ----------------------------------------- */

static void SendAttn (register Byte *s)
{
	while (Busy)
		;
	while (*s) {
		switch (*s) {
			case 0xDD:		/* Send break signal */
				/* sendbrk(); */
				break;
			case 0xDE:		/* Sleep 1 second */
				Sleep(60);
				break;
			default:
				SerialSend(s, 1, &Busy);
				while (Busy)
					if (CheckCancel())
						longjmp(CancelEnv, CANCEL);
		}
		++s;
	}
}

/* ----- Swap 4 bytes in long ------------------------------------------ */

static unsigned long Swap4 (register unsigned long n)
{
	register short i;
	register unsigned long x = 0;

	for (i = 0; i < 4; ++i) {
		x <<= 8;
		x |= n & 0xFF;
		n >>= 8;
	}
	return x;
}

/* ----- Swap 2 bytes in short ----------------------------------------- */

static unsigned short Swap2 (register unsigned short n)
{
	register short i;
	register unsigned short x = 0;

	for (i = 0; i < 2; ++i) {
		x <<= 8;
		x |= n & 0xFF;
		n >>= 8;
	}
	return x;
}

/* ----- Adjust header ------------------------------------------------- */

static void AdjustHeader (register Byte type, register HEADER *p)
{
	switch (type) {
		case ZACK:
		case ZRPOS:
		case ZDATA:
		case ZEOF:
		case ZCRC:
		case ZCHALLENGE:
		case ZFREECNT:
		case ZCOMPL:
			p->position = Swap4(p->position);
			break;
		case ZRINIT:
			p->zrinit.bufsize = Swap2(p->zrinit.bufsize);
			break;
	}
}

/* ----- Send a byte as two hex digits --------------------------------- */

static Byte PutHex (register Byte c)
{
	static Byte digits[] = {
		'0','1','2','3','4','5','6','7','8','9','a','b','c','d','e','f' };

	SendByte(digits[(c >> 4) & 0xF]);
	SendByte(digits[c & 0xF]);
	return c;
}

/* ----- Send a byte with ZMODEM escape sequence encoding -------------- */

static Byte SendLine (register Byte c)
{
	register Byte b = c;
	static Byte lastsent = 0;	/* Last byte sent */

	if (c & 0x60)		/* Non control character */
		SendByte(lastsent = c);
	else {
		switch (c) {
			case ZDLE:
				SendByte(ZDLE);
				SendByte(lastsent = c ^ 0x40);
				break;
			case CR:	/* Escape CR followed by @ (Telenet net escape) */
			case 0x80 | CR:
				if (!Zctlesc && (lastsent & 0x7F) != '@') {
					SendByte(lastsent = c);
					break;
				}
				/* Fall thru */
			case DLE:
			case 0x80 | DLE:
			case XON:
			case 0x80 | XON:
			case XOFF:
			case 0x80 | XOFF:
				SendByte(ZDLE);
				c ^= 0x40;
				SendByte(lastsent = c);
				break;
			default:
				if (Zctlesc && !(c & 0x60)) {
					SendByte(ZDLE);
					c ^= 0x40;
				}
				SendByte(lastsent = c);
		}
	}
	return b;
}

/* ----- Send ZMODEM HEX header ---------------------------------------- */

static void SendHexHeader (register Byte type, HEADER hdr)
{
	register short i;
	register unsigned short crc;

	AdjustHeader(type, &hdr);

#ifdef MONITOR
	{
		Byte mon[256];
		FormatStr(mon, (Byte *)"\pTransmit HEX header %s",
			ZFrameType(type));
		MonitorText(mon);
		MonitorDump(&hdr.b[0], 4);
	}
#endif

	InitTxBuffer();
	SendByte(ZPAD);								/* 1 */
	SendByte(ZPAD);								/* 1 */
	SendByte(ZDLE);								/* 1 */
	SendByte(ZHEX);								/* 1 */
	crc = UpdateCRC(PutHex(type), 0);
	for (i = 0; i < 4; ++i)
		crc = UpdateCRC(PutHex(hdr.b[i]), crc);	/* 8 */
	PutHex((crc >> 8) & 0xFF);					/* 2 */
	PutHex(crc & 0xFF);							/* 2 */
	SendByte(CR);								/* 1 */
	SendByte(LF);								/* 1 */
	if (type != ZFIN && type != ZACK)
		SendByte(XON);							/* 1 */
	/*SerialFlush();*/
	TransmitTxBuffer();				/* Worst case:	19 bytes */
}

/* ----- Send ZMODEM binary header ------------------------------------- */

static void SendBinaryHeader (register Byte type, HEADER hdr)
{
	register short i;
	register unsigned short crc;

	AdjustHeader(type, &hdr);

#ifdef MONITOR
	{
		Byte mon[256];
		FormatStr(mon, (Byte *)"\pTransmit BIN header %s",
			ZFrameType(type));
		MonitorText(mon);
		MonitorDump(&hdr.b[0], 4);
	}
#endif

	InitTxBuffer();
	SendByte(ZPAD);								/* 1 */
	SendByte(ZDLE);								/* 1 */
	SendByte(ZBIN);								/* 1 */
	crc = UpdateCRC(SendLine(type), 0);
	for (i = 0; i < 4; ++i)
		crc = UpdateCRC(SendLine(hdr.b[i]), crc);	/* 8 */
	SendLine((crc >> 8) & 0xFF);					/* 2 */
	SendLine(crc & 0xFF);							/* 2 */
	if (type != ZDATA)
		SerialFlush();											/*****/
	TransmitTxBuffer();				/* Worst case:	15 bytes */
}

/* ----- Send binary array with ending ZDLE sequence ------------------- */

static void SendData (
	register Byte *buf,
	unsigned short length,
	Byte frameend)
{
	register unsigned short crc;

#ifdef MONITOR
	{
		Byte mon[256];
		FormatStr(mon, (Byte *)"\pTransmit %i bytes %s",
			length, ZFrameEnd(frameend));
		MonitorText(mon);
		MonitorDump(buf, length);
	}
#endif

	InitTxBuffer();
	for (crc = 0 ; length; ++buf, --length) {
		SendLine(*buf);
		crc = UpdateCRC(*buf, crc);
	}
	SendByte(ZDLE);
	SendByte(frameend);
	crc = UpdateCRC(frameend, crc);
	SendLine((crc >> 8) & 0xFF);
	SendLine(crc & 0xFF);
	if (frameend == ZCRCW) {
		SendByte(XON);
		SerialFlush();
	}
	TransmitTxBuffer();
}

/* ----- Read character with timeout ----------------------------------- */

static short ReadByte (
	register Byte *c,				/* Character (result) */
	register unsigned long timeout)	/* Timeout in ticks */
{
	register unsigned long start;
	register long count;
	static short abort = 0;	/* CAN counter */

	/* If a character is available don't check for cancel,
	this speeds up reception */

	if (!SerialRead(c, 1)) {
    	start = Ticks;
		do {
			if ((Ticks - start) > timeout)
				return TIMEOUT;
			if (!(count = SerialRead(c, 1)) && CheckCancel())
				longjmp(CancelEnv, CANCEL);
        } while (count == 0);
	}
	if (*c == CAN) {
		if (++abort >= 5)
			longjmp(CancelEnv, ABORT);
	} else
		abort = 0;
    return FINE;
}

/* ----- Wait for ZPAD character --------------------------------------- */

static short WaitZPAD (
	unsigned long timeout,		/* Timeout in ticks */
	register unsigned short n)	/* Max bytes before start of frame */
{
	Byte c;

	for (;;)
		switch (ReadByte(&c, timeout)) {
			case FINE:
				if ((c & 0x7F) == ZPAD)
					return FINE;
				if (--n == 0)
					return ERROR;
				break;
			case TIMEOUT:
				return TIMEOUT;
		}
}

/* ----- Read character with timemout, eat parity, XON and XOFF -------- */

static short ReadByte2 (
	register Byte *c,				/* Character (result) */
	register unsigned long timeout)	/* Timeout in ticks */
{
	for (;;) {
		if (ReadByte(c, timeout))
			return TIMEOUT;
		switch (*c &= 0x7F) {
			case XON:
			case XOFF:
				break;
			default:
				if (Zctlesc && !(*c & 0x60))
					break;
			case CR:
			case LF:
			case ZDLE:
				return FINE;
		}
	}
}

/* ----- Decode two lower case hex digits into an 8 bit byte value ----- */

static short GetHex (register Byte *c)
{
	register short i;
	Byte n;

	*c = 0;
	for (i = 0; i < 2; ++i) {
		*c <<= 4;
		if (ReadByte2(&n, Settings.ZTimeout))
			return TIMEOUT;
		if (n >= '0' && n <= '9')
			n -= '0';
		else
			if (n >= 'a' && n <= 'f')
				n -= 'a' - 10;
			else
				return ERROR;
		*c += n;
	}
	return FINE;
}

/* ----- Read a byte, checking for ZMODEM escape encoding -------------- */

static short ZdleRead (register Byte *c)
{
	do {
		if (ReadByte(c, Settings.ZTimeout))
			return TIMEOUT;
		if (*c & 0x60)						/* Non-control character */
			return FINE;
		switch (*c) {						/* Control character */
			case ZDLE:
			case XON:
			case 0x80 | XON:
			case XOFF:
			case 0x80 | XOFF:
				break;						/* Ignore XON/XOFF */
			default:
				if (!Zctlesc)
					return FINE;
				break;						/* Ignore ctrl if escaping */
		}
	} while (*c != ZDLE);

	/* Previous character was ZDLE */

	for (;;) {
		if (ReadByte(c, Settings.ZTimeout))
			return TIMEOUT;
		switch (*c) {
			case ZCRCE:
			case ZCRCG:
			case ZCRCQ:
			case ZCRCW:
				return *c | GOTOR;			/* Frame end */
			case ZRUB0:
				*c = DEL;
				return FINE;
			case ZRUB1:
				*c = 0x80 | DEL;
				return FINE;
			case XON:
			case 0x80 | XON:
			case XOFF:
			case 0x80 | XOFF:
				break;						/* Ignore XON/XOFF */
			default:
				if (Zctlesc && !(*c & 0x60))
					break;					/* Ignore ctrl if escaping */
				if ((*c & 0x60) != 0x40)	/* Must be -10- ---- */
					return ERROR;
				*c ^= 0x40;					/* Invert bit 6 */
				return FINE;
		}
	}
}

/* ----- Receive a binary style header (type and position) ------------- */

static short ReceiveBinaryHeader (
	register Byte *type,
	register Byte *hdr)
{
	register short n, err;
	register unsigned short crc;
	Byte c;

	/* Header type */

	if (err = ZdleRead(type))
		return err;		/* TIMEOUT, ERROR, GOTxxxx */
	crc = UpdateCRC(*type, 0);

	/* Header info (4 bytes) */

	for (n = 0; n < 4; ++n, ++hdr) {
		if (err = ZdleRead(hdr))
			return err;	/* TIMEOUT, ERROR, GOTxxxx */
		crc = UpdateCRC(*hdr, crc);
	}

	/* CRC (2 bytes) */

	if (err = ZdleRead(&c))
		return err;		/* TIMEOUT, ERROR, GOTxxxx */
	crc = UpdateCRC(c, crc);
	if (err = ZdleRead(&c))
		return err;		/* TIMEOUT, ERROR, GOTxxxx */
	if (crc = UpdateCRC(c, crc))
		return ERROR;	/* Bad CRC */

	return FINE;
}

/* ----- Receive a hex style header (type and position) ---------------- */

static short ReceiveHexHeader (
	register Byte *type,
	register Byte *hdr)
{
	register short n, err;
	register unsigned short crc;
	Byte c;

	/* Header type */

	if (err = GetHex(type))
		return err;
	crc = UpdateCRC(*type, 0);

	/* Header info (4 bytes) */

	for (n = 0; n < 4; ++n, ++hdr) {
		if (err = GetHex(hdr))
			return err;
		crc = UpdateCRC(*hdr, crc);
	}

	/* CRC (2 bytes) */

	if (err = GetHex(&c))
		return err;
	crc = UpdateCRC(c, crc);
	if (err = GetHex(&c))
		return err;
	if (crc = UpdateCRC(c, crc))
		return ERROR;	/* Bad CRC */

	/* Throw away possible CR/LF */

	if (ReadByte(&c, 1) == FINE && (c & 0x7F) == CR)
		ReadByte(&c, 1);

	return FINE;
}

/* ----- Read a ZMODEM header (either binary or hex) ------------------- */

static short GetHeader (
	register Byte *type,
	register HEADER *hdr)
{
	register short err;
	register short n = GARBAGE;
	Byte c;
#ifdef MONITOR
	short headerNature = 0;
#endif

	/* Wait for ZPAD character */

	if (err = WaitZPAD(Settings.ZTimeout, GARBAGE))
		goto done;

	/* Just got ZPAD character, wait for ZDLE character */

	do {
		if (err = ReadByte2(&c, Settings.ZTimeout))
			goto done;
		switch (c) {
			case ZDLE:		/* This is what we want */
			case ZPAD:		/* May be more than one ZPAD */
			case ZPAD | 0x80:
				break;
again:
			default:		/* Garbage */
				if (--n == 0) {
					err = ERROR;
					goto done;
				}
				break;
		}
	} while (c != ZDLE);

	/* Just got ZPAD-ZDLE sequence */

	if (err = ReadByte2(&c, Settings.ZTimeout))
		goto done;
	switch (c) {
		case ZBIN:		/* It's a binary header */
#ifdef MONITOR
			headerNature = 1;
#endif
			err = ReceiveBinaryHeader(type, hdr->b);
			break;
		case ZHEX:		/* It's a hex header */
#ifdef MONITOR
			headerNature = 2;
#endif
			err = ReceiveHexHeader(type, hdr->b);
			break;
		default:		/* Garbage */
			goto again;
	}
	AdjustHeader(*type, hdr);

	/* ZPAD ZDLE ZBIN/ZHEX ... */

done:

#ifdef MONITOR
	{
		static char *binhex[] = { "\pBIN", "\pHEX" };
		Byte mon[256];
		if (headerNature) {
			FormatStr(mon, (Byte *)"\pReceived %s header %s error %i",
				binhex[headerNature - 1],
				ZFrameType(*type),
				err);
			MonitorText(mon);
			MonitorDump(hdr->b, 4);
		} else {
			FormatStr(mon, (Byte *)"\pReceived error %i", err);
			MonitorText(mon);
		}
	}
#endif

	return err;
}

/* ----- Receive array with ending ZDLE sequence and CRC --------------- */

static short ReceiveData (
	register Byte *buf,		/* Buffer for data */
	unsigned short length,	/* Max size of buffer */
	unsigned short *count,	/* Number of data bytes received */
	Byte *framend)			/* Frame end character received */
{
	register unsigned short crc = 0;
	register Byte *max = buf + length;
	register short err;
	Byte c;

	*framend = 0xFF;
	while (buf <= max) {
		switch (err = ZdleRead(&c)) {
			case FINE:				/* Data byte */
				crc = UpdateCRC(c, crc);
				*buf++ = (ConvertNL && c == LF) ? CR : c;
				break;
			case GOTCRCE:			/* Frame end, CRC follows */
			case GOTCRCG:
			case GOTCRCQ:
			case GOTCRCW:
				*count = length - (max - buf);
				crc = UpdateCRC(*framend = c, crc);
				if (err = ZdleRead(&c))
					goto done;
				crc = UpdateCRC(c, crc);
				if (err = ZdleRead(&c))
					goto done;
				if (crc = UpdateCRC(c, crc))
					err = ERROR;	/* Bad CRC */
				else
					err = FINE;
				goto done;
			default:				/* TIMEOUT, ERROR, ... */
				*count = length - (max - buf);
				goto done;
		}
	}
	*count = length - (max - buf);
	err = ERROR;					/* Data subpacket too long */

done:
#ifdef MONITOR
	{
		Byte mon[256];
		FormatStr(mon, (Byte *)"\pReceived %i bytes, end %s (error: %i)",
			*count, ZFrameEnd(*framend), err);
		MonitorText(mon);
		MonitorDump(buf - *count, *count);
	}
#endif
	return err;
}

/* ----- Update progress indicator ------------------------------------- */

static void Update (register Byte *text, register long error)
{
	register unsigned long n = Mark - StartPos;
	register unsigned long m = (Length > StartPos) ? Length - StartPos : 0;

	UpdateProgress(
		Mark,
		Length,
		(m && n) ? ((Time - StartTime)*(m - n))/n : 0,
		Count,
		error,
		text);
}

/* ----- Make info string for progress dialog -------------------------- */

static void MakeInfo(void)
{
	register Byte s[256];
	Byte *c;
	Byte *v;

	c = Zctlesc ? (Byte *)"\pEscape-Ctrl" : EmptyStr;
	switch (Vers) {
		case 0:
			v = MyString(STR_P, P_TEXT);
			break;
		case 1:
			v = MyString(STR_P, P_BINARY1);
			break;
		case 2:
			v = MyString(STR_P, P_BINARY2);
			break;
		default:
			v = EmptyStr;
	}
	FormatStr(s, (Byte *)"\pZModem %s rx=%i %s", c, Rxbuflen, v);
	InfoProgress(s);
}

/* ----- Convert between Macintosh time and ZModem time ---------------- */

static DateTimeRec ZTimeOrigin = { 1970, 1, 1, 0, 0, 0, 0 };

static void ConvertTime (Boolean toMac, unsigned long *t)
{
	unsigned long t0;

	Date2Secs(&ZTimeOrigin, &t0);
	*t = toMac ? (*t + t0) : (*t - t0);
}

/* ===== TRANSMIT ====================================================== */

/* ----- Fill buffer --------------------------------------------------- */

static short ReadBuffer (
	register long *count,
	register Byte *buf)
{
	register long x = *count;

	return FSRead(RefNum, count, buf) || *count != x;
}

/* ----- Set file position --------------------------------------------- */

static short Seek (register unsigned long position)
{
	return SetFPos(RefNum, fsFromStart, position);
}

/* ----- Respond to receiver's complaint, get back in sync ------------- */

static short GetInSync (
	Boolean flag,	/* TRUE: ZACK always returns */
	Byte *type)
{
	register short retry;
	register short err;
	HEADER hdr;

	for (retry = 0; retry < Settings.ZRetries; ++retry) {
		Update((Byte *)"\psync", retry);
		switch (GetHeader(type, &hdr)) {
			case FINE:
				switch (*type) {
					case ZRPOS:
						if (Seek(hdr.position)) {
							err = ERROR;
							goto done;
						}
						Lrxpos = Mark = hdr.position;
						if (Lastsync == hdr.position &&
								++BeenHereB4 > 4 && Count > 32)
									Count /= 2;
						Lastsync = hdr.position;
						err = FINE;
						goto done;
					case ZACK:
						Lrxpos = hdr.position;
						if (flag || Mark == hdr.position) {
							err = FINE;
							goto done;
						}
						break;	/* Ignore this ZACK */
					case ZCAN:
					case ZABORT:
					case ZFIN:
						goto error;
					case ZRINIT:
					case ZSKIP:
						err = FINE;
						goto done;
				}
				break;
			case TIMEOUT:
				goto error;
		}
		hdr.position = 0;
		SendHexHeader(ZNAK, hdr);
	}
error:
	err = ERROR;
done:
	Update(EmptyStr, 0);
	return err;
}

/* ----- Send the data in the file ------------------------------------- */

static short SendFileData (void)
{
	short junkcount;		/* Counts garbage chars received */
	unsigned long txwcnt;	/* Counter used to space ACK requests */
	short eofseen;			/* End of file seen (file read error) */
	Byte type;				/* Header type */
	HEADER hdr;				/* Header */
	short err;				/* Error code */
	short newcnt;			/* Controls receiver's buffer */
	long n;					/* Transmit buffer length */
	Byte e;					/* Frame end character */


	Lastsync = (StartPos = Lrxpos = Mark) - 1;
	junkcount = 0;
	BeenHereB4 = 0;

somemore:
	if (FALSE) {

waitack:
		junkcount = 0;
		err = GetInSync(FALSE, &type);
gotack:
		if (err)
			return ERROR;
		switch (type) {
			case ZSKIP:
			case ZRINIT:
				return FINE;
			case ZACK:
			case ZRPOS:
				break;
			default:
				return ERROR;
		}

		/* Check reverse channel (but don't wait) */

		if (CheckCancel())
			longjmp(CancelEnv, CANCEL);
		while (SerialCheck()) {
			Byte c;
			SerialFastRead(&c, 1);
			switch (c) {
				case CAN:
				case ZPAD:
					err = GetInSync(TRUE, &type);
					goto gotack;
				case XOFF:		/* Wait a while for an XON */
				case XOFF | 0x80:
					ReadByte(&c, 100);
			}
		}

	}	/* somemore */

	/* Send ZDATA header */

	hdr.position = Mark;
	SendBinaryHeader(ZDATA, hdr);

	/* Send one or more data subpackets */

	newcnt = Rxbuflen;
	txwcnt = 0;
	do {

		/* Send data subpacket */

		n = Count;
		if (eofseen = ReadBuffer(&n, Buffer))
			e = ZCRCE;
		else if (junkcount > 3)
			e = ZCRCW;
		else if (Mark == Lastsync)
			e = ZCRCW;
		else if (Rxbuflen && (newcnt -= n) <= 0)
			e = ZCRCW;
		else if (Txwindow && (txwcnt += n) >= Txwspac) {
			txwcnt = 0;
			e = ZCRCQ;
		} else
			e = ZCRCG;
		SendData(Buffer, n, e);
		Mark += n;
		Update(EmptyStr, 0);

		/* Wait for ZACK if necessary */

		if (e == ZCRCW)
			goto waitack;

		/* Check reverse channel (but don't wait) */

		if (CheckCancel())
			longjmp(CancelEnv, CANCEL);
		while (SerialCheck()) {
			Byte c;
			SerialFastRead(&c, 1);
			switch (c) {
				case CAN:
				case ZPAD:
					err = GetInSync(TRUE, &type);
					if (!err && type == ZACK)
						break;
					/* zcrce - dinna wanna starta ping-pong game */
					SendData(Buffer, 0, ZCRCE);
					goto gotack;
				case XOFF:		/* Wait a while for an XON */
				case XOFF | 0x80:
					ReadByte(&c, 100);
				default:
					++junkcount;
					break;
			}
		}

		/* Make sure to stay in transmit window */

		if (Txwindow) {
			while ((Mark - Lrxpos) >= Txwindow) {
				if (e != ZCRCQ)
					SendData(Buffer, 0, e = ZCRCQ);
				err = GetInSync(TRUE, &type);
				if (err || type != ZACK) {
					SendData(Buffer, 0, ZCRCE);
					goto gotack;
				}
			}
		}
	} while (!eofseen);

	/* Send ZEOF header, wait for ZRINIT header from receiver */

	for (;;) {
		hdr.position = Mark;
		SendBinaryHeader(ZEOF, hdr);
		if (GetInSync(FALSE, &type))
			return ERROR;
		switch (type) {
			case ZRINIT:
			case ZSKIP:
				return FINE;
			case ZACK:
				break;
			case ZRPOS:
				goto somemore;
			default:
				return ERROR;
		}
	}
}

/* ----- Invite receiver ----------------------------------------------- */

static short Invitation (void)
{
	HEADER hdr;
	register short retry;
	register short err;

	for (retry = 0; retry < Settings.ZRetries; ++retry) {
		Update(EmptyStr, retry);
		hdr.position = 0;
		SendHexHeader(ZRQINIT, hdr);
		if ((err = WaitZPAD(Settings.ZTimeout, 1000)) == FINE) {
			Update(EmptyStr, 0);
			return FINE;
		}
	}
	return TIMEOUT;
}

/* ----- Send invitation to receiver, get receiver's parameters -------- */

static short SessionStartup (void)
{
	register short retry;
	register short err;
	register Byte *message = EmptyStr;
	Byte type;
	HEADER hdr;

	StartTime = Time;
	for (retry = 0; retry < Settings.ZRetries; ++retry) {
		Update(message, retry);
		switch (GetHeader(&type, &hdr)) {
			case FINE:
				switch (type) {
					case ZRINIT:		/* This is what we want */
						message = (Byte *)"\pZRINIT";

						/* Set transfer parameters */

						Rxbuflen = hdr.zrinit.bufsize;
						Txwindow = Settings.ZWindow;
						Txwspac = Settings.Zcrcq;
						Count = Settings.ZPacket;
#ifdef XXX
						if (Txwindow < 256)
							Txwindow = 256;
						Txwindow = (Txwindow/64) * 64;
						Txwspac = Txwindow/4;
						if (Count > Txwspac ||
								(!Count && Txwspac < ZMAXSPLEN))
							Count = Txwspac;
#endif
						if (!hdr.zrinit.canfdx)
							Txwindow = 0;
						if (Rxbuflen && Count > Rxbuflen)
							Count = Rxbuflen;
#ifdef MONITOR
	{
		Byte mon[256];
		if (hdr.zrinit.canvhdr)
			MonitorText((Byte *)"\pCANVHDR");
		if (hdr.zrinit.esc8)
			MonitorText((Byte *)"\pESC8");
		if (hdr.zrinit.escctl)
			MonitorText((Byte *)"\pESCCTL");
		if (hdr.zrinit.canfc32)
			MonitorText((Byte *)"\pCRC32");
		if (hdr.zrinit.canlzw)
			MonitorText((Byte *)"\pLZW");
		if (hdr.zrinit.canrle)
			MonitorText((Byte *)"\pRLE");
		if (hdr.zrinit.canbrk)
			MonitorText((Byte *)"\pBRK");
		if (hdr.zrinit.canovio)
			MonitorText((Byte *)"\pOVIO");
		if (hdr.zrinit.canfdx)
			MonitorText((Byte *)"\pFDX");
		FormatStr(mon,
			(Byte *)"\pRxbuflen=%i Count=%i Txwindow=%l Txwspac=%l",
			Rxbuflen, Count, Txwindow, Txwspac);
		MonitorText(mon);
	}
#endif

						/* No need to send an ZSINIT frame if ... */

						Zctlesc = hdr.zrinit.escctl || Escape(); 
						if (!Escape() || hdr.zrinit.escctl) {
							err = FINE;
							goto done;
						}
						for (retry = 0; retry < Settings.ZRetries; ++retry) {
							/* ZSINIT header */
							hdr.position = 0;
							hdr.zsinit.escctl = 1;
							SendBinaryHeader(ZSINIT, hdr);
							Update(message, retry);
							/* Our attention string (empty string) */
							type = 0;
							SendData(&type, 1, ZCRCW);
							/* Wait for ZACK header from receiver */
							if (!GetHeader(&type, &hdr) && type == ZACK) {
								err = FINE;
								goto done;
							}
						}
						err = TIMEOUT;
						goto done;

					case ZCHALLENGE:	/* Echo challenge number */
						SendHexHeader(ZACK, hdr);
						message = (Byte *)"\pZCHALLENGE";
						break;
					case ZCOMMAND:		/* They didn't see out ZRQINIT */
						hdr.position = 0;
						SendHexHeader(ZRQINIT, hdr);
						message = (Byte *)"\pZRQINIT";
						break;
					case ZRQINIT:
						if (!hdr.zrqinit.command)
							break;	/* Ignore (our echo) */
						/* Fall thru */
					default:
						hdr.position = 0;
						SendHexHeader(ZNAK, hdr);
						message = (Byte *)"\p->ZNAK";
						break;
				}
			case TIMEOUT:
				err = TIMEOUT;
				goto done;
			default:
				hdr.position = 0;
				SendHexHeader(ZNAK, hdr);
				message = (Byte *)"\p->ZNAK";
				break;
			}
	}
	err = TIMEOUT;	/* Retry count exhausted */
done:
	Update(EmptyStr, 0);
	return err;
}

/* ----- Send file information ----------------------------------------- */

static short SendFileInfo (
	Byte *name,					/* File name */
	unsigned long size,			/* File size */
	Boolean text,				/* Text file? */
	unsigned long modif,		/* Modification date */
	unsigned long *startpos)	/* Start position */
{
	register Byte *p;
	register Byte s[30];
	register short retry;
	register short err;
	Byte *message;
	Byte buffer[128];
	Byte type;
	HEADER hdr;

	/* File name as zero terminated ASCII string */

	p = buffer;
	memcpy(p, name + 1, *name);
	p += *name;
	*p++ = 0;

	/* File size as decimal ASCII digits */

	NumToString(size, s);
	memcpy(p, s + 1, *s);
	p += *s;
	*p++ = ' ';

	/* Modification date: octal string = seconds since 1/1/1970 UTC */
	ConvertTime(FALSE, &modif);	/* -> ZModem time */

	{
		register Byte *t = s + sizeof(s);
		register unsigned long n = modif;

		do {
			*(--t) = n % 8 + '0';
			n /= 8;
		} while (n);
		memcpy(p, t, n = sizeof(s) - (t - s));
		p += n;
	}
	*p++ = ' ';

	/* File mode (octal string) */

	*p++ = '0';
	*p++ = ' ';

	/* Serial number (octal string) */

	*p++ = '0';
	*p++ = ' ';

	/* Number of files remaining (decimal number) */

	*p++ = '1';
	*p++ = ' ';

	/* Number of bytes remaining (decimal number) */

	NumToString(size, s);
	memcpy(p, s + 1, *s);
	p += *s;
	*p++ = 0;	/* Final NULL */

	/* Send ZFILE header with ZModem conversion, management and transport
	options followed by a ZCRCW data subpacket containing the file name,
	file length, modification date. */

	message = EmptyStr;
	for (retry = 0; retry < Settings.ZRetries; ++retry) {
		hdr.position = 0;
		hdr.zfile.conv = text ? 0 : ZCBIN;
		hdr.zfile.manage = ZMCLOB;	/* Replace existing file */
		SendBinaryHeader(ZFILE, hdr);
		SendData(buffer, p - buffer, ZCRCW);
again:
		Update(message, retry);
		if (GetHeader(&type, &hdr)) {	/* Error */
			message = EmptyStr;
			continue;
		}
		switch (type) {
			case ZRINIT:
				message = (Byte *)"\pZRINIT";
				if (WaitZPAD(Settings.ZTimeout, 100) == FINE)
					goto again;
				break;
			case ZSKIP:
				/* The receiver may respond with a ZSKIP header, which
				makes the sender proceed to the next file (if any) in the
				batch. */
				*startpos = 0x7FFFFFFF;
				err = FINE;
				goto done;
			case ZRPOS:
				*startpos = hdr.position;
				err = FINE;
				goto done;
		}
	}
	err = TIMEOUT;
done:
	Update(EmptyStr, 0);
	return err;
}

/* ----- Session cleanup ----------------------------------------------- */

static SessionCleanup (void)
{
	register short retry;
	HEADER hdr;
	Byte type;
	static Byte oo[] = "\pOO";	/* Over and out */

	for (retry = 0; retry < Settings.ZRetries; ++retry) {
		hdr.position = 0;
		SendHexHeader(ZFIN, hdr);
		if (GetHeader(&type, &hdr))
			continue;
		if (type == ZFIN) {
			SendString(oo + 1, oo[0]);
#ifdef MONITOR
			MonitorText(oo);
#endif
			return;
		}
	}
}

/* ----- Send a file --------------------------------------------------- */

short ZTransmit (Byte *name, short volume, long directory)
{
	short err;
	OSType creator, type;
	long create, modif;
	Byte tempname[20];

	StartTime = Time;
	Length = Mark = StartPos = 0;
	if (Sending || Transfer)
		return fBsyErr;

	/* Only send non-TEXT files as MacBinary if requested. TEXT files are
	never sent as MacBinary. For MacBinary we convert the file first,
	then use that temporay file to transmit. The temporay file will be
	deleted if we are finsihed. */

	if (err = InfoFile(volume, directory, name,
			&creator, &type, &create, &modif))
		return err;
	if (Vers = (Settings.Binary && type != TEXT) ? 2 : 0) {
		NumToString(Time, tempname);
		if ((err = Convert(volume, directory, name,
				Settings.volume, Settings.directory, tempname)) ||
			(err = InfoFile(Settings.volume, Settings.directory, tempname,
				&creator, &type, &create, &modif)) ||
			(err = OpenFile(Settings.volume, Settings.directory,
				tempname, &RefNum))) {
			DeleteFile(Settings.volume, Settings.directory, tempname);
			return err;
		}
	} else {
		if (err = OpenFile(volume, directory, name, &RefNum))
			return err;
	}

	if (!(Buffer = (Byte *)NewPtr(ZMAXSPLEN))) {
		err = memFullErr;
		goto done1;
	}
	if (err = NewTxBuffer(TRUE))
		goto done1;
	Transfer = Transfer_Tx;
	SetItemStyle(GetMenu(FILE), TRANSMIT, ACTIVE);
	DrawProgressDialog(P_TFILE, name);
	GetEOF(RefNum, (long *)&Length);
	UpdateProgress(0, Length, 0, 0, 0, EmptyStr);
	MakeInfo();

#ifdef MONITOR
	MonitorOpen((Byte *)"\pTRANSMIT.DUMP", Settings.volume,
		Settings.directory);
	MonitorText((Byte *)"\pStart of Z-Modem transmit");
#endif

	Zctlesc = Escape();
	if (err = setjmp(CancelEnv)) {	/* Cancel or abort */
		SerialAbort();
		while (Busy)
			;
		if (err == CANCEL)
			SendCancel();			/* Send cancel string */
		goto done;
	}
	if ((err = Invitation()) || (err = SessionStartup()) ||
		(err = SendFileInfo(name, Length, type == 'TEXT', modif, &Mark)))
		goto done;
	if (Mark != 0x7FFFFFFF) {	/* Not skip */
		if (Seek(Mark))
			goto done;

		/* Send file data */

		if (err = SendFileData())
			goto done;
	}
	SessionCleanup();

done:

#ifdef MONITOR
	MonitorText((Byte *)"\pEnd of Z-Modem transmit");
	MonitorClose();
#endif

	RemoveCancelDialog();
	Statistics(Mark - StartPos, Time - StartTime, err);
	SetItemStyle(GetMenu(FILE), TRANSMIT, 0);
	Transfer = 0;
done1:
	if (RefNum) {
		FSClose(RefNum);
		RefNum = 0;
	}
	DisposeTxBuffer();
	if (Buffer)
		DisposPtr((Ptr)Buffer);
	if (Vers)
			DeleteFile(Settings.volume, Settings.directory, tempname);
	return err;
}

/* ===== RECEIVE ======================================================= */

/* ----- Free bytes on the current file system ------------------------- */

static unsigned long GetFreeSpace (void)
{
	return 0xFFFFFFFF;	/* many free bytes ... */
}

/* ----- Ack a ZFIN packet, let byegones be byegones ------------------- */

static void AckOverAndOut (void)
{
	register short retry;
	register short err;
	HEADER hdr;
	Byte c;

	hdr.position = 0;
	for (retry = 0; retry < 3; ++retry) {
		SendHexHeader(ZFIN, hdr);
		err = ReadByte(&c, Settings.ZTimeout);
#ifdef MONITOR
		{
			Byte mon[256];
			FormatStr(mon, (Byte *)"\pOver c='%c' err=%i", c, err);
			MonitorText(mon);
		}
#endif
		if (err == FINE && c == 'O') {
			err = ReadByte(&c, 60);
#ifdef MONITOR
			{
				Byte mon[256];
				FormatStr(mon, (Byte *)"\pOut c='%c' err=%i", c, err);
				MonitorText(mon);
			}
#endif
			return;	/* Over and Out */
		}
	}
}

/* ----- Initialize for Zmodem receive attempt ------------------------- */

static short TryZReceive (
	HEADER *filehdr)		/* Return sender's options */
{
	register Byte *message;	/* Message to display */
	HEADER hdr;				/* Header */
	Byte type;				/* Header type received */
	short retry;			/* Retry counter */
	short err;				/* Error code */

	for (retry = 0, message = EmptyStr; retry < Settings.ZRetries; ) {
		hdr.position = 0;
		if (TryZhdrType == ZRINIT) {	/* Capability flags */
			hdr.zrinit.bufsize = Settings.ZBuffer;
			hdr.zrinit.canfdx = 1;
			hdr.zrinit.canovio = 1;
			hdr.zrinit.escctl = Escape();
		}
		SendHexHeader(TryZhdrType, hdr);
		if (TryZhdrType == ZSKIP)		/* Don't skip too far */
			TryZhdrType = ZRINIT;
again:
		Update(message, retry);
		++retry;
		if (GetHeader(&type, &hdr) == FINE) {
			switch (type) {
				case ZFILE:
					*filehdr = hdr;		/* Return file options */
					TryZhdrType = ZRINIT;
					if (!ReceiveData(Buffer, ZMAXSPLEN, &Count, &type) &&
							type == ZCRCW) {
						err = FINE;
						goto done;		/* File name received */
					}
					SendHexHeader(ZNAK, hdr);
					message = (Byte *)"\pZFILE error";
					goto again;
				case ZSINIT:
					Zctlesc = hdr.zsinit.escctl;
					if (!ReceiveData(Attn, ZATTNLEN, &AttnLen, &type) &&
							type == ZCRCW) {
						hdr.position = 1;
						SendHexHeader(ZACK, hdr);
						message = (Byte *)"\pZSINIT ok";
						goto again;
					}
					SendHexHeader(ZNAK, hdr);
					message = (Byte *)"\pZSINIT error";
					goto again;
				case ZFREECNT:
					hdr.position = GetFreeSpace();
					SendHexHeader(ZACK, hdr);
					message = (Byte *)"\pZFREECNT";
					goto again;
				case ZCOMPL:
					message = (Byte *)"\pZCOMPL";
					goto again;
				case ZFIN:
					Update((Byte *)"\pZFIN", retry);
					AckOverAndOut();
					Count = 0;			/* Nothing in Buffer */
					err = FINE;
					goto done;
			}
		} else
			message = EmptyStr;
	}
	err = TIMEOUT;
	Count = 0;							/* Nothing in Buffer */
done:
	Update(EmptyStr, 0);
	return err;
}

/* ----- Process incoming file information header ---------------------- */

static OSErr ProcedeHeader (
	HEADER filehdr,			/* Sender's options */
	Byte *name,				/* File name extracted (pascal string) */
	unsigned long *length,	/* File length extracted */
	unsigned long *modif)	/* Modification time extracted */
{
	register short err;
	register Byte *p, *max;
	register Byte c;
	Boolean exists;			/* File exists */
	Boolean append;			/* Append to existing file */
	OSType creator, type;
	long create, modif2;

	*name = 0;
	*length = 0;
	*modif = 0;
	RefNum = 0;

	/* Extract fields from file information header */

	max = (p = Buffer) + Count;
	err = 0;
	while (p < max) {
		c = *p++;
		switch (err) {
			case 0:	/* File name (C-string) */
				if (c == '\0') {
					memcpy(name + 1, Buffer, *name = p - 1 - Buffer);
					++err;	/* Goto next state */
				}
				break;
			case 1:	/* File length (decimal string, space) */
				if (c == ' ' || c == '\0')
					++err;	/* Goto next state */
				else
					*length = *length * 10 + (c & 0x0F);
				break;
			case 2:	/* File modification time (octal string, space) */
				if (c == ' ' || c == '\0') {
					ConvertTime(TRUE, modif);	/* -> Macintosh time */
					++err;	/* Goto next state */
				} else
					*modif = *modif * 8 + (c & 0x0F);
				break;
			default:
				break;
		}
	}
	if (err < 3)			/* Got no modification time...	*/
		*modif = Time;		/* ...so use actual time		*/

	/* See if file exists */

	exists = (InfoFile(Settings.volume, Settings.directory, name,
		&creator, &type, &create, &modif2) == noErr);
	append = exists &&
		creator == Application.signature &&
		type == Application.ztype;

	/* Skip file if not present at rx */

	if (filehdr.zfile.skip && !exists)
		return 100;

	/* Delete existing file */

	if (exists && filehdr.zfile.manage == ZMCLOB) {
		DeleteFile(Settings.volume, Settings.directory, name);
		exists = append = FALSE;
	}

	/* Create new file */

	if (!exists) {
		if (err = CreateFile(Settings.volume, Settings.directory,
				name, Application.signature, Application.ztype))
			return err;
		exists = TRUE;
		append = FALSE;
	}

	/* Open file (append if necessary) */

	if (err = OpenFile(Settings.volume, Settings.directory, name, &RefNum))
		return err;
	if (append  && (err = SetFPos(RefNum, fsFromLEOF, 0))) {
		FSClose(RefNum);
		RefNum = 0;
		return err;
	}

	return FINE;
}

/* ----- Write buffer to file ------------------------------------------ */

static OSErr WriteFile (void)
{
	long count = Count;
	long df, rf;
	Byte name2[64];

	/* Check for MacBinary header. If valid MacBinary header is complete,
	use file name and file size from this header instead ZModem header. */

	if (Settings.Binary) {
		if (Mark < BinHeaderLength)
			memcpy(BinHdr, Buffer,
				(Count < (BinHeaderLength - Mark)) ?
				Count : BinHeaderLength - Mark);
		if (Mark >= BinHeaderLength && Vers < 0) {
			Vers = BinCheckHeader(BinHdr, name2, &df, &rf);
			MakeInfo();
			if (Vers > 0) {
				NameProgress(name2);
				Length = BinHeaderLength +
					df + Filler(BinHeaderLength, df) +
					rf + Filler(BinHeaderLength, rf);
			}
		}
	}

	/* Write buffer to file */

	return FSWrite(RefNum, &count, Buffer);
}

/* ----- File was completly received ----------------------------------- */

static void CleanupFile (
	Byte *name,				/* File name from ZModem header */
	unsigned long modif)	/* File modification time */
{
	short ref;
	Byte name2[64];			/* File name from MacBinary header */
	Byte name3[64];			/* Temporary file name */
	Byte header[BinHeaderLength];
	long count, data, resource;
	short err, err2;

	/* Set file creator/type and creation/modification time */

	if (SetInfoFile(Settings.volume, Settings.directory, name,
			Settings.binCreator, Settings.binType, modif, modif))
		return;

	/* Read file header to see if it is MacBinary */

	if (!Settings.Binary ||
			OpenFile(Settings.volume, Settings.directory, name, &ref))
		return;
	count = sizeof(header);
	if (FSRead(ref, &count, header) ||
			count != sizeof(header) ||
			!BinCheckHeader(header, name2, &data, &resource)) {
		FSClose(ref);
		return;
	}

	/* Create a new file with an unique name to become the final file */

	NumToString(Time, name3);
	if (CreateFile(Settings.volume, Settings.directory, name3,
			'????', '????')) {
		FSClose(ref);
		return;
	}
	if (BinOpenWrite(Settings.volume, Settings.directory, name3, header)) {
		DeleteFile(Settings.volume, Settings.directory, name3);
		FSClose(ref);
		return;
	}

	/* Extract from MacBinary file to new file */

	err = CopyFile(ref);
	FSClose(ref);
	err2 = BinCloseWrite();
	if (err || err2) {
		DeleteFile(Settings.volume, Settings.directory, name3);
		return;
	}

	/* Delete original file, rename extracted file to name from MacBinary
	header (if this does not work use name from ZModem header). */

	DeleteFile(Settings.volume, Settings.directory, name);
	if (FileRename(Settings.volume, Settings.directory, name3, name2))
		FileRename(Settings.volume, Settings.directory, name3, name);
}

/* ----- Receive a file ------------------------------------------------ */

static short ReceiveFile (
	HEADER filehdr)			/* Sender's options */
{
	register short err;		/* Error code */
	register short retry;	/* Retry counter */
	register Byte *message;	/* Message to display */
	Byte name[256];			/* File name from Z header (Pascal string) */
	Byte name2[64];			/* File name from MacBin hdr (Pascal string) */
	unsigned long modif;	/* File modification date (Mac time) */
	HEADER hdr;				/* Header */
	Byte type;				/* Header type received */
	Byte end;				/* Data frame type */

	/* Get ready to receive new file */

	Vers = -1;
	StartPos = Mark = Length = 0;
	if (ProcedeHeader(filehdr, name, &Length, &modif)) {
		/* Error or skip, but that's fine! */
		TryZhdrType = ZSKIP;
		return FINE;
	}
	GetFPos(RefNum, (long *)&StartPos);
	Mark = StartPos;

	/* If resuming previous receive fill MacBinary header. If we already
	have a valid MacBinary header use the name and the file size from this
	header instead of from the ZModem header. */

	if (Settings.Binary && Mark > 0) {
		long count = (Mark > BinHeaderLength) ? BinHeaderLength : Mark;
		long df, rf;

		SetFPos(RefNum, fsFromStart, 0);
		FSRead(RefNum, &count, BinHdr);
		SetFPos(RefNum, fsFromStart, Mark);
		if (count >= BinHeaderLength)
			if (Vers = BinCheckHeader(BinHdr, name2, &df, &rf))
				Length = BinHeaderLength +
					df + Filler(BinHeaderLength, df) +
					rf + Filler(BinHeaderLength, rf);
	}
	NameProgress(Vers > 0 ? name2 : name);
	Rxbuflen = Settings.ZBuffer;
	MakeInfo();

	/* Receiving loop */

	for (retry = 0, message = EmptyStr; ; ) {
		hdr.position = Mark;
		SendHexHeader(ZRPOS, hdr);
nxthdr:
		Update(message, retry);
		switch (GetHeader(&type, &hdr)) {
			case FINE:
				switch (type) {
					case ZNAK:
						++retry;
						if (retry > Settings.ZRetries) {
							err = ERROR;
							goto done;
						}
						message = (Byte *)"\phdr-ZNAK";
						break;
					case ZFILE:
						err = ReceiveData(Buffer, ZMAXSPLEN, &Count, &end);
						message = (Byte *)"\phdr-ZFILE";
						break;
					case ZEOF:
						/* Ignore eof if it's at wrong place - force a
						timeout because the eof might have gone out before
						we sent our zrpos. */
						message = (Byte *)"\phdr-ZEOF";
						if (hdr.position != Mark)
							goto nxthdr;			/* Ignore this eof */
						FSClose(RefNum);
						RefNum = 0;
						Update(message, retry);
						CleanupFile(name, modif);
						err = FINE;					/* Normal eof */
						goto done;
					case ZSKIP:
						message = (Byte *)"\phdr-ZSKIP";
						FSClose(RefNum);
						RefNum = 0;
						err = FINE;					/* Sender skipped it */
						goto done;
					case ZDATA:
						message = (Byte *)"\phdr-ZDATA";
						if (hdr.position != Mark) {
							++retry;
							if (retry > Settings.ZRetries) {
								err = ERROR;
								goto done;
							}
							SendAttn(Attn);
							break;
						}
moredata:
						Update(message, retry);
						switch (ReceiveData(Buffer, ZMAXSPLEN, &Count, &end)) {
							case TIMEOUT:
								++retry;
								if (retry > Settings.ZRetries) {
									err = ERROR;
									goto done;
								}
								message = (Byte *)"\pdata-TIMEOUT";
								break;
							case ERROR:
								++retry;
								if (retry > Settings.ZRetries) {
									err = ERROR;
									goto done;
								}
								SendAttn(Attn);
								message = (Byte *)"\pdata-ERROR";
								break;
							case FINE:
								retry = 0;
								WriteFile();
								Mark += Count;
								switch (end) {
									case ZCRCW:
										message = (Byte *)"\pdata-ZCRCW";
										hdr.position = Mark;
										SendHexHeader(ZACK, hdr);
										goto nxthdr;
									case ZCRCQ:
										message = (Byte *)"\pdata-ZCRCQ";
										hdr.position = Mark;
										SendHexHeader(ZACK, hdr);
										goto moredata;
									case ZCRCG:
										message = (Byte *)"\pdata-ZCRCG";
										goto moredata;
									case ZCRCE:
										message = (Byte *)"\pdata-ZCRCE";
										goto nxthdr;
								}
								break;
						}
						break;
					default:
						err = ERROR;
						goto done;
				}
				break;
			case TIMEOUT:
				++retry;
				if (retry > Settings.ZRetries) {
					err = ERROR;
					goto done;
				}
				message = (Byte *)"\phdr-TIMEOUT";
				break;
			case ERROR:
				++retry;
				if (retry > Settings.ZRetries) {
					err = ERROR;
					goto done;
				}
				SendAttn(Attn);
				message = (Byte *)"\phdr-ERROR";
				break;
		}
	}

done:
	Update(EmptyStr, retry);
	return err;
}

/* ----- Receive one or more files ------------------------------------- */

static short ReceiveFiles (void)
{
	register short err;
	HEADER filehdr;			/* Sender's options */

	Mark = StartPos = 0;
	StartTime = Time;
	Zctlesc = Escape();
	TryZhdrType = ZRINIT;
	Attn[0] = 0;			/* Clear sender's attention string */
	AttnLen = 0;

	for (;;) {
		NameProgress(EmptyStr);
		InfoProgress(EmptyStr);
		UpdateProgress(0, 0, 0, 0, 0, EmptyStr);
		StartTime = Time;
		err = TryZReceive(&filehdr);
		if (!Count) {
			if (err)
				Statistics(0, Time - StartTime, err);
			return err;
		}
		ConvertNL = filehdr.zfile.conv == ZCNL;
		err = ReceiveFile(filehdr);
		Statistics(Mark - StartPos, Time - StartTime, err);
		if (err)
			return err;
	}
}

/* ----- ZModem receive ------------------------------------------------ */

short ZReceive (void)
{
	register short err = 0;

	if (Sending || Transfer)
		return fBsyErr;
	StartTime = Time;
	RefNum = 0;						/* No file open yet */
	Mark = Length = StartPos = 0;
	if (!(Buffer = (Byte *)NewPtr(BUFFERSIZE))) {
		err = memFullErr;
		goto done;					/* Not enough memory */
	}
	if (err = NewTxBuffer(FALSE))	/* We only will send headers */
		goto done;					/* Not enough memory */
	Transfer = Transfer_Rx;
	SetItemStyle(GetMenu(FILE), RECEIVE, ACTIVE);
	if (Settings.handshake == 1)	/* XON/OFF must be off */
		SerialHandshake(0);
	DrawProgressDialog(P_RFILE, EmptyStr);
	UpdateProgress(0, 0, 0, 0, 0, EmptyStr);

#ifdef MONITOR
	MonitorOpen((Byte *)"\pRECEIVE.DUMP", Settings.volume,
		Settings.directory);
	MonitorText((Byte *)"\pStart of Z-Modem receive");
#endif

	if (err = setjmp(CancelEnv)) {	/* Prepare cancel or abort */
		Statistics(Mark - StartPos, Time - StartTime, err);
		SerialAbort();
		while (Busy)
			;
		if (err == CANCEL)
			SendCancel();			/* Send cancel string */
	} else
		err = ReceiveFiles();		/* Receive file(s) */

#ifdef MONITOR
	MonitorText((Byte *)"\pEnd of Z-Modem receive");
	MonitorClose();
#endif

	RemoveCancelDialog();
	SerialHandshake(Settings.handshake);
	SetItemStyle(GetMenu(FILE), RECEIVE, 0);
	Transfer = 0;

done:
	if (RefNum) {					/* Close any open file */
		FSClose(RefNum);
		RefNum = 0;
	}
	DisposeTxBuffer();				/* Get rid of transmit buffer */
	if (Buffer)						/* Get rid of receive buffer */
		DisposPtr((Ptr)Buffer);
	return err;
}
