/* $Header: pch.h,v 2.0.1.1 87/01/30 22:47:16 lwall Exp $
 *
 * $Log:	pch.h,v $
 * Revision 2.0.1.1  87/01/30  22:47:16  lwall
 * Added do_ed_script().
 * 
 * Revision 2.0  86/09/17  15:39:57  lwall
 * Baseline for netwide release.
 * 
 */

EXT FILE *pfp INIT(Nullfp);		// patch file pointer


void	re_patch				(void);
void	open_patch_file			(char* filename);
void	set_hunkmax				(void);
void	grow_hunkmax			(void);
bool	there_is_another_patch	(void);
int		intuit_diff_type		(void);
void	next_intuit_at			(long file_pos, long file_line);
void	skip_to					(long file_pos, long file_line);
bool	another_hunk			(void);
char*	pgets					(char* bf, int sz, FILE* fp);
bool	pch_swap				(void);
LINENUM	pch_first				(void);
LINENUM	pch_ptrn_lines			(void);
LINENUM	pch_newfirst			(void);
LINENUM	pch_repl_lines			(void);
LINENUM	pch_end					(void);
LINENUM	pch_context				(void);
short	pch_line_len			(LINENUM line);
char	pch_char				(LINENUM line);
char*	pfetch					(LINENUM line);
LINENUM	pch_hunk_beg			(void);
void	do_ed_script			(void);
