#ifndef H_PAP
#define H_PAP

#include <AppleTalk.h>


enum
{
	noPrinterChosenErr = 1,
	printerNotFoundErr,
	notPAPPrinterErr
};


typedef struct
{
	long      systemStuff;
	Str255    statusStr;

} PAPStatusRec, *PAPStatusPtr;

                
extern pascal short	PAPOpen		(short *refNum, char *printerName, short flowQuantum,
								 PAPStatusPtr statusBuf);
extern pascal short	PAPRead		(short refNum, char *buffer, short *length, short *eol,
								 short *compState);
extern pascal short	PAPWrite	(short refNum, char *buffer, short length, short eol,
								 short *compState);
extern pascal short	PAPClose	(short refNum);
extern pascal short	PAPStatus	(char *printerName, PAPStatusPtr statusBuff,
								 AddrBlock *netAddr);
extern OSErr		PAPLoad		(StringPtr *pLWName, StringPtr *pLWType,
								 StringPtr *pLWZone);
extern OSErr		PAPUnload	(void);

extern StringHandle	ChooserPrinterName (void);
extern short		OpenPrinterFile (StringHandle hDriverFName);


#endif /* H_PAP */
