
/* query.c */
Boolean DoQueryEvt(EventRecord theEvent);
void NewQuery(void);
Boolean NewQueryBlock(short dbcode, QueryHdl *new, short gBooleanOp);
Boolean NewQueryWindow(short w, WDPtr wdp, QueryHdl queryHdl, StringPtr title);
void DisposeQuery(WDPtr wdp);
void UpdateQueryRec(WDPtr wdp);
void DuplicateQuery(WDPtr wdp);
void OpenQuery(void);
Boolean LoadQuery(short w, StringPtr fName, short vRefNum);
