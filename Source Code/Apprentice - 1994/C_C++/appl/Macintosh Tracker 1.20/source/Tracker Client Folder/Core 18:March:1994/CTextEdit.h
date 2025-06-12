/* CTextEdit.h */

#pragma once

#include "CViewRect.h"

class CScrap;
class CArray;
class CEnclosure;

#define SelectText (True)
#define DontSelectText (False)

struct	CTextEdit	:	CViewRect
	{
		TEHandle					TextBox;
		static CArray*		ListOfTextEdits;
		static CTextEdit*	CurrentTextEdit;
		short							PointSize;
		short							FontID;
		Handle						UndoText;
		short							UndoOperation;
		short							UndoCursorStart;
		short							UndoCursorEnd;
		EXECUTE(MyBoolean Initialized;)

		/* */			CTextEdit();
		/* */			~CTextEdit();
		void			ITextEdit(LongPoint TheStart, LongPoint TheExtent, Handle DefaultText,
								MyBoolean Selected, short TheFontID, short ThePointSize,
								CWindow* TheWindow, CEnclosure* TheEnclosure);
		void			DoMouseDown(MyEventRec Event);
		MyBoolean	DoKeyDown(MyEventRec Event);
		void			DoUpdate(void);
		void			DoEnable(void);
		void			DoDisable(void);
		MyBoolean	DoMenuCommand(ushort MenuCommandValue);
		void			DoIdle(long TimeSinceLastEvent);
		void			EnableMenuItems(void);
		Handle		GetTextCopy(void);
		void			SetText(Handle TheNewText);
		MyBoolean	BecomeKeyReceiver(void);
		MyBoolean	RelinquishKeyReceivership(void);
	};
