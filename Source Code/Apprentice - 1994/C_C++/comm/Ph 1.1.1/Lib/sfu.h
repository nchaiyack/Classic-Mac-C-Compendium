/*______________________________________________________________________

	sfu.h - Standard File Utilities Interface.
	
	Copyright © 1988-1991 Northwestern University.
_____________________________________________________________________*/

#ifndef __sfu__
#define __sfu__

extern void sfu_StandardGetFile (FileFilterProcPtr fileFilter, short numTypes,
	SFTypeList typeList, StandardFileReply *reply);
extern void sfu_StandardPutFile (Str255 prompt, Str255 defaultName,
	StandardFileReply *reply);
extern void sfu_GetFileOrFolder (FileFilterYDProcPtr fileFilter, short numTypes,
	SFTypeList typeList, StandardFileReply *reply,
	short dlgID6, short dlgID7, Point where, 
	void *yourDataPtr, Str255 doitButtonName, Str255 prompt);
	
#endif
