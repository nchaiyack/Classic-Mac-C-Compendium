#include <stdio.h>

int main()
{
	FILE *f1;
	FILE *f2;
	FILE *g;
	int c;
	f1 = fopen("vineyard:eric:incoming:tickle.1","r");
	f2 = fopen("vineyard:eric:incoming:tickle.2","r");
	g = fopen("vineyard:eric:incoming:tickle.all","w");
	c = getc(f1);
	while (c != EOF) {
		putc(c,g);
		c = getc(f1);
	}
	c = getc(f2);
	while (c != EOF) {
		putc(c,g);
		c = getc(f2);
	}
	fclose(f1);
	fclose(f2);
	fclose(g);
}
