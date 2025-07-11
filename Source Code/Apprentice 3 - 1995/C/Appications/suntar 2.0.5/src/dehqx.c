
/*******************************************************************************\

BinHex module

part of suntar, �1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/


/* #include <StdFilePkg.h> */

#include <string.h>

#include "suntar.h"
#include "windows.h"

#define ASM


#define hibyte(x)		(((char *) &(x))[0])
#define lobyte(x)		(((char *) &(x))[1])

unsigned short current_crc;
long hqx_length;
static unsigned char conv_tab[256];
short show_info;
unsigned short max_hqx_header;
short info_file_open=0;
short info_file_refnum;
Boolean save_info;
static Boolean EOF_reached;
static short first_in_buffer=0,last_in_buffer=0;
static unsigned char hqxbuf[256];
extern short more_in_bytes;	/* non abusarne !!!! */

#define RUNCHAR 0x90


void raise_hqx_error(void);
void init_CRCtab(void);

unsigned short CRCtable[256];

void close_info_file()
{
if(info_file_open>0){
	flush_hd_buffer();
	FSClose(info_file_refnum);
	}
info_file_open=0;
}

void freeze_info_file(void);
void freeze_info_file()
{
/* I'm at the end of the header: till version 2.0.3 now I closed the file,
but then I realized that a number of hqx files are E-mail letters, the
sender typically places the hqx data at the end of the letter, but anyway
before the signature which is automatically added by the mailer. And
since this signature is really part of the information contained in the
file, ignoring it is not a good thing.
So I flush the buffer, since it's the same buffer used for the binary
data encoded by the hqx file, but I don't close the info file.
*/

if(info_file_open>0 && flush_hd_buffer())
	write_error_message();
}


void termina_info_file(void);
void termina_info_file()
{

if(info_file_open==0){	/* dovrei creare il .info, ma prima dei dati hqx non c'era nulla,
						sarebbe un po' strano crearlo ora... preferisco lasciare
						perdere */
	if(hqx_length > 16 && save_info && di.file_format!=ff_uuencode)	/* well, that variable
					does NOT hold the number of remaining chars, at least if
					those chars are before the final ':' (the buffering does
					not go past that ':'); on the other hand I don't check for 
					the presence of that ':', and I've read that
					"Some old programs produced an extra exclamation mark (!) 
					immediately before the final colon", hence I don't want to
					check that no extra chars are present.
					*/
		printf(in_Italia ? "%ld caratteri dopo i dati Binhex sono stati ignorati\n" :
				"%ld characters follow BinHex data: ignored\n", hqx_length);

	}
else if(info_file_open>0){
	extern ParamBlockRec pb;
	extern char *hd_buffer;
	extern unsigned char mac_file_name[];
	extern long bytes_in_hd_buffer,hd_buffer_size;
	Boolean dummy;
	unsigned char b;
	static char cont_string[]="\r----- BinHex data extracted to ";

	/* reinit_hd_buffering(); non necessario, il buffer � vuoto avendo appena
	chiuso il file binario e quindi flushato */

	if(di.file_format!=ff_uuencode){
		strcpy(hd_buffer,cont_string);
		mcopy(hd_buffer+sizeof(cont_string)-1,&mac_file_name[1],mac_file_name[0]);
		strcpy(hd_buffer+sizeof(cont_string)-1+mac_file_name[0],"\r");
		bytes_in_hd_buffer=sizeof(cont_string)-1+1+mac_file_name[0];
		}
	pb.ioParam.ioRefNum=info_file_refnum;

	if(!EOF_reached && di.file_format!=ff_uuencode)
		while(hqx_length>0 && (b=(unsigned char)get_hqx_byte(&dummy))!=':')
			;	/* skip extra hqx bytes... it should be better to check that
				they are quite few, but if the CRC is correct there should be no
				problems */

	/* now skip extra spaces and newlines */
	b=' ';
	while(hqx_length>0 && conv_tab[b=(unsigned char)get_hqx_byte(&dummy)]== 254 )
		;
	if(conv_tab[b]!=254)
		if(write_hd((char*) &b, 1) ) pbsyserr(&pb);
		
	while(hqx_length>0){
		b=(unsigned char)get_hqx_byte(&dummy);
		if(write_hd((char*) &b, 1) ) pbsyserr(&pb);
		}
	/* if(flush_hd_buffer()) pbsyserr(&pb); no, ci pensa close_info_file */
	close_info_file();
	}
}

short ci_strcmp(p1,p2)
register char *p1,*p2;
/* case-insensitive compare (it's not general purpose: the second string must be
lowercase, and the return value is 0 if equal, 1 if not) */
{
register unsigned char c;
while(*p2){
	if(!(c=*p1++)) return 1;
	if(c>='A'&&c<='Z') c+= 'a'-'A';
	if(c!= *p2++) return 1;
	}
return *p1 != '\0';
}

short is_hqx_name()
{
/* returns 0 if the name does not terminate by .hqx, -1 if it does but .hqx is
preceded by "part" (or "p" preceded by a non-letter) followed by one or two digits, 
1 otherwise.
(obviously, files broken in parts can't be extracted on the fly, and it's customary
to call them contentname.part1.hqx, but we've seen also a_very_long_name.p2.hqx
or long_application_name1.1p3.hqx, and startup10.hqx probably is not a part)
The char comparisons are case-insensitive
*/
register short i,l;
static char p[]={'p','a','r','t'};
l=strlen(full_name);

if(l <=4 ) return 0;

if( ci_strcmp(full_name+l-4,".hqx" ) ) return 0;

/* se sono qui c'era .hqx, ma se c'� anche part devo non convertire */
	
if( l < 9 || tarh.name[l-5] <'0' || tarh.name[l-5] >'9' ) return 1;
if(tarh.name[l-6]>='0'&&tarh.name[l-6]<='9') l--;

if(l<9) return 1;
if( (tarh.name[l-6]|0x20) == 'p' && ((i=tarh.name[l-7]|0x20)<'a'||i>'z') ) return -1;
for(i=0;i<4;i++)
	if( (tarh.name[l-9+i]|0x20) != p[i] ) return 1;
return -1;	/* il .hqx � preceduto da part1 o part2 etc., sarebbe inutile cercare 
				di convertire */
}

Boolean is_uu_name(void);
Boolean is_uu_name()
{
unsigned short l=strlen(full_name);
if(l <=3 || ci_strcmp(full_name+l-3,".uu" ) ) return false;
return true;
}


