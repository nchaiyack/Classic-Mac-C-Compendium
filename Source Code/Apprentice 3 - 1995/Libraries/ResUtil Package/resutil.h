/* resutil.h */

extern OSErr CopyResource( ResType rType, short rID, short src, short dest );
extern OSErr InstallResource( short rf, Handle theRes, ResType rType, short rID, StringPtr name, short attr );
extern OSErr ClearResID( short rf, ResType rType, short rID );
