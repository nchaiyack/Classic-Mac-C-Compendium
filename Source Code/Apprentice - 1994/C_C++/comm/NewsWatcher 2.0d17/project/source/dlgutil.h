typedef pascal void (*UserItemProcPtr)(DialogPtr, short);

typedef void (*CustomCopyCommandHandler)(DialogPtr);

Boolean DlgGetCheck (DialogPtr dlg, short item);
void DlgSetCheck (DialogPtr dlg, short item, Boolean value);
void DlgToggleCheck (DialogPtr dlg, short item);
ControlHandle DlgGetControl (DialogPtr dlg, short item);
short DlgGetCtlValue (DialogPtr dlg, short item);
void DlgSetCtlValue (DialogPtr dlg, short item, short value);
short DlgGetNumber (DialogPtr dlg, short item);
void DlgSetNumber (DialogPtr dlg, short item, short value);
void DlgGetCString (DialogPtr dlg, short item, char *value);
void DlgSetCString (DialogPtr dlg, short item, char *value);
void DlgGetPString (DialogPtr dlg, short item, StringPtr value);
void DlgSetPString (DialogPtr dlg, short item, StringPtr value);
void DlgEnableItem (DialogPtr dlg, short item, Boolean enabled);
void DlgSetUserItem (DialogPtr dlg, short item, UserItemProcPtr proc);
void DlgSetPict (DialogPtr dlg, short item, PicHandle pict);
void DlgEraseItems (DialogPtr dlg, short first, short last);
pascal void	DlgGrayBorderItem (DialogPtr dlg, short item);
void DlgFlashButton (DialogPtr dlg, short item);

void SetDialogCustomCopyHandler (DialogPtr dlg, CustomCopyCommandHandler theHandler);
void SetItemNumeric (DialogPtr dlg, short item, Boolean numeric);
void SetItemReturnIsLegal (DialogPtr dlg, short item, Boolean returnIsLegal);
void SetItemPopupTypeinItem (DialogPtr dlg, short item, short popupTypeinItem);
void SetItemKeyEquivalent (DialogPtr dlg, short item, char keyEquivalent);
void SetItemMaxLength (DialogPtr dlg, short item, short maxLength);
void SetItemPassword (DialogPtr dlg, short item, char *password); 
void SetItemReadOnly (DialogPtr dlg, short item, Handle theText, 
	short fontNum, short fontSize);

void PrepDialog (void);
DialogPtr MyGetNewDialog (short id);
void MyDisposDialog (DialogPtr dlg);
void MyShortenDITL (DialogPtr dlg, short numberItems);
void MyAppendDITL (DialogPtr dlg, Handle theDITL, DITLMethod theMethod);
pascal Boolean DialogFilter (DialogPtr theDialog,
	EventRecord *theEvent, short *itemHit);
void MyModalDialog (ModalFilterProcPtr filterProc, short *itemHit,
	Boolean hasCancelButton, Boolean returnIsOK);
void ErrorMessage (char *msg);
void UnexpectedErrorMessage (OSErr err);
void NewsServerErrorMessage (char *serverCommand, char *serverResponse);
void MailOrFTPServerErrorMessage (char *serverResponse);