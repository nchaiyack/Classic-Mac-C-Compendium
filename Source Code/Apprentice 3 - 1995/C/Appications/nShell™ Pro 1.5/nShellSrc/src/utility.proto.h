/* ==========================================

	utility.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

int  Ask(Str255 s, int size);
void error_note(Str255 what);
void myModal(int ID);
void Notify(Str255 s, int size);
void outline_item(DialogPtr theDialog, short theItem);
void SimpleModal(int ID);
int	 TestGestaltBit(OSType selector,int bit);
int  TestResError( void );
