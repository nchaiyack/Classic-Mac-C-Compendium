/*
	Terminal 2.2
	"XModem.c"
*/

#ifdef THINK_C
#include "MacHeaders"
#endif
#ifdef applec
#pragma load ":(Objects):MacHeadersMPW"
#pragma segment XModem
#endif

#include "XModem.h"
#include "CRC.h"
#include "Strings.h"
#include "Utilities.h"
#include "CancelDialog.h"
#include "Port.h"
#include "Text.h"
#include "Main.h"
#include "MacBinary.h"
#include "Document.h"

/* #define MONITOR */

#ifdef MONITOR
#include "Monitor.h"
#include "FormatStr.h"
#endif

#define SOH			0x01
#define STX			0x02
#define EOT			0x04
#define ACK			0x06
#define NAK			0x15
#define CAN			0x18

#define LONG		Settings.XModemtimeout	/* Timeout between blocks */
#define SHORT		LONG/4					/* Timeout between bytes */
#define WAITMAX		LONG*10					/* While ignoring ... */
#define RETRY		10						/* Normal retries */
#define RETRY_CRC	RETRY/2					/* Negociating retries */

#define TIME		((eof && mark) ? ((Time-time)*(eof-mark))/mark : 0)

#define XModemStr		MyString(STR_P, P_XMODEM)
#define YModemStr		MyString(STR_P, P_YMODEM)
#define CRCStr			MyString(STR_P, P_CRC)
#define CKSMStr			MyString(STR_P, P_CHECKSUM)
#define Blk1KStr		MyString(STR_P, P_1K)
#define TextStr			MyString(STR_P, P_TEXT)
#define Bin1Str			MyString(STR_P, P_BINARY1)
#define Bin2Str			MyString(STR_P, P_BINARY2)
#define TimeoutStr		MyString(STR_P, P_TIMEOUT)
#define InvalidStr		MyString(STR_P, P_INVALID)
#define RetryStr		MyString(STR_P, P_REPEAT)
#define Block1Str		MyString(STR_P, P_BLOCKERR1)
#define Block2Str		MyString(STR_P, P_BLOCKERR2)
#define ChecksumStr		MyString(STR_P, P_CRCERR)
#define VerifyStr		MyString(STR_P, P_VERIFY)
#define EndStr			MyString(STR_P, P_END)
#define ReverifyStr		MyString(STR_P, P_REVERIFY)
#define NegociateStr	MyString(STR_P, P_NEGOCIATE)

typedef union {
	unsigned short w;
	Byte b[2];
} CRC;

typedef struct {
	short n;			/* Data bytes */
	Byte b[3+1024+2];	/* Block buffer */
} BLOCK;

/* ----- Extract file info from batch header block --------------------- */

static void ExtractFileInfo(
	register Byte *buffer,
	Byte *name,
	long *size)
{
	register Byte *p;
	register Byte *max = buffer + 128;
	register Byte c;
	Byte number[128];

	*name = 0;
	p = name + 1;
	while (buffer < max) {
		c = *buffer++;
		if (c) {
			*p++ = c;
			++(*name);
		} else
			break;
	}
	*number = 0;
	p = number + 1;
	while (buffer < max) {
		c = *buffer++;
		if (c && c != ' ') {
			*p++ = c;
			++(*number);
		} else
			break;
	}
	StringToNum(number, size);
}

/* ----- Set up batch header block with file info ---------------------- */

static void MakeFileInfo(
	register Byte *buffer,
	register Byte *name,
	register long size)
{
	register Byte *max = buffer + 128;
	register short n;
	register Byte s[20];

	if (size) {
		if (n = *name) {
			memcpy(buffer, name + 1, n);
			buffer += n;
		}
		*buffer++ = 0;
		NumToString(size, s);
		memcpy(buffer, s + 1, n = *s);
		buffer += n;
	}
	while(buffer < max)
		*buffer++ = 0;
}

