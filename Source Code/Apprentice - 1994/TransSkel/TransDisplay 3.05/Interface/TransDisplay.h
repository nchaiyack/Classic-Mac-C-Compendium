/*
 * TransDisplay.h - TransDisplay header file
 *	
 * For TransDisplay version 3.05.
 */

# ifndef	__TRANSDISPLAY_H__

# define	__TRANSDISPLAY_H__


typedef	pascal void (*TDispActivateProcPtr) (Boolean);

/*
 * Function prototypes
 */

pascal WindowPtr NewDWindow (Rect *bounds, StringPtr title, Boolean visible,
					WindowPtr behind, Boolean goAway, long refCon);
pascal WindowPtr GetNewDWindow (short resourceNum, WindowPtr behind);
pascal Boolean IsDWindow (WindowPtr wind);
pascal TEHandle GetDWindowTE (WindowPtr wind);
pascal void SetDWindowStyle (WindowPtr wind,
						short font,
						short size,
						short wrap,
						short just);
pascal void SetDWindowPos (WindowPtr wind, short lineNum);
pascal void SetDWindowNotify (WindowPtr wind, TDispActivateProcPtr p);
pascal void SetDWindowFlush (WindowPtr wind, long maxText, long flushAmt);
pascal void SetDWindow (WindowPtr wind);
pascal WindowPtr GetDWindow (void);
pascal void FlushDWindow (WindowPtr wind, long byteCount);

pascal void	DisplayText (Ptr text, long len);
pascal void	DisplayString (StringPtr str);
pascal void	DisplayCString (char *str);
pascal void	DisplayLong (long l);
pascal void	DisplayShort (short i);
pascal void	DisplayChar (short c);
pascal void	DisplayLn (void);
pascal void	DisplayBoolean (Boolean b);
pascal void	DisplayHexChar (short c);
pascal void	DisplayHexShort (short i);
pascal void	DisplayHexLong (long l);
pascal void	DisplayOSType (OSType type);

/* DisplayInt() and DisplayHexInt() are deprecated and will disappear */

# define	DisplayInt		DisplayShort
# define	DisplayHexInt	DisplayHexShort

# endif /* __TRANSDISPLAY_H__ */