void check_CRC()
{
/* read the stored CRC value from the BinHex file and compare it with the
computed value
*/


/* per la verit� non sarebbe necessario, il CRC � fatto in modo che se si calcola
su tutti i dati, CRC compreso, il risultato deve essere 0
-- really, the polynomial CRC (such as CCITT CRC) has this property: 
if you don't compute it on 0 twice (remove the two calls) but compute on the 
two CRC bytes (get the value of current_crc after the read_hqx(&not_care,2) ), 
then current_crc is 0 if and only if the computation matches the stored CRC
(that's why a XOR b = 0 iff a=b, and for a couple of properties of the CRC). 
But doing it the same way as the originating routine is easier to understand
*/

#if 1
short calcolato,letto;
CalcCRC(0);
CalcCRC(0);

calcolato=current_crc;
read_hqx(&letto,2);
/*dp("calcolato=%x,letto=%x,corrente=%x len=%ld:%d\n",calcolato,letto,current_crc,
	hqx_length,last_in_buffer-first_in_buffer);*/
if( calcolato!=letto ){
	printf("Invalid CRC\n");
	if(!ignore_errors) raise_hqx_error();
	}
#else
char not_care[2];
read_hqx(not_care,2);
if( current_crc!=0 ){
	printf("Invalid CRC\n");
	if(!ignore_errors) raise_hqx_error();
	}
#endif
}



short hqx_header()
{
/* read the header of a BinHex file and place all informations in a struct
which follows the MacBinary format.
In the file "hqx-format.txt", available in public domain archives, containing
contributions by Dave Johnson and Tom Coradeschi, one can read:
>The header format consists of a one byte name length, then the mac
>file name, then a null.  The rest of the header is 20 bytes long,
>and contains the usual file type, creator/author, file flags, data
>and resource lengths, and the two byte crc value for the header.

*/
static char msg[]="Invalid BinHex header\n";

EOF_reached=false;
/* handle all the text before the BinHex data. If there is nothing else,
return immediately */
FindStart();

if(!hqx_length) return -1;		/* sono gi� in fondo, capita se non ho trovato l'inizio */

current_crc=0;
last_in_buffer=first_in_buffer=0;
read_hqx(&macbinh.nlen,1);			/* file name length */
if(macbinh.nlen==0 || macbinh.nlen>63){
	printf(msg);
	raise_hqx_error();
	}
read_hqx(&macbinh.name,macbinh.nlen);	/* rest of file name */

if(read_hqx(&macbinh.zero,1),macbinh.zero!=0 && !ignore_errors){	/* a zero byte */
	printf(msg);
	raise_hqx_error();
	}

read_hqx(&macbinh.finfo,10);	/* type, creator, Finder flags etc. */
macbinh.protected=0;
macbinh.zero=0;
read_hqx(&macbinh.dflen,8);		/* data fork & resource fork lengths (two long ints) */
/*printf("sizes=%lx %lx\n",macbinh.dflen,macbinh.rflen);*/

/* queste informazioni non c'� bisogno di settarle, basta modificare la routine
che chiama PBSetFInfo in modo che rispetti i valori correnti :
-- the creation and modification dates are missing, but set_binhex will 
	fill those fields

*(long*)&macbinh.finfo.fdLocation = 0;
macbinh.finfo.fdFldr=0;
GetDateTime (&macbinh.cdate);
macbinh.mdate=macbinh.cdate;
*/

check_CRC();
return 0;

}



/***************** findstart *********************/

static char startstr[] = "(This file must be converted with BinHex 4.0)";

void FindStart ()
{
/* routine ispirata a quella (in Pascal) di Peter Lewis, ma perfezionata
-- This routine is vaguely inspired to the Pascal procedure by
Peter Lewis (deHQX.p), but it does not fail if the start string was modified
or deleted from the file. E.g. Downline creates BinHex file but places
in the string its own name rather than BinHex 4.0
*/

short pos,stat;
unsigned short timeout=di.file_format==ff_binhex?65535:max_hqx_header;
short oldpos=0;
register short i,b;
Boolean flush_needed;
extern Boolean all_listonly;

/* per evitare di scandire centinaia di Kbytes per nulla, ho un "timeout" (20 settori) 
e la possibilit� di gestire anche files in cui la scritta di cui sopra � stata tolta, 
col che c'� solo un ":" seguito da uno dei primi codici della stringa hqx (� la 
lunghezza del nome, in teoria max 63 ma in pratica max 31, diviso per 4 per prendere 
solo 6 bit...) i quali codici sono tutti cose strane da stare dopo un : 
-- a ':' at the start of the line must be followed by one of the first codes
in the hqx decoding string (the file name length can't be longer than 12*4+3 =51:
it's not the "official" limit of 63, but ":0" could be meaningful...
The routine is much more complex than it should be due to the explicit 
handling of buffering with both the file and console output 
*/

show_info = SHOWINFO;
save_info = SAVEINFO;

if(show_info) disable_autoflush(2);
stat=0;
pos = 0;
do{
	b = get_hqx_byte(&flush_needed);

/* controllo nel caso manchi la stringa ma ci siano i dati BinHex */
	if(b==LF||b==CR||b=='\f')
		stat=0;
	else if(b==':')
		stat++;
	else if(stat==1 && conv_tab[b]<=12){	/*un ':' a inizio riga seguito da...*/
		unget_char();
		flush_hqx_header(0);
		if( show_info) {
			start_of_line();
			enable_autoflush();
			}
		/* I've NOT found the start string, but since some programs (e.g. Downline)
		use a different string that's not necessary an error: I'm at the beginning
		of hqx data and my job has finished */
		freeze_info_file();
		return;
		}
	else if(stat==0 && (b==' '||b=='\t') )
		;
	else
		stat=2;
/* controllo per il caso non ci siano n� stringa n� dati BinHex */
	if(--timeout==0){
		char buffer[512];
		start_of_line();
		printf(in_Italia?"Non ho trovato dati BinHex nei primi %u bytes\n":
			"BinHex header not found within %u bytes\n",
			di.file_format==ff_binhex?(short)65535:max_hqx_header);
		if(show_info) enable_autoflush();

		if(! save_info)
			raise_hqx_error();
		else{
			long nbytes;
			flush_hqx_header(0);		/* ora passo in modalita' salvataggio 
							normale, vorr� dire che mando tutto nel .info 
							-- BinHex data not found => since the .info file
							contains a copy of all what was read, continue
							to fill it, without any further search */
			while(hqx_length>0){
				if(hqx_length>=512L) nbytes=512; else nbytes= hqx_length;
				hqx_length -= nbytes;
				if(readblock(buffer,nbytes)!=0)
					error_message_1("Read error %d\n",err_code);
				nbytes=macize_ASCII(buffer,nbytes,0);
				if(write_hd(buffer,nbytes))
					write_error_message();
				}
			}
		if(flush_hd_buffer())
			write_error_message();
		close_info_file();
		return;
		}
/* controllo presenza stringa */
	if (b == startstr[pos])
		pos ++;
	else
		pos = b == startstr[0] ? 1: 0;
/* copia dei caratteri sulla console e/o file .info */
	if(flush_needed || pos==sizeof(startstr)-1){
		if(save_info && oldpos!=0 && pos!=sizeof(startstr)-1)
			if(write_hd(startstr,oldpos))	/* � poco pulito farlo cos�, fidandosi
				che il file � aperto e contiene quello che deve contenere...*/
				write_error_message();

		if(show_info && oldpos!=0 && pos!=sizeof(startstr)-1){
			for(i=0;i<oldpos;i++)
				put_char(startstr[i]);	/* la volta prima avevo erroneamente omesso
							dei caratteri ritenendo fossero parte della start string
				-- when the buffer had to be flushed, I could have seen a part 
				of the start string: I didn't save it, but if I'm here than I
				discovered that it was NOT the start string, hence those chars
				had to be written out: for the console, do it here, for the .info
				file it's flush_hqx_header which does that */
			}
		oldpos=pos;
		flush_hqx_header(pos);
		}

	}
while( pos < sizeof(startstr)-1);

/* I've found the start string ! */

freeze_info_file();
if(show_info){
	start_of_line();
	enable_autoflush();
	}

do{
	b = get_hqx_byte(&flush_needed);
	}
while (b!=':');

}

