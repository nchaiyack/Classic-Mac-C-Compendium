#include <stdio.h>

putint() { printf("%4d  ",(int)(rand() %1000) ); }
putfloat() { printf("%9.4f  ",(float) (rand()%1000)*2.127);  }

main(ac,av) int ac; char**av; {
char *fmt;
int i,j,len,n;

if (ac!=3) 
	{ fprintf(stderr,"gen ascii data. usage: %s fmt n\n",av[0]); exit(0); }
sscanf(av[2],"%d",&n); fmt = av[1]; 
fprintf(stderr,"gen for [%s} %d vals\n",fmt,n);

len = strlen(fmt);
for(i=0;i<n;i++) {
  for(j=0;j<len;j++) 
    switch(fmt[j]) {
      case 'd': putint(); break;
      case 'f': putfloat(); break;
      default : break;
      }
  putchar('\n');
  }
} /* main */
