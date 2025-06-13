/* $Header: inp.h,v 2.0 86/09/17 15:37:25 lwall Exp $
 *
 * $Log:	inp.h,v $
 * Revision 2.0  86/09/17  15:37:25  lwall
 * Baseline for netwide release.
 * 
 */

EXT LINENUM input_lines INIT(0);	// how long is input file in lines
EXT LINENUM last_frozen_line INIT(0);	// how many input lines have been
					// irretractibly output


void	re_input	(void);
void	scan_input	(char* filename);
char*	ifetch		(Reg1 LINENUM line, int whichbuf);
bool	rev_in_string	(char* string);
