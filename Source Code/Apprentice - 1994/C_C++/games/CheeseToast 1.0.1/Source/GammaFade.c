/******************************************************************************
 GammaFade.c
							
 ******************************************************************************/

#include <QDOffscreen.h>
#include <QuickDraw.h>
#include <Video.h>
#include <Traps.h>

GammaTblPtr      newGammaTable;
GammaTblPtr      savedGammaTable;
GDHandle         mainDevice;
CTabHandle       mainDeviceColors;
VDGammaRecord    newGamma;
VDSetEntryRecord setEntriesRec;
unsigned char    *newTable;
unsigned char    *savedTable;
VDGammaRecord    currentGamma;
GammaTblPtr      currentGammaTable;
Boolean			 gammaFadedOut;

void GammaFadeOut(void);
void GammaFadeIn(void);
pascal short MyVideoStatus(short refNum, short code, Ptr param);

Boolean IsGammaFaded()
{
	return gammaFadedOut;
}

void GammaFadeOut(void)
{
	QDErr			error;
	short           count;
	long            fadeAmount;
	short           channelIndex;
	Ptr				csPtr;
	long			tableSize;
	
	if (IsGammaFaded())
		return;

	mainDevice = GetMainDevice ();

	/* Grab the main device�s gamma table; VideoStatus in Sample.a */
	error = MyVideoStatus( (**mainDevice).gdRefNum, cscGetGamma,
			(Ptr)&currentGamma );

	/* Grab a pointer to the main device�s gamma table */
	currentGammaTable = (GammaTblPtr)currentGamma.csGTable;

	// JAB 12/23/93
	if (currentGammaTable->gDataWidth > 16)
		return;
	tableSize = (long) currentGammaTable->gDataCnt * (currentGammaTable->gDataWidth <= 8? 1 : 2);

	/* Alloc mem for new gamma table */
	savedGammaTable = (GammaTblPtr)NewPtr( sizeof (GammaTbl) +
			tableSize * currentGammaTable->gChanCnt +
			currentGammaTable->gFormulaSize );

	/* Copy main device�s gamma into new gamma except for table itself */
	BlockMove( (Ptr)currentGammaTable, (Ptr)savedGammaTable,
			sizeof (GammaTbl) + tableSize * currentGammaTable->gChanCnt +
			currentGammaTable->gFormulaSize );

	/* Alloc mem for new gamma table */
	newGammaTable = (GammaTblPtr)NewPtr( sizeof (GammaTbl) +
			tableSize * currentGammaTable->gChanCnt +
			currentGammaTable->gFormulaSize );

	/* Copy main device�s gamma into new gamma except for table itself */
	BlockMove( (Ptr)savedGammaTable, (Ptr)newGammaTable,
			sizeof (GammaTbl) );

	/* Fade out the screen */
	for (fadeAmount = 100; fadeAmount >= 0; fadeAmount -= 4)
	{
		/* Get pointers to the actual tables */
		newTable = (unsigned char *)&newGammaTable->gFormulaData +
				newGammaTable->gFormulaSize;
		savedTable = (unsigned char *)&savedGammaTable->gFormulaData +
				savedGammaTable->gFormulaSize;

		/* For each channel in the table, fade it */
		for (channelIndex = 0; channelIndex < savedGammaTable->gChanCnt; ++channelIndex)
		{
			/* For each entry in each table channel, fade it */
			if (savedGammaTable->gDataWidth <= 8) {
				for (count = 0; count < savedGammaTable->gDataCnt; count++)
					newTable[count] = savedTable[count] * fadeAmount / 100L;
			}
			else {
				unsigned short *nt,*st;
				nt = (unsigned short *) &newTable[0];
				st = (unsigned short *) &savedTable[0];
				for (count = 0; count < savedGammaTable->gDataCnt; count++)
					nt[count] = st[count] * fadeAmount / 100L;
			}
			/* Point to the next channels */
			newTable += newGammaTable->gDataCnt;
			savedTable += savedGammaTable->gDataCnt;
		}

		/* Write the new gamma table to the main device */
		newGamma.csGTable = (Ptr)newGammaTable;
		csPtr = (Ptr) &newGamma;
		error = Control( (**mainDevice).gdRefNum, cscSetGamma, (Ptr) &csPtr );

		/* Force the main device to use the new gamma table */
		mainDeviceColors = (**(**mainDevice).gdPMap).pmTable;
		setEntriesRec.csTable = (ColorSpec *)&(**mainDeviceColors).ctTable;
		setEntriesRec.csStart = 0;
		setEntriesRec.csCount = (**mainDeviceColors).ctSize;
		csPtr = (Ptr) &setEntriesRec;
		error = Control ((**mainDevice).gdRefNum, cscSetEntries, (Ptr) &csPtr);
	}
	gammaFadedOut = true;
}

