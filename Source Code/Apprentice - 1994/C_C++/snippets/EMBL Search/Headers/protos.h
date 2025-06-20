/*
******************************* Prototypes ***************************
*/

#define _H_protos

/* appleevents.c */
extern void DoHighLevelEvent(EventRecord event);
extern void InitAppleEvents(void);

/* browse.c */
extern Boolean BrowseIndex(short dbcode,short field, StringPtr value);

/* bsearch.c */
extern Boolean CDIndex_BSearch(void *key, StringPtr fName, short fd, u_long size,
		u_long nrec, short (*compare )(void *, void *), void *rec, u_long *hitrec);

/* cd.c */
extern void	InitCD(void);
extern Boolean InitDB(short vRefNum, StringPtr volName);

/* centerda.c */
extern void CenterDA(ResType what, short id, short offset);

/* checkapp.c */
extern void CheckAppFiles(void);
extern Boolean DoDocOpen(OSType fType,short vRefNum,StringPtr fName, WDPtr *wdpPtr);
extern Boolean DoDocPrint(OSType fType,short vRefNum,StringPtr fName);

/* click.c */
extern void DoResContentClick(EventRecord *eventPtr, Rect *viewRectPtr, WDPtr wdp);
extern void DoSeqContentClick(EventRecord *eventPtr, Rect *viewRectPtr, WDPtr wdp);

/* copy.c */
extern void CopySelection(WDPtr wdp);
extern void CutSelection(WDPtr wdp);
extern void PasteSelection(WDPtr wdp);
extern void ClearSelection(WDPtr wdp);

/* environment.c */
extern void CheckEnvironment(void);

/* events.c */
extern void DoEvent(EventRecord theEvent);
extern void HandleActivates(WindowPtr wPtr,Boolean activateFlag);

/* export.c */
extern void ExportRes(WDPtr wdp);
extern void ExtractEname(CString80Hdl bufHdl, short pos, StringPtr ename);
extern void BuildFName(StringPtr filename, StringPtr ename, short format);
extern Boolean Export2(short input,long ann_offset,long seq_offset,short dbcode,
				StringPtr fName, char *inpBuf,char *outBuf,short output,StringPtr outFName,
				DialogPtr myDialog);
extern void DisableMenuBar(void);
extern void EnableMenuBar(void);

/* findhits.c */
extern HitmapHdl DoSearch(short what, StringPtr key, short dbcode);
extern Boolean FindHits(char *key, short dbcode, HitmapHdl hitmapHdl, u_short target_recsize,
					StringPtr targetFName, u_long target_nrec,
					short (*compare )(void *, void *), StringPtr hitFName,
					short maxkeylen);
extern Boolean FindEntryname(char *key, short dbcode, HitmapHdl hitmapHdl);
extern short ename_compare(void *key, void *rec);
extern short general_compare(void *key, void *rec);

/* getpath.c */
extern Boolean GetPathFromDirID(short vRefNum, long DirID, StringPtr fName, char *fullPathName);
extern Boolean GetPathFromWD(short vRefNum, StringPtr fName, char *fullPathName);

/* hitstorage.c */
extern Boolean FillDEBuffer(ResultHdl resHdl, short first, Boolean bCache);
extern Boolean NewHitlist(HitmapHdl hitmapHdl, short dbcode, HitlistHdl *new, short *nhits);
extern Boolean InitResultRec(ResultHdl *new, short dbcode, HitmapHdl hitmapHdl,
					QueryHdl queryHdl);
extern void DisposeResRec(ResultHdl resRecHdl);
extern Boolean GetSelectState(HitlistHdl hlHdl, short pos);
extern void SetSelectState(HitlistHdl hlHdl, short pos, Boolean state);
extern Boolean NewHitmap(HitmapHdl *new, short dbcode);
extern void OrHitmaps(HitmapHdl hmHdl1, HitmapHdl hmHdl2);
extern void AndHitmaps(HitmapHdl hmHdl1, HitmapHdl hmHdl2);
extern void NotHitmap(HitmapHdl hmHdl);

