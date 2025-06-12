/*
 * mmc_macmail interface to to turn off/on mailcheck while macmail is running
 * should be called every five min or so to inhibit mailcheck
 */

#define MMC_normal 	(0L)
#define MMC_inhibit (1L)

/*
 * set desire for inhibit and name as a str255
 */
void mmc_macmail(long want_inhibit,char *name);
