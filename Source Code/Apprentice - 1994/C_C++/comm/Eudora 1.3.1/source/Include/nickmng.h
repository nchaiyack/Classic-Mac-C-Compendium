/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
int RegenerateAliases(UHandle *intoH,short cmdName);
long CountAliasTotal(UHandle aliases,long offset);
long CountAliasAlias(UHandle aliases,long offset);
long CountAliasExpansion(UHandle aliases,long offset);
short ReplaceAlias(UHandle aliases,UPtr oldName,UPtr newName,Handle text);
void RemoveAlias(UHandle aliases,UPtr name);
short AddAlias(UHandle aliases,UPtr name,Handle addresses);
void MakeMessNick(MyWindowPtr win,short modifiers);
void MakeCompNick(MyWindowPtr win);
void MakeMboxNick(MyWindowPtr win,short modifiers);
void MakeCboxNick(MyWindowPtr win);
void FlattenListWith(Handle h,Byte c);
void NewNick(Handle addresses);
Boolean SaveAliases(void);
#define MAX_NICKNAME 30