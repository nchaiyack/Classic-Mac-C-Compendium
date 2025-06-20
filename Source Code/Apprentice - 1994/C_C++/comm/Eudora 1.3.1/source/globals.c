/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Main
/**********************************************************************
 * Global variables for POP mail
 **********************************************************************/
int FontID; 						/* font parameters */
int FontSize;
int FontWidth;					/* width of average char in font */
int FontLead;
int FontDescent;
int FontAscent;
Boolean AlertsTimeout;	/* do alerts timeout? */
Boolean CommandPeriod;	/* has cmd-. been pressed lately? */
Boolean Done; 					/* set to True when we're done processing */
Boolean DontTranslate;	/* don't use translation tables */
Boolean FontIsFixed;		/* is font fixed-width? */
short InBG; 						/* whether or not we're in the background */
Boolean NoSaves;				/* don't prompt user to save documents */
Boolean SFWTC;					/* Somebody Fooled With The Cursor */
Boolean ScrapFull;			/* is there stuff on the scrap? */
Boolean UseCTB; 				/* are we using the CTB? */
Boolean ExcludeHeaders; /* exclude headers from Save As? */
Boolean Paragraphs; 		/* Reparagraph on Save As? */
Boolean AmQuitting; 		/* are we quitting? */
Boolean BadBinHex;			/* a bad binhex file was found */
Boolean WrapWrong;			/* wrap the wrong way */
Boolean HasHelp;				/* do we have balloon help? */
Boolean UUPCIn; 				/* UUPC mail input */
Boolean UUPCOut;				/* UUPC mail output */
Boolean HasPM;					/* Do we have the process manager? */
Boolean NoAttachments;	/* don't recognize attachments file */
Boolean NewTables;			/* using new translation table scheme */
BoxCountHandle BoxCount;/* list of mailboxes for find */
GrafPtr InsurancePort;	/* a port for use when no others are available */
UHandle  RcvBuffer; 		/* stuff for line-oriented net input */
ICMPReport ICMPMessage; /* the last such report */
MessType **MessList;		/* list of open messages */
MyWindow *HandyWindow;	/* spare window record */
RgnHandle MouseRgn; 		/* a pen for the mouse */
Byte NewLine[4];				/* current newline string */
#ifndef KERBEROS
Str31 Password; 				/* our current password */
#endif
Str31 SecondPass;				/* secondary password */
THPrint PageSetup;			/* our Page Setup */
TOCType **TOCList;			/* list of open TOC's */
UHandle Aliases;				/* our aliases */
UHandle Notes;					/* notes on our aliases */
UPtr TransIn; 					/* Translation table for incoming characters */
UPtr TransOut;					/* Translation table for outgoing characters */
UndoType Undo;					/* stuff for Undo */
int CTBTimeout; 				/* current timeout for use with CTB */
HostInfoQHandle HIQ;		/* queue of pending DNS lookups */
int RcvSize;
int RcvSpot;
int SendQueue;					/* # of messages waiting to be sent */
uLong ForceSend;				/* next delayed queue */
long **BoxMap;					/* map of menu id's to dirId's */
long AlertTicks;				/* ticks at which to cancel alert */
long MyDirId; 					/* dirid of our directory */
short **BoxLines; 			/* where the lines go in a mailbox window */
short AliasRefCount;		/* how many subsystems are using aliases? */
short ICMPAvail;				/* there is an ICMP report available */
short MyVRef; 					/* VRefNum of our directory */
short RunType;					/* Production, Debugging, or Steve */
short SettingsRefN; 		/* refNum of settings file */
struct NMRec *MyNMRec;	/* notification record */
uLong CheckTicks; 			/* ticks of last mail check */
UHandle  AttachedFiles; /* list of attachments saved */
MenuHandle CheckedMenu; /* currently checked mailbox menu */
short CheckedItem;			/* currently checked mailbox item */
Handle SpareSpace;			/* extra memory for emergencies */
Boolean EjectBuckaroo;	/* out of memory; die at next opportunity */
unsigned short WhyTCPTerminated;				/* why the ctp connection died */
short CTBHasChars;
ConnHandle CnH;
short LogRefN;					/* ref number of open log file */
short LogLevel;					/* current logging level */
long LogTicks;					/* time log was opened */
TOCHandle DamagedTOC;
short FakeTabs;		/* cache this pref for performance reasons */
struct AddressRange CodeAddress[NUM_CODE];
Handle WrapHandle;
short ClickType;	/* single, double, or triple */
Boolean POPSecure;
short DropRefN;				/* maildrop reference number */
short SysRefN;				/* system file reference number */
#ifndef SLOW_CLOSE
FastPBPtr FastList;	/* connections to be closed */
#endif
#ifdef PERF
TP2PerfGlobals ThePGlobals;
#endif
#ifdef DEBUG
short BugFlags;
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
long SpinSpot;
#endif
TransVector CurTrans;
TransVector CTBTrans = {CTBConnectTrans,CTBSendTrans,CTBRecvTrans,CTBDisTrans,CTBDestroyTrans,CTBTransError,CTBSilenceTrans,GenSendWDS,CTBWhoAmI,NetRecvLine};
TransVector TCPTrans = {TCPConnectTrans,TCPSendTrans,TCPRecvTrans,TCPDisTrans,TCPDestroyTrans,TCPTransError,TCPSilenceTrans,TCPSendWDS,TCPWhoAmI,NetRecvLine};
TransVector UUPCTrans = {nil,UUPCSendTrans,nil,UUPCDry,UUPCDry,nil,nil,GenSendWDS,nil,UUPCRecvLine};

/**********************************************************************
 * a few temp vars for macros
 **********************************************************************/
uLong M_T1, M_T2, M_T3;

