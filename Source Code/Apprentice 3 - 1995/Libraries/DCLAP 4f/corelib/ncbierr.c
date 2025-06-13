/*   ncbierr.c
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
* $Revision: 2.32 $
*
* File Description:  Error handling functions
*
* Modifications:
* --------------------------------------------------------------------------
* Date     Name        Description of modification
* -------  ----------  -----------------------------------------------------
* 04-13-93 Schuler     Added TRACE, VERIFY, ASSERT macros.
* 04-13-93 Schuler     Added AbnormalExit function.
* 05-11-93 Schuler     Added ErrSetHandler function.
* 05-21-93 Schuler     Remove PROTO from ErrSetHandler implementation (oops)
* 05-26-93 Schuler     Nlm_TraceStr flushes stderr, if appropriate
* 07-26-93 Schuler     Moved globals into a (per-app) context struct
* 07-26-93 Schuler     Fixed ErrClear()
* 11-15-93 Schuler     Fixed double error reporting problem in ErrPost()
* 12-13-93 Schuler     Message file support and lots of other changes.
* 12-22-93 Schuler     Added log_level setting (min. severity to log)
* 01-03-94 Schuler     Added ErrSaveOptions and ErrRestoreOptions
* 01-04-94 Schuler     Added code to get settings from NCBI config file
* 01-07-94 Schuler     Fixed bug in ErrMsgRoot_Read().
* 01-10-94 Schuler     Added check for not_avail in ErrMsgRoot_fopen()
* 01-10-94 Schuler     Fixed bug in ErrGetMsgRoot()
* 01-13-94 Schuler     Added an handful of typecasts to supress warnings
* 01-23-94 Schuler     Fixed bug in ErrSetOpts
* 02-02-94 Schuler     ErrOpt structure has fields for actopt, logopt
* 02-02-94 Schuler     Revisions related to change in message file format
* 02-02-94 Schuler     Use TEST_BITS macro throughout
* 02-02-94 Schuler     Improved back.compatability of ErrGetOpts/ErrSetOpts
* 02-10-94 Schuler     Workaround for obsolete ERR_IGNORE option
* 02-10-94 Schuler     Fixed bug in ErrSetFatalLevel
* 02-18-94 Schuler     Fix to deal with possibility of userstrings being NULL
* ==========================================================================
*/


#include <ncbi.h>
#include <ncbiwin.h>

char * g_corelib = "CoreLib";
#undef  THIS_MODULE
#define THIS_MODULE g_corelib

static char *_filename = __FILE__;
#undef  THIS_FILE
#define THIS_FILE _filename

#ifdef VAR_ARGS
#include <varargs.h>
#define VSPRINTF(buff,fmt)         \
	{                              \
		va_list args;              \
		va_start(args);            \
		vsprintf(buff,fmt,args);   \
		va_end(args);              \
	}

#else
#include <stdarg.h>
#define VSPRINTF(buff,fmt)         \
	{                              \
		va_list args;              \
		va_start(args,fmt);        \
		vsprintf(buff,fmt,args);   \
		va_end(args);              \
	}
#endif


struct AppErrInfo
{
	ErrDesc desc;
	ErrOpts opts;
	ErrHookProc	hook;
	unsigned long ini_mask;
	unsigned long ini_bits;
	unsigned int any_error :1;
	unsigned int err_formatted :1;
	unsigned int debug_mode :1;
	Nlm_sizeT ustrcnt;
	Nlm_sizeT ustrlen;
	char  logfile[PATH_MAX];
	char  msgpath[PATH_MAX];
	ErrMsgRoot *idxlist;
	ErrMsgNode *node;
};

typedef struct AppErrInfo *AppErrInfoPtr;


static char * _szPropKey = "_AppErrInfo";
static char * _szSevKey [] = { "", "SEV_INFO", "SEV_WARNING", "SEV_ERROR", "SEV_FATAL" };
static char * _szSevDef [] = { "", "NOTE:", "WARNING:", "ERROR:", "FATAL ERROR:" };
static char * _szSeverity[5];
static char _busy;

char *GetScratchBuffer PROTO((void));
static AppErrInfoPtr GetAppErrInfo PROTO((void));

#define FUSE_BITS(inf)  ( ((inf)->ini_bits & (inf)->ini_mask) | ((inf)->opts.flags & ~((inf)->ini_mask)) )
#define TEST_BITS(inf,x)  (FUSE_BITS(inf) & (x))



/***************************************************************************\
|                           POSTING AN ERROR                                |
\***************************************************************************/

/*-------------------------------------------------------------------------
* ErrPost   [Schuler]
*
* MODIFICATIONS:
* 04-13-93 Schuler  
* 07-26-93 Schuler   Modified to use AppErrInfo struct
* 11-15-93 Schuler   Fixed bug that resulted in reporting errors twice
* 12-14-93 Schuler   Modified to call the new ErrPostStr function
* 01-14-94 Schuler   Check _busy flag
*/
 
#ifdef VAR_ARGS
void CDECL  Nlm_ErrPost (context, errcode, fmt, va_alist)
  int context;
  int errcode;
  const char *fmt;
  va_dcl
#else
void CDECL Nlm_ErrPost (int context, int errcode, const char *fmt, ...)
#endif
{
	AppErrInfoPtr info = GetAppErrInfo();
	ErrSev sev = (context==CTX_DEBUG) ? SEV_INFO : SEV_FATAL;
	if (_busy)
	{
		if (sev == SEV_FATAL)  AbnormalExit(1);
		return;
	}
	info->desc.context = context;
	VSPRINTF(info->desc.errtext,fmt);
	info->err_formatted =1;
	(void)Nlm_ErrPostStr(sev,errcode,0,NULL);
}


/*-------------------------------------------------------------------------
*	ErrPostEx   [Schuler, 12-13-93]
*
* MODIFICATIONS:
* 01-14-94 Schuler   Check _busy flag
*/

#ifdef VAR_ARGS
int CDECL  Nlm_ErrPostEx (sev, lev1, lev2, fmt, va_alist)
  ErrSev sev;
  int lev1;
  int lev2;
  const char *fmt;
  va_dcl
#else
int CDECL Nlm_ErrPostEx (ErrSev sev, int lev1, int lev2, const char *fmt, ...)
#endif
{
	AppErrInfoPtr info = GetAppErrInfo();
	if (_busy)
	{
		if (sev == SEV_FATAL)  AbnormalExit(1);
		return 0;
	}
	VSPRINTF(info->desc.errtext,fmt);
	info->err_formatted =1;
	return Nlm_ErrPostStr(sev,lev1,lev2,NULL);
}


/*-------------------------------------------------------------------------
* ErrPostStr   [Schuler, 12-13-93]
*
* MODIFICATIONS:
* 12-22-93 Schuler   Only logs error if severity >= log_level
* 01-14-94 Schuler   Check _busy flag
* 02-03-94 Schuler   Use severity from message file if there is one
* 02-10-94 Schuler   Workaround for obsolete ERR_IGNORE option
* 02-18-94 Schuler   Check for NULL user strings before printing
*/

