OSErr OpenTheResFile(FSSpec* fs, short* oldRefNum, short* newRefNum, Boolean* alreadyOpen,
	Boolean readOnly);
void CloseTheResFile(short oldRefNum, short newRefNum, Boolean alreadyOpen);
Handle SafeReleaseResource(Handle resHandle);
Handle SafeDisposeIconSuite(Handle iconHandle);
MenuHandle SafeDisposeMenu(MenuHandle theMenu);
OSErr AddIndString(StringPtr theStr, short resID);
OSErr DeleteIndString(short resID, short index);
OSErr SetIndString(StringPtr theStr,short resID,short strIndex);
