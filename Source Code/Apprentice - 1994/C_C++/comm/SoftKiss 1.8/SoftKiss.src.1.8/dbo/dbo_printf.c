/*
 * dbo_stdout.c - debugging print system
 * by Aaron Wohl
 * Public domain
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15208
 * home: 412-731-6159
 * work: 412-268-5032
 */

#ifdef DBO_ENABLED
#include "dbo_stdio.h"
#include "dbo_font.h"
#include <string.h>

/*
 * per field options
 */
#define dbo_OPT_LONG 	(1)
#define dbo_OPT_MODF 	(2)
#define dbo_OPT_LEFT 	(4)
#define dbo_OPT_BASE16  (8)
#define dbo_OPT_MINUS	(16)

/*
 * use this file for stdout
 */
static dbo_FILE dbo_stdout_FILE;

/*
 * initialize the passed dbo file for full screen
 * on startup screen
 */
void dbo_fopen(dbo_FILE_pt af)
{
	static Point zp={0,0};
	af->win_tl=zp;
	af->win_cur=zp;
	af->win_br.v= (*((short *)0x0C22)); /*ColLines*/
	af->win_br.h= (*((short *)0x0C20)); /*RowBits*/
	af->mem= (*((unsigned char **)0x824)); /*ScrnBase*/
	af->row_bytes= (*((short *)0x106));	/*ScreenRow*/
	af->wrap=TRUE;
	af->nl_extra=4;				/*white space at end of line*/
	af->plimit=400;				/*max bytes out per printf*/
	af->int_is4=FALSE;			/*default to 2 byte integers*/
	af->just_kidding=FALSE;
	af->dbo_guard1=dbo_GUARD1;
	af->dbo_guard2=dbo_GUARD2;
}

/*
 * write one character to a file
 */
void dbo_fputc(register dbo_FILE_pt af,char ich)
{
	int i;
	unsigned char ch=ich&0x7F;
	unsigned char *mem_base;
	unsigned char *font_info= &dbo_draw_font[ch][0];
	if(af->just_kidding)
		return;
	if(ch=='\n') {
		long line_len=af->win_br.h-af->win_cur.h;
		mem_base=af->mem+(af->win_cur.h>>3)+af->win_cur.v*af->row_bytes;
		if(line_len>af->nl_extra)
			line_len=af->nl_extra;
		if(line_len>0)
		  for(i=0;i<DBO_LINE_HEIGHT;i++,mem_base+=af->row_bytes)
			memset(mem_base,0,line_len);
		af->win_cur.v+=DBO_LINE_HEIGHT;
		af->win_cur.h=af->win_tl.h;
		return;
	}
	if((af->win_cur.h+DBO_CHAR_PIXEL_WIDTH)>=af->win_br.h) {
		if(!af->wrap)
			return;
		af->win_cur.h=af->win_tl.h;
		af->win_cur.v+=DBO_LINE_HEIGHT;
	}
	if((af->win_cur.v+(DBO_LINE_HEIGHT-1))>=af->win_br.v)
		af->win_cur.v=af->win_tl.v;
	mem_base=af->mem+(af->win_cur.h>>3)+af->win_cur.v*af->row_bytes;
 	for(i=0;i<DBO_LINE_HEIGHT;i++,mem_base+=af->row_bytes)
		*mem_base = *font_info++;
	af->win_cur.h+=8;
}

/*
 * v array version of printf
 */
