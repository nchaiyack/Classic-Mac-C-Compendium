/* CTextEdit.c */

#include "CTextEdit.h"
#include "EventLoop.h"
#include "MenuController.h"
#include "CMyScrap.h"
#include "CArray.h"
#include "CWindow.h"
#include "Memory.h"


#define RedoXorBit (0x0080)
#define UndoNone (0x0000)
#define UndoDelete (0x0001)
#define RedoDelete (UndoDelete | RedoXorBit)
#define UndoPaste (0x0002)
#define RedoPaste (UndoPaste | RedoXorBit)
#define UndoTyping (0x0003)
#define RedoTyping (UndoTyping | RedoXorBit)
#define UndoCut (0x0004)
#define RedoCut (UndoCut | RedoXorBit)
#define UndoCopy (0x0005)
#define RedoCopy (UndoCopy | RedoXorBit)

#define CantUndoName (0x00830000 | UndoNone)
#define UndoDeleteName (0x00830000 | UndoDelete)
#define RedoDeleteName (0x00830000 | RedoDelete)
#define UndoPasteName (0x00830000 | UndoPaste)
#define RedoPasteName (0x00830000 | RedoPaste)
#define UndoTypingName (0x00830000 | UndoTyping)
#define RedoTypingName (0x00830000 | RedoTyping)
#define UndoCutName (0x00830000 | UndoCut)
#define RedoCutName (0x00830000 | RedoCut)
#define UndoCopyName (0x00830000 | UndoCopy)
#define RedoCopyName (0x00830000 | RedoCopy)

/* this lets the user tab between text edit records in the same window */
/* tabbing occurs in the order they are created */
static CArray*			CTextEdit::ListOfTextEdits = NIL;

static CTextEdit*		CTextEdit::CurrentTextEdit = NIL;


/* */			CTextEdit::CTextEdit()
	{
		CTextEdit*	Temp;
		long				Index;

		if (ListOfTextEdits == NIL)
			{
				ListOfTextEdits = new CArray;
				ListOfTextEdits->IArray(sizeof(CTextEdit*),32);
			}
		Temp = this;
		Index = ListOfTextEdits->AppendElement();
		ListOfTextEdits->PutElement(Index,&Temp);
		UndoOperation = UndoNone;
		UndoText = NIL;
	}


/* */			CTextEdit::~CTextEdit()
	{
		CTextEdit*	Temp;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::~CTextEdit called on uninitialized object."));
		Temp = this;
		ListOfTextEdits->KillElement(&Temp);
		if (ListOfTextEdits->GetNumElements() == 0)
			{
				delete ListOfTextEdits;
				ListOfTextEdits = NIL;
			}
		TEDispose(TextBox);
		DeregisterIdler(this);
		if (UndoText != NIL)
			{
				ReleaseHandle(UndoText);
			}
	}


void			CTextEdit::ITextEdit(LongPoint TheStart, LongPoint TheExtent, Handle DefaultText,
						MyBoolean Selected, short TheFontID, short ThePointSize, CWindow* TheWindow,
						CEnclosure* TheEnclosure)
	{
		Rect						TempLoc;
		TEHandle				TextBoxTemp;
		Cursor**				CursRes;

		ERROR(Initialized == True,PRERR(ForceAbort,
			"CTextEdit::ITextEdit called on already initialized object."));
		EXECUTE(Initialized = True);
		CursRes = GetCursor(iBeamCursor);
		ERROR(CursRes==NIL,PRERR(ForceAbort,
			"CTextEdit::ITextEdit couldn't get cursor resource."));
		MyCursor = **CursRes;
		IViewRect(TheStart,TheExtent,TheWindow,TheEnclosure);
		if (DefaultText == NIL)
			{
				DefaultText = AllocHandle(0);
			}
		FontID = TheFontID;
		PointSize = ThePointSize;
		SetUpPort();
		TheWindow->SetText(TheFontID,0,srcOr,ThePointSize,0);
		TempLoc.left = 3 + Origin.x;
		TempLoc.top = 3 + Origin.y;
		TempLoc.right = TheExtent.x - 3 + Origin.x;
		TempLoc.bottom = TheExtent.y - 3 + Origin.y;
		TextBoxTemp = TENew(&TempLoc,&TempLoc);
		TextBox = TextBoxTemp;
		if (TextBox==NIL)
			{
				PRERR(ForceAbort,"CTextEdit::ITextEdit couldn't allocate TE record.");
			}
		HLock(DefaultText);
		TESetText(*DefaultText,HandleSize(DefaultText),TextBox);
		HUnlock(DefaultText);
		if (Selected)
			{
				TESetSelect(0,HandleSize(DefaultText),TextBox);
			}
		ReleaseHandle(DefaultText);
	}


