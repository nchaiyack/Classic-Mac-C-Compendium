/*******************************************************************************\

printf module

part of suntar, �1991,1994 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

/* it's a not 100% standard version of printf: it does not support most
options (%4d and %4c are supported but %4s %04d %-4d are not) and has a
new descriptor %P for Pascal strings

However, it has a sophisticated handling of buffering and flushing
*/

#define WIN_INDEX 0
/* I found no interest in supporting windows other then window 0, but
if you place the buffer and some flags in the window struct it would be
easy to adapt it to write to any window; the easiest way is to use a variable
as WIN_INDEX, obviously flushing the buffer every time it's changed
*/

#define FILE_PRINT
#include "windows.h"
#include "suntar.h"

static Boolean first_time=true;
static char no_flush = 0;
static char last_char=CR;
short log_refnum=0;
short list_buf_size;
static Str63 nome_file_aperto;

void my_flush(Boolean);
void printdec(unsigned long,short,char);
void print_oct(long);
void printhex(long,Boolean);
void new_console(void);

void toggleLog(void);
void closeLog(void);

void toggleLog()
{
extern MenuHandle hackersMenu;
unsigned char itemname[32];
if(!log_refnum){
	OSErr err;
	my_SF_Put("\pCreate a log file","\psuntar.log");
	if(reply.good){
		if((err=new_text_file(&log_refnum))==noErr){
			if(my_windows[WIN_INDEX].used){
				ParamText("\pSave also the current contents of the console ?",PNS,PNS,PNS);
				if(my_modal_dialog(130,NULL,0)==1){
					void log_menu_command(long);
					long t;

					HLock((**my_windows[WIN_INDEX].TEH).hText);
					t=(**my_windows[WIN_INDEX].TEH).teLength;
					FSWrite(log_refnum,&t,*((**my_windows[WIN_INDEX].TEH).hText));
					HUnlock((**my_windows[WIN_INDEX].TEH).hText);

					log_menu_command(((long)hackID<<16)+hmCreateLog);
					}
				}
			GetIndString (itemname,131,2);
			SetItem(hackersMenu,hmCreateLog,itemname);
			}
		else
			printf("Error %d\n",err);
		}
	}
else{
	FSClose(log_refnum);
	log_refnum=0;
	GetIndString (itemname,131,1);
	SetItem(hackersMenu,hmCreateLog,itemname);
	}
}

void closeLog()
{
if(log_refnum){
	flush_console();
	toggleLog();
	}
}


static void new_console()
{
	/* create the window: since no older window exists, new_window will
	use the index 0 (guessing the index is not good programming style, but
	since suntar uses only one window it works) */
	GrafPtr savePort;
	GetPort( &savePort );
	last_char=CR;
	new_window();
	first_time=false;
	SetPort( &my_windows[WIN_INDEX] );
	TEAutoView (true,my_windows[WIN_INDEX].TEH);
	aggiorna_nome_aperto();
	SetPort( savePort );
}


void disable_autoflush(mode)
short mode;		/*	1: does not flush at the end of a printf, but only when the
						buffer is almost full (and the last character is a newline)
					2: as above, and it does not scroll the window as
						part of the flush */
{
/* the printf routine updates the screen, adjusts the scroll bars and
scrolls the window so that the cursor is in sight, but
a) that takes a lot of time
b) if the printed lines are larger than the window, that may yield a lot of 
    silly horizontal scrolling
Hence, in the most critical sections of suntar updates are delayed and 
scrolls are disabled.
*/
if(first_time)	/* i.e., if no console window still exists... */
	new_console();
no_flush=mode;
}

void enable_autoflush()
{
	no_flush=0;
	flush_console();
}

