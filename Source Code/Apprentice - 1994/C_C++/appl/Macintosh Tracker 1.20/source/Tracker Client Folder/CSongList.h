/* CSongList.h */

#pragma once

#include "CViewRect.h"

class CMyDocument;
class CVScrollBar;

#define MAXTYPEDCHARS (32)

struct	CSongList	:	CViewRect
	{
		CMyDocument*	Document;
		CVScrollBar*	VScroll;
		ulong					LastClickTime;
		long					StartingIndex;
		short					LineHeight;
		uchar					CharBuffer[MAXTYPEDCHARS];
		short					CharBufferLength;
		ulong					LastCharTime;

		void				ISongList(CMyDocument* TheDocument, CWindow* TheWindow);
		void				DoMouseDown(MyEventRec Event);
		MyBoolean		DoKeyDown(MyEventRec Event);
		void				DoUpdate(void);
		long				Hook(short OperationID, long Operand1, long Operand2);
		MyBoolean		DoMenuCommand(ushort MenuCommandValue);
		void				EnableMenuItems(void);
		void				Redraw(long Start, long End);
		void				RecalculateScrollBars(void);
	};
