/*______________________________________________________________________

	rpp.h - Report Printing Module Interface.
	
	Copyright � 1988-1991 Northwestern University.
_____________________________________________________________________*/

#ifndef __rpp__
#define __rpp__

typedef void (*rpp_UpdateAll)(void);

typedef struct rpp_PrtBlock {
	THPrint				hPrint;
	short					fontNum;
	short					fontSize;
	short					topMargin;
	short					botMargin;
	short					leftMargin;
	short					rightMargin;
	Boolean				reverseOrder;
	Boolean				header;
	unsigned char		*title;
	short					titleSep;
	short					titleFont;
	short					titleStyle;
	short					titleSize;
	unsigned char		*titleTmpl;
	unsigned char		*docName;
	short					dlogID;
	short					tabConID;
	short					emptyPageRangeID;
	short					ditlID;
	short					sizeRangeID;
	short					marginsTooBigID;
	short					truncateRightID;
	short					truncateBottomID;
	short					minFontSize;
	short					maxFontSize;
	Boolean				menuPick;
	rpp_UpdateAll		updateAll;
} rpp_PrtBlock;

extern OSErr rpp_Print (Handle repHandle, Boolean printOne, rpp_PrtBlock *p);
extern OSErr rpp_StlDlog (rpp_PrtBlock *p, Boolean *canceled);

#endif