void flush_console()
{
	GrafPtr savePort;

	if(!my_windows[WIN_INDEX].used) return;
	GetPort( &savePort );
	SetPort( &my_windows[WIN_INDEX] );
	my_flush(true);

	if( last_char==CR||last_char==LF){		/* to avoid horizontal scrolling, don't scroll
			to show the current cursor position if it's not at beginning of line */
		update_console();
		TESelView( my_windows[WIN_INDEX].TEH);
		}
	if(curr_window==&my_windows[WIN_INDEX]){
		SCR_BAR_AND_TEXT_IN_SYNC
		}
/* else 
	per ora la scrollbar � comunque bianca, non importa aggiornarla, poi ci pensa 
	MainEvent al momento dell'evento di attivazione
	-- the scrollbar is currently white, it will be updated by MainEvent at the
	activate event */

	SetPort( savePort );
}


#ifndef SUNTAR
/* suntar does not use it, preferring the more powerful prompt(), but it was
tested and debugged, so you may use it if you need it: it's a full implementation
of the standard gets function, allowing all Mac-style editing tools on the
line which is being typed */
char * gets(buf)
char*buf;
{
short io_start;

/* The window was read-only: now, it's broken in two parts, the bottom
becomes read/write, but any existing text, up to the prompt which
probably you printf-ed just before calling gets, remains read-only;
furthermore, the read-write portion is one line (even Paste is altered
so that you can't paste a newline character) */
flush_all();
io_start=my_windows[WIN_INDEX].lastPrompt=(**my_windows[WIN_INDEX].TEH).selEnd;

/* wait until the user types a return or enter */
do{
	MainEvent();
	}
while(my_windows[WIN_INDEX].lastPrompt!=32767);
/* set the window as read-only again, and copy what appears after the prompt 
in the output string */
HLock((**my_windows[WIN_INDEX].TEH).hText);
{register char *p= buf;
register char *q= *((**my_windows[WIN_INDEX].TEH).hText) +io_start;
register short i=(**my_windows[WIN_INDEX].TEH).teLength-io_start;
if(log_refnum){
		long t=i;
		FSWrite(log_refnum,&t,q);
		}
while(i--)
	*p++=*q++;
*--p='\0';	/* il '\n' non va ritornato... */
}
HUnlock((**my_windows[WIN_INDEX].TEH).hText);
return buf;

}
#endif

void start_of_line()
{
/* guarantees that the last character in the window is a carriage return */
if(last_char!=CR&&last_char!=LF)
	put_char(CR);
}

void vai_a_capo()
{
	if(first_time) new_console();
	put_char(CR);
	if(!no_flush)
		flush_console();

}

void one_empty_line()
/* guarantees that the last two characters in the window are carriage returns */
{
if(last_char!=CR){
	put_char(CR);
	if(last_char!=CR)		/* could be LF now... */
		put_char(CR);
	}
}


