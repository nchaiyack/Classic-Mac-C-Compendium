/*  test2.c
**
** Tests the sscanf implementation so that if an
** item is not scanned, it's variable is not changed.
*/

#include <stdio.h>

int mysscanf(const char *s, const char *f, ...);

main()
{
	char *s="";
	char *f="";
	float f1=-99;
	int i1=-99, i2=-99;
	char s1[40]={"Brent"};
	int c1,c2;
	
	c1 = mysscanf(s, f, &i1, &i2);
	c2 =   sscanf(s, f, &i1, &i2);
printf("  mine: %d >%s< >%s,%d,%d<\n", c1, s, s1, i1, i2);
printf("theirs: %d >%s< >%s,%d,%d<\n", c2, s, s1, i1, i2);
}