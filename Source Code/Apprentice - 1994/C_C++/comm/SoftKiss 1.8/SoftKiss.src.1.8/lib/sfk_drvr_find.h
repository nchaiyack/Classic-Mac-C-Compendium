/*
 * find the sofktiss driver
 * by Aaron Wohl (aw0g+@andrew.cmu.edu) jul 1990
 * Carnegie-Mellon University
 * Special Projects
 * Pittsburgh, PA 15213-3890
 * (412)-268-5032
 *
 * Unlike other source files which are for THINK C compilation
 * only, this module compiles with MPW c also.  This lets macmail use it
 * to find MailCheck.  So be carefull about sizeof(int) in THINK == 2
 * and sizeof(int) in MPW == 4 if you make any changes.
 */

/*
 * returns driver refnum of softkiss or
 * zero if not loaded
 * free_ref_num is set to the refnum of a utable entry
 * that is not in use (in memory or in system file).
 */
short sfk_drvr_find(unsigned char *driverName,short *free_ref_num);