int LIBCALL Nlm_ErrPostStr (ErrSev sev, int lev1, int lev2, const char *str)
{
	AppErrInfoPtr info = GetAppErrInfo();
	ErrMsgRootPtr root  = NULL;
	ErrMsgNodePtr node1 = NULL;
	ErrMsgNodePtr node2 = NULL;
	int severity = sev;
	
	if (_busy)
	{
		if (sev == SEV_FATAL)  AbnormalExit(1);
		return 0;
	}

	/* ----- Fill in the fields of the error descriptor ----- */
	info->any_error = 1;
	info->desc.severity = sev;
	info->desc.errcode = lev1;
	info->desc.subcode = lev2;

	/* ----- set up the root and node for message file ----- */
	if ( (info->desc.module[0] != '\0') )
	{
		root = ErrGetMsgRoot(info->desc.module);
		for (node1=root->list; node1; node1=node1->next)
		{
			if (node1->code == info->desc.errcode)
			{
				for (node2=node1->list; node2; node2=node2->next)
					if (node2->code == info->desc.subcode)
						break;
				break;
			}
		}
	}
	info->desc.root = root;
	info->desc.node = node2 ? node2 : node1;
	if (info->desc.node != NULL && info->desc.node->sev != SEV_NONE)
		severity = info->desc.node->sev;

	/* ----- format some strings ----- */
	if (!info->err_formatted)
	{
		info->desc.errtext[0] = '\0';
		if (str != NULL)
			strncat(info->desc.errtext,str,ERRTEXT_MAX);
	}
	if (info->desc.context != 0)
		sprintf(info->desc.codestr,"[%03d:%03d] ",info->desc.context,info->desc.errcode);
	else if (node1 != NULL && (TEST_BITS(info,EO_XLATE_CODES)))
	{
		if (node2 != NULL)
			sprintf(info->desc.codestr,"[%s.%s] ",node1->name,node2->name);
		else
			sprintf(info->desc.codestr,"[%s] ",node1->name);
	}
	else
	{
		sprintf(info->desc.codestr,"[%03d.%03d] ",info->desc.errcode,info->desc.subcode);
	}


	/* ----- Log the error according to the current options ----- */
	if (severity >= info->opts.log_level)
	{
		
#ifdef WIN_DUMB
		if (TEST_BITS(info,EO_LOGTO_STDOUT))
			fflush(stderr);
		if (TEST_BITS(info,EO_LOGTO_STDERR))
			fflush(stdout);
#endif

		if (TEST_BITS(info,EO_LOG_SEVERITY))
		{
			ErrLogPrintf("%s ",_szSeverity[severity]);
		}
		if (TEST_BITS(info,EO_LOG_CODES))
		{
			if (info->desc.module[0])
				ErrLogPrintf("%s ",info->desc.module);
			ErrLogPrintStr(info->desc.codestr);
		}
		if (TEST_BITS(info,EO_LOG_FILELINE))
		{
			ErrLogPrintf("{%s, line %d} ",info->desc.srcfile,info->desc.srcline);
		}
		if (TEST_BITS(info,EO_LOG_USERSTR))
		{
			const ValNode *node;
			for (node=info->desc.userstr; node; node=node->next)
			{
				if (node->data.ptrvalue != NULL)
					ErrLogPrintf("%s ",(char*)node->data.ptrvalue);
			}
		}
		if (TEST_BITS(info,EO_LOG_ERRTEXT))
		{
			ErrLogPrintStr(info->desc.errtext);
			ErrLogPrintStr("\n");
		}
		if (node1 != NULL && TEST_BITS(info,EO_LOG_MSGTEXT))
		{
			ErrMsgNodePtr node = (node2==NULL) ? node1 : node2;
			const char *text = ErrGetExplanation(root,node);
			ErrLogPrintStr(text);
		}
	}

	/* ----- Workaround for obsolete ERR_IGNORE option ----- */
	if (info->opts.actopt == ERR_IGNORE)
	{
		ErrClear();
		return ANS_NONE;
	}

	/* ----- Call the user-installed hook function if there is one ----- */
	if (info->hook != NULL)
	{
		int retval;
		if ((retval = (*info->hook)(&(info->desc))) != 0)
		{
			ErrClear();
			return retval;		
		}
	}

	/* ----- If not already handled, perform default error handling ----- */
	if (severity >= info->opts.msg_level || severity >= info->opts.die_level)
		return ErrShow();
	
	return ANS_OK;
}

/*-------------------------------------------------------------------------
*	ErrLogPrintf   [Schuler, 12-13-93]
*
*  Formats the string and passes it along to ErrLogPrintStr().
*/
#ifdef VAR_ARGS
void CDECL Nlm_ErrLogPrintf (fmt, va_alist)
  const char *fmt;
  va_dcl
#else
void CDECL Nlm_ErrLogPrintf (const char *fmt, ...)
#endif
{
	char *buffer = GetScratchBuffer();
	VSPRINTF(buffer,fmt);
	ErrLogPrintStr(buffer);
}

/*-------------------------------------------------------------------------
* ErrLogPrintStr   [Schuler, 12-13-93]
*
* Sends a string of text to whichever output streams have been enabled
* for error logging (stderr, trace, or logfile).
*
* MODIFICATIONS
* 12-15-93 Schuler   Added fflush(stdout) before writing to stderr.
*/
void LIBCALL Nlm_ErrLogPrintStr (const char *str)
{
	AppErrInfoPtr info = GetAppErrInfo();
                                                   
	if (str != NULL)
	{
		size_t bytes = strlen(str);
		if (bytes > 0)
		{
			if (TEST_BITS(info,EO_LOGTO_STDOUT))
			{
				fflush(stderr);
				fwrite(str,1,bytes,stdout);
				fflush(stdout);
			}
			if (TEST_BITS(info,EO_LOGTO_STDERR))
			{
				fflush(stdout);
				fwrite(str,1,bytes,stderr);
				fflush(stderr);
			}
			if (TEST_BITS(info,EO_LOGTO_TRACE))
			{
				Nlm_TraceStr(str);
			}
			if (TEST_BITS(info,EO_LOGTO_USRFILE))
			{
				FILE *fd = FileOpen(info->logfile,"a+");
				if (fd != NULL)
				{
					fwrite(str,1,bytes,fd);
					FileClose(fd);
				}
			}
		}
	}
}