/* ----- Make info string for progress dialog -------------------------- */

static void MakeInfo(
	register Boolean crc,		/* CRC option */
	register Boolean k,			/* 1K option */
	register short vers)		/* MacBinary version */
{
	register Byte s[256];

	if (Settings.batch)
		memcpy(s, YModemStr, YModemStr[0] + 1);
	else
		memcpy(s, XModemStr, XModemStr[0] + 1);
	if (crc)
		Append(s, CRCStr);
	else
		Append(s, CKSMStr);
	if (k)
		Append(s, Blk1KStr);
	switch(vers) {
		case 0:
			Append(s, TextStr);
			break;
		case 1:
			Append(s, Bin1Str);
			break;
		case 2:
			Append(s, Bin2Str);
			break;
	}
	InfoProgress(s);
}

/* ----- Calculate checksum -------------------------------------------- */

static Byte Checksum(
	register Byte *buffer,
	register short length)
{
	register Byte cksm = 0;

	while(length--)
		cksm += *buffer++;
	return cksm;
}

/* ----- Receive character with timeout -------------------------------- */

static short Receive(
	register Byte *character,
	register long ticks)
{
	register long start;

	start = Ticks;
	while ((Ticks - start) < ticks) {
		if (CheckCancel())
			return CANCEL;
		if (SerialRead(character, 1))
			return FINE;
	}
	return TIMEOUT;
}

/* ----- Ignore characters until timeout ------------------------------- */

static short Ignore(register long timeout)
{
	register short err;
	register long start;
	Byte c;

	start = Ticks;
	while ((err = Receive(&c, timeout)) == FINE)
		/* Be sure not to ignore incoming characters forever! */
		if ((Ticks - start) > WAITMAX)
			return CANCEL;
	return err;	/* CANCEL or TIMEOUT */
}

/* ----- Receive characters with timeout ------------------------------- */

static short ReceiveBuffer(
	register Byte *buffer,
	register long count,
	register long ticks)
{
	register long start;
	register short err;

	ticks += SerialDuration(Settings.portSetup, count);
	start = Ticks;
	while ((Ticks - start) < ticks) {
		if (CheckCancel()) {
			err = CANCEL;
			goto done;
		}
		if (SerialCheck() >= count) {
			SerialFastRead(buffer, count);
			err = FINE;
			goto done;
		}
	}
	err = TIMEOUT;
done:
	return err;
}

/* ----- XModem receive ------------------------------------------------ */

enum {
	STATE_NAK,		/* NAKed a data block */
	STATE_ACK,		/* ACKed data block */
	STATE_ACK2,		/* re-ACKed a data block */
	STATE_EOT,		/* ACKed EOT */
	STATE_BLK0,		/* ACKed header block (batch) */
	STATE_END		/* ACKed header block, file name length is 0 (batch) */
};

