void MyStandardGetFile(FileFilterProcPtr fileFilter, short numTypes,
	SFTypeList typeList, StandardFileReply *reply);
void MyStandardPutFile (const Str255 prompt, const Str255 defaultName,
	StandardFileReply *reply);
void MyStandardGetDirectory (StandardFileReply *reply, long *dirID);