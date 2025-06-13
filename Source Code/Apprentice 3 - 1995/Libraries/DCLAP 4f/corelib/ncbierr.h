/*   ncbierr.h
* ===========================================================================
*
*                            PUBLIC DOMAIN NOTICE
*               National Center for Biotechnology Information
*
*  This software/database is a "United States Government Work" under the
*  terms of the United States Copyright Act.  It was written as part of
*  the author's official duties as a United States Government employee and
*  thus cannot be copyrighted.  This software/database is freely available
*  to the public for use. The National Library of Medicine and the U.S.
*  Government have not placed any restriction on its use or reproduction.
*
*  Although all reasonable efforts have been taken to ensure the accuracy
*  and reliability of the software and data, the NLM and the U.S.
*  Government do not and cannot warrant the performance or results that
*  may be obtained by using this software or data. The NLM and the U.S.
*  Government disclaim all warranties, express or implied, including
*  warranties of performance, merchantability or fitness for any particular
*  purpose.
*
*  Please cite the author in any work or product based on this material.
*
* ===========================================================================
*
* File Name:  ncbierr.c
*
* Author:  Schuler, Sirotkin (UserErr stuff)
*
* Version Creation Date:   9-19-91
*
* $Revision: 2.22 $
*
* File Description:  Error handling functions
*
* Modifications:
* --------------------------------------------------------------------------
* Date      Name        Description of modification
* --------  ----------  -----------------------------------------------------
* 12-10-93  Schuler     Major Revision.  New APIs include:  ErrPostEx, 
*                       ErrSetMessageLevel, ErrSetFatalLevel, 
* ==========================================================================
*/

#ifndef _NCBIERR_
#define _NCBIERR_

