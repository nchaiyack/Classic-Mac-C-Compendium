
/* export.c */
void ExportRes(WDPtr wdp);
Boolean Export2(short input, long ann_offset, long seq_offset, short dbcode, StringPtr fName, char *inpBuf, char *outBuf, short output, StringPtr outFName, DialogPtr myDialog);
void ExtractEname(CString80Hdl bufHdl, short pos, StringPtr ename);
void BuildFName(StringPtr filename, StringPtr ename, short format);
void DisableMenuBar(void);
void EnableMenuBar(void);
