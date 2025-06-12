/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
/**********************************************************************
 * handling the alias panel
 **********************************************************************/
Boolean AliasClose(MyWindowPtr win);
void OpenAliases(void);
void AliasesFixFont(void);
Boolean AliasWinIsOpen(void);
void ChooseAlias(short which);
Boolean CanMakeNick(void);
void AliasWinGonnaSave(void);
void AliasWinDidSave(void);