void GammaFadeIn(void)
{
	long             fadeAmount;
	unsigned char    *newTable;
	unsigned char    *savedTable;
	short            count;
	short            channelIndex;
	VDGammaRecord    newGamma;
	Ptr				 csPtr;
	QDErr			error;
	VDSetEntryRecord setEntriesRec;

	// SysBeep(1);

	if (!IsGammaFaded())
		return;

	/* Fade in the screen */
	for (fadeAmount = 0; fadeAmount <= 100; fadeAmount += 4)
	{
		/* Get pointers to the actual tables */
		newTable = (unsigned char *)&newGammaTable->gFormulaData +
				newGammaTable->gFormulaSize;
		savedTable = (unsigned char *)&savedGammaTable->gFormulaData +
				savedGammaTable->gFormulaSize;

		/* For each channel in the table, fade it */
		for (channelIndex = 0; channelIndex < savedGammaTable->gChanCnt; ++channelIndex)
		{
			/* For each entry in each table channel, fade it */
			if (savedGammaTable->gDataWidth <= 8) {
				for (count = 0; count < savedGammaTable->gDataCnt; count++)
					newTable[count] = savedTable[count] * fadeAmount / 100L;
			}
			else {
				unsigned short *nt,*st;
				nt = (unsigned short *) &newTable[0];
				st = (unsigned short *) &savedTable[0];
				for (count = 0; count < savedGammaTable->gDataCnt; count++)
					nt[count] = st[count] * fadeAmount / 100L;
			}

			/* Point to the next channels */
			newTable += newGammaTable->gDataCnt;
			savedTable += savedGammaTable->gDataCnt;
		}

		/* Write the new gamma table to the main device */
		newGamma.csGTable = (Ptr) newGammaTable;
		csPtr = (Ptr) &newGamma;
		error = Control ((**mainDevice).gdRefNum, cscSetGamma, (Ptr) &csPtr);

		/* Force the main device to use the new gamma table */
		mainDeviceColors = (**(**mainDevice).gdPMap).pmTable;
		setEntriesRec.csTable = (ColorSpec *)&(**mainDeviceColors).ctTable;
		setEntriesRec.csStart = 0;
		setEntriesRec.csCount = (**mainDeviceColors).ctSize;
		csPtr = (Ptr) &setEntriesRec;
		error = Control ((**mainDevice).gdRefNum, cscSetEntries, (Ptr) &csPtr);
	}

	DisposPtr( (Ptr)newGammaTable );
	DisposPtr( (Ptr)savedGammaTable );

	gammaFadedOut = 0;
}

pascal short MyVideoStatus (short refNum, short code, Ptr param)
{
	struct	CntrlParam	sBlock;
	short	result;
	sBlock.ioCRefNum = refNum;
	sBlock.csCode = code;
	*((long *) &sBlock.csParam[0]) = (long) param;

	asm {
		lea		sBlock, a0
		dc.w	0xA005
		move.l	(sp)+,a0
		move.w	d0, result				
	}
	return result;
}