/* info.c */
extern void	ShowAbout(void);
extern void ShowDBInfo(void);

/* locate.c */
extern void Locate(void);
extern Boolean GetDirPath(DirSpec *dirSpec,StringPtr path);

/* main.c */
extern void	main(void);

/* menus.c */
extern void	MaintainMenus(void);
extern void UpdateOtherCreator(void);

/* pref.c */
extern void GetPrefs(void);
extern void WritePrefs(void);
extern Boolean PickNewCreator(void);
extern void GeneralOptions(void);

/* print.c */
extern void PrepPrint(void);
extern void PrintDialog(void);
extern void PrintIt(WDPtr wdp);

/* pstr.c */
extern StringPtr pstrcpy(StringPtr s1, StringPtr s2);
extern StringPtr pstrcat(StringPtr s1, StringPtr s2);
extern short pstrcmp(StringPtr s1, StringPtr s2);
extern short pstrncmp(StringPtr s1, StringPtr s2, short n);
extern short pstrlen(StringPtr s);

/* query.c */
extern Boolean DoQueryEvt(EventRecord theEvent);
extern Boolean NewQueryBlock(short dbcode, QueryHdl *new, short gBooleanOp);
extern Boolean NewQueryBlock(short dbcode, QueryHdl *new, short gBooleanOp);
extern Boolean NewQueryWindow(short w, WDPtr wdp, QueryHdl queryHdl, StringPtr title);
extern void DisposeQuery(WDPtr wdp);
extern void NewQuery(void);
extern void UpdateQueryRec(WDPtr wdp);
extern void OpenQuery(void);
extern Boolean LoadQuery(short w,StringPtr fName, short vRefNum);
extern void DuplicateQuery(WDPtr wdp);

/* results.c */
extern Boolean NewResultWindow(short w, WDPtr wdp, ResultHdl resHdl, StringPtr title);
extern void DisposeResults(WDPtr wdp);
extern void DrawResWinAll(WDPtr wdp, short dummy);
extern void SelectAllResults(WDPtr wdp, Boolean what);
extern void SelectDraw(WDPtr wdp, short i, short what);
extern void AdjustResText(WindowPtr wPtr, short oldvalue, short newvalue,
					scrollBarType which);
extern void DrawResHeader(WDPtr wdp, ResultHdl resHdl);
extern void DoResClicks(WDPtr wdp, EventRecord *eventPtr);
extern Boolean Load1Results(short w, StringPtr fName, short vRefNum);
extern void LoadResults(void);
extern void HideShowResSelections(WDPtr wdp,Boolean show);

/* save.c */
extern Boolean DoSave(WDPtr wdp, Boolean save);

/* sequence.c */
extern Boolean NewSequenceWindow(ResultHdl resHdl, short bufPos);
extern void DisposeSequence(WDPtr wdp);
extern Boolean FillLineBuffer(SeqRecHdl seqRecHdl, short first);
extern void DrawSeqWinAll(WDPtr wdp, short dummy);
extern void AdjustSeqText(WindowPtr wPtr, short oldvalue, short newvalue,
				scrollBarType which);
extern void DoSeqClicks(WDPtr wdp, EventRecord *eventPtr);
extern Boolean SaveSeqData(StringPtr fName,short vRefNum,SeqRecHdl seqRecHdl);
extern void SelectAllSeq(WDPtr wdp,Boolean what);
extern void HideShowSeqSelections(WDPtr wdp, Boolean show);

/* Show_help.c */
extern void Show_help( short help_dlog_id, short help_text_id,
	short base_pict_id, StringPtr default_filename, StringPtr default_menuname );
	
/* util.c */
extern u_long ConvertLong(u_long *val);
extern u_short ConvertShort(u_short *val);
extern char *str2upper(char *str);
extern char *rtrim(char *str);
extern char *ltrim(char *str);
extern char *rpad(char *str, char c, short len);
extern char *compress(char *str);
extern Boolean linetype(char *line, char *id);
extern OSErr DeleteMacFile(StringPtr fName, short vRefNum);
extern OSErr CreateMacFile(StringPtr fName, short vRefNum, OSType creator,
					OSType type, Boolean bShowErrMsg);
