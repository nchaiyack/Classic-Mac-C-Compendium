Boolean IsEqualFSSpec (FSSpec *file1, FSSpec *file2);
Boolean GiveTime (void);
Boolean StatusWindow (char *text);
void UpdateStatus (void);
void CloseStatusWindow (void);
OSErr MyIOCheck (OSErr err);
Ptr MyNewPtr (Size byteCount);
Handle MyNewHandle (Size byteCount);
void MySetHandleSize (Handle h, Size newSize);
OSErr MyHandToHand (Handle *theHndl);
OSErr MyDisposPtr (Ptr thePtr);
OSErr MyDisposHandle (Handle theHndl);
OSErr MyMemErr (void);
Boolean IsAppWindow (WindowPtr wind);
Boolean IsDAWindow (WindowPtr wind);
Boolean IsStatusWindow (WindowPtr wind);
void pstrcpy (StringPtr to, StringPtr from);
short GetPixelDepth (Rect *r);
OSErr VolNameToVRefNum (StringPtr name, short *vRefNum);
Boolean GetFontNumber (Str255 fontName, short *fontNum);

pascal void InitCursorCtl (Handle id);
pascal void SpinCursor (short num);
short strcasecmp (const char *s1, const char *s2);
short strncasecmp (const char *s1, const char *s2, short n);

void SetPortTextStyle(const TextStyle *);
void GetPortTextStyle(TextStyle *);

/*
 * Some useful error-checking macros
 */
#define FailErr(test, label)	if ((err=(test)) != noErr) goto label
#define FailNIL(test, label)	do { \
									if ((test)==nil) { err = MyMemErr(); goto label; } \
								} while (0) /* (nasty trick to require semicolon) */
#define FailMemErr(label)		if ((err=MemError()) != noErr) goto label

