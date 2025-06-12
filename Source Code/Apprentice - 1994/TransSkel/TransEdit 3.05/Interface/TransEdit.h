/*
	TransEdit.h - TransEdit header file
	
	For TransEdit version 3.05.
*/


# ifndef	__TRANSEDIT_H__

# define	__TRANSEDIT_H__


typedef	pascal void (*TEditKeyProcPtr) (void);
typedef	pascal void (*TEditActivateProcPtr) (Boolean);
typedef	pascal void (*TEditCloseProcPtr) (void);


pascal WindowPtr NewEWindow (Rect *bounds, StringPtr title, Boolean visible,
							WindowPtr behind, Boolean goAway,
							long refNum, Boolean bindToFile);
pascal WindowPtr GetNewEWindow (short resourceNum, WindowPtr behind, Boolean bindToFile);
pascal TEHandle GetEWindowTE (WindowPtr wind);
pascal Boolean GetEWindowFile (WindowPtr wind, SFReply *fileInfo);
pascal Boolean IsEWindow (WindowPtr wind);
pascal Boolean IsEWindowDirty (WindowPtr wind);
pascal void SetEWindowProcs (WindowPtr wind, TEditKeyProcPtr pKey,
					TEditActivateProcPtr pActivate, TEditCloseProcPtr pClose);
pascal void SetEWindowStyle (WindowPtr wind, short font,
						short size, short wrap, short just);
pascal void EWindowOverhaul (WindowPtr wind, Boolean showCaret,
						Boolean recalc, Boolean dirty);
pascal void SetEWindowCreator (OSType creat);
pascal Boolean EWindowSave (WindowPtr wind);
pascal Boolean EWindowSaveAs (WindowPtr wind);
pascal Boolean EWindowSaveCopy (WindowPtr wind);
pascal Boolean EWindowClose (WindowPtr wind);
pascal Boolean EWindowRevert (WindowPtr wind);
pascal Boolean ClobberEWindows (void);
pascal void EWindowEditOp (short item);

/* from FakeAlert.h */

pascal short FakeAlert (StringPtr s1, StringPtr s2, StringPtr s3, StringPtr s4,
					short nButtons, short defButton, short cancelButton,
					StringPtr t1, StringPtr t2, StringPtr t3);

# endif /* __TRANSEDIT_H__ */