void find_uu_start (void);
void find_uu_start ()
{
/* as above, but for uuencoded files */
short pos;
static char startstr[] = "\nbegin";
short oldpos=0;
register short i,b;
Boolean flush_needed;
extern Boolean all_listonly;

show_info = false;
save_info = !listonly;
pos = 1;	/* being at start of file is being at start of line */
do{
	b = get_hqx_byte(&flush_needed);
	if(b==LF+CR-'\n') b='\n';	/* that is, if it's the other one of the two,
								independently of whether '\n' is CR or LF */

/* controllo presenza stringa */
	if (b == startstr[pos])
		pos ++;
	else
		pos = b=='\n' ? 1 : 0;
/* copia dei caratteri sulla console e/o file .info */
	if(flush_needed || pos==sizeof(startstr)-1){
		if(save_info && oldpos!=0 && pos!=sizeof(startstr)-1)
			if(write_hd(startstr,oldpos))
				write_error_message();

		#if 0
		if(show_info && oldpos!=0 && pos!=sizeof(startstr)-1){
			for(i=0;i<oldpos;i++)
				put_char(startstr[i]);
			}
		#endif
		oldpos=pos;
		flush_hqx_header(pos);
		}
	}
while( pos < sizeof(startstr)-1);

if(flush_hd_buffer())
	write_error_message();
freeze_info_file();
#if 0
if(show_info){
	start_of_line();
	enable_autoflush();
	}
#endif

do
	b = get_hqx_byte(&flush_needed);
while(conv_tab[b]==254);
while(b>='0'&&b<='9')
	b = get_hqx_byte(&flush_needed);
while(conv_tab[b]==254)
	b=get_hqx_byte(&flush_needed);
i=0;
while(b!=CR && b!=LF){
	tarh.name[i++]=b;
	b=get_hqx_byte(&flush_needed);
	}
tarh.name[i]=0;
if(i==0)
	strcpy(tarh.name,"uudecode.out");
full_name=strcpy(nam_buf,tarh.name);

}


/*
io uso tre buffer: quello di uscita � il solito buffer, quello di ingresso 
contiene il blocco letto dal disco; in pi�, le conversioni sono fatte a
gruppi di 4 bytes, col che ho un buffer intermedio che contiene solo ...
-- The BinHex conversion is performed in two steps, by two routines
each having its own input buffer
*/

static unsigned char hqxchars[]=
 "!\"#$%&\'()*+,-012345689@ABCDEFGHIJKLMNPQRSTUVXYZ[`abcdefhijklmpqr";

void init_hqx_tab()
/* initialize the conversion table: need be called only once */
{
register short i;
for( i= 0 ;i<=255; i++)
	conv_tab[i] = 255;
for (i = 0;i<64;i++)
	conv_tab[hqxchars[i]]=i;
conv_tab[LF]=conv_tab[CR]=conv_tab[' ']=conv_tab['\t']=conv_tab['\f']= 254;

init_CRCtab();
}



void read_hqx(destination,nbytes)

/* in suntar 1.2, this routine was changed, optimizing for speed: the 
loop present in the callers was moved here, so that register variables
may hold their values for a while, and are not swapped out of the register
after being used just a couple of times */

register char *destination;
long nbytes;
/* last conversion stage: it handles the RLL encoding in the BinHex format.
From the file "hqx-format.txt":

>There is some run length encoding, where the character to be repeated
>is followed by a 0x90 byte then the repeat count.  For example, ff9004
>means repeat 0xff 4 times.  The special case of a repeat count of zero
>means it's not a run, but a literal 0x90.  2b9000 => 2b90.

>*** Note: the 9000 can be followed by a run, which means to repeat the
>0x90 (not the character previous to that).  That is, 2090009003 means
>a 0x20 followed by 3 0x90's.
*/

