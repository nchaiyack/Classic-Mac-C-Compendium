// asm routines, indexes are zero-based

StringPtr GetPtrIndHString(Handle resH, unsigned short index);
void GetIndHString(StringPtr dest, Handle resH, unsigned short index);
//short KeyState(unsigned short k );
Boolean ModifiersState(short mask);
OSType Str2OSType(ConstStr255Param theStr);
void OSType2Str(OSType typ, StringPtr theStr);
Handle GetFullPath(const FSSpecPtr fss, Boolean AUXpresent);
void fabc2pstr(unsigned char *cs, StringPtr destps);
Boolean IsOnScreen(const RectPtr r);
Boolean IsOnScreenWeak(Point pt);