void prompt(buf,nbytes)
/* come gets, ma ha un controllo di overflow e un valore iniziale
-- similar to gets, but it has a buffer size parameter and the
text input line has an initial value, which is selected so that you
may delete it by typing any character
*/
char*buf;
short nbytes;
{
short io_start;
GrafPtr savePort;

flush_console();
io_start=(**my_windows[WIN_INDEX].TEH).selEnd;
if(buf[0]){
	short old_log;
	register char*p=buf;
	old_log=log_refnum;
	log_refnum=0;
#if 0	/* now Think C is more rigorous on type checks, and there is no way to match
		the parameters ( '...' in the prototype is NOT compatible with the use of
		vararg !).
		In such cases, the normal solution would be to compile this function with
		less type checking, but in Think C such flags are global, they can't be
		set/reset on part of the project */
	while(*p) put_char(*p++);
	put_char(' ');	/* the space seems superfluous, but without it the whole
			line appears selected, and that's not very pretty to be seen */
#else
	printf("%s ",buf);
#endif
	flush_all();
	log_refnum=old_log;
	}
GetPort( &savePort );
SetPort(&my_windows[WIN_INDEX]);
TESetSelect((long)io_start,(**my_windows[WIN_INDEX].TEH).selEnd-(buf[0]!=0),my_windows[WIN_INDEX].TEH);
update_console();
TESelView( my_windows[WIN_INDEX].TEH);
SetPort(savePort);
if(curr_window != &my_windows[WIN_INDEX])	/* potrebbe esserci un desk accessory */
	SelectWindow(&my_windows[WIN_INDEX]);

my_windows[WIN_INDEX].lastPrompt=io_start;

do{
	MainEvent();
	if(is_abort_command()){
		GetPort( &savePort );
		SetPort(&my_windows[WIN_INDEX]);
		TESetSelect((long)io_start,(long)32767,my_windows[WIN_INDEX].TEH);
		ShowCursor();
		TEDelete(TEH);
		my_windows[WIN_INDEX].lastPrompt=32767;		/* rimettilo readonly */
		SetPort(savePort);
		accept_abort_command();
		}
	}
while(my_windows[WIN_INDEX].lastPrompt!=32767);

HLock((**my_windows[WIN_INDEX].TEH).hText);
{register char *p= buf;
register char *q= *((**my_windows[WIN_INDEX].TEH).hText) +io_start;
register short i=(**my_windows[WIN_INDEX].TEH).teLength-io_start;
if(i>nbytes) i=nbytes;
mcopy(p,q,i);
if(log_refnum){
		long t=i;
		FSWrite(log_refnum,&t,p);
		}
*--p='\0';	/* the '\n' is not included in the string */
}
HUnlock((**my_windows[WIN_INDEX].TEH).hText);
}

#include <stdarg.h>

void printf(const char* fmt,...)
{
va_list ap;
register const char*format=fmt;
long val;
register char*string;
short longform;
short fieldsize;


	if(first_time) new_console();

	va_start(ap,fmt);
	while(*format!='\0'){
		if(*format!='%')
			put_char(*format);
		else{
			format++;
			fieldsize=0;
			if(*format=='.') format++;	/* ATTENTION ! that's because I'm supporting
									only %.10s, and since %10s is not supported, by
									now I simply ignore that character */
			while(*format>='0' && *format<='9')
				fieldsize= fieldsize*10+(*format++-'0');		/* currently the
						fieldsize is supported only by %c %d %ld %u and %lu and
						the precision is supported only by %s */
			if(longform = *format=='l')
				format++;
			switch(*format){
			case '%':
				put_char('%');
				break;
			case 'D':
				longform=1; /* e prosegui...*/
			case 'd':
				if(!longform)
					val= (long) va_arg(ap,short);
				else
					val= va_arg(ap,long);
				{char sign=0;
				if(val<0){
					val= -val;
					sign='-';
					}
				printdec((unsigned long)val,fieldsize,sign);
				}
				break;
			case 'u':
				if(!longform)
					val= (long) va_arg(ap,unsigned short);
				else
					val= va_arg(ap,unsigned long);
				printdec((unsigned long)val,fieldsize,0);
				break;
			case 'X':
				longform=1; /* e prosegui...*/
			case 'x':
				if(!longform)
					val= (long) va_arg(ap,short);
				else
					val= va_arg(ap,long);
				printhex(val,longform);
				break;
			case 'o':
				if(!longform)
					val= (long) va_arg(ap,short);
				else
					val= va_arg(ap,long);
				print_oct(val);
				break;
			case 'c':
				while(--fieldsize >0)
					put_char(' ');
				put_char( va_arg(ap,short) );
				break;
			case 's':
				string= va_arg(ap,char*);
				if(fieldsize==0) fieldsize=32767;
				while(*string && fieldsize--)
					put_char(*string++);
				break;
			case 'P':
/* � un formato non standard C, ma mi fa molto comodo: una stringa formato Pascal
-- a non-standard C format: a Pascal string, extremely useful on the Mac
*/
				string= va_arg(ap,char*);
				val= *string++;
				while(--val>=0)
					put_char(*string++);
				break;
#if 0
			case 'H':
/* another nonstandard format: a C string pointed by an Handle */
				{Handle h= va_arg(ap,(Handle));
				HLock(h);
				string=*h;
				while(*string)
					put_char(*string++);
				HUnlock(h);
				}
				break;
#endif
			default:
				SysBeep(5);	/* invalid format descriptor */
			}
			}
			format++;
		}
	va_end(ap);
	if(!no_flush)
		flush_console();
}