{
register unsigned char b;

register unsigned char *first=&hqxbuf[first_in_buffer];
register short k=last_in_buffer-first_in_buffer;
register unsigned short curr_crc=current_crc;
register unsigned short *crc_tab=CRCtable;
register unsigned char cnt;
/*long old_nbytes=nbytes;*/

while(nbytes--){
	if( k < 4 ){
		/* buffer quasi vuoto: riporta quello che resta in testa e riempi di nuovo
		-- refill the buffer by shifting what remains and calling
		read_3_hqx until the buffer is full or the hqx data is over
		*/
		#if 0
		mcopy(hqxbuf,first,k);
		#else
		/* mcopy has a big overhead, since k is <=3 it's better to do that directly... */
		if(k){
			hqxbuf[0]=*first++;
			if(k>=2){
				hqxbuf[1]=*first++;
				if(k==3)
					hqxbuf[2]=*first;
				}
			}
		#endif
		
		first=hqxbuf;
		k += read_3_hqx(&hqxbuf[k],sizeof(hqxbuf)-k);
		last_in_buffer =k;
		if( k<=0 ){
			extern char*hd_buffer;
			extern long bytes_in_hd_buffer,hd_buffer_size;
			beep_in_foreground();
			start_of_line();
			printf("hqx: EOF reached\n");
			if(destination>=hd_buffer && destination<hd_buffer+hd_buffer_size)
				bytes_in_hd_buffer=destination-hd_buffer;
			raise_hqx_error();
			}
		}
	/* sembra troppo semplice per quello che deve fare, ma ho studiato tutti i casi,
	compresi i run di RUNCHAR e la coppia di RUNCHAR, scritto che si deve fare e fatto
	il merge delle azioni comuni fra pi� rami, e questo � il risultato
	-- I decrement the run length count directly in the input buffer, without
	extracting the RLL code from the buffer until the count goes to 0.
	OK, it's not immediately clear that these instructions work: but I considered
	all possible cases and did a flow graph of the resulting operations, than began
	to merge equal boxes at the end of different paths: I've got a lot of merging
	and the result is this very simple set of instructions */
	b=*first++;	
	if(b==RUNCHAR){	/* and first[0] surely 0 */
		if(k <4 || first[1]!=RUNCHAR || !first[2] ){
				first ++; k -=2;
				}
		else{
			if ( --first[2] <=1 ) {
				/* see below */
				if(first[2]==0){
					first += 3; k -= 4;
					}
				else{
					first++; k-=2;
					first[1]=0;
					}
				}
			else
				first--;	/* ho sbagliato a incrementarlo */
			}
		}
	else{
		if (k >=3 && first[0]==RUNCHAR && (cnt=first[1])!=0 ){
/* run length may be repeated, for example A4 90 FF 90 FF 90 0A means A4 is 
repeated 520 times. Unfortunately, suntar 1.1 did not handle those cases correctly */
			first[1]=--cnt;
			if( cnt >1 )
				first--;
			else{
				if(cnt==0){
					first+=2; k-=3;
					}
				else{	/* must copy the char to be repeated so that if there is
						a further run length, it appears to be applied to it; it's
						easier to exit from the decrement state when the count is 1
						and copy anyway than exit when it's 0 and copy only if there
						is a further run length */
					first++; k-=2;
					*first=b;
					}
				}
			}
		else
			k--;	/* caso normale: � il pi� corto e non � seguito da
					un salto incondizionato */
		}

	*destination++ = b;

#ifndef ASM
	CalcCRC(b);
/*dp("CRC su %d(%x)\n",b,current_crc);*/
#else
	/* save parameter passing + function call, and some moves */
/*dp("CRC su %d,",b);*/
	asm{
		rol.w	#8,curr_crc	/* scambia i byte */
		moveq.l	#0,d1
		move.b	curr_crc,d1	/* in d1 il byte alto del current_crc */
		move.b	b,curr_crc	/* in curr_crc byte basso<< 8 + current_byte */
		add.w	d1,d1	/* si potrebbe evitare sfruttando la scalatura degli indici
					del 68020, ma allora bisognerebbe duplicare il loop
					principale per i casi con e senza 68020... */
		move.w	0(crc_tab,d1.w),d1
		eor.w	d1,curr_crc
		}

#endif


	}
first_in_buffer = first - hqxbuf;
current_crc=curr_crc;
}


static char UNEXP_EOF[]="Unexpected end of file reached\n";

#define FAST_GET_HQX_BYTE(b)	\
if(!more_in_bytes)				\
	b=(unsigned char)get_hqx_byte(&dummy);	\
else{							\
	if((--hqx_length)<0){		\
		printf(UNEXP_EOF);		\
		raise_hqx_error();		\
		}						\
	b= (unsigned char)disk_buffer[511-(--more_in_bytes)];\
	}



short read_3_hqx(obuf,maxsize)
/* this one too was optimazed by moving here the loop which
was in the caller */

register unsigned char *obuf;
register short maxsize;
{
/* read four BinHex characters and convert them to three bytes
From the file "hqx-format.txt":
>The first and last characters are each a ':'.  After the first ':',
>the rest of the file is just string of 6 bit encoded characters.
>All newlines and carriage returns are to be ignored.
That is, data is sliced in 6-bit pieces, and 3 bytes (24 bits) yield 4
pieces, each encoded by a printable ASCII char (0='!' etc., see the
conversion table)
*/

Boolean dummy;
unsigned char ibuf[4];
register short i,b;
register short nbytes;
register unsigned char *pti;
short n_errors=0;
static char msg[]="Error: not a BinHex character (dec %d)\n";

nbytes=0;
maxsize -= 3;

while(maxsize>=0 && !EOF_reached){

	nbytes += 3;
	pti=ibuf;
	for(i=0;i<4;i++){
		do{
			FAST_GET_HQX_BYTE(b)
			}
		while((*pti=conv_tab[b])==(unsigned char)254 );
			/* I ignore spaces too: hqx-format.txt does not tell about them,
			but we happened to download a file which did have spaces,
			and neither Stuffit nor Compact Pro succeeded to extract it,
			only Peter Lewis' DeHQX did the job. Now, suntar too does extract
			that file */
		if( *pti++==(unsigned char)255){
			if(b==':'){
				EOF_reached=true;
				nbytes += ((i+i+i)>>2)-3;	/* i*6 bits/8 bits -3 */
				for(;i<4;i++)
					ibuf[i]=0;
				break;
				}
		 	printf(msg,b);
		 	if(!ignore_errors || ++n_errors>=10)
  				raise_hqx_error();
  			else
  				i--,pti--;
  			}
  		}
/*dp("<%x %x %x %x>\n",ibuf[0],ibuf[1],ibuf[2],ibuf[3]);*/
#ifndef ASM
	*obuf++ = ((ibuf[0] << 2) | (ibuf[1] >> 4));	/* these three instructions */
	*obuf++ = ((ibuf[1] << 4) | (ibuf[2] >> 2)); 	/* are from Dave Johnson's xbin.c */
	*obuf++ = ((ibuf[2] << 6) | ibuf[3]);			/* (for UNIX machines) */
#else
/* no visible speed gain, but since I've written and tested it, I prefer to use it... */
	asm{
		lea		ibuf,a0
		move.b	(a0)+,d0
		lsl.b	#2,d0
		move.b	(a0)+,d1
		move.b	d1,d2
		lsr.b	#4,d1
		or.b	d1,d0
		move.b	d0,(obuf)+
		lsl.b	#4,d2
		move.b	(a0)+,d1
		move.b	d1,d0
		lsr.b	#2,d1
		or.b	d1,d2
		move.b	d2,(obuf)+
		lsl.b	#6,d0
		or.b	(a0),d0
		move.b	d0,(obuf)+
		}
#endif

/*dp("{%x %x %x}\n",obuf[-3],obuf[-2],obuf[-1]);*/
	maxsize -= 3;
	}
return nbytes;
}


