/* CNumberEdit.h */

#pragma once

#include "CTextEdit.h"
#include "CSimpleButton.h"

struct	CNumberEdit	:	CTextEdit
	{
		ulong					LastChange;
		MyBoolean			Dirty;

		void				INumberText(LongPoint TheStart, LongPoint TheExtent,
									short TheFontID, short ThePointSize, CWindow* TheWindow,
									CEnclosure* TheEnclosure);
		MyBoolean		DoKeyDown(MyEventRec Event);
		MyBoolean		DoMenuCommand(ushort MenuCommandValue);
		long				GetValue(void);
		void				SetValue(long Value);
		void				StoreValue(void);
		void				DoIdle(long Stupid);
		MyBoolean		RelinquishKeyReceivership(void);
		void				DoDisable(void);
	};