static void printdec(val,fieldsize,sign)
register unsigned long val;
register short fieldsize;
char sign;
{
char buf[14];
register short i=-1;
do{
	buf[++i]='0'+(short)(val%10);
	val /= 10;
	}
while(val);
if(sign) buf[++i]=sign;
fieldsize-=i;
while(--fieldsize > 0)
	put_char(' ');
while(i>=0) put_char(buf[i--]);
}

static void print_oct(val)
register long val;
{
char buf[14];
register short i=-1;
do{
	buf[++i]='0'+(short)(val&7);
	val >>=3;
	}
while(val>0);
while(i>=0) put_char(buf[i--]);
}

static void printhex(val,longform)
register long val;
Boolean longform;
{
short digit,i=longform?28:12;
short non_in_testa=0;
while(i>=0){
	if(!i) non_in_testa=1;
	digit= (val>>i) & 0xF;
	if(digit!=0 || non_in_testa){
		put_char (digit<=9 ? '0'+digit : 'A'-10+digit );
		non_in_testa=1;
		}
	i-=4;
	}
}


char *printf_buf;
static short chars_in_buffer=0;

void put_char(c)
char c;
{
if(c==LF) c=CR;
if(c!= CR || last_char==LF || last_char==CR )
	last_char=c;
else
	last_char=LF;		/* meaning one carriage return but not two, CR means
						at least two carriage returns */
printf_buf[chars_in_buffer++]=c;
if(chars_in_buffer>=list_buf_size || chars_in_buffer>=list_buf_size-64 && c==CR ){
	GrafPtr savePort;
	GetPort( &savePort );
	SetPort( &my_windows[WIN_INDEX]);
	my_flush(no_flush<2);
	SetPort( savePort );
	}
}

void update_console()
{
/* per uno stupido bug: il TextEdit non � pensato per scrivere su una finestra
che sta in background e lo scroll (almeno quello automatico) non provvede a scrollare
anche l'update region, col che l'update successivo � sulla posizione vecchia,
non scrollata, del buco bianco. Unica soluzione, fare un update subito... ovviamente,
per essere sicuri bisognerebbe chiamarla in continuazione, io mi limito a chiamarla
quando chiudo una finestra di dialogo, quando ci sono forti probabilit� di
provocare un update
-- TextEdit has a bad bug: the TESelView routine does scroll the text,
but the update region is not scrolled (that is, OffsetRgn is not called) to 
remember that the "white hole" left by an old window now closed was moved. 
Hence, when the update event arrives it updates part of the window which needn't 
an update and leaves part of the white hole. 
 Only solution by now: don't call TeSelView when the update region is not empty:
that is, either don't use TextEdit to implement the printf function (as a text
editor it's still good, unless you type so quickly that a KeyDown event arrives 
between the window closing and the update) or do your own implementation of TESelView 
or call this function just before calling TESelView
*/
if(!my_windows[WIN_INDEX].used)
	return;
else{
	RgnHandle h=((WindowPeek)&my_windows[WIN_INDEX])->updateRgn;
	RgnPtr p;
	if(h && (p= *h) && p->rgnSize>=10)
		if( *((long*)&p->rgnBBox) != *((long*)&p->rgnBBox.bottom) ) /* this test
				is redundant, but it may avoid a relatively expensive toolbox call */
			if(!EmptyRgn (h)) UpdateWindow(&my_windows[WIN_INDEX]);
	}	
}


