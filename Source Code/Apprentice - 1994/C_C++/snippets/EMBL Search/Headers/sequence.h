
/* sequence.c */
Boolean NewSequenceWindow(ResultHdl resHdl, short bufPos);
void DisposeSequence(WDPtr wdp);
Boolean FillLineBuffer(SeqRecHdl seqRecHdl, short first);
void DrawSeqWinAll(WDPtr wdp, short dummy);
void AdjustSeqText(WindowPtr wPtr, short oldvalue, short newvalue, scrollBarType which);
void DoSeqClicks(WDPtr wdp, EventRecord *eventPtr);
Boolean SaveSeqData(StringPtr fName, short vRefNum, SeqRecHdl seqRecHdl);
void SelectAllSeq(WDPtr wdp, Boolean what);
void HideShowSeqSelections(WDPtr wdp, Boolean show);