unsigned char get_hqx_byte(flush_needed)
/* I could implement it by calling readblock, but the hqx decoding has already
so much overhead for each byte that I thought that any instructions saving
was welcome, hence I copied the body of readblock deleting instructions which
are useless when reading one byte at a time
*/
Boolean *flush_needed;
{
short i;

if((--hqx_length)<0){
	beep_in_foreground();
	start_of_line();
	printf(UNEXP_EOF);
	raise_hqx_error();
	}

if(more_in_bytes==0){
	*flush_needed=false;
/* le stesse cose che fa readblock... */
	if(bar_archive)
		bar_check_floppy_swap(0);
	else
		tar_check_floppy_swap(0);

	leggi_settore(sect_n,&disk_buffer);

 	if((i=check_error_and_events())<0) raise_error();
 	if(i>0){	/* "missing disk" was converted to a pause event, now the disk
 				should be in, but I must repeat the operation */
		leggi_settore(sect_n,&disk_buffer);
 		if(check_error()) raise_error();
 		}
	more_in_bytes=511;
	sect_n++;
	settori_passati++;
	}
else
	*flush_needed= !--more_in_bytes;
return disk_buffer[511-more_in_bytes];
}


short enough_space_for_info(void);
static short enough_space_for_info()
{
short vrefnum;
long l;
short i;

	if(curr_vrefnum)
		vrefnum=curr_vrefnum;
	else{
		volumeParam param;
		char buffer[50];
		param.ioNamePtr=buffer;
		if(PBGetVolSync (&param) !=noErr) return 0;
		vrefnum=param.ioVRefNum;
		}
	if(GetVInfo (0,NULL,&vrefnum,&l)!=noErr)
		return 0;
	if( l >= min(10240L,hqx_length+1024L))	/* I don't know the size, but it can't
						be bigger than the whole file (hqx_length was already
						decremented, hence I add something to it) and very rarely
						it's bigger than 10 K */
		return 0;

	for(;;){
		extern char *titoli_full[];
		i=semimodalDialog(141,NULL,NULL,titoli_full,3,
			   in_Italia?"\pSpazio su disco scarso: puoi voler cancellare\r\
qualcosa prima di cliccare su un bottone\roppure puoi abortire il comando":
"\pScarce space on destination volume: you\r\
might wish to delete something before clicking a\r\
button or you may abort the current command",
			   NULL,NULL,teJustCenter,true,NULL);
		if(i==2){
		 	/* continue elsewhere */
			select_directory();
			if(reply.good) return 0;
			}
		else if(i==3)	/* skip */
			return 1;
		else
			return 0; /* continue here: maybe the user deleted some files,
					or thinks that this .info file is smaller than the available
					space */
		}
}

void open_info_file()
/* creates the .info file: that includes building its name... */
{
register short i,l;
extern OSType filecreator,filetype;
extern Boolean devo_chiudere_out;
char *ext= di.file_format==ff_uuencode?".uu":".hqx";
short lext;
lext=strlen(ext);

/* compute the name */

l=strlen(full_name);

if(l>lext && !ci_strcmp(full_name+l-lext,ext)){
	for(i=l-1;i>=0 && (full_name)[i]!='/';i--)
		;
	/* ora i � -1 o punta ad un '/', quindi la lunghezza del nome � : */
	i=l-i-1;
	if(i>=31)
		strcpy(full_name+l-lext+1, "inf");	/* overwrites hqx\0 */
	else{
		/*SetHandleSize(full_pathname,l+2);*/
		strcpy(full_name+l-lext+1,"info");
		}
	}
else{	/* if opened by Open File, it may not contain .hqx */
	/*SetHandleSize(full_pathname,l+6);*/
	strcat(full_name,".info");
	}
/*unix_to_mac();*/

filecreator=text_creator;
filetype='TEXT';

/* now, check for enough space (must have a private routine for that since
I don't know how long the .info file will be) */

if(enough_space_for_info()){
	info_file_open=-1;		/* skip this file... */
	return;
	}

/* finally, open the file ! */

init_deDOSize();
if(create_file(my_c2pstr(full_name),fsWrPerm)==0){
	extern ParamBlockRec pb;
	devo_chiudere_out=false;
	info_file_refnum=pb.ioParam.ioRefNum;
	info_file_open=1;
	/* no need to use connect_to_hd_buffering, it uses create_file */
	}
else
	info_file_open=-1;	/* duplicate name + cancel: don't create an info file ! */
my_p2cstr(full_name);	/* must be a C string for being used for the contained file */
}