int dbo_vfprintf(dbo_FILE_pt af,const char *fmt,va_list ap)
{
	register int ch;
	long plimit;		/*max chars left to print*/
	int def_options;
	if((af==dbo_stdout)||			/*if want stdout*/
		((((long)af)&0x1)!=0)||		/*or invalid address for file structure*/
		(af->dbo_guard1!=dbo_GUARD1)|| /*or not a vailid file structure*/
		(af->dbo_guard2!=dbo_GUARD2)) {
		af= &dbo_stdout_FILE;
		if((af->dbo_guard1!=dbo_GUARD1)||(af->dbo_guard2!=dbo_GUARD2))
			dbo_fopen(af);
	}
	plimit=af->plimit;
	def_options=(af->int_is4?dbo_OPT_LONG:0);
	if(af->just_kidding)goto done;
	for (ch=*fmt;ch!=0;ch=*++fmt) {
		long field_width=0;
		int options=def_options;
		char num_buf[40];			/*convert numbers to text here*/
#define NUM_BUF_END (&num_buf[(sizeof(num_buf)-1)])
		char *sout= NUM_BUF_END;	/*string to print*/
		int slen= -1;				/*limit of chars to print -1 for till null*/
		unsigned long num=0;
		char fill_ch=' ';
		*NUM_BUF_END=0;
		if(plimit<=0)
			 goto done;	/*printed as much as allowed*/
		if(ch==0)
			return;			/*or as desired*/
		if(ch!='%') {
		    put_ch:					/*put current character unquoted*/
		    if(ch==0)goto done;
			dbo_fputc(af,ch);
			plimit--;
			continue;
		}
		if((ch= *++fmt)==0)
			goto done;
		/*parse modifyers and options*/
		for(;;ch= *++fmt) {
			if(ch=='0')
				fill_ch='0';
			else if(ch=='#')
				options|=dbo_OPT_MODF;
			else if((ch=='l')||(ch=='L'))
				options|=dbo_OPT_LONG;
			else if((ch=='h')||(ch=='H'))
				options&= ~dbo_OPT_LONG;
			else if(ch=='-')
				options|=dbo_OPT_LEFT;
			else if((ch>='0')&&(ch<='9')) {
					for(;ch>='0' && ch<='9';ch=*++fmt)
						field_width= field_width*10+(ch-'0');
					fmt--;
			} else break;
		}
		switch(ch) {
			case 'x':
			case 'X':
				options|=dbo_OPT_BASE16;
				/*fall thru*/
			case 'u':
				if(options&dbo_OPT_LONG)
					num=va_arg(ap,unsigned long);
				else
					num=va_arg(ap,unsigned short);
				goto out_num;
			case 'd':
				if(options&dbo_OPT_LONG)
					num=va_arg(ap,long);
				else
					num=va_arg(ap,short);
				if(((long)num)<0) {
					options|=dbo_OPT_MINUS;
					num= -(((long)num));
				}
			  out_num:
				if(options&dbo_OPT_BASE16)
					do{
					  *--sout= "0123456789abcdef"[num&0xf];
					  num>>=4;
					}while(num!=0);
				else
					do{
					  *--sout=(num % 10)+'0';
					  num/=10;
					}while(num!=0);
				if(options&dbo_OPT_MINUS)
					*--sout='-';
				goto put_string;
			case 's':
				sout=va_arg(ap,char *);
				/*if have a bogus pointer say so*/
				if(sout==0)sout= (char*)"\p(null)";
				if(!(options&dbo_OPT_MODF))
					goto put_string;
				slen= (*sout)&0xff;
				sout++;
				goto put_string;
			case 'c':
				if(options&dbo_OPT_LONG)
					num=va_arg(ap,long);
				else
					num=va_arg(ap,short);
				*--sout=num;
				slen=1;
				goto put_string;
				break;
			case '%':
			default:
				goto put_ch;
		}
	put_string:
	/*
	 * sout - string to write
	 * slen - <0 for null terminated or length to write
	 * fieldwidth > 0 - justify in this field width
	 * fieldwidth = 0 - no justify
	 * options - see options defs above
	 */
	 /*if right justifying put out leading fill*/
	 if(slen<0)slen=strlen(sout);
	 if(field_width>0) {
	 	if(slen>field_width)slen=field_width;
	 	field_width-=slen;
	 	if((options&dbo_OPT_LEFT)==0)
	 		while(field_width>0) {
	 			dbo_fputc(af,fill_ch);
				if(--plimit <=0) goto done;
				field_width--;
			}
	}
	/*actually put out a field*/
 	while(--slen >=0) {
 		dbo_fputc(af,*sout++);
		if(--plimit <=0)  goto done;
	}
	/*if left justifying, fill right side of field*/
 	while(field_width>0) {
 		dbo_fputc(af,fill_ch);
		if(--plimit <=0)  goto done;
		field_width--;
	}
	}
done:
  return af->plimit-plimit;
}

/*
 * debugging fprintf
 */
int dbo_fprintf(dbo_FILE *af,const char *fmt,...)
{
	return(dbo_vfprintf(af,fmt,__va(fmt)));
}

/*
 * debugging vprintf
 */
int dbo_vprintf(const char *fmt,void *p)
{
	return(dbo_vfprintf(dbo_stdout,fmt,p));
}

/*
 * debugging printf
 */
int dbo_printf(const char *fmt,...)
{
	return(dbo_vprintf(fmt,__va(fmt)));
}

/*
 * position cursor in the stdout dbo window
 */
void dbo_goto(int x,int y)
{
	dbo_fgoto(dbo_stdout,x,y);
}

/*
 * position cursor in a dbo window
 */
void dbo_fgoto(dbo_FILE *af,int x,int y)
{
	af->win_cur.h=af->win_tl.h+DBO_CHAR_PIXEL_WIDTH*x;
	af->win_cur.v=af->win_tl.v+DBO_LINE_HEIGHT*y;
}

/*
 * clear window
 */
void dbo_clear(void)
{
	dbo_fclear(dbo_stdout);
}

/*
 * clear file window
 */
void dbo_fclear(dbo_FILE *af)
{
	long i;
	unsigned char *mem_base=af->mem+(af->win_tl.h>>3)+((long)af->win_tl.v)*af->row_bytes;
	long line_len=(af->win_br.h-af->win_tl.h)>>3;
	long win_height=(af->win_br.v-af->win_tl.v);
	if(af->just_kidding)
		return;
	for(i=0;i<win_height;i++,mem_base+=af->row_bytes)
		memset(mem_base,0,line_len);
}
#endif