/*-------------------------------------------------------------------------
*	ErrSetContext   [Schuler, 12-13-93]
*
*  NOTE: Don't call this function directly 
*
* MODIFICATIONS
* 02-03-94 Schuler   Return 1 if busy
*/
int LIBCALL Nlm_ErrSetContext (const char *ctx, const char *fname, int line, int db)
{
	AppErrInfoPtr info;
	
	if (_busy)  return 1;
	info = GetAppErrInfo();
	
	info->desc.module[0] = '\0';
	if (ctx != NULL)
		strncat(info->desc.module,ctx,MODSTR_MAX);
	info->desc.srcfile[0] = '\0';
	if (fname != NULL)
	{
		const char *p;
		if ((p = strrchr(fname,DIRDELIMCHR)) != NULL)
			++p;
		else
			p = fname;
		strncat(info->desc.srcfile,p,SRCFILE_MAX);
	}
	info->desc.srcline = line;
	info->debug_mode = (unsigned)db;
	return 0;
}


/***************************************************************************\
|                     FETCHING AND REPORTING ERRORS                         |
\***************************************************************************/

/*-------------------------------------------------------------------------
 * ErrFetch  [Schuler]
 *
 * MODIFICATIONS
 * 07-26-93 Schuler   Modified to use AppErrInfo struct
 */
int LIBCALL  Nlm_ErrFetch (ErrDesc *err)
{
	if (!Nlm_ErrCopy(err))
		return FALSE;
   	ErrClear();
	return TRUE;
}


/*-------------------------------------------------------------------------
 * ErrCopy  [Schuler, 07-26-93]
 *
 * MODIFICATIONS:
 * 12-12-93 Schuler   Check info->any_error flag before copying
 */
