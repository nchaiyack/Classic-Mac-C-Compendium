/****
 * mini.print.h
 *
 *	Public interface for mini.print.c
 *
 ****/

int DoPageSetUp(void);
int PrintText(char	**hText, long length, GrafPtr gp, int tabPixels);
