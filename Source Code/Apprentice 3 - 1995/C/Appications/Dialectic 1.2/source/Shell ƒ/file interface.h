/**********************************************************************\

File:		file interface.h

Purpose:	This is the header file for file interface.c

\**********************************************************************/

Boolean GetSourceFile(FSSpec *editFile);
Boolean GetDestFile(FSSpec *destFS, Boolean *deleteTheThing, Str255 theTitle);
pascal OSErr MyMakeFSSpec(short vRefNum, long parID, Str255 fileName,
	FSSpecPtr myFSS);