void flush_hqx_header(pos)
short pos;
/* it's rather complex: it must write data to the console and the .info
file, but both of them may be disabled.
Furthermore, it's called when the buffer is officially empty, but obviously 
its bytes still contain the characters which FindStart has read and discarded
since they were not the start of the BinHex data
*/
{
/* attenzione, faccio affidamento sul fatto che nel buffer ci restano i bytes letti,
e che grazie all'ubbidienza al parametro flush_needed della routine precedente 
quelli non ancora salvati partono all'offset 0 */
extern short log_refnum;
register short i,j;
if( ! save_info && ! show_info ) return;

j=512-more_in_bytes-pos;
if(!j) return;
if(show_info==1 && j>100)
	printf("=============== info ===============\n\n");	/* la prima volta */
if(show_info){
	register char last_char;
	show_info++;
	if(log_refnum&&show_info>2) show_info=2;
	disable_autoflush(2);	/* in alcuni casi un messaggio (nuovo disco...) pu� averlo
							abilitato */
	for(i=0;i<j;i++){	/* butto fuori il contenuto del buffer, ma 
						togliendo un eventuale inizio della start string
					-- pos!=0 means that the last pos chars might be part of the
					start string, I must not write them
					*/
		if(last_char==CR){
			last_char=disk_buffer[i];
			if(last_char!=LF) put_char(last_char);
			}
		else{
			last_char=disk_buffer[i];
			put_char(last_char);
			}
		
		if((last_char==CR || last_char==LF) && show_info>=4 && more_in_bytes==0){
			printf(in_Italia?"... (testata lunga, non mostrata per intero)\n\n\n":
			"... (long header, further lines are not shown)\n\n\n");
			enable_autoflush();
			show_info=0;
			}
		}
	}

if( save_info && j>0){
	if(!info_file_open){
		if(j<20){	/* since the file data always starts at the beginning
			of the buffer, it may happen only after finding the start of the hqx data,
			so the whole info file would be so short, and probably it's only blank space ! */
			Boolean gooddata=false;
			for(i=0;i<j;i++){
				if(disk_buffer[i]!=CR && disk_buffer[i]!=CR && disk_buffer[i]!=' ' &&
				   disk_buffer[i]!='-')
					gooddata=true;
				}
			if(!gooddata) return;
			}
		open_info_file();	/* the info file is opened on the fly
			here, since FindStart did not open it: if no text precedes the
			BinHex data, the info file is not created */
		}
	if(info_file_open>0){
		short nbytes=macize_ASCII(disk_buffer,j,0);
		if(write_hd(disk_buffer,nbytes)){
			beep_in_foreground();
			write_error_message();
			}
		}
	}
}

void hqx_end_of_file()
{
hqx_length -= more_in_bytes;	/* caution: the buffer may contain up to 511 bytes
		when hqx_length is 0, and hqx_length may be -1 (it's decremented before
		testing it) hence the result may be negative */
more_in_bytes=0;
if(hqx_length>0)
	sect_n+= (hqx_length+511) >>9;
hqx_length=0;
}

static void raise_hqx_error()
/* handles the hqx decoding error by skipping the rest of the file and closing
the open files so that untar_hqx will be able to return regularly, and the
extraction or list will continue with next file */
{

if(di.file_format==ff_uuencode||di.file_format==ff_binhex) raise_error();

close_or_del_out_file();
close_info_file();
enable_autoflush();

hqx_end_of_file();

if( fase==reading_disk || fase==selected_reading || fase==hack_listing){
	if(! (fase==reading_disk&&listonly || fase==hack_listing) ){
		one_empty_line();
		printf(in_Italia?"****** Estrazione BinHex interrotta":
		"****** BinHex extraction was aborted");
		printf(" ******\n\n");
		}
	longjmp(main_loop,-2);
	}
else
	longjmp(main_loop,-1);

}


void init_CRCtab()
{
int i;
register unsigned short curr_crc;
for(i=0;i<256; i++){
	/* since this routine is called only once, one might use a slower
	computation in C, or avoid it altogether initializing CRCtable
	by a list of constants created running this function only once
	on the programmer's computer and then deleting this routine */
	curr_crc=i<<8;
	asm{
		moveq	#0,d1
		
		move.w	#7,d2
	loop:
		lsl.b	#1,d1
		roxl.w	#1,curr_crc
		bcc.s	@noxor
		eor.w	#0x1021,curr_crc	; CRC CCITT
	noxor:
		dbra	d2,@loop
		}
	CRCtable[i]=curr_crc;
	}
}


/* assembly language is faster: since you must repeat the body of the loop 
for each BIT (not byte !) of the file, a fast CRC routine is essential if
your BinHex routine must not be very slow...
Well, later I discovered that CRC computation may be performed by table
lookup, and obviously a nonciclic operation is faster than a loop which
must be repeated 8 times per byte
*/
#ifdef ASM

#if 1
void CalcCRC(current_byte)
char current_byte;
{
/* scritta da Gabriele Speranza, 17 luglio 1993
I don't know who invented this method: I've seen it in unarj, but
that was incompatible with the CCITT CRC, I had to use
the mathematical definition of polynomial modulo 2 division
in order to understand what the formula had to be
*/
#ifndef ASM
current_crc = ((lowbyte(current_crc)<<8) + (unsigned char)current_byte) ^ CRCtable[highbyte(current_crc)];
#else
asm{
	move.w	current_crc,d0
	rol.w	#8,d0	/* scambia i byte */
	moveq.l	#0,d1
	move.b	d0,d1	/* in d1 il byte alto del current_crc */
	move.b	current_byte,d0	/* in d0 byte basso<< 8 + current_byte */
	add.w	d1,d1	/* on a 68020 or later may be avoided by using the "scaled indexed" addressing mode */
	lea		CRCtable,A0
	move.w	0(A0,d1.w),d1
	eor.w	d1,d0
	move.w	d0,current_crc
	}
#endif
}

#else

void CalcCRC(current_byte)
char current_byte;
/* from calcCRC.a in the deHQX source code by Peter Lewis, adapting
it to the C parameter passing conventions and the Think-C half-assembly-half-C
syntax
*/
{
asm{

	move.w	current_crc,d0
	move.b	current_byte,d1
	
	move.w	#7,d2
loop:
	lsl.b	#1,d1
	roxl.w	#1,d0
	bcc.s	@noxor
	eor.w	#0x1021,d0	; CRC CCITT
noxor:
	dbra	d2,@loop
	move.w	d0,current_crc
	}
/*dp("CRC su %d(%x)\n",current_byte,current_crc);*/
}
#endif

#else

void CalcCRC (v)
{
/* I don't remember where I've got this one, anyway in public domain */
short temp;
register short 	i;

	for (i = 1;i<=8;i++){
		temp = current_crc&0x8000;
		current_crc = (current_crc<<1) | (v>>7)&1 ;
		if( temp )
			current_crc ^= 0x1021;
		v <<= 1;
		}
}
#endif

/***************************************************************/

/*			hqx encoding routines 		*/

/* suntar is mainly an extractor, but I had to write these routines since there
was a MacBinary encoder in the command "New" and BinHex is just as much important
(BinHex is not offered as a format when writing to a tar archive, but that's
because the length of BinHex files can't be determined before starting the
conversion (there is a form of compression...) hence one does not know what
to write in the tar header)
*/

long current_run_length;	/* just to be safe with overflows */
unsigned char repeated_char;
#define bytes_on_current_line first_in_buffer

static char colon_cr[]={':',CR,CR,':'};

void init_hqx_encoding(struct binh_type*);
void fine_binhexed(void);
void encode_hqx_bytes(char*,short);
void write_run_length(void);
void write_binhexed(void);


