/*
 * killdata.h
 *
 * Internal data types and variables used by the Kill File system
 */


/* EKillAction
 * What to do when an article is matched by a killfile entry
 */
typedef enum { kKill=1, kKeep, kHighlight } EKillAction;

/* TKillEntry
 * One entry in the killfile: a string and what to match it with
 */
typedef struct {
	char			**string;		/* String to search for */
	short			header;			/* Which header to search in */
	EMatchType		match;			/* What type of string matching */
	EKillAction		action;			/* What to do with matches */
	unsigned long	expires;		/* When this entry expires */
	short			highlight;		/* Highlight color */
	Boolean			ignoreCase;		/* Ignore case of strings? */
} TKillEntry;

/* TGroupKill
 * Holds the killfile info for one group.
 * A list of entries, plus how a boolean telling how to combine them.
 */
typedef struct {
	Handle			groupName;		/* Handle to the group name */
	TKillEntry		**entries;		/* Handle to the entries */
	short			numEntries;		/* # of kill entries */
	Boolean			noMatchKill;	/* TRUE: articles not matching one
										of the patterns are killed */
} TGroupKill, **TGroupKillHdl;

/* gKillFile
 * This holds all of the killfile info at run time.
 * It is a handle to an array which in turn contains
 * handles to the individual TGroupKill objects.
 */
extern TGroupKillHdl **gKillFile;

/*-------------------------------------------------------------------------
 * Internal routines
 */
 
/*
 * These are "constructors" and "destructors" for the above data types
 */
OSErr			InitKillEntry(TKillEntry *entry, const char *str);
OSErr			CopyKillEntry(const TKillEntry *from, TKillEntry *to);
void			DelKillEntry(TKillEntry *entry);

TGroupKillHdl	NewGroupKill(char *groupName);
TGroupKillHdl	CloneGroupKill(TGroupKillHdl aKill);
void			DelGroupKill(TGroupKillHdl aKill);

TGroupKillHdl	FindGroupKill(const char *groupName, short *index);
OSErr			StoreGroupKill(TGroupKillHdl newKill);

/*
 * Routines for converting from header name to a # and back
 */
short			KillHeaderNum(const char *);
char *			KillHeaderName(short);

#define kNumKillHeaders 9	/* Max # returned by KillHeaderNum */
#define kKillSubject	0	/* Header # corresponding to Subject */

void			FindGroupName(WindowPtr wind, CStr255 name);