short XReceive(
	Byte *fname,					/* File name */
	short volume,					/* Volume reference number */
	long directory)					/* Directory id */
{
	Byte buffer[3+1024+2];			/* Block buffer */
	Byte *message;					/* Message for progress dialog */
	short limit;					/* Maximum size of buffer */
	short dataLength;				/* 128 or 1024 */
	short err;						/* Error code */
	Byte expected;					/* Current block number */
	short retry;					/* Retry counter */
	long mark;						/* Current file mark */
	long blk;						/* Block number expected */
	long time;						/* Starting time */
	long count;						/* Current length of block buffer */
	short offset;					/* Data offset in buffer */
	Byte c;							/* Character to transmit */
	Byte ck[3];						/* Characters to transmit */
	CRC crc;						/* Checksum or CRC */
	short ref;						/* File reference number */
	Byte name1[128];				/* Original name */
	Byte name2[128];				/* Name from MacBinary header */
	long eof;						/* File length */
	long size;						/* File size from batch header */
	short vers;						/* MacBinary version */
	Byte *usedName;					/* File name used */
	Boolean useCRC;					/* CRC option */
	short use1K;					/* 1K option */
	short canceled = 0;				/* Used if CAN received */
	Boolean open;					/* File to write into is open */
	Boolean open2;					/* File to write into is open */
	short state;					/* Current state */

#ifdef MONITOR
	Byte mon[255];
#endif

	if (Sending || Transfer)
		return fBsyErr;
	Transfer = Transfer_Rx;
	SetItemStyle(GetMenu(FILE), RECEIVE, ACTIVE);
	SerialBinary(Settings.portSetup);
	if (Settings.handshake == 1)	/* XON/OFF must be off */
		SerialHandshake(0);

	*name1 = 0;
	if (Settings.batch) {
		volume = Settings.volume;
		directory = Settings.directory;
	} else
		if ((Byte)*fname > 0)
			memcpy(name1, fname, *fname + 1);
	DrawProgressDialog(P_RFILE, name1);

#ifdef MONITOR
	MonitorOpen("\pRECEIVE.DUMP", Settings.volume, Settings.directory);
	MonitorText("\pStart of X-Modem receive");
#endif

	/* Goto here if during batch file transfer a file has been successfully
	received and saved. */

newfile:
	message = EmptyStr;
	blk = expected = Settings.batch ? 0 : 1;
	mark = eof = size = 0;
	vers = ref = 0;
	open = open2 = FALSE;
	UpdateProgress(0,0,0,0,0,EmptyStr);
	InfoProgress(EmptyStr);
	state = STATE_BLK0;
	time = Time;

	/* Goto here if during batch file transfer the header block was
	received and ACKed, and file name extracted. */

negociate:
	useCRC = Settings.XModemCRC;
	use1K = Settings.XModem1K;
	if (useCRC) {
		retry = RETRY_CRC + 1;
		c = 0;					/* Use string in ck[] */
		ck[1] = 'C';
		if (use1K == 2) {		/* "CK" CRC and 1K option */
			ck[2] = 'K';
			ck[0] = 2;
		} else					/* "C" CRC option, automatic 1K */
			ck[0] = 1;
	} else {
		ck[0] = 0;
		retry = RETRY;
		c = NAK;				/* Use byte in c */
	}

	/* Goto to here if a NAK (or "C", "CK") must be sent. */
	
repeat:
	/* Make sure line is cleared if NAKing (if not previous CAN) */
	if (!canceled && state == STATE_NAK &&
			((err = Ignore(SHORT)) == CANCEL))
		goto finished;

repeat1:
	state = STATE_NAK;
	if (retry) {
		retry--;
		if (c)
			c = NAK;
	} else {					/* Retry count exhausted */
		if (c) {
			err = TIMEOUT;		/* This is a real error */
			goto finished;
		}
		useCRC = use1K = FALSE;	/* Check for CRC/1K failed */
		retry = RETRY;
		c = NAK;
	}

	/* Goto here if an ACK must be sent. */
	
transmit:
	if (c)
		SerialSend(&c, 1, &Busy);			/* NAK or ACK */
	else
		SerialSend(ck + 1, ck[0], &Busy);	/* "C" or "CK" */

#ifdef MONITOR
	if (c)
		FormatStr(mon, "\pTransmit %i", c);
	else
		FormatStr(mon, "\pTransmit '%s'", ck);
	MonitorText(mon);
#endif

	/* By putting UpdateProgress() and disk writes here, the Mac is
	already receiving the next block while we do these time consuming
	operations: this makes the file transfer much much faster! */

	UpdateProgress(mark,eof,TIME,blk,(c?RETRY:RETRY_CRC)-retry,message);
	while (Busy)		/* Wait until SerialSend() finished */
		;

	/* Dispatcher: see what must be done after our transmission */

	switch (state) {
		case STATE_EOT:		/* ACKed EOT */
			err = FINE;
			goto finished;
		case STATE_END:		/* ACKed final header block (batch) */
			err = FINE;
			goto done;
		case STATE_BLK0:	/* ACKed header block (batch) */
			expected++;	/* Modulo 256 */
			blk++;
			if (Settings.batch == 1)	/* "Official" Y-Modem batch */
				goto negociate;
			else						/* Red Ryder interpretation */
				break;
		case STATE_NAK:		/* NAKed received block */
		case STATE_ACK2:	/* Re-ACKed data block */
			break;
		case STATE_ACK:		/* ACKed data block */
			if (count > 0 && (err = vers ?
					BinWrite(&count, buffer + offset) :
					FSWrite(ref, &count, buffer + offset)))
				goto finished;	/* Write errror */
			expected++;	/* Modulo 256 */
			blk++;
#ifdef MONITOR
			FormatStr(mon, "\pExpecting block #%i", expected);
			MonitorText(mon);
#endif
			break;
	}

	/* Wait for 1st byte of next block */

	err = Receive(buffer, LONG);

#ifdef MONITOR
	if (err)
		MonitorText("\pFirst byte timeout");
	else {
		FormatStr(mon, "\pFirst byte %i", *buffer);
		MonitorText(mon);
	}
#endif

	if (err) {
		if (err == CANCEL)
			goto finished;
		message = TimeoutStr;
		goto repeat1;	/* Transmit NAK (or "C", "CK") */
	}

	/* We just got one byte. Only EOT, SOH/STX or CAN are accepted. */

	if (buffer[0] == CAN)
		if (canceled) {
			err = ABORT;	/* 2 consecutive CANs ==> abort */
			goto finished;
		} else
			++canceled;
	else
		canceled = 0;

	if (buffer[0] == EOT) {
		state = STATE_EOT;
		retry = RETRY;
		c = ACK;
		goto transmit;
	}

	/* SOH ==> 128 bytes, STX ==> 1024 bytes per block */

	if (buffer[0] != SOH && !(use1K && buffer[0] == STX)) {
		message = InvalidStr;
		goto repeat;	/* Transmit NAK (or "C", "CK") */
	}
	limit = (dataLength=(buffer[0]==SOH)?128:1024) + (useCRC?5:4);

	/* Just received SOH/STX, now get all the rest. */

	err = ReceiveBuffer(buffer + 1, limit - 1, LONG);

#ifdef MONITOR
	if (err)
		MonitorText("\pBlock timeout");
	else {
		MonitorText("\pBlock received");
		MonitorDump(buffer, limit);
	}
#endif

	if (err) {
		if (err == CANCEL)
			goto finished;
		message = TimeoutStr;
		goto repeat1;	/* Transmit NAK (or "C", "CK") */
	}

	/* Verify the block complement and checksum received. */

	if (buffer[1] + buffer[2] != 0xFF) {
		message = Block1Str;
		goto repeat;	/* Transmit NAK (or "C", "CK") */
	}
	if (useCRC) {					/* 2 byte CRC */
		crc.b[0] = buffer[limit - 2];
		crc.b[1] = buffer[limit - 1];
		err = (crc.w != CalcCRC(buffer + 3, dataLength, 0));
	} else							/* 1 byte checksum */
		err = (buffer[limit - 1] != Checksum(buffer + 3, dataLength));
	if (err) {
		message = ChecksumStr;
		goto repeat;	/* Transmit NAK (or "C", "CK") */
	}

	/* Are we expecting this block ? */
	
	if (buffer[1] == expected) {	/* Expected block number */
		if (!c)		/* Initial handshake is finished */
			MakeInfo(useCRC, use1K, 0);
		message = VerifyStr;
		count = dataLength;
		offset = 3;
		if (blk == 0) {				/* Batch header block */
			/* Extract the file name from the Y-Modem header block. If a
			file with that name exists it is deleted. An empty name signals
			the end of the batch transfer. */
			ExtractFileInfo(buffer + 3, name1, &size);
			eof = size + Filler(128, size);
			NameProgress(name1);
			if (*name1 == 0)
				state = STATE_END;
			else {
				state = STATE_BLK0;
				DeleteFile(volume, directory, name1);
			}
		} else {					/* Not batch header block */
			state = STATE_ACK;
			if (blk == 1) {
				/* Try to use the file name from the MacBinary header. If
				this file exists already use the first name. */
				if (Settings.Binary) {		/* MacBinary check */
					long df, rf;
					vers = BinCheckHeader(buffer + 3, name2, &df, &rf);
				}
				if (vers) {
					err = CreateFile(volume, directory,
						usedName = name2, 
						Settings.binCreator, Settings.binType);
					if (err == dupFNErr)
						err = CreateFile(volume, directory,
							usedName = name1, 
							Settings.binCreator, Settings.binType);
				} else
					err = CreateFile(volume, directory,
						usedName = name1, 
						Settings.binCreator, Settings.binType);
				if (err)
					goto finished;		/* Create error */
				mark += count;
				if (vers) {				/* Valid MacBinary header */
					NameProgress(usedName);
					MakeInfo(useCRC, use1K, vers);
					if (err = BinOpenWrite(volume, directory, usedName,
							buffer + 3))
						goto finished;	/* Open error */
					open2 = TRUE;
					BinGetEOF(&eof);
					/* Skip MacBinary header, but write the rest. (actual
					write only after ACK has been sent) */
					count -= BinHeaderLength;
					offset += BinHeaderLength;
				} else {				/* Not MacBinary */
					if (err = OpenFile(volume,directory,usedName,&ref))
						goto finished;	/* Open error */
					open2 = TRUE;
					/* The first block is no MacBinary header, so it must
					be written as data block (actual write only after ACK
					has been sent) */
				}
			} else	/* Not the first data block */
				open = TRUE;
			if (open) {			/* Not the first data block */
				mark += count;
				/* (actual write only after ACK has been sent) */
			}
			/* If file size is known, make correction */
			if (size && mark > size) {
				count -= mark - size;
			}
		} /* (Not batch header block) */
	} else {						/* Not the expected block number */
		Byte previous = expected;
		--previous;	/* Modulo 256 */
		if (buffer[1] != previous) {
			message = Block2Str;	/* Lost synchronization */
			goto repeat;			/* Transmit NAK (or "C", "CK") */
		}
		message = ReverifyStr;
		state = STATE_ACK2;
	}
	retry = RETRY;
	c = ACK;
	goto transmit;	/* Transmit ACK */

	/* Goto here if transfer of single file is finished, because EOT has
	been ACKed (err == 0) or because there was an error (err != 0). */

finished:
	time = Time - time;
	if (open2) {		/* If file is open, then close it */
		if (vers)
			BinCloseWrite();
		else
			FSClose(ref);
		if (err)	/* If error delete uncomplete file */
			DeleteFile(volume, directory, usedName);
	}
	Statistics(mark, time, err);
	FlushVol(0, volume);
	if (!err && Settings.batch) {	/* Try again if batch file transfer */
		NameProgress(EmptyStr);
		goto newfile;
	}

	/* Goto here if batch header block has zero length file name, meaning
	that the batch file transfer is over. */

done:

#ifdef MONITOR
	MonitorText("\pEnd of X-Modem receive");
	MonitorClose();
#endif

	RemoveCancelDialog();
	SerialReset(Settings.portSetup);
	SerialHandshake(Settings.handshake);
	SetItemStyle(GetMenu(FILE), RECEIVE, 0);
	Transfer = 0;
	return err;
}