static void my_flush(do_scroll)
Boolean do_scroll;
{
if(chars_in_buffer){
	/* window 0 is the console window, and old text must be deleted if the
	TextEdit limit of 32 kbytes is approached... */
	if( ((**my_windows[0].TEH).teLength >maxTElength-4000 ||
		/* according to TN 237, there is another limit to the size of a TextEdit
		record: the destination rect must not be taller than 32767 pixels: 
		that limit is more stringent only if the line height in pixels is bigger
		than the average line length in characters, a very rare situation,
		unless you set the font size to 36 or 48, or use a small autowrap window  */
	    (**my_windows[0].TEH).nLines > 32000/(**my_windows[0].TEH).lineHeight) )
			SilentSuppression(50);

	TEAutoView (false,my_windows[WIN_INDEX].TEH);	/* otherwise, a lot of horizontal scrolling...*/
	TESetSelect((long)32767,(long)32767,my_windows[WIN_INDEX].TEH);
	TEInsert(printf_buf,(long)chars_in_buffer,my_windows[WIN_INDEX].TEH);
	TEAutoView (true,my_windows[WIN_INDEX].TEH);
	if(do_scroll && (last_char==CR||last_char==LF) ){
		while(Button()){		/* sometimes one wants to stop scrolling for a while, 
							in order to read the text, and the Pause command is
							sometimes too much, in this way the scroll stops
							until you release the mouse button: however, menus and
							task switches must not be disabled  */
			EventRecord		myEvent;
			WindowPtr		whichWindow;
			short code;
			OSEventAvail(kNoEvents, &myEvent);	/* we aren't interested in any events */
			code=FindWindow( myEvent.where, &whichWindow );
			if(code==inDesk || code==inMenuBar || code==inSysWindow)
				break;
			if(code==inContent && isDAwindow(whichWindow) )
				break;
			}
		update_console();
		TESelView( my_windows[WIN_INDEX].TEH);
		}
	if(log_refnum){
		long t=chars_in_buffer;
		FSWrite(log_refnum,&t,printf_buf);
		}
	chars_in_buffer=0;
	}
}


void stampa_stringa_stato(unsigned char*);
void stampa_stringa_stato(p)
unsigned char*p;
{
if(!first_time){
	GrafPtr	savePort;
	Rect	nameRect;
	extern short current_font,current_size,current_style;

	nameRect = ((WindowPtr)&my_windows[WIN_INDEX])->portRect;
	nameRect.top = nameRect.bottom-SBarWidth+1;
	nameRect.right = nameRect.left + ROOM_BEFORE_HSCROLL-1;
	nameRect.left ++;

	GetPort( &savePort );
	SetPort(&my_windows[WIN_INDEX]);
	/*EraseRect(&nameRect);*/
	
	TextFont(3);
	TextSize(9);
	TextFace(0);

	TextBox(&p[1],(long)p[0],&nameRect,teJustLeft);
	TextFont(current_font);
	TextSize(current_size);
	TextFace(current_style);

	SetPort( savePort );
	}
}


void aggiorna_nome_aperto()
{

	if(di.tipo_device==d_scsi){
		pStrcpy(nome_file_aperto,"\pSCSI  ");
		nome_file_aperto[6]='0'+di.drive_number;
		}
	else if(di.tipo_device==d_driver){
		pStrcpy(nome_file_aperto,"\pdrive   ");
		if(di.drive_number>=10){
			nome_file_aperto[7]=di.drive_number/10+'0';
			nome_file_aperto[8]=di.drive_number%10+'0';
			}
		else
			nome_file_aperto[7]=di.drive_number+'0';
		}
	else if(di.tipo_device!=d_file)
		nome_file_aperto[0]=0;
	stampa_stringa_stato(nome_file_aperto);

}


void aggiorna_nome_file_aperto()
{
pStrcpy(nome_file_aperto,reply.fName);
aggiorna_nome_aperto();
}

