/* ==========================================

	sfget.c
	
	Copyright (c) 1994,1995 Newport Software Development
	
   ========================================== */

Boolean sfget(Str255 path);
pascal  short sfget_hook(short item,DialogPtr theDlg,Ptr userData);
void    sfget_button(StringPtr selName,DialogPtr theDlg);
Boolean sfget_same_file(FSSpec *file1,FSSpec *file2);
OSErr   sfget_get_desktop_folder(short vRefNum);
OSErr   sfget_ensure_name(FSSpec *fSpec);
