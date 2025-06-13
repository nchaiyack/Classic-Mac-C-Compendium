/* ==========================================

	menu.proto.h
	
	Copyright (c) 1993,1994,1995 Newport Software Development
	
   ========================================== */

void menu_command(long mResult);
void menu_edit(int theItem);
void menu_file(int item);
void menu_hilite(void);
void menu_hilite_noshell(void);
void menu_hilite_shell(ShellH shell);
void menu_init(void);
void menu_search(int theItem);