/* ----- Create data block --------------------------------------------- */

#define LEVEL 896		/* 7 128-Byte blocks */

static short ReadFile(
	register BLOCK *filebuf,
	short ref,
	Boolean useCRC,
	short use1K,
	short vers,
	register Byte *block,
	long *size,
	long *mark)
{
	long count;
	register short err;
	CRC crc;

	if (*size) {					/* Still some data left? */
		if (use1K && *size > LEVEL) {
			filebuf->n = 1024;
			filebuf->b[0] = STX;
		} else {
			filebuf->n = 128;
			filebuf->b[0] = SOH;
		}
		filebuf->b[1] = *block;
		filebuf->b[2] = 0xFF - *block;
		++(*block);
		count = filebuf->n;			/* Next block from file */
		if (vers)
			err = BinRead(&count, filebuf->b + 3);
		else
			err = FSRead(ref, &count, filebuf->b + 3);
		if (err && err != eofErr)
			return err;
		*mark += filebuf->n;
		*size -= count;
		while(count < filebuf->n) {	/* Last (partial) block */
			filebuf->b[count + 3] = 0;
			count++;
		}
		if (useCRC) {
			crc.w = CalcCRC(filebuf->b + 3, filebuf->n, 0);
			filebuf->b[filebuf->n + 3] = crc.b[0];
			filebuf->b[filebuf->n + 4] = crc.b[1];
			filebuf->n += 3 + 2;
		} else {
			filebuf->b[filebuf->n + 3] =
				Checksum(filebuf->b + 3, filebuf->n);
			filebuf->n += 3 + 1;
		}
	} else {
		filebuf->b[0] = EOT;
		filebuf->n = 1;
	}
	return noErr;
}