void			CTextEdit::DoMouseDown(MyEventRec Event)
	{
		Point		Temp;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::DoMouseDown called on uninitialized object."));
		SetUpPort();
		Event.Where = MyGlobalToLocal(Event.Where);
		Temp.h = Event.Where.x + Origin.x;
		Temp.v = Event.Where.y + Origin.y;
		if ((KeyReceiverViewRect == this) || ((KeyReceiverViewRect != this)
			&& (BecomeKeyReceiver())))
			{
				CurrentTextEdit = this;
				TEClick(Temp,((Event.Modifiers & shiftKey) != 0),TextBox);
				SetUpPort();
			}
	}


MyBoolean	CTextEdit::DoKeyDown(MyEventRec Event)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::DoKeyDown called on uninitialized object."));
		if (KeyReceiverViewRect != this)
			{
				return False;
			}
		 else
			{
				if (UndoOperation != UndoTyping)
					{
						UndoOperation = UndoTyping;
						if (UndoText != NIL)
							{
								ReleaseHandle(UndoText);
							}
						UndoText = DupSysHandle((**TextBox).hText);
						SetTag(UndoText,"TEUndo");
						UndoCursorStart = (**TextBox).selStart;
						UndoCursorEnd = (**TextBox).selEnd;
					}
				SetUpPort();
				if ((Event.Message & 0x000000ff) == 0x09) /* test for tab character */
					{
						CTextEdit*	NextTextEdit;
						long				Scan;
						long				NumElements;
						long				Increment;

						NumElements = ListOfTextEdits->GetNumElements();
						Scan = 0;
						while (Scan < NumElements)
							{
								ListOfTextEdits->GetElement(Scan,&NextTextEdit);
								if (NextTextEdit == this)
									{
										goto EscapeOnePoint;
									}
								Scan += 1;
							}
						EXECUTE(PRERR(ForceAbort,
							"A TextEdit is missing from the master CTextEdit list."));
					 EscapeOnePoint:
						if ((Event.Modifiers & shiftKey) == 0)
							{
								Increment = 1;
							}
						 else
							{
								Increment = -1;
							}
						do
							{
								Scan += Increment;
								if (Scan >= NumElements)
									{
										Scan = 0;
									}
								if (Scan < 0)
									{
										Scan = NumElements - 1;
									}
								ListOfTextEdits->GetElement(Scan,&NextTextEdit);
							} while ((NextTextEdit->Window != this->Window)
								|| (!NextTextEdit->Enabled));
						NextTextEdit->DoMenuCommand(mEditSelectAll);
						NextTextEdit->BecomeKeyReceiver();
					}
				 else
					{
						if ((KeyReceiverViewRect == this) || ((KeyReceiverViewRect != this)
							&& (BecomeKeyReceiver())))
							{
								if (((**TextBox).selStart != (**TextBox).selEnd)
									&& ((Event.Message & 0x000000ff) == 8))
									{
										DoMenuCommand(mEditClear);
									}
								 else
									{
										TEKey(Event.Message & 0x000000ff,TextBox);
										SetUpPort();
									}
							}
					}
				return True;
			}
	}


