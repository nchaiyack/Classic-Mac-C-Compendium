/*****
 * ShellInterface.h
 *
 *		Public interface for ShellInterface.c
 *
 ****/
/*
 * Copyright (C) 1985-1992  New York University
 * Copyright (C) 1994 George Washington University
 * 
 * This file is part of the GWAdaEd system, an extension of the Ada/Ed-C
 * system.  See the Ada/Ed README file for warranty (none) and distribution
 * info and also the GNU General Public License for more details.
 */
enum {
kProgressBarWidth = 256	// keep this a power of 2 for faster operations
};	// assuming a clever C compiler [Fabrizio Oddone]

void AdjustMenus(void); 
void HandleMenu (long mSelect);
void SetUpMenus(void);

void SetUpWindow(Boolean useProgress);
void DrawWContents (short active);
void ProgressBar(unsigned short progress);

void WriteErrorCode(short errCode);


/* InternalError() - display alert message, write error results
 * to results file, and call the longjmp.  This routine never
 * returns!
 */

typedef struct {
	short	id;		// less than zero, then use str below
#ifndef powerc	// align long-sized fields under 68K [Fabrizio Oddone]
	short	M68KeffPad;
#endif
	char	*msg;
} ErrMsgs[4];

void InternalError(ErrMsgs msgs, int errCode, char *file, int line);

void ShowMessage(char *msg);
void AbortMessage(char *msg, int errCode);