void init_hqx_encoding(macbinheader)
struct binh_type *macbinheader;
{
short j;
last_in_buffer=0;
current_run_length=0;
repeated_char='\0';

writeblock(startstr,sizeof(startstr)-1);

writeblock(&colon_cr[1],3);

bytes_on_current_line=1;	/* the ':' */
current_crc=0;

encode_hqx_bytes(&macbinheader->nlen,macbinheader->nlen+1);
j=0;
encode_hqx_bytes(&j,1);
encode_hqx_bytes(&macbinheader->finfo,10);
encode_hqx_bytes(&macbinheader->dflen,8);
CalcCRC(0);
CalcCRC(0);
j=current_crc;
encode_hqx_bytes(&j,2);
}


void encode_hqx_bytes(buffer,n_bytes)
register char *buffer;
short n_bytes;
{
while(n_bytes--){
	CalcCRC(*buffer);
	if(*buffer==repeated_char)
		current_run_length++;
	else{
		if(current_run_length){
/*dp("runlength di %d per %ld volte\n",repeated_char,current_run_length);*/
			write_run_length();
			}
		current_run_length=1;
		repeated_char=*buffer;
		}
	/*if(*buffer>' '&&*buffer<127)dp("encoding %d(%c)\n",*buffer,*buffer);
	else dp("encoding %d\n",*buffer);*/
	buffer++;
	}
}

void write_run_length()
{
/*dp("runlength di %d per %ld volte\n",repeated_char,current_run_length);*/
while(current_run_length>255){
	/* per la verit� non � necessario ripetere il carattere ogni volta,
	ma farlo riduce la probabilit� di problemi, e poi quando non lo faceva
	non mi funzionava e non ho troppa voglia di scoprire dove era il guaio
	visto che adesso va */
	hqxbuf[last_in_buffer++]=repeated_char;
	if(repeated_char==RUNCHAR) hqxbuf[last_in_buffer++]=0;
	if(last_in_buffer>=sizeof(hqxbuf)-1) write_binhexed();	/* room for zero or one
		byte, but this routine often writes two bytes without further checking
		so it's better to consider the buffer as full */
	current_run_length-=255;
	hqxbuf[last_in_buffer++]=RUNCHAR;
	hqxbuf[last_in_buffer++]=255;
	if(last_in_buffer>=sizeof(hqxbuf)-1) write_binhexed();
	}
if(current_run_length>=3	/*	run lengths are 3 bytes long hence are convenient only
	for at least 4 chars, but runs of 0x90 are 4 bytes long, while 0x90 must
	be encoded as two bytes, hence they are convenient for 3, and since normal
	characters are not worse for 3... */
   ){
	hqxbuf[last_in_buffer++]=repeated_char;
	if(repeated_char==RUNCHAR) hqxbuf[last_in_buffer++]=0;
	if(last_in_buffer>=sizeof(hqxbuf)-1) write_binhexed();
	hqxbuf[last_in_buffer++]=RUNCHAR;
	hqxbuf[last_in_buffer++]=current_run_length;
	if(last_in_buffer>=sizeof(hqxbuf)-1) write_binhexed();
	current_run_length=0;
	}
else{
	while(current_run_length--){
		hqxbuf[last_in_buffer++]=repeated_char;
		if(repeated_char==RUNCHAR) hqxbuf[last_in_buffer++]=0;
		if(last_in_buffer>=sizeof(hqxbuf)-1) write_binhexed();
		}
	}
/*current_run_length=0;*/
}


void write_binhexed()
{
extern short more_space;
unsigned char b[4];
register unsigned char*p=hqxbuf;
register unsigned char *q;
short i;

while(last_in_buffer>=3){
	last_in_buffer-=3;
/*dp("in =%x %x %x\n",p[0],p[1],p[2]);*/

	b[0]= hqxchars[(*p>>2) /* &0x3F*/];
	b[1]= hqxchars[((*p<<4)&0x30) + ((*(p+1)>>4) /*&0xF*/)];
	p++;
	b[2]= hqxchars[((*p<<2)&0x3C) + ((*(p+1)>>6)/*&0x3*/)];
	p++;
	b[3]= hqxchars[*p&0x3F];
	p++;
/*dp("out=%x %x %x %x\n",b[0],b[1],b[2],b[3]);*/
	if(bytes_on_current_line<=60){
		if(more_space>=4){
			q=&disk_buffer[512-more_space];
			*q++=b[0];
			*q++=b[1];
			*q++=b[2];
			*q=b[3];
			more_space-=4;
			}
		else/**/
			writeblock(b,4);

		bytes_on_current_line+=4;
		}
	else{
		for(i=0;i<4;i++){
			if(bytes_on_current_line==64){
				writeblock(&colon_cr[1],1);
				bytes_on_current_line=0;
				}
			bytes_on_current_line++;
			writeblock(&b[i],1);
			}
		}
	}
#if 0
if(last_in_buffer) mcopy(hqxbuf,p,last_in_buffer);	/* one or two bytes... */
#else
if(last_in_buffer){
	hqxbuf[0]=*p++;
	if(last_in_buffer==2)
		hqxbuf[1]=*p;
	}
#endif
}


