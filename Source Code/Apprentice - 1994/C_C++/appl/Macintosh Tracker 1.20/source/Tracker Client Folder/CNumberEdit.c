/* CNumberEdit.c */

#include "CNumberEdit.h"
#include "Memory.h"
#include "CWindow.h"
#include "MenuController.h"


#define CHANGEINTERVAL (120)


void				CNumberEdit::INumberText(LongPoint TheStart, LongPoint TheExtent,
									short TheFontID, short ThePointSize, CWindow* TheWindow,
									CEnclosure* TheEnclosure)
	{
		ITextEdit(TheStart,TheExtent,NIL,SelectText,TheFontID,ThePointSize,TheWindow,
			TheEnclosure);
		Dirty = False;
	}


MyBoolean		CNumberEdit::DoKeyDown(MyEventRec Event)
	{
		switch ((uchar)(Event.Message & charCodeMask))
			{
				case 0x08:
				case 0x09:
				case '0':
				case '1':
				case '2':
				case '3':
				case '4':
				case '5':
				case '6':
				case '7':
				case '8':
				case '9':
					Dirty = True;
				 DoSomethingPoint:
					LastChange = TickCount();
					return inherited::DoKeyDown(Event);
				case (uchar)0x1c:  /* macintosh left arrow */
				case (uchar)0x1d:  /* macintosh right arrow */
					goto DoSomethingPoint;
				case 13:
					StoreValue();
					return True;
				default:
					return False;
			}
	}


MyBoolean		CNumberEdit::DoMenuCommand(ushort MenuCommandValue)
	{
		if (inherited::DoMenuCommand(MenuCommandValue))
			{
				Dirty = True;
				LastChange = TickCount();
				return True;
			}
		 else
			{
				return False;
			}
	}


void				CNumberEdit::DoIdle(long Stupid)
	{
		inherited::DoIdle(Stupid);
		if ((TickCount() - LastChange > CHANGEINTERVAL) && Dirty)
			{
				StoreValue();
			}
	}


MyBoolean		CNumberEdit::RelinquishKeyReceivership(void)
	{
		StoreValue();
		return inherited::RelinquishKeyReceivership();
	}


void				CNumberEdit::StoreValue(void)
	{
		Dirty = False;
	}


long				CNumberEdit::GetValue(void)
	{
		long			Result;
		Handle		Temp;

		Temp = GetTextCopy();
		Result = String2Int(Temp);
		ReleaseHandle(Temp);
		return Result;
	}


void				CNumberEdit::SetValue(long Value)
	{
		Handle		Temp;

		Temp = Int2String(Value);
		HLock(Temp);
		TESetText(*Temp,HandleSize(Temp),TextBox);
		DoUpdate();
		ReleaseHandle(Temp);
		CTextEdit::DoMenuCommand(mEditSelectAll);
		Dirty = False;
	}


void				CNumberEdit::DoDisable(void)
	{
		Handle		Temp;

		inherited::DoDisable();
		Temp = AllocHandle(0);
		HLock(Temp);
		TESetText(*Temp,HandleSize(Temp),TextBox);
		DoUpdate();
		ReleaseHandle(Temp);
	}
