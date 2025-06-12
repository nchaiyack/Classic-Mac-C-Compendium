/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/************************************************************************
 * declarations for binhexing and unbinhexing macintosh files
 ************************************************************************/
short SaveHexBin(UHandle text,long size,long estMessageSize);
void EndHexBin(void);
void BeginHexBin(void);

/************************************************************************
 * states for the converter
 ************************************************************************/
typedef enum {
	NotHex, CollectName, CollectInfo, DataWrite, DataCrc1, DataCrc2,
	RzWrite, RzCrc1, RzCrc2, Excess, HexDone, HexSLimit
} HexBinStates;
Boolean WantTheFile(UPtr name,short *vRef);
Boolean AutoWantTheFile(UPtr name,short *vRef,Boolean ohYesYouDo);