void encode_hqx_fork(long);
void encode_hqx_fork(fsize)
/* obviously it's inspired to tar_writefork */
  long fsize;
{
extern unsigned char repeated_char;
extern long current_run_length;
extern Boolean devo_chiudere_in;
extern char*hd_buffer;
extern long hd_buffer_size;

void write_run_length(void);
void encode_hqx_bytes(char*,short);

register unsigned char*p;
register long i;
register unsigned char reg_rep_char;
register long reg_curr_run_len;
short j;

current_crc=0;
pb.ioParam.ioPosMode = fsAtMark;
pb.ioParam.ioBuffer = hd_buffer/* (Ptr) &tarh*/; /* this buffer is currently unused, hence
			I may use it without telling that to anybody else */
pb.ioParam.ioReqCount = hd_buffer_size;
reg_rep_char=repeated_char;
reg_curr_run_len=current_run_length;
while (fsize) {
	if (fsize < pb.ioParam.ioReqCount)
		pb.ioParam.ioReqCount = fsize;
	i=pb.ioParam.ioReqCount;
	fsize -= pb.ioParam.ioReqCount;
	if (PBReadSync(&pb) || pb.ioParam.ioActCount != pb.ioParam.ioReqCount) {
		short err = pb.ioParam.ioResult;
		(void) PBCloseSync(&pb);
		devo_chiudere_in=false;
		pb.ioParam.ioResult = err;
		pbsyserr(&pb);
		}
	/* since encode_hqx_bytes is so short, I don't call it and save a call, a return
	and all the register saving/restoring */
	p=hd_buffer/*tarh.name*/;
	while(i--){
		if(*p==reg_rep_char)
			reg_curr_run_len++;
		else{
			if(reg_curr_run_len==1){	/* optimize by doing that directly */
				hqxbuf[last_in_buffer]=reg_rep_char;
				if(reg_rep_char==RUNCHAR) hqxbuf[++last_in_buffer]=0;
				if(++last_in_buffer>=sizeof(hqxbuf)-1) write_binhexed();
				}
			else if(reg_curr_run_len){
				current_run_length=reg_curr_run_len;
				repeated_char=reg_rep_char;
				write_run_length();
				}
			reg_curr_run_len=1;
			reg_rep_char=*p;
			}
		#ifndef ASM
		CalcCRC(*p);
		#else
		asm{
			move.w	current_crc,d0
			rol.w	#8,d0
			moveq.l	#0,d1
			move.b	d0,d1
			move.b	reg_rep_char,d0
			add.w	d1,d1
			lea		CRCtable,A0
			move.w	0(A0,d1.w),d1
			eor.w	d1,d0
			move.w	d0,current_crc
			}
		#endif
		/*if(*p>' '&&*p<127)dp("encoding %d(%c)\n",*p,*p);
		else dp("encoding %d\n",*p);*/
		p++;
		}
	}
current_run_length=reg_curr_run_len;
repeated_char=reg_rep_char;
CalcCRC(0);
CalcCRC(0);
j=current_crc;	/* can't use the original, which will be altered... */
encode_hqx_bytes(&j,2);

get_openfile_location(pb.ioParam.ioRefNum);

if (PBCloseSync(&pb)) pbsyserr(&pb);
devo_chiudere_in=false;
}


void fine_binhexed()
{
void svuota_buffer_binhex(void);

write_run_length();
write_binhexed();

if(last_in_buffer){	/* less then a triple, that is one or two bytes... */
	unsigned char b[4];
	short i;
	hqxbuf[last_in_buffer] /*=	hqxbuf[last_in_buffer+1]*/= 0;

/*dp("ultimo:in=%x %x %x\n",hqxbuf[0],hqxbuf[1],hqxbuf[2]);*/
	b[0]= (hqxbuf[0]>>2) /* &0x3F*/;
	b[1]= ((hqxbuf[0]<<4)&0x30) + ((hqxbuf[1]>>4) /*&0xF*/);
	b[2]= ((hqxbuf[1]<<2)&0x3C) /*+ ((hqxbuf[2]>>6)&0x3) */; /* hqxbuf[2] is 0 */
	/*b[3]= hqxbuf[2]&0x3F;*/
/*dp("out=");*/

	for(i=0;i<=last_in_buffer;i++){	/* should be i<(last_in_buffer*4+2)/3, but for 1 or 2... */
		if(bytes_on_current_line==64){
			writeblock(&colon_cr[1],1);
			bytes_on_current_line=0;
			}
		bytes_on_current_line++;
/*dp("%x ",b[i]);*/
		b[0]=hqxchars[b[i]];
		writeblock(&b[0],1);
		}
/*dp("\n");*/
	}
writeblock(&colon_cr[0],2);
svuota_buffer_binhex();
}

void encode_uu_datafork(long);
void encode_uu_datafork(fsize)
/* obviously it's inspired to tar_writefork */
  long fsize;
{
extern char*hd_buffer;
extern long hd_buffer_size;
#define ENC(c) ((c) + ' ')

register unsigned char*p,*q;
register long i;
register short chars_in_line=0;
char buffer[62];

buffer[0]=CR;
writeblock(buffer,1);

pb.ioParam.ioPosMode = fsAtMark;
pb.ioParam.ioBuffer = hd_buffer/* (Ptr) &tarh*/; /* this buffer is currently unused, hence
			I may use it without telling that to anybody else */
pb.ioParam.ioReqCount = hd_buffer_size - hd_buffer_size%3;	/* it's much better if it's
															a multiple of 3 */
q=&buffer[1];
while (fsize) {
	if (fsize < pb.ioParam.ioReqCount)
		pb.ioParam.ioReqCount = fsize;
	i=pb.ioParam.ioReqCount;
	fsize -= pb.ioParam.ioReqCount;
	if (PBReadSync(&pb) || pb.ioParam.ioActCount != pb.ioParam.ioReqCount) {
		short err = pb.ioParam.ioResult;
		(void) PBCloseSync(&pb);
		devo_chiudere_in=false;
		pb.ioParam.ioResult = err;
		pbsyserr(&pb);
		}
	p=hd_buffer/*tarh.name*/;
	/*while(i) moving the test on i==0 after "if(i<3) saves a comparison on each
	execution of the body of the loop */
	for(;;){
		if(i<3){	/* since the buffer is a multiple of 3, may happen only
					at the end of file */
			if(i==0) break;
			p[i]=0;		/* I never like to write garbage to the output file, */
			p[i+1]=0;	/* if extra bytes mst be sent I like to write zeros,
						not any thing is currently in the buffer */
			chars_in_line += i;
			i=0;
			}
		else{
			chars_in_line+=3;
			i-=3;
			}
		*q++=ENC((*p>>2) /* &0x3F*/);
		*q++=ENC(((*p<<4)&0x30) + ((*(p+1)>>4) /*&0xF*/));
		p++;
		*q++=ENC(((*p<<2)&0x3C) + ((*(p+1)>>6)/*&0x3*/));
		p++;
		*q++=ENC(*p&0x3F);
		p++;
		
		if(chars_in_line==45){	/* since chars_in_line is increased by 3 except at the
								end of file, it can't go beyond 45 without being
								exactly equal to 45 */
			*q=CR;		/* !!! attento, solo se su file, quando potr�
						scrivere su archivio tar deve essere LF */
			buffer[0]=ENC(45);
			writeblock(buffer,62);
			chars_in_line=0;
			q=&buffer[1];
			}
		}
	}
/* a questo punto ovviamente devo flushare il buffer */
if(q!=&buffer[1]){
	*q++=CR;
	buffer[0]=ENC(chars_in_line);
	writeblock(buffer,q-buffer);
	}
writeblock(" \rend\r",6);
#undef ENC
}

