void	CONFIGUREunload(void);
void Cenviron( void);
void Cftp( void);
Boolean GetApplicationType(OSType *type);
void	BoundsCheck(long *value, long high, long low);
void EditConfigType(ResType ConfigResourceType, Boolean (*EditFunction)());
Boolean EditTerminal(StringPtr PrefRecordNamePtr);
Boolean EditSession(StringPtr PrefRecordNamePtr);
Boolean EditFTPUser(StringPtr PrefRecordNamePtr);

pascal short ColorBoxModalProc( DialogPtr dptr, EventRecord *evt, short *item);
PROTO_UPP(ColorBoxModalProc, ModalFilter);
pascal ColorBoxItemProc(DialogPtr theDlg, short itemNo);
PROTO_UPP(ColorBoxItemProc, UserItem);