#ifdef __cplusplus
extern "C" {
#endif

#include <ncbilcl.h>
#include <ncbistd.h>
#include <ncbimisc.h>


#ifndef THIS_FILE
#define THIS_FILE  __FILE__
#endif
#ifndef THIS_MODULE
#define THIS_MODULE  NULL
#endif

#ifdef _DEBUG
#define DBFLAG 1
#else
#define DBFLAG 0
#endif


#define E_NoMemory      1
#define E_File          2
#define E_FOpen     3
#define E_FRead     4
#define E_FWrite    5
#define E_CdEject   9
#define E_Math          4
#define E_SGML          3
#define E_Programmer  999


struct _ErrDesc;     /** Error Descriptor **/
typedef struct _ErrDesc ErrDesc;
#ifdef DCLAP
typedef const FAR void *ErrDescPtr;
#else
typedef const FAR *ErrDescPtr;
#endif

struct _ErrOpts;      /** Error Options **/
typedef struct _ErrOpts ErrOpts;
#ifdef DCLAP
typedef const FAR void *ErrOptsPtr;
#else
typedef const FAR *ErrOptsPtr;
#endif

struct _ErrMsgRoot;  /** Root of error message tree **/
typedef struct _ErrMsgRoot  ErrMsgRoot;
typedef const ErrMsgRoot FAR *ErrMsgRootPtr;

struct _ErrMsgNode;  /** Node in error message tree **/
typedef struct _ErrMsgNode ErrMsgNode;
typedef const ErrMsgNode FAR *ErrMsgNodePtr;


/* prototype for user-supplied error handler */
typedef int (LIBCALLBACK *ErrHookProc) PROTO((const ErrDesc *err));

/***************************************************************************\
|                           POSTING AN ERROR                                |
\***************************************************************************/

enum _ErrSev { SEV_NONE=0, SEV_INFO, SEV_WARNING, SEV_ERROR, SEV_FATAL };
typedef enum _ErrSev ErrSev;

#define SEV_MIN  SEV_INFO
#define SEV_MAX (SEV_FATAL+1)

void CDECL Nlm_ErrPost VPROTO((int ctx, int code, const char *fmt, ...));
int CDECL Nlm_ErrPostEx VPROTO((ErrSev sev, int lev1, int lev2, const char *fmt, ...));
int LIBCALL Nlm_ErrPostStr PROTO((ErrSev sev, int lev1, int lev2, const char *str));
void CDECL Nlm_ErrLogPrintf VPROTO((const char *fmt, ...));
void LIBCALL Nlm_ErrLogPrintStr PROTO((const char *str));
int LIBCALL Nlm_ErrSetContext PROTO((const char *ctx, const char *fn, int ln, int db));

#define ErrPost	\
	(Nlm_ErrSetContext(THIS_MODULE,THIS_FILE,__LINE__,DBFLAG)) ? (void)0 : \
	Nlm_ErrPost

#define ErrPostEx	\
	(Nlm_ErrSetContext(THIS_MODULE,THIS_FILE,__LINE__,DBFLAG)) ? 0 : \
	Nlm_ErrPostEx

#define ErrPostStr  \
	(Nlm_ErrSetContext(THIS_MODULE,THIS_FILE,__LINE__,DBFLAG)) ? 0 : \
	Nlm_ErrPostStr

#define ErrLogPrintf  Nlm_ErrLogPrintf
#define ErrLogPrintStr Nlm_ErrLogPrintStr

/***************************************************************************\
|                     FETCHING AND REPORTING ERRORS                         |
\***************************************************************************/
#define ERRTEXT_MAX  512
#define CODESTR_MAX  64
#define MODSTR_MAX   32
#define SRCFILE_MAX  92

struct _ErrDesc 
{
	short severity;
	short context;  /* OBSOLETE */
	char  module[MODSTR_MAX];
	int   errcode;
	int   subcode;
	char  codestr[CODESTR_MAX];
	char  srcfile[SRCFILE_MAX];
	int   srcline;
	char  errtext[ERRTEXT_MAX];
	const ValNode *userstr;
	const ErrMsgRoot *root;
	const ErrMsgNode *node;
};

int LIBCALL Nlm_ErrFetch PROTO((ErrDesc FAR *err));
int LIBCALL Nlm_ErrCopy PROTO((ErrDesc FAR *err));
void LIBCALL Nlm_ErrClear PROTO((void));
int LIBCALL Nlm_ErrShow PROTO((void));

#define ErrShow     Nlm_ErrShow
#define ErrFetch    Nlm_ErrFetch
#define ErrClear    Nlm_ErrClear
#define ErrCopy     Nlm_ErrCopy
#define ErrPeek()   Nlm_ErrCopy(NULL)


/***************************************************************************\
|                           MESSAGE FILE SUPPORT                            |
\***************************************************************************/
struct _ErrMsgRoot
{ 
	ErrMsgRoot *next;
	ErrMsgNode *list;
	const char *name;
	/*unsigned int busy :1;*/
	/*unsigned int not_avail :1;*/
};

struct _ErrMsgNode
{ 
	ErrMsgNode *next;
	ErrMsgNode *list;
	const char *name;
	int code;
	int sev;
	long cofs, clen;
	long tofs, tlen;
	const char *cstr;	
	const char *tstr;
};

/** Don't use these functions **/
ErrMsgRootPtr LIBCALL ErrGetMsgRoot PROTO((const char *context));
const char* LIBCALL ErrGetExplanation PROTO((ErrMsgRootPtr idx, ErrMsgNodePtr item));

/***************************************************************************\
|                             CUSTOMIZATION                                 |
\***************************************************************************/
struct _ErrOpts
{
	unsigned long flags;
	short log_level;
	short msg_level;
	short die_level;
	short actopt;
	short logopt;
};

typedef Nlm_Uint1 ErrStrId;

int LIBCALL Nlm_ErrSetLogfile PROTO((const char *filename, unsigned long flags));
ErrHookProc LIBCALL Nlm_ErrSetHandler PROTO((ErrHookProc));
ErrStrId LIBCALL Nlm_ErrUserInstall PROTO((const char *msg, ErrStrId magic_cookie));
Nlm_Boolean LIBCALL Nlm_ErrUserDelete PROTO((ErrStrId magic_cookie));
void LIBCALL Nlm_ErrUserClear PROTO((void));
int LIBCALL ErrSetLogLevel PROTO((ErrSev level));
int LIBCALL ErrGetLogLevel PROTO((void));
int LIBCALL ErrSetMessageLevel PROTO((ErrSev level));
int LIBCALL ErrGetMessageLevel PROTO((void));
int LIBCALL ErrSetFatalLevel PROTO((ErrSev level));
int LIBCALL ErrGetFatalLevel PROTO((void));
unsigned long LIBCALL ErrSetOptFlags PROTO((unsigned long flags));
unsigned long LIBCALL ErrClearOptFlags PROTO((unsigned long flags));
unsigned long LIBCALL ErrTestOptFlags PROTO((unsigned long flags));
void LIBCALL ErrSaveOptions PROTO((ErrOpts *opts)); 
void LIBCALL ErrRestoreOptions PROTO((const ErrOpts *opts));

/* Error Option (EO) flags */
#define EO_LOG_SEVERITY  0x00000001L
#define EO_LOG_CODES     0x00000002L
#define EO_LOG_FILELINE  0x00000004L
#define EO_LOG_USERSTR   0x00000008L
#define EO_LOG_ERRTEXT   0x00000010L
#define EO_LOG_MSGTEXT   0x00000020L
#define EO_MSG_SEVERITY  0x00000100L
#define EO_MSG_CODES     0x00000200L
#define EO_MSG_FILELINE  0x00000400L
#define EO_MSG_USERSTR   0x00000800L
#define EO_MSG_ERRTEXT   0x00001000L
#define EO_MSG_MSGTEXT   0x00002000L
#define EO_LOGTO_STDOUT  0x00010000L
#define EO_LOGTO_STDERR  0x00020000L
#define EO_LOGTO_TRACE   0x00040000L
#define EO_LOGTO_USRFILE 0x00080000L
#define EO_XLATE_CODES   0x01000000L
#define EO_WAIT_KEY      0x02000000L
#define EO_PROMPT_ABORT  0x04000000L
#define EO_BEEP          0x08000000L

#define EO_ALL_FLAGS     0x0F0F3F3FL

#define EO_SHOW_SEVERITY (EO_LOG_SEVERITY | EO_MSG_SEVERITY)
#define EO_SHOW_CODES    (EO_LOG_CODES | EO_MSG_CODES)
#define EO_SHOW_FILELINE (EO_LOG_FILELINE | EO_MSG_FILELINE)
#define EO_SHOW_USERSTR  (EO_LOG_USERSTR | EO_MSG_USERSTR)
#define EO_SHOW_ERRTEXT  (EO_LOG_ERRTEXT | EO_MSG_ERRTEXT)
#define EO_SHOW_MSGTEXT  (EO_LOG_MSGTEXT | EO_MSG_MSGTEXT)

#define EO_DEFAULTS      (EO_SHOW_SEVERITY | EO_LOG_CODES | EO_XLATE_CODES | \
				EO_SHOW_USERSTR | EO_SHOW_ERRTEXT)

