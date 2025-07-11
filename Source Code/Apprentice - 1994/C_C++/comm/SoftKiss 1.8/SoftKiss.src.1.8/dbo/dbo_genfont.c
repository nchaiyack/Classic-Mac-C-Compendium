/*
 * dbo_genfont.c - generate font image array
 * by Aaron Wohl
 * Public domain
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15208
 * home: 412-731-6159
 * work: 412-268-5032
 */

#include <stdio.h>
#include "dbo_stdio.h"
#include "dbo_font.h"
#include <string.h>
#include <time.h>

static char *fname="dbo_font.c";
static FILE *of;	/*output file, generated c code*/
static char font_table[DBO_LINE_HEIGHT][DBO_CHARSET_SIZE];

/*
 * setup a port to draw in font_table
 */
static void init_port(GrafPtr port)
{
  OpenPort(port);
  port->portBits.baseAddr= (Ptr) font_table;
  port->portBits.rowBytes=DBO_CHARSET_SIZE;
  port->portBits.bounds.top=0;
  port->portBits.bounds.left=0;
  port->portBits.bounds.bottom=DBO_LINE_HEIGHT;
  port->portBits.bounds.right=DBO_CHAR_PIXEL_WIDTH*DBO_CHARSET_SIZE;
  port->portRect=port->portBits.bounds;
  RectRgn(port->visRgn,&port->portRect);
  TextFont(monaco);
  TextSize(9);
  TextFace(condense);
}

/*
 * fill in the font_table with the bits for each character
 */
static void fill_font_table()
{
  GrafPort port;
  int line_base;
  int i;
  int cwidth;
  init_port(&port);
  memset(font_table,0,sizeof(font_table));
  for(i=0;i<DBO_CHARSET_SIZE;i++) {
    cwidth=CharWidth(i);
    if(cwidth>DBO_CHAR_PIXEL_WIDTH)cwidth=DBO_CHAR_PIXEL_WIDTH;
    cwidth/=2;
  	MoveTo(i*DBO_CHAR_PIXEL_WIDTH+cwidth,DBO_LINE_VOFFSET);
	DrawChar(i);
  }
 
  ClosePort(&port);
}

/*
 * print out the bits to make a character
 */
static void dump_char(char achar)
{
	int i;
	char *sep="";
	achar&=0x7f;
	if(achar==0x7f)
		fprintf(of," /* delete 0x7f*/ ");
	else if(achar==' ')
		fprintf(of," /* space %02x */ ",' ');
	else if(achar<' ')
		fprintf(of," /* '^%c' %02x */ ",achar+'@',achar);
	else
		fprintf(of," /* '%c' %02x */ ",achar,achar);
	fprintf(of," {");
	for(i=0;i<DBO_LINE_HEIGHT;i++) {
		fprintf(of,"%s0x%02x",sep,font_table[i][achar]);
		sep=",";
		}
	fprintf(of,"}");
}

/*
 * dump start of program
 */
static void dump_header()
{
	time_t now=time(0L);
	fprintf(of,
"/*\n"
" * dbo_font.c - automaticly generated .c file\n"
" * by Aaron Wohl\n"
" * Public domain\n"
" * 6393 Penn Ave #303\n"
" * Pittsburgh PA, 15208\n"
" * home: 412-731-6159\n"
" * work: 412-268-5032\n"
" *\n"
" * genfont.c generated at ");
	fprintf(of,"%s",ctime(&now));
	fprintf(of,
" *\n"
" */\n"
"\n"
"#ifdef DBO_ENABLED\n"
"#include \"dbo_stdio.h\"\n"
"#include \"dbo_font.h\"\n"
"#include <string.h>\n"
"\n"
"unsigned char dbo_draw_font[DBO_CHARSET_SIZE][DBO_LINE_HEIGHT]={\n");
}

/*
 * put out end of trailer
 */
static void dump_trailer()
{
	fprintf(of,
		"\n"
		"};\n"
		"#endif\n");
}

/*
 * dump out the font table
 */
static void dump_font(void)
{
  int i;
  char *sep="";
  dump_header();
  for(i=0;i<DBO_CHARSET_SIZE;i++) {
    fprintf(of,"%s",sep);
  	dump_char(i);
  	sep=",\n";
  }
  dump_trailer();
}

int main()
{
  printf("dbo_genfont font table generator built on %s %s\n",__DATE__,__TIME__);
  fill_font_table();
  printf("writeing font table to %s\n",fname);
  if((of=fopen(fname,"w"))==0) {
  	printf("error opening genfont.c\n");
  	return 1;
  }
  dump_font();
  if(fclose(of)!=0) {
  	printf("error opening genfont.c\n");
  	return 1;
  }
  printf("done ok\n");
}