void			CTextEdit::DoUpdate(void)
	{
		Rect		ShowRect;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::DoUpdate called on uninitialized object."));
		SetUpPort();
		if (!Enabled)
			{
				Window->SetGreyishTextOr();
			}
		ShowRect = (**TextBox).viewRect;
		Window->LEraseRect(ZeroPoint,Extent);
		Window->LFrameRect(ZeroPoint,Extent);
		TEUpdate(&ShowRect,TextBox);
	}


MyBoolean	CTextEdit::BecomeKeyReceiver(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::BecomeKeyReceiver called on uninitialized object."));
		if (inherited::BecomeKeyReceiver()) /* if underlying routine succeeds */
			{
				SetUpPort();
				TEActivate(TextBox);
				RegisterIdler(this,15);
				return True;
			}
		 else
			{
				return False;
			}
	}


MyBoolean	CTextEdit::RelinquishKeyReceivership(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::RelinquishKeyReceivership called on uninitialized object."));
		if (inherited::RelinquishKeyReceivership())
			{
				SetUpPort();
				TEDeactivate(TextBox);
				DeregisterIdler(this);
				if (UndoText != NIL)
					{
						ReleaseHandle(UndoText);
						UndoText = NIL;
					}
				UndoOperation = UndoNone;
				return True;
			}
		 else
			{
				return False;
			}
	}


void			CTextEdit::DoEnable(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::DoEnable called on uninitialized object."));
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);
		inherited::DoEnable();
	}


void			CTextEdit::DoDisable(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::DoDisable called on uninitialized object."));
		SetUpPort();
		Window->InvalidateLong(ZeroPoint,Extent);
		inherited::DoDisable();
	}


MyBoolean	CTextEdit::DoMenuCommand(ushort MenuCommandValue)
	{
		OSErr		Error;

		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::DoMenuCommand called on uninitialized object."));
		SetUpPort();
		switch (MenuCommandValue)
			{
				case mEditCut:
					UndoOperation = UndoCut;
					if (UndoText != NIL)
						{
							ReleaseHandle(UndoText);
						}
					UndoText = DupSysHandle((**TextBox).hText);
					SetTag(UndoText,"TEUndo");
					UndoCursorStart = (**TextBox).selStart;
					UndoCursorEnd = (**TextBox).selEnd;
					TECut(TextBox);
					Scrap->SetScrap(DupSysHandle(TEScrapHandle()),'TEXT');
					return True;
				case mEditCopy:
					UndoOperation = UndoCopy;
					if (UndoText != NIL)
						{
							ReleaseHandle(UndoText);
						}
					UndoText = DupSysHandle((**TextBox).hText);
					SetTag(UndoText,"TEUndo");
					UndoCursorStart = (**TextBox).selStart;
					UndoCursorEnd = (**TextBox).selEnd;
					TECopy(TextBox);
					Scrap->SetScrap(DupSysHandle(TEScrapHandle()),'TEXT');
					return True;
				case mEditPaste:
					if (Scrap->GetScrapType() == 'TEXT')
						{
							UndoOperation = UndoPaste;
							if (UndoText != NIL)
								{
									ReleaseHandle(UndoText);
								}
							UndoText = DupSysHandle((**TextBox).hText);
							SetTag(UndoText,"TEUndo");
							UndoCursorStart = (**TextBox).selStart;
							UndoCursorEnd = (**TextBox).selEnd;
							Scrap->ExportScrap();
							Error = TEFromScrap();
							/* ERROR(Error != noErr,
								PRERR(AllowResume,"CTextEdit::DoMenuCommand TEFromScrap error.")); */
							TEPaste(TextBox);
						}
					return True;
				case mEditClear:
					UndoOperation = UndoDelete;
					if (UndoText != NIL)
						{
							ReleaseHandle(UndoText);
						}
					UndoText = DupSysHandle((**TextBox).hText);
					SetTag(UndoText,"TEUndo");
					UndoCursorStart = (**TextBox).selStart;
					UndoCursorEnd = (**TextBox).selEnd;
					TEDelete(TextBox);
					return True;
				case mEditSelectAll:
					TESetSelect(0,(**TextBox).teLength,TextBox);
					return True;
				case mEditUndo:
					if (UndoOperation != UndoNone)
						{
							Handle		Temp;
							short			SelStartTemp;
							short			SelEndTemp;

							SetUpPort();
							Window->LEraseRect(ZeroPoint,Extent);
							Temp = DupSysHandle((**TextBox).hText);
							SetTag(Temp,"TEUndo");
							SelStartTemp = (**TextBox).selStart;
							SelEndTemp = (**TextBox).selEnd;
							HLock(UndoText);
							TESetText(*UndoText,HandleSize(UndoText),TextBox);
							HUnlock(UndoText);
							ReleaseHandle(UndoText);
							(**TextBox).selStart = UndoCursorStart;
							(**TextBox).selEnd = UndoCursorEnd;
							UndoCursorStart = SelStartTemp;
							UndoCursorEnd = SelEndTemp;
							UndoText = Temp;
							SetUpPort();
							DoUpdate();
							if ((UndoOperation == UndoCut) || (UndoOperation == RedoCut) ||
								(UndoOperation == UndoCopy) || (UndoOperation == RedoCopy))
								{
									Scrap->UndoScrapOp();
								}
							UndoOperation = UndoOperation ^ RedoXorBit;
						}
					break;
				default:
					return False;
			}
	}