/* flags for ErrSetLogfile */
#define ELOG_APPEND		 0x00000001
#define ELOG_BANNER      0x00000002
#define ELOG_NOCREATE    0x00000004

#define ErrSetHandler Nlm_ErrSetHandler
#define ErrUserInstall Nlm_ErrUserInstall
#define ErrUserDelete  Nlm_ErrUserDelete
#define ErrUserClear   Nlm_ErrUserClear
#define ErrSetLogfile   Nlm_ErrSetLogfile
#define ErrSetLog(x)  Nlm_ErrSetLogfile((x),ELOG_BANNER|ELOG_APPEND)

/***************************************************************************\
|                               DEBUGGING                                   |
\***************************************************************************/
void LIBCALL Nlm_AssertionFailed PROTO((const char*, const char*,const char*,int));
void LIBCALL Nlm_TraceStr PROTO((const char*));
void CDECL   Nlm_Trace VPROTO((const char*, ...));
void LIBCALL Nlm_AbnormalExit PROTO((int));

#define AbnormalExit  Nlm_AbnormalExit

#ifdef _DEBUG

#ifndef TRACE                        
#define TRACE	Nlm_Trace
#endif
#ifndef ASSERT
#define ASSERT(expr)  ((expr) ? \
	(void)0 : Nlm_AssertionFailed(#expr,THIS_MODULE,THIS_FILE,__LINE__))
#endif
#ifndef VERIFY
#define VERIFY(expr)  ASSERT(expr)
#endif

#else /* ! _DEBUG */

#ifndef TRACE
#define TRACE	1 ? (void)0 : Nlm_Trace
#endif
#ifndef ASSERT
#define ASSERT(expr)	((void)0)
#endif
#ifndef VERIFY
#define VERIFY(expr)	((void)(expr))
#endif

#endif  /* ! _DEBUG */

/********************* OBSOLETE STUFF BELOW THIS LINE **********************/

/* actopt codes */
#define ERR_CONTINUE  1
#define ERR_IGNORE    2
#define ERR_ADVISE    3
#define ERR_ABORT     4
#define ERR_PROMPT    5
#define ERR_TEE       6

/* logopt codes */
#define ERR_LOG_ON    1
#define ERR_LOG_OFF   2

void LIBCALL Nlm_ErrGetOpts PROTO((short * actopt, short * logopt));
void LIBCALL Nlm_ErrSetOpts PROTO((short actopt, short logopt));
#define ErrGetOpts  Nlm_ErrGetOpts
#define ErrSetOpts  Nlm_ErrSetOpts

/* error context codes */
#define CTX_DEBUG      0
#define CTX_UNKNOWN    1
#define CTX_ERRNO      2
#define CTX_NCBICORE   64
#define CTX_NCBIASN1   65
#define CTX_NCBICD	   66
#define CTX_NCBIOBJ    67
#define CTX_NCBILMA    68
#define CTX_NCBIGBPARSE 69
#define CTX_NCBIPIRPARSE 70
#define CTX_NCBI2GB    71
#define CTX_NCBIBB2ASN 72
#define CTX_NCBIMATH   73
#define CTX_NCBIMED2ASN 74
#define CTX_NCBISEQENTRY 75
#define CTX_NCBISEQPORT   200
#define CTX_NCBIIDLOAD 300
#define CTX_NCBIIDPROCESS 301
#define CTX_NCBIIDRETRIEVE 302
#define CTX_NCBIAUTHINPARSE 303
#define CTX_KB2ASN 304

#define CTX_RESERVED   10000
/* context codes > 10000 are available for application use */
#define CTX_USER	   32767    /* default user application context */

/* error codes for CTX_NCBICORE */
#define CORE_UNKNOWN    1
#define CORE_NULLPTR    2    /* NULL pointer passed as an argument */
#define CORE_NULLHND    3    /* NULL handle passed as an argument */
#define CORE_MEMORY     4    /* Memory allocation failure */
#define CORE_BAD_COOKIE 5    /* ErrPost User install misused */
#define CORE_BAD_SGML   6    /* SGML entity or range error */

#define CORE_FILE_ACCESS  10  /* Error accessing file */
#define CORE_FILE_CREATE  11  /* Error creating file */
#define CORE_FILE_OPEN    12  /* Error opening file */
#define CORE_FILE_READ    13  /* Error reading file */
#define CORE_FILE_WRITE   14  /* Error writing file */
#define CORE_FILE_ERROR   15  /* any other file I/O error */

#define ERRPOST(x)	\
	Nlm_ErrSetContext(THIS_MODULE,THIS_FILE,__LINE__,DBFLAG), \
	Nlm_ErrPost x

#define Nlm_ErrSetLog(x)  Nlm_ErrSetLogfile((x),ELOG_BANNER|ELOG_APPEND)

#define EO_LOG_STDOUT  EO_LOGTO_STDOUT
#define EO_LOG_STDERR  EO_LOGTO_STDERR
#define EO_LOG_TRACE   EO_LOGTO_TRACE 
#define EO_LOG_USRFILE EO_LOGTO_USRFILE

#ifdef __cplusplus
}
#endif

#endif
