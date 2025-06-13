#ifndef __UTILITIES__
#define __UTILITIES__

Boolean				CommandKeyIsDown(void);
Boolean				ControlKeyIsDown(void);
DialogPtr			CreateNewDialog(short dlgID, short okID, short cancelID);
void				FlashDialogItem(DialogPtr dlg, short itemToFlash);
short				GetDialogItemState(DialogPtr dlg, short controlNumber);
short				GetItemKind(DialogPtr dlg, short item);
Handle				GetItemHandle(DialogPtr dlg, short item);
Rect				GetItemRect(DialogPtr dlg, short item);
short				GetNextVolume(short thisVolume);
void				GetVersionString(short versID, Boolean getShort, StringPtr theString);
void				HandleToStr255(StringHandle sh, StringPtr s);
Boolean				KeyIsDown(short keyCode);
StringHandle		NewStringSys(ConstStr255Param s);
Boolean				OptionKeyIsDown(void);
void				ReportError(short errStringIndex, short errorNumber);
void				SetDialogItemState(DialogPtr dlg, short controlNumber, short value);
void				SetItemKind(DialogPtr dlg, short item, short kind);
void				SetItemHandle(DialogPtr dlg, short item, Handle handle);
void				SetItemRect(DialogPtr dlg, short item, Rect* rect);
void				SetRadioButton(DialogPtr dlg, short buttonNumber, 
						short *previousRadio);
void				ToggleCheckBox(DialogPtr dlg, short buttonNumber);
Boolean				VolHasCatSearch(short vRefNum);

short				MyOpenResFile(FSSpecPtr file, short perm, Boolean *closeIt);
short				UseMyResFile(void);
Handle				MyGetResource(ResType type, short resID);
short				GetMyRefNum(FSSpecPtr file);

#endif