int LIBCALL  Nlm_ErrCopy (ErrDesc FAR *err)
{
	AppErrInfoPtr info = GetAppErrInfo();
	if (info->any_error)
	{
		if (err != NULL)
			memcpy((void*)err,(void*)&(info->desc),sizeof(ErrDesc));
		return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------------------------------
 * ErrClear   [Schuler, 07-26-93]
 *
 * MODIFICATIONS
 * 12-13-93 Schuler   Just clear flags instead of zeroing the whole struct.
 */
void LIBCALL Nlm_ErrClear ()
{
	AppErrInfoPtr info = GetAppErrInfo();
	info->any_error = 0;
	info->err_formatted = 0;
	info->debug_mode = 0;
	info->desc.context = 0;
}

/*-------------------------------------------------------------------------
* ErrShow   [Schuler]
*
* MODIFICATIONS
* 12-13-93 Schuler   Rewritten to use msg_level and die_level settings.
* 12-14-93 Schuler   Now returns the result of Message().
* 12-15-93 Schuler   Added proper handling of Abort/Retry/Ignore choice.
* 12-21-93 Schuler   Added special case for WIN_DUMB + logging to stderr
* 12-24-94 Schuler   Changed Message to MsgAlert
* 12-27-94 Schuler   Removed Beep() because MsgAlert is taking care of it
* 01-31-94 Schuler   Put Beep() back because MsgAlert is no longer doing it.
* 02-03-94 Schuler   Now honors all EO_MSG_... option flags.
* 02-18-94 Schuler   Check for NULL user strings before copying.
*
* TO DO
* - allocate buffer for the message
* - honor option flags
*/
int LIBCALL Nlm_ErrShow ()
{
	AppErrInfoPtr info = GetAppErrInfo();
	if (info->any_error)
	{
		int answer = ANS_OK;
		int severity = info->desc.severity;
		if (info->desc.node != NULL && info->desc.node->sev != SEV_NONE)
			severity = info->desc.node->sev;
		if (severity >= info->opts.msg_level)
		{
			size_t bytes;
			const char *caption = (char*)GetAppProperty("AppName");
			char *message;
			const char *msgtext;
			char *p;
			MsgKey key;

			/* ----- beep if requested ----- */
			if (TEST_BITS(info,EO_BEEP))
				Nlm_Beep();
				
            /* ----- set up the buffer to hold the error message ---- */
			if (TEST_BITS(info,EO_MSG_MSGTEXT))
				msgtext = ErrGetExplanation(info->desc.root,info->desc.node);
			else
				msgtext = NULL;
			/* forget about the options for now -- this is the max we would need*/
			bytes = 2 + strlen(info->desc.module)
				+ 32    /* severity string.  TEMPORARY (need to keep track of longest string) */
				+ 2  + strlen(info->desc.codestr)  /* error codes */
				+ 16 + strlen(info->desc.srcfile)  /* source file and line number */
				+ 2  + info->ustrcnt + info->ustrlen  /* user strings */
				+ 2  + strlen(info->desc.errtext)  /* error message */
				+ 2  + ((msgtext) ? strlen(msgtext) : 0);  /* verbose message */

			if ((message = (char*)Malloc(bytes)) == NULL)
				message = info->desc.errtext;
			else 
			{				
				/* ----- format the message in the buffer ----- */
				p = message;
				if (TEST_BITS(info,EO_MSG_SEVERITY))
				{
					p = strchr(strcpy(p,_szSeverity[severity]),'\0');
					*p++ = ' ';
				}
				if (TEST_BITS(info,EO_MSG_CODES))
				{
					p = strchr(strcpy(p,info->desc.module),'\0');
					*p++ = ' ';
					p = strchr(strcpy(p,info->desc.codestr),'\0');
					*p++ = ' ';
				}
				if (TEST_BITS(info,EO_MSG_FILELINE))
				{
					/*p = strchr(strcpy(p,info->desc.srcfile),'\0');*/
					sprintf(p,"{%s line %d} \n",info->desc.srcfile,info->desc.srcline);
					p = strchr(p,'\0');
				}
				if (TEST_BITS(info,EO_MSG_USERSTR))
				{
					const ValNode *node;
					for (node=info->desc.userstr; node; node=node->next)
					{
						if (node->data.ptrvalue != NULL)
						{
						p = strchr(strcpy(p,(char*)node->data.ptrvalue),'\0');
						*p++ = ' ';
						}
					}
				}
				if (TEST_BITS(info,EO_MSG_ERRTEXT))
				{
					p = strchr(strcpy(p,info->desc.errtext),'\0');
				}
				if (TEST_BITS(info,EO_MSG_MSGTEXT) && msgtext!=NULL)
				{
					if (p != message)  *p++ = '\n';
					p = strchr(strcpy(p,msgtext),'\0');
				}
			}
						
			/* ----- show the message ----- */
			if (TEST_BITS(info,EO_PROMPT_ABORT)) key = KEY_ARI;
			else if (TEST_BITS(info,EO_WAIT_KEY)) key = KEY_OK;
			else key = KEY_NONE;
			answer = MsgAlertStr(key,(ErrSev)severity,caption,message);
			
			/* ----- clean up ----- */
			if (message != info->desc.errtext)
				Free(message);
		}
		
		/* ----- die if appropriate ----- */
		if (severity >= info->opts.die_level)
		{
			int code = info->desc.errcode;
			AbnormalExit(code);
		}
		ErrClear();
		return answer;
	}
	return 0;
}



/***************************************************************************\
|                           MESSAGE FILE SUPPORT                            |
\***************************************************************************/
static ErrMsgRoot * new_ErrMsgRoot PROTO((const char *name));
static void delete_ErrMsgRoot PROTO((ErrMsgRoot *idx));
static FILE * ErrMsgRoot_fopen PROTO((ErrMsgRoot *ctx, const char *mode));
static ErrMsgNode * new_ErrMsgNode PROTO((const char *name, int code, ErrSev sev));
static void delete_ErrMsgNode PROTO((ErrMsgNode *item));
static ErrSev atosev PROTO((const char *sevstr));


/*-------------------------------------------------------------------------
* new_ErrMsgRoot, delete_ErrMsgRoot   [Schuler, 12-09-93]
*
* Constructor/Destructor for ErrMsgRoot
*
* MODIFICATIONS
*
* TO DO
* - implement destructor
*/

static ErrMsgRoot * new_ErrMsgRoot (const char *context)
{
	ErrMsgRoot *idx = (ErrMsgRoot*) MemNew(sizeof(ErrMsgRoot));
	if (idx != NULL)
		idx->name = (context) ? StrSave(context) : NULL;
	return idx;
}

static void delete_ErrMsgRoot (ErrMsgRoot *idx)
{
	/* not implemented */
}

/*-------------------------------------------------------------------------
* ErrMsgRoot_fopen    [Schuler, 12-09-93]
*
* Opens an error message file corresponding to an error context.
*
* MODIFICATIONS
* 01-07-94 Schuler   Looks in the current directory first.
* 01-10-94 Schuler   Uses FileOpen() instead of fopen() (now that 
*                    reentrancy problem is solved).
* TO DO
* remove mode argument -- always use "rb"
*/

#define MODFNAME_MAX 32

static FILE * ErrMsgRoot_fopen (ErrMsgRoot *ctx, const char *mode)
{
	AppErrInfoPtr info = GetAppErrInfo();
	FILE *fd;
	char file[MODFNAME_MAX];
	char path[PATH_MAX];	
	char *p1 = file;
	const char *p2 = ctx->name;
	int i, ch;

	/***if (ctx->not_avail)
		return NULL;***/
	
	for (i=0; (ch= *p2++) != 0 && i<MODFNAME_MAX-5; ++i)
	{
		if (isalpha(ch)) ch = tolower(ch);
		*p1++ = (char)ch;
	}
	strcpy(p1,".msg");

	if ((fd = FileOpen(file,mode)) == NULL)
	{
		strcpy(path,info->msgpath);
		strncat(path,file,sizeof(path));
		fd = FileOpen(path,mode);
	}

	/***if (fd == NULL)
		ctx->not_avail = 1;***/
	return fd;
}


/*-------------------------------------------------------------------------
* ErrMsgRoot_Read    [Schuler, 12-08-93]
*
* Scans an error message file and builds a data structure that contains
* the mappings between integer error codes and strings as well as file
* offset for the verbose error explanations so they can later be retrieved
* when (or if) needed.
*
* MODIFICATIONS
* 01-07-94 Schuler   Fixed bug that resulted in failure to set the text 
*                    length for the last item in the file.
* 01-10-94 Schuler   Added _busy flag to indicate that we are in the process
*                    of parsing the file.  Otherwise, there is a problem if 
*                    you post an error while attempting to read corelib.msg 
*                    (leads to infinite recursion).
* 01-10-94 Schuler   Added linenumber reporting for all error messages.
* 01-23-94 Schuler   Changed all ErrPostEx to ErrLogPrintf (don't want to 
*                    be posting errors in here -- will recurse ad infinitum).
* 01-21-94 Schuler   Changed fclose() to FileClose()
* 02-02-94 Schuler   Revisions for new file format
*
* TO DO
* Check for missing integer code
* Capture comments
*/

static ErrSev atosev (const char *sevstr)
{
	int i;
	if (sevstr)  
	{
		for (i=SEV_MIN; i<SEV_MAX; ++i)
		{
			if (strcmp(sevstr,_szSevKey[i]) ==0)
				return (ErrSev)i;
		}
	}
	return SEV_NONE;
}

static int ErrMsgRoot_Read (ErrMsgRoot *idx)
{
	FILE *fd;

	if (idx->list != NULL)
		return TRUE;	
	if (_busy /*|| idx->not_avail*/ )
		return FALSE;
	_busy = 1;
		
	if ((fd = ErrMsgRoot_fopen(idx,"rb")) == NULL)
	{
		_busy = 0;
		return FALSE;
	}
	else
	{
		char line[80], *p;
		int linenum=0;
		long tofs, cofs, tmpofs;
		ErrMsgNode *lev1;
		ErrMsgNode *lev2;
		int any_text;
		int any_comt;
		
		/* ----- Look for MODULE line ----- */
		while (fgets(line,sizeof line,fd))
		{
			linenum++;
			if (strchr("#\r\n",line[0]))  continue;
			if (strncmp(line,"MODULE",6) ==0)
			{
				p = strtok(line," \t\r\n");
				p = strtok(NULL," \t\r\n");
				if (strcmp(idx->name,p) !=0)
					ErrLogPrintf("Context string mismatch (%s vs %s)\n",idx->name,p);
				break;                 
			}
			else
			{
				_busy = 0;
				ErrLogPrintf("Syntax error: \"MODULE\" expected, line %d\n",linenum);
				return FALSE;
			}
		}
	
		/* ----- Process the rest of the file ----- */
		any_text = any_comt = FALSE;
		lev1 = lev2 = NULL;
		tmpofs = ftell(fd);
		
		while (fgets(line,sizeof line,fd))
		{
			linenum++;
				
			if (line[0] == '$')
			{
				if (any_text)
				{
					ErrMsgNode *n = lev2 ? lev2 : lev1;
					if (n != NULL)
					{
						n->tofs = tofs;
						n->tlen = tmpofs - tofs;
					}
					any_text = FALSE;
				}
			}
			
			if (line[0]=='$' && line[1]=='$')
			{
				/*** NEED ERROR CHECKING HERE ***/
				char *tok1 = strtok(line+2,", \t\r\n");
				char *tok2 = strtok(NULL,", \t\r\n");
				char *tok3 = strtok(NULL,", \t\r\n");
				
				ASSERT(tok1 && tok2);
				
				lev2 = NULL;
				lev1 = new_ErrMsgNode(tok1,atoi(tok2),atosev(tok3));
				if (idx->list == NULL)
					idx->list = lev1;
				else
				{
					ErrMsgNode *t0=NULL, *t1;
					for (t1=idx->list; TRUE; t1=t1->next)
					{
						if (t1==NULL || lev1->code < t1->code)
						{
							if (t0 == NULL)
								idx->list = lev1;
							else
								t0->next = lev1;
							lev1->next = t1;
							break;
						}
						if (lev1->code == t1->code)
						{
							ErrLogPrintf("Code %d duplicated, line %d\n",t1->code,linenum); 
							break;
						}
						t0 = t1;
					}
				}
				/*any_text = FALSE;*/
			}
			else if (line[0]=='$' && line[1]=='^')
			{
				/*** NEED ERROR CHECKING HERE ***/
				char *tok1 = strtok(line+2,", \t\r\n");
				char *tok2 = strtok(NULL,", \t\r\n");
				char *tok3 = strtok(NULL,", \t\r\n");
				
				ASSERT(tok1 && tok2);
				
				lev2 = new_ErrMsgNode(tok1,atoi(tok2),atosev(tok3));
				if (lev1->list == NULL)
					lev1->list = lev2;
				else
				{
					ErrMsgNode *t0=NULL, *t1;
					for (t1=lev1->list; TRUE; t1=t1->next)
					{
						if (t1==NULL || lev2->code < t1->code)
						{
							if (t0 == NULL)
								lev1->list = lev2;
							else
								t0->next = lev2;
							lev2->next = t1;
							break;
						}
						if (lev2->code == t1->code)
						{
							ErrLogPrintf("Code %d duplicated, line %d\n",t1->code,linenum); 
							break;
						}
						t0 = t1;
					}
				}
				/*any_text = FALSE;*/
			}
			else if (line[0] == '#')
			{
				if (!any_comt)
				{
					cofs = tmpofs;
					any_comt = TRUE;
				}
			}
			else
			{
				if (any_comt)
				{
					ErrMsgNode *n = lev2 ? lev2 : lev1;
					if (n != NULL)
					{
						n->cofs = cofs;
						n->clen = tmpofs - cofs;
					}
				}
				if (!any_text)
				{
					/* if (not a blank line) */
						tofs = tmpofs;
						any_text = TRUE;
				}
			}
			tmpofs = ftell(fd);
		}
		
		if (any_text)
		{
			ErrMsgNode *n = lev2 ? lev2 : lev1;
			if (n != NULL)
			{
				n->tofs = tofs;
				n->tlen = tmpofs - tofs;
			}
		}
		
		FileClose(fd);
	}

	_busy = 0;
	return TRUE;
}

/*-------------------------------------------------------------------------
 * new_ErrMsgNode, delete_ErrMsgNode    [Schuler, 12-08-93]
 *
 * Constructor/Destructor for ErrMsgNode
 *
 * MODIFICATIONS
 * 02-02-94 Schuler   Changed arg list to include severity token
 */

static ErrMsgNode * new_ErrMsgNode (const char *name, int code, ErrSev sev)
{
	ErrMsgNode *item = (ErrMsgNode*) MemNew(sizeof(ErrMsgNode));
	if (item != NULL)
	{
		item->name = (name) ? StrSave(name) : NULL;
		item->code = code;
		item->sev =sev;
	}
	return item;
}

static void delete_ErrMsgNode (ErrMsgNode *item)
{
	if (item != NULL)
	{
		ErrMsgNode *n1, *n2;
		for (n1=item->list; n1; n1=n2)
		{
			n2 = n1->next;
			delete_ErrMsgNode(n1);
		}
		MemFree((void*)item->name);
		MemFree((void*)item->tstr);
		MemFree((void*)item->cstr);
		MemFree((void*)item);
	}
}


/*-------------------------------------------------------------------------
 * ErrGetMsgRoot    [Schuler, 12-09-93]
 *
 * Gets the index structure for an error context, creating and initializing
 * it if necessary.
 *
 * MODIFICATIONS
 * 01-10-94 Schuler   Changed to call ErrMsgRoot_Read() *after* linking 
 *                    into list.  Otherwise, if an error is posted from
 *                    within ErrMsgRoot_Read(), it won't find the CoreLib
 *                    modlue and will read it again (ad infinitum).
 */

ErrMsgRootPtr LIBCALL ErrGetMsgRoot (const char *context)
{
	AppErrInfoPtr info = GetAppErrInfo();
	ErrMsgRoot *idx, *idx0=NULL;
	int d;
	
	for (idx=info->idxlist; TRUE; idx=idx->next)
	{
		if (idx==NULL || (d = strcmp(idx->name,context)) >0)
		{
			ErrMsgRoot *idx2 = new_ErrMsgRoot(context);
			idx2->next = idx;
			if (idx0 == NULL)
				info->idxlist = idx2;
			else
				idx0->next = idx2;
			ErrMsgRoot_Read(idx2);
			return (ErrMsgRootPtr) idx2;
		}
		if (d == 0)
			return (ErrMsgRootPtr) idx;

		idx0 = idx;
	}
	return NULL;
}


/*-------------------------------------------------------------------------
 * ErrGetExplanation    [Schuler, 12-09-93]
 * 
 * Gets the explanatory text for a particular error from an error 
 * message file.
 *
 * MODIFICATIONS
 * 02-02-94 Schuler   Fix for memory leak when length==0
 * 02-02-94 Schuler   Now exported (was static)
 */

const char* LIBCALL ErrGetExplanation (ErrMsgRootPtr idx, ErrMsgNodePtr item)
{
	if (idx != NULL && item != NULL && item->tlen >0)
	{
		size_t bytes;
		char *text;

		if (item->tstr != NULL)
			return item->tstr;
			
		bytes = (size_t)item->tlen;
		if ((text= (char*)MemNew(bytes+1)) != NULL)
		{
			FILE *fd = ErrMsgRoot_fopen((ErrMsgRoot *) idx,"rb");
			if (fd != NULL)
			{
				if (fseek(fd,item->tofs,SEEK_SET) !=0)
					goto ErrReturn;
				if (fread(text,1,bytes,fd) != bytes)
					goto ErrReturn;
				*(text+bytes) = '\0';    /* make sure null-terminated */
				fclose(fd);
				return text;
			}
		ErrReturn :
			fclose(fd);
			MemFree(text);
			return NULL;
		}		
	}
	return NULL;
}


/***************************************************************************\
|                             CUSTOMIZATION                                 |
\***************************************************************************/

/*-------------------------------------------------------------------------
* ErrSetLogfile  [Schuler]
*
* MODIFICATIONS
* 07-26-93 Schuler   Modified to use AppErrInfo struct
* 12-17-93 Schuler   Changed back to returning a Boolean
* 01-21-94 Schuler   Renamed & added flags argument.
* 01-21-94 Schuler   Changed fopen/fclose to FileOpen/FileClose
*
* TO DO
* - honor ELOG_NOCREATE flag
*/

int LIBCALL  Nlm_ErrSetLogfile (const char *filename, unsigned long flags)
{
	static char *fmode_append = "a+";
	static char *fmode_overwrite = "w";
	
	AppErrInfoPtr info = GetAppErrInfo();
    char *fmode = (flags & ELOG_APPEND) ? fmode_append : fmode_overwrite;
    FILE *fp;
        
    if ((fp = FileOpen(filename,fmode)) == NULL)
    	return FALSE;
    if (flags & ELOG_BANNER)
    {
	    char buffer[64];
	    int  i;

		Nlm_DayTimeStr(buffer,TRUE,TRUE);
		fputc('\n', fp);
		for (i=0; i<24; i++)  fputc ('=', fp);
		fprintf(fp, "[ %s ]",buffer);
		for (i=0; i<24; i++)  fputc ('=', fp);
		fputc ('\n', fp);
	}
	FileClose(fp);
	
	strncpy(info->logfile,filename,PATH_MAX);
    return TRUE;
}


/*-------------------------------------------------------------------------
 * ErrSetHandler	[Schuler, 05-11-93]
 *
 * Allows the application to set a hook procedure that will be called
 * when an error is posted via ErrPost.  It is always called regardless
 * of any error reporting/logging options that may have been set.  The
 * return value is the pointer to the previous error hook procedure 
 * if there was one.
 * 
 * MODIFICATIONS
 * 05-21-93 Schuler   
 */
ErrHookProc LIBCALL Nlm_ErrSetHandler (ErrHookProc hookNew)
{
	AppErrInfoPtr info = GetAppErrInfo();
	ErrHookProc hookOld =info->hook;
	info->hook = hookNew;
	return hookOld;
}


/*-------------------------------------------------------------------------
 * ErrUserInstall   [Sirotkin]
 *
 * Add or replace a user-string.
 *
 * MODIFICATIONS
 * 12-15-93 Schuler   No longer keeps track of string lengths.
 */

static char *_strNull = "(null)";

ErrStrId LIBCALL Nlm_ErrUserInstall (const char *msg, ErrStrId magic_cookie)
{
	AppErrInfoPtr info = GetAppErrInfo();
	ValNode *list = (ValNode *) info->desc.userstr;
	ValNode *node;
	ErrStrId cookie;
	
	if (msg == NULL)
		msg = _strNull;

	if (magic_cookie)
	{
		for (node=list; node; node=node->next)
		{
			if (node->choice == magic_cookie)
			{
				/** replace **/
				info->ustrlen -= strlen((char*)node->data.ptrvalue);
				MemFree(node->data.ptrvalue);
				node->data.ptrvalue = (void*) StringSave(msg);
				info->ustrlen += strlen(msg);
				return magic_cookie;
			}
		}
		ErrPostEx(SEV_WARNING,-1,0,
				"ErrUserInstall:  bad string id (%d)",
				(int) magic_cookie);
		return 0;
	}
	else
	{
		for (cookie = 1; cookie < 255; ++cookie)
		{
			for (node=list; node; node=node->next)
			{
				if (node->choice == cookie)
					break;
			}
			if (node==NULL)  /* free cookie is magic */
			{
				node = ValNodeNew((ValNode*)info->desc.userstr);
				if ( ! info->desc.userstr)
					info->desc.userstr = node;
				node->choice = cookie;
				node->data.ptrvalue = StringSave(msg);
				info->ustrlen += strlen(msg);
				info->ustrcnt ++;
				return cookie;
			}
		}
		/*ErrPostEx(SEV_WARNING,-1,0,"ErrUserInstall:  no more string id's");*/
		return 0;
	}
}


/*-------------------------------------------------------------------------
 * ErrUserDelete   [Sirotkin]
 *
 * Delete a single user-string.
 *
 * MODIFICATIONS
 * 12-15-93 Schuler   No longer keeps track of string lengths.
 */

Nlm_Boolean LIBCALL Nlm_ErrUserDelete (ErrStrId magic_cookie)
{
	AppErrInfoPtr info = GetAppErrInfo();
	ValNodePtr node = ValNodeExtract((ValNode**)(&info->desc.userstr), magic_cookie);

	if (node)
	{
		ASSERT(node->data.ptrvalue != NULL);
		info->ustrlen -= strlen((char*)node->data.ptrvalue);
		info->ustrcnt --;
		ValNodeFreeData(node);
		return TRUE;
	}
	return FALSE;
}


/*-------------------------------------------------------------------------
 * ErrUserClear   [Sirotkin]
 *
 * Deletes the entire list of user-strings.
 */
void LIBCALL Nlm_ErrUserClear (void)
{
	AppErrInfoPtr info = GetAppErrInfo();
	ValNodeFreeData((ValNode*)info->desc.userstr);
	info->ustrlen = 0;
	info->ustrcnt = 0;
}



/*-------------------------------------------------------------------------
 * ErrGetLogLevel, ErrSetLogLevel   [Schuler, 12-22-93]
 *
 * Get/Set the minimum severity level that will be logged.
 */
int LIBCALL ErrSetLogLevel (ErrSev level)
{
	AppErrInfoPtr info = GetAppErrInfo();
	int prev = info->opts.log_level;
	info->opts.log_level = MAX(SEV_INFO,MIN(level,SEV_MAX));
	return prev;
}

int LIBCALL ErrGetLogLevel ()
{                             
	return GetAppErrInfo()->opts.log_level;
}


/*-------------------------------------------------------------------------
 * ErrGetMessageLevel, ErrSetMessageLevel   [Schuler, 12-14-93]
 *
 * Get/Set the minimum severity level that will be reported via Message().
 */
int LIBCALL ErrSetMessageLevel (ErrSev level)
{
	AppErrInfoPtr info = GetAppErrInfo();
	int prev = info->opts.msg_level;
	info->opts.msg_level = MAX(SEV_INFO,MIN(level,SEV_MAX));
	return prev;
}

int LIBCALL ErrGetMessageLevel ()
{                             
	return GetAppErrInfo()->opts.msg_level;
}


/*-------------------------------------------------------------------------
 * ErrGetFatalLevel, ErrSetFatalLevel   [Schuler, 12-14-93]
 *
 * Get/Set the minimum severity level that will cause an abnormal exit.
 */
int LIBCALL ErrSetFatalLevel (ErrSev level)
{
	AppErrInfoPtr info = GetAppErrInfo();
	int prev = info->opts.die_level;
	info->opts.die_level = MAX(SEV_INFO,MIN(level,SEV_MAX));
	return prev;
}

int LIBCALL ErrGetFatalLevel ()
{
	return GetAppErrInfo()->opts.die_level;
}


/*-------------------------------------------------------------------------
 * ErrSetOptFlags, ErrClearOptFlags, ErrTestOptFlags   [Schuler, 12-14-93]
 *
 * Set, clear, or test the current state of any error option flag.
 */
unsigned long LIBCALL ErrSetOptFlags (unsigned long flags)
{
	AppErrInfoPtr info = GetAppErrInfo();
	info->opts.flags |= (flags & EO_ALL_FLAGS);
	return info->opts.flags;
}

unsigned long LIBCALL ErrClearOptFlags (unsigned long flags)
{
	AppErrInfoPtr info = GetAppErrInfo();
	info->opts.flags &= ~(flags & EO_ALL_FLAGS);
	return info->opts.flags;
}

unsigned long LIBCALL ErrTestOptFlags (unsigned long flags)
{
	AppErrInfoPtr info = GetAppErrInfo();
	return (info->opts.flags & flags);
}


/*-------------------------------------------------------------------------
 * ErrSaveOptions  [Schuler, 01-03-94]
 *
 */
void LIBCALL ErrSaveOptions (ErrOpts *opts)
{
	AppErrInfoPtr info = GetAppErrInfo();
	
	ASSERT(opts != NULL);
	memcpy((void*)opts,(void*)(&info->opts),sizeof(ErrOpts));
}


/*-------------------------------------------------------------------------
 * ErrRestoreOptions  [Schuler, 01-03-94]
 */
 
void LIBCALL ErrRestoreOptions (const ErrOpts *opts)
{
	AppErrInfoPtr info = GetAppErrInfo();
	
	ASSERT(opts != NULL);
	memcpy((void*)(&info->opts),(void*)opts,sizeof(ErrOpts));
}


/*-------------------------------------------------------------------------
* ErrGetOpts  [Schuler]
*
* MODIFICATIONS
* 07-26-93 Schuler   Modified to use AppErrInfo struct
* 02-02-94 Schuler   Added local option saving strategy
*/

static ErrOpts _eo_save[2];

void LIBCALL  Nlm_ErrGetOpts (short * erract, short * errlog)
{
	AppErrInfoPtr info = GetAppErrInfo();
	
	if (erract != NULL)
	{
		int i;
		
		for (i=0; i<DIM(_eo_save); ++i)
		{
			if (_eo_save[i].flags ==0) 
				break;
		}
		if (i<DIM(_eo_save))
		{
			_eo_save[i] = info->opts;
			*erract = -(1+i);
		}
		else
		{
			TRACE("ErrGetOpts: overflow\n");
			*erract = info->opts.actopt;
		}
	}
	if (errlog != NULL)
		*errlog = info->opts.logopt;
}


/*-------------------------------------------------------------------------
* ErrSetOpts  [Schuler]
*
* MODIFICATIONS
* 07-26-93 Schuler   Modified to use AppErrInfo struct
* 12-15-93 Schuler   Modified to map old settings to the new options.
* 12-21-93 Schuler   Now always starts from defaults and then twiddles.
* 01-27-94 Schuler   Fixed bug whereby logopt not always saved.
* 02-02-94 Schuler   Added local option saving strategy
*/


void LIBCALL  Nlm_ErrSetOpts (short actopt, short logopt)
{
	AppErrInfoPtr info = GetAppErrInfo();
	
	if (actopt < 0)
	{
		int i = -(actopt+1);
		ASSERT(i>=0 && i<DIM(_eo_save));
		if (_eo_save[i].flags !=0)
		{
			info->opts = _eo_save[i];
			_eo_save[i].flags =0;
			return;
		}
		ErrLogPrintf("*** Improper use of ErrGetOpts/ErrSetOpts ***\n");
	}
	else
	{
		TRACE("ErrSetOpts: 1st-time call, nothing to restore\n");
	}

	/****info->opts.flags = EO_DEFAULTS;
	info->opts.log_level = SEV_INFO;****/
	
	if (actopt !=0)
		info->opts.flags &= ~EO_PROMPT_ABORT;

	switch (actopt)
	{
		case ERR_CONTINUE:
		case ERR_IGNORE:
			info->opts.msg_level = SEV_MAX;
			info->opts.die_level = SEV_MAX;
			break;
		case ERR_TEE:
			info->opts.msg_level = SEV_WARNING;
			info->opts.die_level = SEV_MAX;
			break;
		case ERR_ADVISE:
			info->opts.msg_level = SEV_WARNING;
			info->opts.die_level = SEV_MAX;
			break;
		case ERR_ABORT:
			info->opts.msg_level = SEV_WARNING;
			info->opts.die_level = SEV_FATAL;
			break;
		case ERR_PROMPT:
			info->opts.msg_level = SEV_WARNING;
			info->opts.die_level = SEV_FATAL;
			info->opts.flags |= EO_PROMPT_ABORT;
			break;
	}		
	if (actopt != 0)
		info->opts.actopt = actopt;

	
	switch(logopt)
	{
		case ERR_LOG_ON:
			info->opts.flags |=  EO_LOGTO_USRFILE;
			break;
		case ERR_LOG_OFF:
			info->opts.flags &= ~EO_LOGTO_USRFILE;
			break;	
		default:
			logopt = 0;	
	}
	if (logopt != 0)
		info->opts.logopt = logopt;
	
			
	/***** if anybody liked this, I can put it back...
	if (logopt != 0)
	{
		int   i;
		FILE *fp;
		if ((fp = FileOpen(info->logfile, "a+")) != NULL)
		{
			fputc ('\n', fp);
			for (i=0; i<4; i++)  fputc (' ', fp);
			for (i=0; i<21; i++)  fputc ('-', fp);
			fprintf (fp, " error logging %sabled ",
			                logopt==ERR_LOG_ON ? " en" : "dis");
			for (i=0; i<21; i++)  fputc ('-', fp);
			fputc ('\n', fp);
			FileClose(fp);
		}
	}
	*****/
}


/***************************************************************************\
|                               DEBUGGING                                   |
\***************************************************************************/


/*-------------------------------------------------------------------------
 * Trace   [Schuler, 04-13-93]
 *
 * Formats a string and sends it to the "trace device" (see the
 * description of Nlm_TraceStr about "trace device").  Normally, it
 * is desirable to trace some (possibly verbose) informational messages
 * during the development phase of a program, but inhibit them in the
 * final version that is released to end users.  The TRACE macro supports
 * this style of usage, calling Nlm_Trace only if _DEBUG is defined.
 * For example:
 *
 *		TRACE("!@#$ screwed up (%s,%d)\n",__FILE__,___LINE__);
 *
 * Note that it is declared as CDECL (because of the variable argument
 * list) and may therefore not be callable from some other programming
 * languages, such as Basic and Pascal.
 *
 * MODIFICATIONS
 * Schuler 07-26-93
 * Schuler 12-14-93   Merged varargs and stdargs versions of the function
 */

#ifdef VAR_ARGS
void Nlm_Trace (fmt, va_alist)
 const char *fmt;
 va_dcl
#else
void CDECL  Nlm_Trace (const char *fmt, ...)
#endif
{
	char *buff = GetScratchBuffer();
	VSPRINTF(buff,fmt);
	Nlm_TraceStr(buff);
}

		

/*-------------------------------------------------------------------------
* TraceStr   [Schuler, 04-13-93]
*
* Users of the C and C++ should use the TRACE macro and not call this
* function directly.  This function, unlike Nlm_Trace, it may be called 
* other programming languages, such as Basic and Pascal.
*
* MODIFICATIONS
* 05-26-93 Schuler  
* 12-15-93 Schuler   Changed to use TRACE_TO_FILE macro.  
* 12-15-93 Schuler   Removed tracing to stderr (caused an echo echo ...).
* 01-21-94 Schuler   Changed fopen/fclose to FileOpen/FileClose
*/

void LIBCALL Nlm_TraceStr (const char *str)
{
	if (str==NULL)  return;

#ifdef TRACE_TO_STDOUT
	fprintf(stdout,"%s",str);
#endif

#ifdef TRACE_TO_AUX
#if defined(WIN16) || defined(WIN32)
	OutputDebugString(str);
#endif
#endif /* TRACE_TO_AUX */

#ifdef TRACE_TO_FILE
	{
		FILE *fd = FileOpen("trace.log","a+");
		if (fd != NULL)
		{
			fprintf(fd,"%s",str);
			FileClose(fd);
		}
	}
#endif /* TRACE_TO_FILE */
}


/*-------------------------------------------------------------------------
* Nlm_AssertionFailed   [Schuler, 04-13-93]
*
* Function needed to support the ASSERT and VERIFY macros.
*
* MODIFICATIONS
* 05-11-93 Schuler
* 12-15-93 Schuler   Use ErrLogPrintf() inseead of Nlm_Trace()
* 12-15-93 Schuler   Added call to Message() so user knows what happened!
* 01-21-94 Schuler   Changed MSG_OK to MSG_POST
*/
void LIBCALL Nlm_AssertionFailed (const char *expression, const char *module,
				const char *fname, int linenum)
{
	ErrLogPrintf("\nAssertion Failed:  %s\n", expression);
	if (module != NULL)
		ErrLogPrintf("  %s",module);
	if (fname != NULL)
		ErrLogPrintf("  %s, line %d",fname,linenum);
	ErrLogPrintStr("\n");
	Message(MSG_POST,"Assertion Failed:\n%s",expression);
	AbnormalExit(1);
}


/*-------------------------------------------------------------------------
 * AbnormalExit   [Schuler, 04-13-93]
 *
 * Terminates the application immediately.  This should only be done
 * as a last resort since some (possibly necessary) cleanup code will
 * not get executed.
 *
 * MODIFICATIONS
 * Schuler  05-11-93
 * Schuler  07-26-93
 */
void LIBCALL  Nlm_AbnormalExit (int code)
{
	ErrLogPrintStr("\n***** ABNORMAL PROGRAM EXIT *****\n");

#ifdef OS_MAC
	ExitToShell();
#else
#ifdef WIN_MSWIN
	FatalExit(code);
#else
	exit(code);
#endif
#endif
}



/***************************************************************************\
|                         STATIC HELPER FUNCTIONS                           |
\***************************************************************************/

/*-------------------------------------------------------------------------
* GetAppErrInfo  [Schuler, 07-26-93]
*
* MODIFICATIONS
* 12-13-93 Schuler   Added initialization for new settings.
* 01-04-94 Schuler   Added code to get settings from ncbi.ini
* 01-27-94 Schuler   Set & clear _busy when reading from ncbi.ini
* 02-01-94 Schuler   Updated _flag array
*/
 
static char * _file = "ncbi";
static char * _section = "ErrorProcessing";

struct FlagInf { char *key; unsigned long bits; };
static struct FlagInf _flag [] = 
{
	"EO_LOG_SEVERITY",  EO_LOG_SEVERITY,
	"EO_LOG_CODES",     EO_LOG_CODES,
	"EO_LOG_FILELINE",  EO_LOG_FILELINE,
	"EO_LOG_USERSTR",   EO_LOG_USERSTR,
	"EO_LOG_ERRTEXT",   EO_LOG_ERRTEXT,
	"EO_LOG_MSGTEXT",   EO_LOG_MSGTEXT,
	"EO_MSG_SEVERITY",  EO_MSG_SEVERITY,
	"EO_MSG_CODES",     EO_MSG_CODES,
	"EO_MSG_FILELINE",  EO_MSG_FILELINE,
	"EO_MSG_USERSTR",   EO_MSG_USERSTR,
	"EO_MSG_ERRTEXT",   EO_MSG_ERRTEXT,
	"EO_MSG_MSGTEXT",   EO_MSG_MSGTEXT,
	"EO_LOGTO_STDOUT",  EO_LOGTO_STDOUT,
	"EO_LOGTO_STDERR",  EO_LOGTO_STDERR,
	"EO_LOGTO_TRACE",   EO_LOGTO_TRACE,
	"EO_LOGTO_USRFILE", EO_LOGTO_USRFILE,
	"EO_XLATE_CODES",   EO_XLATE_CODES,
	"EO_WAIT_KEY",      EO_WAIT_KEY,
	"EO_PROMPT_ABORT",  EO_PROMPT_ABORT,
	"EO_BEEP",          EO_BEEP
};

static struct AppErrInfo * GetAppErrInfo()
{
	AppErrInfoPtr info = (AppErrInfoPtr) GetAppProperty(_szPropKey);
	
	if (info == NULL)
	{
		char buffer[80], *p;
		int i;
		
		info = (struct AppErrInfo*) MemGet(sizeof(struct AppErrInfo), TRUE);
		if (info == NULL)  
			Message(MSG_FATAL,"Out of memory");
		info->opts.actopt = ERR_ABORT;       /* OBSOLETE */
		info->opts.logopt = ERR_LOG_OFF;     /* OBSOLETE */
		info->opts.flags = EO_DEFAULTS;
		info->opts.log_level = SEV_INFO;
		info->opts.msg_level = SEV_WARNING;
		info->opts.die_level = SEV_FATAL;
		strcpy(info->logfile,"error.log");

		_busy = 1;

		if (GetAppParam(_file,_section,"MsgPath","",info->msgpath,PATH_MAX -2))
			FileBuildPath(info->msgpath, NULL, NULL);		

		for (i=SEV_MIN; i<SEV_MAX; ++i)
		{
			GetAppParam(_file,_section,_szSevKey[i],_szSevDef[i],buffer,sizeof buffer);
			if (buffer[0] == '"')
			{
				if ( (p = strchr(&buffer[1],'"')) != NULL )  *p = '\0';
				p = &buffer[1];
			}
			else
			{
				p = buffer;
			}
			_szSeverity[i] = StrSave(p);
		}

		for (i=0; i<DIM(_flag); ++i)
		{
			p = _flag[i].key;
			if (GetAppParam(_file,_section,p,"",buffer,sizeof buffer))
			{
				info->ini_mask |= _flag[i].bits;
				if (strchr("1TtYy",buffer[0]))
					info->ini_bits |= _flag[i].bits;
			}
		}
		
		_busy = 0;

		SetAppProperty(_szPropKey,(void*)info);
	}
	return info;
}