extern OSErr OpenMacFile(StringPtr fName, short vRefNum, short *output,
					Boolean bShowErrMsg);
extern OSErr OpenMacFileReadOnly(StringPtr fName, short vRefNum, short *output,
					Boolean bShowErrMsg);
extern OSErr WriteMacFile(short output, long *count, void *what, StringPtr fName,
					Boolean bShowErrMsg);
extern OSErr ReadMacFile(short output, long *count, void *what, StringPtr fName,
					Boolean bShowErrMsg);
extern OSErr FSSpecToHFS(FSSpec *theFSS, short *wdRefNum, StringPtr fName);
extern Boolean GetNewFilename(Str255 oldFName, short *newVRefNum, Str255 newFName);
extern OSErr MyResolveAlias(StringPtr fName, short *wdRefNum);
extern Boolean GetVRefNumFromName(StringPtr vName, short *vRefNum);
extern Boolean GetNameFromVRefNum(StringPtr vName, short vRefNum);
extern GrafPtr ChangePort(GrafPtr newPort);
extern SignedByte LockHandleHigh(Handle theHandle);
extern SignedByte MyHLock(Handle theHandle);
extern void StartWaitCursor(void);
extern void RotateWaitCursor(void);
extern pascal Boolean myDialogFilter(DialogPtr myDialog, EventRecord *theEvent,
					short *itemHit);
extern pascal void DrawOKBoxRect(WindowPtr myWindow, short itemNo);
extern pascal void DrawFrame(WindowPtr myWindow,short itemNo);
extern void SetRadioButton(DialogPtr myDialog, short item, short status);
extern short GetRadioButton(DialogPtr myDialog, short item);
extern void ToggleRadioButton(DialogPtr myDialog, short item);
extern void SelRadioButton(DialogPtr myDialog, short firstItem, short n,
							short whichItem);
extern void SetDlgText(DialogPtr myDialog, short item, StringPtr text);
extern void GetDlgText(DialogPtr myDialog, short item, StringPtr text);
extern void InstallUserItem(DialogPtr myDialog, short item, short useRect,
	pascal void (*userRoutine)(WindowPtr myWindow,short itemNo));
extern void FakeClick(DialogPtr myDialog, short item);
extern Boolean CmdPeriod(EventRecord *theEvent);
extern char *LoadErrorStr(short index, Boolean pas);
extern Boolean ErrorMsg(short index);
extern void FatalErrorMsg(short index);
extern void DoInvertRect(Rect *);
extern StringHandle GetNameStringRsrc(void);
extern OSErr WriteNameStringRsrc(short refNum);

/* window.c */
extern Boolean IsAppWindow(WindowPtr wPtr);
extern Boolean IsDAWindow(WindowPtr wPtr);
extern void InvalBars(WindowPtr wPtr);
extern void ClearWindowRec(WDPtr wdp);
extern void PrepWindows(void);
extern void AdjustPosition(short w, Rect *rect);
extern WDPtr FindMyWindow(WindowPtr wPtr);
extern short GetFreeWindow(void);
extern void DoGrow(WindowPtr wPtr, Point where);
extern void GrowScroll(WDPtr wdp);
extern void DoZoom(WindowPtr wPtr, short partCode);
extern Boolean CloseMyWindow(WDPtr wdp, Boolean shift);
extern Boolean CloseAllWindows(Boolean shift);
extern void SetVScroll(WDPtr wdp);
extern void SetHScroll(WDPtr wdp);
extern void AdjustWSize(short wKind, Rect *r, short height, short width);
extern void DoDrawGrowIcon(WindowPtr wPtr);
extern void GetViewRect(WindowPtr wPtr, Rect *r);
extern void AddWindowToMenu(StringPtr name);
extern void DelWindowFromMenu(StringPtr name);

/* xref.c */
extern Boolean ParseDRLine(SeqRecHdl seqRecHdl, char *line);
extern void AddXRefsToMenu(WDPtr wdp);
extern Boolean OpenXRefs(char *name, short dbcode);