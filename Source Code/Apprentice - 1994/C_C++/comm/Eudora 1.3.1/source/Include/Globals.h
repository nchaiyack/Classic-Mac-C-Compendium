#pragma segment Main
/**********************************************************************
 * Global variables for POP mail
 **********************************************************************/
extern int FontID; 						/* font parameters */
extern int FontSize;
extern int FontWidth;					/* width of average char in font */
extern int FontLead;
extern int FontDescent;
extern int FontAscent;
extern Boolean AlertsTimeout;	/* do alerts timeout? */
extern Boolean CommandPeriod;	/* has cmd-. been pressed lately? */
extern Boolean Done; 					/* set to True when we're done processing */
extern Boolean DontTranslate;	/* don't use translation tables */
extern Boolean FontIsFixed;		/* is font fixed-width? */
extern short InBG; 						/* whether or not we're in the background */
extern Boolean NoSaves;				/* don't prompt user to save documents */
extern Boolean SFWTC;					/* Somebody Fooled With The Cursor */
extern Boolean ScrapFull;			/* is there stuff on the scrap? */
extern Boolean UseCTB; 				/* are we using the CTB? */
extern Boolean ExcludeHeaders; /* exclude headers from Save As? */
extern Boolean Paragraphs; 		/* Reparagraph on Save As? */
extern Boolean AmQuitting; 		/* are we quitting? */
extern Boolean BadBinHex;			/* a bad binhex file was found */
extern Boolean WrapWrong;			/* wrap the wrong way */
extern Boolean HasHelp;				/* do we have balloon help? */
extern Boolean UUPCIn; 				/* UUPC mail input */
extern Boolean UUPCOut;				/* UUPC mail output */
extern Boolean HasPM;					/* Do we have the process manager? */
extern Boolean NoAttachments;	/* don't recognize attachments file */
extern Boolean NewTables;			/* using new translation table scheme */
extern BoxCountHandle BoxCount;/* list of mailboxes for find */
extern GrafPtr InsurancePort;	/* a port for use when no others are available */
extern UHandle  RcvBuffer; 		/* stuff for line-oriented net input */
extern ICMPReport ICMPMessage; /* the last such report */
extern MessType **MessList;		/* list of open messages */
extern MyWindow *HandyWindow;	/* spare window record */
extern RgnHandle MouseRgn; 		/* a pen for the mouse */
extern Byte NewLine[4];				/* current newline string */
#ifndef KERBEROS
extern Str31 Password; 				/* our current password */
#endif
extern Str31 SecondPass;				/* secondary password */
extern THPrint PageSetup;			/* our Page Setup */
extern TOCType **TOCList;			/* list of open TOC's */
extern UHandle Aliases;				/* our aliases */
extern UHandle Notes;					/* notes on our aliases */
extern UPtr TransIn; 					/* Translation table for incoming characters */
extern UPtr TransOut;					/* Translation table for outgoing characters */
extern UndoType Undo;					/* stuff for Undo */
extern int CTBTimeout; 				/* current timeout for use with CTB */
extern HostInfoQHandle HIQ;		/* queue of pending DNS lookups */
extern int RcvSize;
extern int RcvSpot;
extern int SendQueue;					/* # of messages waiting to be sent */
extern uLong ForceSend;				/* next delayed queue */
extern long **BoxMap;					/* map of menu id's to dirId's */
extern long AlertTicks;				/* ticks at which to cancel alert */
extern long MyDirId; 					/* dirid of our directory */
extern short **BoxLines; 			/* where the lines go in a mailbox window */
extern short AliasRefCount;		/* how many subsystems are using aliases? */
extern short ICMPAvail;				/* there is an ICMP report available */
extern short MyVRef; 					/* VRefNum of our directory */
extern short RunType;					/* Production, Debugging, or Steve */
extern short SettingsRefN; 		/* refNum of settings file */
extern struct NMRec *MyNMRec;	/* notification record */
extern uLong CheckTicks; 			/* ticks of last mail check */
extern UHandle  AttachedFiles; /* list of attachments saved */
extern MenuHandle CheckedMenu; /* currently checked mailbox menu */
extern short CheckedItem;			/* currently checked mailbox item */
extern Handle SpareSpace;			/* extra memory for emergencies */
extern Boolean EjectBuckaroo;	/* out of memory; die at next opportunity */
extern unsigned short WhyTCPTerminated;				/* why the ctp connection died */
extern short CTBHasChars;
extern ConnHandle CnH;
extern short LogRefN;					/* ref number of open log file */
extern short LogLevel;					/* current logging level */
extern long LogTicks;					/* time log was opened */
extern TOCHandle DamagedTOC;
extern short FakeTabs;		/* cache this pref for performance reasons */
extern struct AddressRange CodeAddress[NUM_CODE];
extern Handle WrapHandle;
extern short ClickType;	/* single, double, or triple */
extern Boolean POPSecure;
extern short DropRefN;				/* maildrop reference number */
extern short SysRefN;				/* system file reference number */
#ifndef SLOW_CLOSE
extern FastPBPtr FastList;	/* connections to be closed */
#endif
#ifdef PERF
extern TP2PerfGlobals ThePGlobals;
#endif
#ifdef DEBUG
extern short BugFlags;
#define BUG0 ((BugFlags&(1<<0))!=0)
#define BUG1 ((BugFlags&(1<<1))!=0)
#define BUG2 ((BugFlags&(1<<2))!=0)
#define BUG3 ((BugFlags&(1<<3))!=0)
#define BUG4 ((BugFlags&(1<<4))!=0)
#define BUG5 ((BugFlags&(1<<5))!=0)
#define BUG6 ((BugFlags&(1<<6))!=0)
#define BUG7 ((BugFlags&(1<<7))!=0)
#define BUG8 ((BugFlags&(1<<8))!=0)
#define BUG9 ((BugFlags&(1<<9))!=0)
#define BUG10 ((BugFlags&(1<<10))!=0)
#define BUG11 ((BugFlags&(1<<11))!=0)
#define BUG12 ((BugFlags&(1<<12))!=0)
#define BUG13 ((BugFlags&(1<<13))!=0)
#define BUG14 ((BugFlags&(1<<14))!=0)
#define BUG15 ((BugFlags&(1<<15))!=0)
extern long SpinSpot;
#endif
extern TransVector CurTrans;
extern TransVector CTBTrans;
extern TransVector TCPTrans;
extern TransVector UUPCTrans;

/**********************************************************************
 * a few temp vars for macros
 **********************************************************************/
extern uLong M_T1, M_T2, M_T3;