/* ----- Create batch header block ------------------------------------- */

static void HeaderBlock(
	register BLOCK* sendbuf,
	Byte *name,
	long eof,
	Boolean useCRC)
{
	CRC crc;

	MakeFileInfo(sendbuf->b + 3, name, eof);
	sendbuf->n = 128;
	sendbuf->b[0] = SOH;
	sendbuf->b[1] = 0x00;
	sendbuf->b[2] = 0xFF;
	if (useCRC) {
		crc.w = CalcCRC(sendbuf->b + 3, sendbuf->n, 0);
		sendbuf->b[sendbuf->n + 3] = crc.b[0];
		sendbuf->b[sendbuf->n + 4] = crc.b[1];
		sendbuf->n += 3 + 2;
	} else {
		sendbuf->b[sendbuf->n + 3] =
			Checksum(sendbuf->b + 3, sendbuf->n);
		sendbuf->n += 3 + 1;
	}
}

/* ----- Negociate with remote receiver -------------------------------- */

enum {
	TX_NEGOCIATE1,		/* Waiting for "C" */
	TX_NEGOCIATE2		/* Waiting for "K" */
};

static short Negociate(
	register Boolean *useCRC,
	register short *use1K)
{
	register short err;
	register short error;
	register short timeout;
	register short state;
	Byte c;
	short canceled = 0;

	InfoProgress(NegociateStr);
	error = 0;
	state = TX_NEGOCIATE1;
	timeout = LONG;
	while (TRUE) {
		if (error >= RETRY)
			return TIMEOUT;
		if ((err = Receive(&c, timeout)) == CANCEL)
			return CANCEL;
		if (!err && c == CAN)
			if (canceled)				/* 2 consecutive CANs ==> abort */
				return ABORT;
			else {
				++canceled;
				continue;
			}
		else
			canceled = 0;

#ifdef MONITOR
		if (err)
			MonitorText("\pNegociating timeout");
		else {
			Byte mon[256];
			FormatStr(mon, "\pNegociating received %i", c);
			MonitorText(mon);
		}
#endif

		switch (state) {
			case TX_NEGOCIATE1:
				if (err) {			/* Timeout */
					timeout = LONG;
					++error;
				}
				switch (c) {
					case 'C':		/* CRC option */
						if (Settings.XModemCRC) {
							*useCRC = TRUE;
							if ((*use1K = Settings.XModem1K) == 2) {
								timeout = SHORT;
								state = TX_NEGOCIATE2;
							} else
								return FINE;
						} else {
							timeout = LONG;
							++error;
							if (Ignore(SHORT) == CANCEL)
								return CANCEL;
						}
						break;
					case NAK:		/* No options */
						*useCRC = *use1K = FALSE;
						return FINE;
					default:		/* Unrecognized */
						timeout = LONG;
						++error;
						if (Ignore(SHORT) == CANCEL)
							return CANCEL;
				}
				break;
			case TX_NEGOCIATE2:
				if (err)			/* Timeout, only "C" was received */
					return FINE;
				if (c == 'K')		/* "CK" was received */
					return FINE;
				/* Unrecognized */
				timeout = LONG;
				++error;
				if (Ignore(SHORT) == CANCEL)
					return CANCEL;
				state = TX_NEGOCIATE1;
				break;
		}
	}
}