void			CTextEdit::DoIdle(long TimeSinceLastEvent)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::DoIdle called on uninitialized object."));
		if (KeyReceiverViewRect == this)
			{
				SetUpPort();
				TEIdle(TextBox);
			}
	}


/* enable the menu items this object can handle */
void			CTextEdit::EnableMenuItems(void)
	{
		ERROR(Initialized != True,PRERR(ForceAbort,
			"CTextEdit::EnableMenuItems called on uninitialized object."));
		if (KeyReceiverViewRect == this)
			{
				Handle		NewName;

				MyEnableItem(mEditCut);
				MyEnableItem(mEditCopy);
				MyEnableItem(mEditPaste);
				MyEnableItem(mEditClear);
				MyEnableItem(mEditSelectAll);
				switch (UndoOperation)
					{
						case UndoDelete:
							NewName = GetCString(UndoDeleteName);
							break;
						case UndoTyping:
							NewName = GetCString(UndoTypingName);
							break;
						case UndoPaste:
							NewName = GetCString(UndoPasteName);
							break;
						case UndoCut:
							NewName = GetCString(UndoCutName);
							break;
						case UndoCopy:
							NewName = GetCString(UndoCopyName);
							break;
						case RedoDelete:
							NewName = GetCString(RedoDeleteName);
							break;
						case RedoTyping:
							NewName = GetCString(RedoTypingName);
							break;
						case RedoPaste:
							NewName = GetCString(RedoPasteName);
							break;
						case RedoCut:
							NewName = GetCString(RedoCutName);
							break;
						case RedoCopy:
							NewName = GetCString(RedoCopyName);
							break;
						default:
							NewName = GetCString(CantUndoName);
							break;
					}
				ChangeName(mEditUndo,NewName);
				if (UndoOperation != UndoNone)
					{
						MyEnableItem(mEditUndo);
					}
			}
	}


Handle		CTextEdit::GetTextCopy(void)
	{
		return DupSysHandle((**TextBox).hText);
	}


void			CTextEdit::SetText(Handle TheNewText)
	{
		Handle		Temp;

		HLock(TheNewText);
		TESetText(*TheNewText,HandleSize(TheNewText),TextBox);
		SetUpPort();
		DoUpdate();
		ReleaseHandle(TheNewText);
		DoMenuCommand(mEditSelectAll);
	}