/* ----- XModem transmit ----------------------------------------------- */

enum {
	TX_HDR,
	TX_DATA,			/* Data transfer in progress */
	TX_EOT				/* EOT is in transmit buffer */
};

short XTransmit(
	Byte *name,						/* File name */
	short volume,					/* Volume reference numbere */
	long directory)					/* Directpry id */
{
	BLOCK buf1, buf2;				/* Block buffers */
	BLOCK *sendbuf, *filebuf;		/* Send buffer, file buffer */
	Byte *message;					/* Message for progress dialog */
	short err;						/* Error code */
	long mark;						/* Current file mark */
	long blk;						/* Current block number */
	long time;						/* Start time */
	long error;						/* Error counter */
	short ref;						/* File reference number */
	Byte c;							/* Character received */
	long eof;						/* Length of file */
	short vers;						/* MacBinary version */
	Boolean useCRC;					/* CRC option */
	short use1K;					/* 1K option */
	Byte block;						/* Block counter (modulo 256) */
	short canceled;					/* CAN counter */
	long size;						/* Bytes remaining to be sent */
	short state;					/* Current state */
	Boolean final = FALSE;			/* If file has been transmited */
	OSType creator, type;
	long create, modif;

	if (Sending || Transfer)
		return fBsyErr;

	/* Only send non-TEXT files as MacBinary if requested. TEXT files are
	never sent as MacBinary. */

	vers = (Settings.Binary &&
		!InfoFile(volume, directory, name,
			&creator, &type, &create, &modif) &&
		type != TEXT) ? 2 : 0;
	if (err = (vers ? BinOpenRead(volume, directory, name) :
			OpenFile(volume, directory, name, &ref)))
		return err;

	/* Prepare file transfer. In batch mode only one file is sent, even if
	the protocol allows for several files to be sent in one session. */

	Transfer = Transfer_Tx;
	SetItemStyle(GetMenu(FILE), TRANSMIT, ACTIVE);
	SerialBinary(Settings.portSetup);
	if (Settings.handshake == 1)	/* XON/OFF must be off */
		SerialHandshake(0);
	if (vers)
		BinGetEOF(&size);
	else
		GetEOF(ref, &size);				/* Exact file size (data fork) */
	eof = size + Filler(128, size);		/* Multiple of 128 bytes */
	mark = 0;
	DrawProgressDialog(P_TFILE, name);
	UpdateProgress(0,eof,0,0,0,EmptyStr);
	sendbuf = &buf1;
	filebuf = &buf2;

#ifdef MONITOR
	MonitorOpen("\pTRANSMIT.DUMP", Settings.volume, Settings.directory);
	MonitorText("\pStart of X-Modem transmit");
#endif

	/* Start by negociating */

	err = Negociate(&useCRC, &use1K);
	time = Time;
	if (err)
		goto finished;
	MakeInfo(useCRC, use1K, vers);

	/* Prepare first block to transmit */
		
	if (Settings.batch) {
		blk = 0;
		HeaderBlock(sendbuf, name, eof, useCRC);
		block = 1;
		state = TX_HDR;
	} else {
		blk = block = 1;
		if (err = ReadFile(sendbuf, ref, useCRC, use1K, vers,
				&block, &size, &mark))
			goto finished;
		state = TX_DATA;
	}
	message = EmptyStr;
	error = 0;

	do {
		/* If repeating the block, then first wait for the line to clear */

		if (error && (err = Ignore(SHORT)) == CANCEL)
			goto finished;

		/* Transmit data block (or EOT) */

		SerialSend(sendbuf->b, sendbuf->n, &Busy);
		if (state == TX_DATA && sendbuf->n == 1 && sendbuf->b[0] == EOT)
			state = TX_EOT;

#ifdef MONITOR
		MonitorText("\pTransmitting block");
		MonitorDump(sendbuf->b, sendbuf->n);
#endif
	
		/* We update the progress report and do disk file reads while the
		Mac is sending the block. This increases the transfer speed! */
	
		UpdateProgress(mark, eof, TIME, blk, error, message);
		if (!error &&
				((state == TX_HDR && !final) || state == TX_DATA)) {
			if (err = ReadFile(filebuf, ref, useCRC, use1K, vers, &block,
					&size, &mark))
				goto finished;
		}
		while(Busy)	/* Wait until transmission is finished */
			;

		/* Receive next character */

		canceled = 0;
		do {
			if ((err = Receive(&c, canceled ? SHORT : LONG)) == CANCEL)
				goto finished;

#ifdef MONITOR
			if (err)
				MonitorText("\pTimeout");
			else {
				Byte mon[256];
				FormatStr(mon, "\pReceived %i", c);
				MonitorText(mon);
			}
#endif

			if (err) {					/* Timeout */
				message = TimeoutStr;
				++error;
			} else
				if (c == CAN)
					if (canceled) {		/* 2 consecutive CANs ==> abort */
						err = ABORT;
						goto finished;
					} else
						++canceled;
				else {
					if (canceled)
						c = 0;
					switch (c) {
						case ACK:
							switch (state) {
								case TX_HDR:
									if (final) {	/* End in batch mode */
										err = FINE;
										goto finished;
									}
									if (Settings.batch == 1) {
										if (err = Negociate(&useCRC,&use1K))
											goto finished;
										MakeInfo(useCRC, use1K, vers);
									}
									state = TX_DATA;
								case TX_DATA:
									message = VerifyStr;
									++blk;
									error = 0;
									/* Switch buffers */
									{
										BLOCK *temp = sendbuf;
										sendbuf = filebuf;
										filebuf = temp;
									}
									break;
								case TX_EOT:
									Statistics(mark, Time - time, 0);
									error = 0;
									final = TRUE;
									eof = 0;
									UpdateProgress(0,0,0,0,0,EmptyStr);
									NameProgress(EmptyStr);
									if (Settings.batch) {
										if (err = Negociate(&useCRC,
												&use1K))
											goto finished;
										MakeInfo(useCRC, use1K, vers);
										blk = 0;
										HeaderBlock(sendbuf, EmptyStr,
											0, useCRC);
										block = 1;
										state = TX_HDR;
									} else {	/* No batch */
										err = FINE;
										goto finished;
									}
									break;
							}
							break;
						case NAK:
							message = RetryStr;
							++error;
							break;
						default:
							message = InvalidStr;
							++error;
							break;
					}
				}
		} while (c == CAN);

		if (error >= RETRY) {		/* Check error counter */
			err = TIMEOUT;
			goto finished;
		}

	} while (TRUE);

	/* Come here if error or finished (err == 0) */

finished:
	RemoveCancelDialog();
	if (err)
		Statistics(mark, Time - time, err);
	if (vers)
		BinCloseRead();
	else
		FSClose(ref);
	SerialReset(Settings.portSetup);
	SerialHandshake(Settings.handshake);
	SetItemStyle(GetMenu(FILE), TRANSMIT, 0);
	Transfer = 0;

#ifdef MONITOR
	MonitorText("\pEnd of X-Modem transmit");
	MonitorClose();
#endif

	return err;
}
