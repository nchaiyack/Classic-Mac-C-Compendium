/* Copyright 1988, Gail Zacharias.  All rights reserved.
 * Permission is hereby granted to copy, reproduce, redistribute or
 * otherwise use this software provided there is no monetary profit
 * gained specifically from its use or reproduction, it is not sold,
 * rented, traded or otherwise marketed, and this copyright notice
 * and the software version number is included prominently in any copy
 * made.
 * This was muntar version 1.0.
 *
 */

/* the original muntar program was then altered to become part of suntar:
*/

/*******************************************************************************\

tar reading module

part of suntar, ©1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/

/*
comments in the text explain which routines were added for suntar by us and
which ones are from muntar, but most of the latter were modified more or less 
heavily by us. In fact, the file is divided into four sections:
a few declarations by Gail Zacharias
a series of routines 100% by Speranza
a series of routines containing also old code by Zacharias but which do new things
a series of old routines by Zacharias: none of them is still in the original
	form, but the calling interface is usually unchanged
*/

#define USA_CURRVREFN



#include "PB_sync.h"
#include "antiglue.h"
#include "windows.h"
#include "suntar.h"

/*#include <FileMgr.h>
#include <OSUtil.h>
#include <HFS.h>*/
#include <string.h>
/* #include <errors.h> */

#define ASM

#define tarsz(n)    (((n)+511L) & -512L)
#define  macbinsz(n)    (((n)+127L) & -128L)

char *strrchr();
OSErr create_directory (char *);
void check_confirmation(void);
void check_conf_dir(char*);
void touch_if_needed(void);


struct binh_type macbinh;
union dsk_buffer dsk_buffer;
tarh_type tarh;
long unixtime;

char * full_name=NULL;
char nam_buf[512];

OSType filecreator,filetype;
unsigned char listonly=0;

/* alcune routines sue un po' riscritte (ho il sospetto esistano in libreria o nella ROM,
ma non so con che nomi e che parametri, tanto vale riscriversele
*/


StringPtr my_c2pstr(p)	/* had to change the name since TH C 5 has its own
						define for c2pstr, and a define takes precedence */
register char p[];
{
register short i,len=strlen(p);

/* if(len>255) printf("colpevole=!%.255s!\n",p); */
if(len>255) error_message("Pascal string too long\n");
for(i=len;i>0;i--)
	p[i]=p[i-1];
p[0]=len;
return (StringPtr) p;
}

char * my_p2cstr(p)
register unsigned char p[];
{
register short i,len=p[0];
for(i=0;i<len;i++)
	p[i]=p[i+1];
p[len]='\0';
return p;
}

/* main() l'ho tolto di torno...
{
printf(
"This is freeware.  If you paid money for this program, you got ripped off.\n");
  return;
}
*/

static short binary=true; /* se convertire LF-CR o no */
#define UNIXTIME	2082844800L  /* Jan 1, 1970 00:00:00 */

#ifndef ASM
/* simple implementation */
void mcopy(dest, src, len)
register char*dest,*src;
register unsigned short len;
{
	while ((short)--len>=0) *dest++ = *src++;
}
#endif


/******************** start of routines by Speranza ***********************/

#ifdef ASM
void mcopy(char * dest,char * src,unsigned short len )

/* optimazed implementation, remembering that the 68000 is a 32 bit machine
and may move 4 bytes at a time, at least if they are placed at an even address
(since the compiler places at even addresses the start of all data structures, 
that means almost always)
By Gabriele Speranza, 17 Nov 1991

on a Macintosh LC a sequence of 1800 mcopy(b1,b2,512) takes 20 ticks with this
version and 75 with the original one, 23 with BlockMove.
BlockMove should be faster on long moves, but:
a) certainly its overhead is bigger (it's a trap) while I'm using mcopy
   only for short blocks
b) it may have side effects (e.g. on the Quadras it invalidates the istruction
   cache) and I hate to call functions with unknown, incontrollable side effects
c) I do NOT want to spend my life searching for the best way to do a thing. It
   suffices to avoid very bad ways to do it. I'm an engineer and my first
   concern about my programs is that they must work, and work reasonably well.
   Perfection is not my goal.
*/

{
extern SysEnvRec	gMac;
/* #define EXPLOIT_020 */
asm {
	move.l	dest,a0
	move.l	src,a1
	move.w	len,d1
		/* 
		#ifndef EXPLOIT_020 / * suntar uses mcopy only for small * /
			/ * copies, and those which are not aligned are rarely big, * /
			/ * probably it's not worth of it, that symbol is NOT defined * /
		if(len>=10 && !((((long)dest)^((long)src))&1) ){
			/ *	if the transfer is long enough and the pointers have the same parity * /
		#else
		if(len>=10 && ( gMac.processor>=env68020 || !((((long)dest)^((long)src))&1) ) ){
			/ * the 68020 and later CPUs do not require same parity * /
		#endif
		*/
	cmp.w	#10,d1
	blt.s	@inloop
	move.l	a0,d0
#ifdef EXPLOIT_020
	cmp.w	#env68020,gMac.processor
	bge		@ok_to_movelong
#endif
	move.l	a1,d2
	eor.l	d0,d2
	lsr.l	#1,d2
	bcs.s	@inloop
ok_to_movelong:

		/* if(((long)dest)&1){
				*dest++ = *src++;	/ * both odd=>move one byte to get both even * /
				len--;
				}
		*/
	lsr.l	#1,d0
	bcc.s	@l1
	move.b	(a1)+,(a0)+
	subq.w	#1,d1
	addq.l	#1,d0	/* se il LSB era 1, aggiungere 1 provoca carry sul bit 1 
					-- bit 0 was shifted out, and was 1: adding 1 to bit 0 will cause
					a carry to bit 1, but bit 1 was shifted to bit 0...*/
		/* if(((long)dest)&2){
			/ * now I have two even addresses, and that's good. But a destination 
			at an address multiple of 4 better exploits a 32 bit bus.
			Obviously, the same is for the source, but in 50% of cases aligning one
			aligns the other too, and the cache works better when reading, hence a 
			misalignment on reading is less bad then on writing.
			Really, I've an LC where the bus is 16 bit wide, and the cache is used
			only for instructions, so this trick is not fully exploited on my machine! * /
			*((short*)dest)++ = *((short*)src)++;
			len -= 2;
			}
		*/
l1:	lsr.l	#1,d0
	bcc.s	@l2
	move.w	(a1)+,(a0)+
	subq.w	#2,d1
		/*
			for(temp=len/4;temp;temp--)
				*((long*)dest)++ = *((long*)src)++;
			len %=4;
		*/
l2:	move.w	d1,d0
	lsr.w	#2,d0
	subq.w	#1,d0
	/* on any 680x0 except 68010 it should be faster by unfolding the loop, i.e.
	performing 4 or more move.l per dbra: I've read that BlockMove is done
	in that way */
loop1:
	move.l (a1)+,(a0)+
	dbra d0,@loop1
	and.w	#3,d1	/* up to 3 further bytes may have to be moved ... */
		/*
			}	/ * end if: now move byte-wise anything is remaining, 
				that is all the job if couldn't move long-wise * /
			while (len-- >0) *dest++ = *src++;
		*/
	bra.s	@inloop
loop2:	move.b (a1)+,(a0)+
inloop:	dbra d1,@loop2
	}
}
#endif


Boolean chiedi_su_questo(short,sector_t,char*,Str255,Str255,Str255);
OsErr verifica_nome(short,sector_t *);
short isMacBin2(unsigned char*);
void make_badname(unsigned char*,Boolean);
Boolean too_long(unsigned char *);
Boolean is_gif_file(char*,long);
short gestisci_nome_doppio(unsigned char*,FileParam *,Boolean);
static void skip_macbinII(short);
void cancella_file_aperto(short);
void print_containing(void);
short inserzione_assicurata(char*,Boolean,Boolean);
void untar_hqx (long);
void set_binhex(void);
void setdata (void);
void untar_data (long,short);
void untar_macbin (long);
void writefork(short,long,short);
void untar_directory (void);
void untar_link (void);
void untar_long_name(void);
void untar_long_linkname(void);
void print_fullname(void);
void test_for_uu(void);
short is_macbin(char*,long,short);
void termina_info_file(void);


void write_uu_fork(void);
void uudecode (long);
void attacca_path_e_nome(void);

jmp_buf main_loop;
long file_date;
unsigned char mac_file_name[120];	/* deve poter contenere un path completo...*/
short openfile_vrefnum;
long openfile_dirID;
extern OSType macbin_creator;

short more_in_bytes;	/* I don't remember the number of bytes currently in the 
					buffer, but more_in_bytes = 512-number of bytes = free space */
sector_t sect_n;
ParamBlockRec pb;	/* è diventata una variabile globale per poter chiudere il file in
					caso di errore; proprio per questo però ha senso che anche il nome
					del file, puntato da un campo del record, sia globale */
#define ioDirID ioFlNum
short floppy_n;
Boolean bar_archive,need_a_touch;
static Boolean notTrueMacBinary,MacBinaryII;
static unsigned short mbIIsec_head_len;
Boolean all_listonly;
Boolean devo_chiudere_out=false;
/* gruppetto di variabili usate per la verifica di un header 'M' del GNU tar, ma
anche per altri scopi, sia in lettura sia in scrittura */
Boolean ultimo_header_valido;
Boolean ultimo_disco_espulso;	/* il disco contenente quell'ultimo header è stato
								espulso ? */
Boolean hasVheader;
char ultimo_header[184];		/* usata anche per altri scopi, viene SEMPRE aggiornata
								coi primi 184 bytes dell'ultimo header, tar o bar, che
								si è letto o scritto
								-- I always copy here the first 184 bytes of any 
								tar or bar file header which is being read (here) or
								written (in tar.c)
								*/
long last_offset;		/* solo per GNU tar */
sector_t next_header_for_POSIX;	/* only for tar archives, useful only for multivolume POSIX:
							-1 if next header is on same disk, otherwise number
							of sector in next disk */
sector_t avail_sectors_for_file;	/* = sectors_on_floppy - sect_n successivo di quando ho 
									caricato ultimo_header
							-- = sectors_on_floppy - the sector following the header
							*/
static const char UNEXP[]="\nUnexpected end of file\n";
static const char
	FINE_PREMATURA_ITA[]="Fine del file prematura\n",
	FINE_PREMATURA_ING[]="Unexpected end of file\n",
	FINE_ARC_ITA[]="Fine archivio\n",
	FINE_ARC_ING[]="End of archive\n",
	BAD_HEADER_CHECS[]="Bad header checksum ",
	GOT_EXPECTED[]="(got %s expected ",
	END_OF_FILE_CR[]="\nEnd of file\n",
	END_OF_VOL_CR[]="\nEnd of volume\n",
	SECT_ON_NEXT[]="(%ld sectors on next volume)\n",
	TESTO_ASCII[]=" testo ASCII",
	ASCII_TEXT[]=" ASCII text",
	COMPUSERVEGIF[]=" CompuServe GIF";

const char
	MSDOSTEXT[]=" (MS-DOS)",
	UNIXTEXT[]=" (UNIX)";


/* titoli italiani dei bottoni dei dialoghi modali 
-- italian titles of the buttons */
char *titoli_si_no[]={"\pSì","\pNo","\pTutti","\pNessuno"};
char *titoli_full[]= {"\pContinua qui","\pContinua altrove","\pNon salvare"};
static char *t_ita[]= {"\pSalva","\pNon salvare","\pSalva tutto","\pSalta tutto",
	"\pContinua altrove","\pTermina estrazione"};
char s_spazi[]="              ";	/* 14 spazi */

#define FLUSH_MIN_SIZE 10240

/****************************/

/* do some buffering towards the hard disk. If I write in relatively small
pieces (e.g. 5Kbytes) the Mac freezes for a few seconds from time to time,
(maybe flushing the disk buffers?) yielding big total times. With 10K
everything works regularly and about two times faster (three times faster than
suntar 1.2.2, thanks to some other improvements).
*/

char*hd_buffer=NULL;
long bytes_in_hd_buffer;

long hd_buffer_size;

void init_hd_buffering()
{
ResrvMem ((Size)hd_buffer_size+4);	/* il +4 serve per l'uudecode... */
hd_buffer=NewPtr((Size) hd_buffer_size+4);
check_allocated(hd_buffer);
}

#define reinit_hd_buffering() bytes_in_hd_buffer=0

OSErr flush_hd_buffer()
{
OSErr err;
if(!bytes_in_hd_buffer) return noErr;

/*printf("flusho %ld bytes\n",bytes_in_hd_buffer);*/

pb.ioParam.ioPosMode = fsAtMark;
pb.ioParam.ioPosOffset = 0;
pb.ioParam.ioBuffer = hd_buffer;
pb.ioParam.ioReqCount=bytes_in_hd_buffer;
bytes_in_hd_buffer=0;
if( (err=PBWriteSync(&pb)) || pb.ioParam.ioResult) return err_code=pb.ioParam.ioResult;

if(pb.ioParam.ioReqCount != pb.ioParam.ioActCount) return err_code=ioErr;
return err_code=noErr;
}

OSErr write_hd(buffer,size)
char*buffer;
long size;
{
long avail=hd_buffer_size-bytes_in_hd_buffer;

/*printf("mi danno %ld bytes\n",size);*/
if(size>=avail){
	if(avail<=10000)
		mcopy(hd_buffer+bytes_in_hd_buffer,buffer,avail);
	else
		BlockMove(buffer,hd_buffer+bytes_in_hd_buffer,avail);
	bytes_in_hd_buffer=hd_buffer_size;
	if(flush_hd_buffer()) return err_code;
	if(!(size-=avail)) return noErr;
	buffer+=avail;
	}
if(size<=10000)
	mcopy(hd_buffer+bytes_in_hd_buffer,buffer,size);
else
	BlockMove(buffer,hd_buffer+bytes_in_hd_buffer,size);
bytes_in_hd_buffer+=size;
err_code=noErr;
return noErr;
}

void connect_to_hd_buffering(refnum)
short refnum;
/* dangerous: it does not check that the buffers are currently unused: suntar
never writes to two files together */
{
pb.ioParam.ioRefNum=refnum;
reinit_hd_buffering();
}

/******************************/

short readblock(buf,nbytes)
/* this routines translates from the sector-based view of the device driver
to the "byte stream" view used by most of the program, by buffering reads
and controlling disk swaps
Never call it asking for more than 512 bytes, the "buffer fill" operation is
performed at most once.
*/
register char*buf;
short nbytes;
{
short i;

/*printf("readblock(%d)\n",nbytes);*/
if(more_in_bytes<nbytes){
	if(more_in_bytes){
		nbytes-=more_in_bytes;
		mcopy(buf,&disk_buffer[512-more_in_bytes],more_in_bytes);
		buf+=more_in_bytes;
		}
	if(bar_archive)
		bar_check_floppy_swap(0);
	else
		tar_check_floppy_swap(0);
	leggi_settore(sect_n,&disk_buffer);	/* nota che in alcuni casi (header BinHex
		e MacBinary) faccio un "backspace" e quindi non posso ottimizzare
		saltando la copia in disk_buffer se sono richiesti 512 bytes allineati */
 /*printf("leggo settore %ld,err=%d\n",(long)sect_n,err_code);*/
 	if((i=check_error_and_events())<0) return -1;
 	if(i>0){	/* "missing disk" was converted to a pause event, now the disk
 				should be in, but I must repeat the operation */
		leggi_settore(sect_n,&disk_buffer);
 		if(check_error()) return err_code;
 		}
	more_in_bytes=512;
	sect_n++;
	settori_passati++;
	}
mcopy(buf,&disk_buffer[512-more_in_bytes],nbytes);
more_in_bytes-=nbytes;
return 0;
}

void unget_block()
{
/* viene chiamata per l'unget dei primi bytes di un file, quindi
-- used to unget the first bytes of a file: they are still in the buffer, hence:
*/
more_in_bytes=512;
}

void unget_char()
{
more_in_bytes++;
}

/*
attenzione che anche nei moduli dehqx e unpit uso i buffer e more_in_bytes !
-- caution: modifying the above functions, take a look at hqx_end_of_file &
get_hqx_byte & fast_get_hqx_byte in dehqx.c, and fast_get_pit_byte in unpit.c,
which are partially copied from the above and use the same data structures
*/


void end_of_file()
{
/* è chiamata per saltare sopra gli ultimi bytes prima dell' header del file
seguente, quindi non c'è che passare al settore successivo
-- it skips the last bytes of the current sector: next read will read
an header which is aligned at the start of a sector
*/
more_in_bytes=0;
}

void skip_file(fsize)
/* it increments the sector number: the disk swap routines are designed to 
handle the case when the requested sector is beyond the disk end, so there
is no need to worry about that */
long fsize;
{
if(fsize<=512)return;	/* il primo settore è già stato letto */
sect_n+= (fsize-1) >>9;
}


/*************************/

void stampa_buffer(sect_n,buffer,allhex)
/* utilissima nel debugging
-- it was written for debugging, then it became a permanent part of the program
executing the View sector command
*/
sector_t sect_n;
unsigned char *buffer;
Boolean allhex;
{short i,j=0,x;
disable_autoflush(2);
for(i=0;i<512;i++){
	if((x=buffer[i])<16)
		printf("0%x",x);
	else if(!allhex&&x>=' '&&x<127)
		printf(" %c",x);
	else
		printf("%x",x);
	if(!allhex&&j==23||allhex&&j==31){
		put_char('\n');
		j=0;
		}
	else{
		j++;
		if(!allhex||(j&3)==0) put_char(' ');
		}
	}
if(!allhex) printf("        end of sector %ld\n",(long)sect_n);
enable_autoflush();
}

/***************************/

short isASCII(buf,fsize,nLF,nCR)
/* decide if the file is ASCII examining the first sector of its data
(which was already read by chkmacbin, or is read and ungetted here)
returns:
1 it's ASCII
0 it's not ASCII (contains NUL, an impossible char in a ASCII file, but
	very very common in binary files)
-1 really it does not seem ASCII
-2 the percentage of ASCII bytes is high enough to deserve a 'TEXT' type,
	maybe that file was created on a Mac which gives a meaning to many
	not ASCII chars (e.g. ©), but it's too low to choose to modify the 
	data by automatically converting any LF to CR...
*/
register char*buf;
long fsize;
short*nLF,*nCR;
{
extern unsigned char isascii_table[];
register short i,n_non_ASCII;
short nbytes;
register unsigned char c;
short n_translated;

nbytes= fsize>512? 512 : fsize;
if(nbytes==0) return -1;  /* troppo corto per 
	decidere, e se è 0 non ha senso dargli l'icona di testo... */
if(buf==disk_buffer && more_in_bytes==0){
	if(readblock(nLF,1)) raise_error();	/* fill the disk_buffer...*/
	unget_block();
	}
*nLF=*nCR=0;
n_non_ASCII=n_translated=0;

for(i=nbytes-1;i>=0;i--){
	c=*buf++;
	if(isascii_table[c]){
		if(c>=128) n_translated++;
		}
	else if(c==CR)
		(*nCR)++;
	else if(c==LF)
		(*nLF)++;
	else if(c==26 && fsize<=512 && i==fsize-1 && *nCR && *nLF)	/* MS-DOS text files have
			a cntrl-Z as their last character, accept it as an ASCII character */
		;
	else{
		/*printf("%d non ascii\n",c);*/
		if(c==0) return 0; /* a NUL is never a char */
		if(++n_non_ASCII>(14+(512>>5))) break;
		}
	}
if( n_non_ASCII<=1 && n_translated<=(nbytes>>3) && (nbytes>=80  || smallFilesAreASCII) ) return 1;
return n_non_ASCII > 14+(nbytes>>5) ? -1 : -2;
}

Boolean print_if_string(format,buffer,size)
const char*format;
char *buffer;
short size;
{
/* for error reporting: something coming from a sector containing errors is printed
only if it looks like a string (only ASCII characters, a terminating '\0', no '\n'... */
short i=0;

size--;
while(i<size && buffer[i]>=' ' && buffer[i]<127 )
	i++;
if(i==0 || buffer[i]) return false;	/* empty string or end of string not reached */
printf(format,buffer);
return true;
}

/***************************/

void my_untar(normal)		/* intermediate-level handler of the Extract and List commands */
Boolean normal;
{
short fine,i;

full_name=NULL;
more_in_bytes=0;
bar_archive=0;
all_listonly=listonly;

if(normal)
	tar_check_floppy_swap(1);	/* ci sono delle inizializzazioni
		-- obviously I don't swap disks before beginning, but it 
		initializes some variables and checks sector 0
		*/
else{		/* extract starting at sector */
	floppy_n=1;
	previousFormat=tar_unknown;
	ultimo_header_valido=false;
	}

do{

	leggi_settore(sect_n,&disk_buffer);
	if(err_code==eofErr)
		fine=true;
	else if(di.tipo_device==d_file && err_code==sectNFErr){
		printf(in_Italia?FINE_PREMATURA_ITA:FINE_PREMATURA_ING);
		return;
		}
	else{
 		if((i=check_error_and_events())<0) raise_error();
 		if(i>0){
			leggi_settore(sect_n,&disk_buffer);
 			if(check_error()) raise_error();
 			}
		fine=check_all_zero(disk_buffer);
		}
	if(!fine){
/*printf("n_settori=%ld\n",((untar_number(&disk_buffer[124],false)+511)/512) );*/

		untar();
		tar_check_floppy_swap(-1);
		}
	}
while(!fine);
if(sect_n==0){
	one_empty_line();
	printf(in_Italia?"Archivio vuoto\n":"No files in the archive\n");
	}
else{
	if(expert_mode)
		print_sector_n(sect_n);
	else
		vai_a_capo();
	printf(in_Italia?FINE_ARC_ITA:FINE_ARC_ING);
	}
}
/************** gestione del formato bar *************/
/* bar format handling */

void my_unbar(normal)
Boolean normal;
{
short fine,i;

bar_archive=1;
all_listonly=listonly;
more_in_bytes=0;

if(normal)
	bar_check_floppy_swap(1);
else{		/* extract starting at sector */
	floppy_n=-1;
	}

do{
	leggi_settore(sect_n,&disk_buffer);
	if(err_code==eofErr)
		fine=true;
	else if(di.tipo_device==d_file && err_code==sectNFErr){
		printf(in_Italia?FINE_PREMATURA_ITA:FINE_PREMATURA_ING);
		return;
		}
	else{
 		if((i=check_error_and_events())<0) raise_error();
 		if(i>0){
			leggi_settore(sect_n,&disk_buffer);
 			if(check_error()) raise_error();
 			}
		fine=check_all_zero(disk_buffer);
		}

	if(!fine){
		unbar();
		bar_check_floppy_swap(-1);
		}
	}
while(!fine);
if(expert_mode)
	print_sector_n(sect_n);
else
	vai_a_capo();
printf(in_Italia?FINE_ARC_ITA:FINE_ARC_ING);
}


short unbar_checksum(buffer,do_error,exp_check)
register unsigned char *buffer;
short do_error;
long *exp_check;
{
register short i;
register long chksum;
long hchksum;

hchksum = untar_number(((barh_type*)buffer)->chksum,do_error);
if(hchksum==-1) return -2;		/* testata non bar */
chksum= ' ' * 8;	/* nel calcolo del checksum, il campo checksum era sostituito
					da spazi */
for (i=0; i < 48; ++i)
	chksum += (unsigned char)buffer[i];
for (i+=8; i < sizeof(tarh); ++i)
	chksum += (unsigned char)buffer[i];

*exp_check=chksum;

if (chksum == hchksum) return 0;	/* tutto OK */

/* il fatto e' questo: quando bar calcola il checksum del settore zero, il campo
size contiene '0','\0'. Però, per un orribile bug, i 10 bytes successivi non vengono 
affatto azzerati ma contengono ancora la parte finale del campo size del disco 
precedente, e se il nuovo valore di size, che viene scritto DOPO aver calcolato il 
checksum, è abbastanza lungo succede che parte dei bytes usati per il calcolo del 
checksum vengono irrimediabilmente sovrascritti, e non ho più modo di calcolarlo 
correttamente.
 Ma d'altra parte a me fa molto comodo poter controllare il checksum, quindi mi rassegno
al fatto che un errore può esserci, ma controllo che questo errore sia compatibile con
questo comportamento errato, senza però strafare, in fondo altre versioni di bar
potrebbero aver corretto il bug: cioè, controllo che il checksum letto sia giustificabile
con un numero di cifre ottali <= del numero di caratteri sovrascritti dal nuovo valore
di size, e ipotizzo anche che la correzione del bug consista nel calcolare il checksum
quando sarebbe logico, dopo aver riscritto size.

-- The checksum computation of the volume header is one of the most 
horrible aspects of bar (file headers are OK, fortunately). For some 
unknown reason, bar saves the volume header in a reserved place, so
the next volume header is built without reinitializing all fields:
1) the volume_num is updated
2) the size field is replaced by "0" (a '0' followed by a '\0'):
   since that field is 12 bytes long, 10 bytes of the old value are not
   altered by that operation
3) the checksum is computed and stored
4) the new value of size is now stored in the size field (strangely, since
   file header checksums are always computed after filling all other fields):
   since it's in octal and is multiple of 512, either it's 0 or it's at least 
   4 bytes long, so it overwrites part of the non-fixed data used to compute 
   the checksum
Hence, the size field is not protected by the checksum (not a tragedy, the
whole file data is not protected by any checksum...) and the cheksum is computed
using some data which is no more stored on the current volume. In fact, bar 
simply ignores the volume checksum, but it could check it by obliging you to 
always insert all disks of the archive in the correct order (on the first disk the
"old size" is 0, on other disks it's the previous size field partially overwritten
by "0").
But suntar allows you to insert disks in any order, starting extraction from
any disk. Furthermore, it automatically distinguishes between a tar and bar
archive, and the checksum of sector 0 looks like the better way to 
guide the choice.
Hence, this function returns:
0 	OK, the checksum is valid for a file header (the write commands of suntar
    compute checksums of volume headers as if they were file headers, by
    updating the size field before computing the checksum)
1   the checksum is bad as a file header, but the error might be due to some 
	binary zeros and some unknown octal digits overwritten by what is currently
	in the size field: that is, it might be a valid checksum for a volume header
-1  the checksum is invalid anyway
-2  the checksum field did not contain an octal string

*/

{
register short s;
s=0;
for(i=0;((barh_type*)buffer)->size[i]!=0; ++i){
	s+= (unsigned char)((barh_type*)buffer)->size[i];
	}
chksum -= s;
}

if( hchksum<chksum || hchksum > chksum + (i+1)*'7' )	/* i contiene ancora la
							dimensione della stringa size, il +1 per il \0 in fondo */
	return -1;	/* bad checksum */
	/* there are still some values which are forbidden: e.g. there is a hole between
	"7\0"=55 and "00"=96, however "7777777\0" is > "00000000" hence the last hole
	occurs when there are 7 bytes */
if(i+1 > 7) return 1;
i= ((short)(hchksum-chksum))/'0';		/* that's the number of octal digits, supposing
									that all other chars are binary zeros */
if(i>=8) i=7;	/* "77777777"/'0' yields 8, but without doubts it contains 7 digits */
if( (short)(hchksum-chksum) <= i*'7')
	return 1;	/* l'errore è giustificabile con i caratteri sporcati dal bug di bar */
return -1;

}



unsigned char guess_bar_linkflag()
/* il campo prima del linkflag che in teoria dovrebbe essere vuoto in pratica è pieno
e a volte sborda sul linkflag: a prima vista sembra che il linkflag sia scritto dopo, 
era comunque sempre '0' per i file, ma '6' per una directory, invece del '5' che
mi aspetterei secondo la codifica tar (un'altra volta era '5', ma quando il campo
precedente non sborda è '\0'). Per di più, gli hard link (ma non quelli simbolici) hanno
il campo size != 0 ( e il linkflag corretto, '1'), anche se poi non segue nessun byte. 
Meglio continuare a fare a meno del linkflag nel capire che razza di entry è... 

-- second horrible feature of bar: the rdev field should always be binary zeros
for conventional files and directories, hence it should not be of interest
to a program running on the Macintosh. On the contrary, it contains data, and
I don't know what that data means. The horrible thing is that when that number
was negative, it's written anyway as a 32 bit unsigned octal number, and that
occupies 12 bytes, overflowing the rdev field and overwriting the linkflag
field: it happens often, about one time out of two (and files in the same 
subdirectories tend to either have all the problem or not have it at all).
 So, the linkflag field can't be relied upon, and it should have been the most
important field of the header, the first thing to look at!
 If the original bar succeed to survive to such a disaster, there must be a way:
here is what we succeeded to guess by a lot of testing:
1) folders may be distinguished since their name ends in '/': that was easy
2) links can be distinguished since the linkname field is full: however,
   unlike tar and unlike what the documentation of bar says, the linkname field
   really starts at the first byte after the '\0' which terminates the string
   in the name field. The simple solution of suntar 1.0, consider that as a
   file entry, works for symbolic links (obviously creating an empty file during
   extraction) but does not work for hard links, whose size field is not 0
   even if the archive contains 0 bytes of their contents
3) anything else is considered to be a normal file
 If there will be any problem in this approach, the fault is not ours: the
fault is of that fellow who after doing such a blatant bug, did not correct
it and allowed it to remain in a program which was used for years and is now
at the version number 4.51, if I remember well. Obviously, if we could have a 
look at the source of the original bar we would be surer of what we are doing,
but that would not make the thing less horrible.
*/
{
short l;
l=strlen(((barh_type*)disk_buffer)->name);
if( ((barh_type*)disk_buffer)->name[l-1] == '/')
	return '5';
else if( ((barh_type*)disk_buffer)->name[l+1] ) /* perché il link name non sta 
			affatto nel campo omonimo...
			-- remember that the linkname is not placed in the linkname field */
	return '1';
else
	return '0';
}


void unbar()
{
unsigned char linkflag;
long exp_check;

disable_autoflush(1);
if(expert_mode) print_sector_n(sect_n);
sect_n++;
more_in_bytes=0;
if( unbar_checksum(&disk_buffer,1,&exp_check) != 0 ){
	printf(BAD_HEADER_CHECS);
	if(expert_mode){
		if( print_if_string(GOT_EXPECTED,((barh_type*)disk_buffer)->chksum,8))
			printf("%lo) ",exp_check);
		}
	if(!ignore_errors) raise_error();
	put_char('\n'); printf(s_spazi);
	}
((barh_type*)disk_buffer)->name[199]=0;
full_name=strcpy(nam_buf,((barh_type*)disk_buffer)->name);

copia_ultimo_header(disk_buffer,sect_n);

file_date= untar_number( &disk_buffer[36],false);

linkflag=guess_bar_linkflag();

if (linkflag=='5')
	untar_directory();
else if(linkflag=='1')
	printf("Link %s -> %.200s%s\n", ((barh_type*)disk_buffer)->name,
		&((barh_type*)disk_buffer)->name[strlen(((barh_type*)disk_buffer)->name)+1],
		listonly?"":in_Italia?" (ignorato)":" (ignored)");
else
	untar_file(untar_number(((barh_type*)disk_buffer)->size,1));
}

void copia_ultimo_header(p,sector_n)
/* copies the first 184 bytes of the header where it must be copied, and 
sets some related variables */

char*p;		/* puntatore al buffer contenente un header di file o directory */
sector_t sector_n;	/* numero del settore che segue quello in cui l'header è stato scritto */
{
mcopy(ultimo_header,p,sizeof(ultimo_header));
ultimo_header_valido=true;
ultimo_disco_espulso=false;
avail_sectors_for_file= di.sectors_on_floppy - sector_n;
/* last_offset non è gestito qui, tocca al chiamante occuparsene
-- last_offset is not assigned here, but it must be assigned by the caller
*/
}

/*************************/

void bar_check_floppy_swap(situation)
short situation;
/* handle floppy swap for bar archives */
{
short i,this_floppy,accept_any_disk;
sector_t previous_sect;
sector_t header_sect=0;

if(situation>0){	/* 1 => il primo settore dell'archivio; 0=>un altro settore richiesto da readblock 
					(entro un file); -1 => un altro settore richiesto per leggere un header
					-- situation =1 => the first sector in the archive; 0 => another 
					sector read through readblock; -1 => a sector which should contain
					a file header */
	floppy_n=1;
	header_sect=di.archive_start_sector;
	sect_n=di.archive_start_sector+1;
	if(fase==hack_listing){
		this_floppy=bar_check_settore0(header_sect,&sect_n,true);
		if(this_floppy<0) raise_error();
		if(sect_n>0)printf("%ld sectors from a previous file",sect_n);
		sect_n++;
		if(sect_n>=di.sectors_on_floppy){
			printf(" (Partly on next volume)\n");	/* a very long file may be splitted
						in three or more disks... */
			longjmp(main_loop,-1);	/* during Expert list, the disk is not 
						ejected...*/
			}
		else
			vai_a_capo();
		return;
		}
	/* e prosegui, per il controllo del settore 0
	-- continue, to check sector 0 */
	}
else if(sect_n<di.sectors_on_floppy || di.tipo_device==d_file)
/* the most common case: the requested sector is in the disk which is already
in the drive */
	return;
else{
	if(floppy_n!=-1)floppy_n++;
	sect_n -= di.sectors_on_floppy-1;	/* -1 per via dell'header...; in modalità normale
									si ottiene sempre 1, in listonly no !
									-- minus 1 because of the header; during extraction, 
									sect_n will be == sectors_on_floppy, during a List
									it will not
									*/
	if(fase==hack_listing){
		if(di.tipo_device==d_file){
			if(sect_n>1) printf(UNEXP);
			else printf(END_OF_FILE_CR);
			}
		else{
			if(sect_n>1) printf(SECT_ON_NEXT,(long)sect_n-1);
			printf(END_OF_VOL_CR);
			}
		longjmp(main_loop,-1);
		}
	}

accept_any_disk= listonly || situation!=0;	/* cioè, non sono in mezzo ad un salvataggio...
		if I'm in the middle of a file saving, to get next disk is more important */

for(;;){

	if(situation<=0){	/* il primo disco è già dentro, gli altri bisogna sostituirli a
						ciò che è dentro...
						-- the first disk is already in the drive, further disks must
						be inserted
						*/
		if(di.tipo_device==d_file) raise_error();	/* dovrò metterci un messaggio di errore, 
				ma non ne ho voglia di farlo ora; idem per GNU...!!! */
		diskEject(true);
		do{ 
			if(fase==hack_reading||floppy_n==-1)	/* succede solo per untar e unbar at sector */
				i=inserzione_assicurata(in_Italia?"\pInserisci il prossimo disco":
					"\pInsert next disk",accept_any_disk,true);
			else{
				char buffer[40];
				strcpy(buffer,in_Italia?"Inserisci il disco numero ":"Insert disk number ");
				my_itoa((long)floppy_n,&buffer[strlen(buffer)]);
				i=inserzione_assicurata(my_c2pstr(buffer),accept_any_disk,true);
				}
			}
		while(i!=0);
		}
	else
		situation=-2;	/* non posso lasciare 1 se resto entro il ciclo
			-- it can't remain 1 inside the loop
			*/

	/* ora guardo se il disco ricevuto è quello che mi aspetto
	-- check whether the inserted disk is the expected disk
	*/

	this_floppy= bar_check_settore0(header_sect,&previous_sect,true);

	if(this_floppy==floppy_n ||
	   (floppy_n==-1||fase==hack_reading) && previous_sect+avail_sectors_for_file==
	   	((untar_number(((barh_type*)ultimo_header)->size,-1)+511)/512) ){
		/* è stato inserito il disco voluto: nel caso di unbar at sector non posso
		fare affidamento sul numero di floppy, ma almeno le dimensioni le controllo */

		printf(in_Italia?"Volume numero %d":"Volume number %d",this_floppy);
		print_if_string(in_Italia?" dell\'archivio %s":" of the archive %s",
			((barh_type*)disk_buffer)->name,100);
		vai_a_capo();

		if(sect_n<di.sectors_on_floppy) return;
		/* può capitare durante il list di un file che risulta suddiviso in più
		di 2 dischi, allora chiede anche quelli intermedi ma al solo scopo di
		sapere quanti settori ci sono
		-- I may be here only during List of a file which is so long to be splitted
		among at least three disks: suntar asks all the disks, but the intermediate 
		ones are ejected soon after reading how many sectors they contain (it's not
		forbidden to use disks of different sizes for a single archive)
		*/
		if(sect_n>=di.sectors_on_floppy)
			printf(in_Italia?"Volume occupato completamente da parte di un file precedente\n":
				"The whole volume is occupied by part of a previous file\n");
		floppy_n=this_floppy+1;
		sect_n -= di.sectors_on_floppy-1;
		avail_sectors_for_file += di.sectors_on_floppy-1;
		situation = -1;

		}
	else if(this_floppy<0){
		if(situation==-2)
			raise_error();		/* l'utente può ridare il comando senza rimetterci nulla
				-- the user may repeat the command and return here in this same situation */
		else
			;	/* non è un disco bar, non c'è che riprovare...
				not a bar disk: remain in the loop, so it will be ejected and a new one
				will be asked for
				*/
		}
	else{
		/* it's not the right disk, but it's a bar disk */
		printf(in_Italia?"Questo è il volume numero %d\n":"Volume number %d\n",this_floppy);
		if(!accept_any_disk){
			beep_in_foreground();
			if(fase!=hack_reading)
				printf(in_Italia?"Errore, io voglio il volume %d, non il %d\n":
				"Error, requested volume is number %d, not %d\n",floppy_n,this_floppy);
			else
				printf("Wrong volume !\n");
			}
		else{
			char pt[6];
			my_itoa((long)floppy_n,&pt[1]);
			pt[0]=strlen(pt+1);
			if( chiedi_su_questo(this_floppy,previous_sect,NULL,
			    in_Italia?"\pLeggo questo volume invece del ":
				"\pMust I use this volume rather than volume ",pt,"\p ?") ){
				floppy_n=this_floppy;
				sect_n = previous_sect;
				ultimo_header_valido=false;
				sect_n++;	/* per tener conto anche del settore 0 */
				return;
				}
			}
		}
	}
}

short inserzione_assicurata(mesg,accept_any_disk,stop_button)
/* as aspetta_inserzione but it never returns without a new disk inserted, 
it rather calls raise_error: if a file is being saved, it asks for confirmation,
since an error closes all open files and hence truncates them */
char*mesg;
Boolean accept_any_disk,stop_button;
{
/* come aspetta inserzione ma non ritorna al chiamante, usando longjmp, se il disco
non viene affatto inserito ed è accettabile che non lo sia */
	short i;
	do{
		i=aspetta_inserzione(mesg,stop_button?button_is_stop:0);
		if(i!=0){
			if(!accept_any_disk){
				ParamText(in_Italia?"\pVa bene lasciare il file incompleto ?":
					"\pDo you want to let this file uncompleted ?",PNS,PNS,PNS);
				if(my_modal_dialog(130,titoli_si_no,2)==1)
					raise_error();
				else
					return i;
				}
			else
				raise_error();
			}
		if(di.tipo_device==d_driver&&di.v.fi.is_not_initialized) diskEject(true);
		}
	while(di.tipo_device==d_driver&&di.v.fi.is_not_initialized);
	return 0;
}


short bar_check_settore0(header_sect,previous_sect,verbose)
/* examine if sector 0 is a valid bar volume header */
sector_t header_sect,*previous_sect;
Boolean verbose;
{
short disk_n;
long offset,exp_check;

leggi_settore(header_sect,disk_buffer);
if(check_error()) return err_code;

offset=untar_number(((barh_type*)disk_buffer)->size,-1);

if(my_atoi(((barh_type*)disk_buffer)->volume_num,&disk_n) ) disk_n = -1;

if(((barh_type*)disk_buffer)->bar_magic[0]!= 'V' || 
   ((barh_type*)disk_buffer)->bar_magic[1] != 0 || disk_n==-1 || offset==-1){
    if(verbose){
		beep_in_foreground();
		printf(in_Italia?"Questo non è un disco in formato bar\n":"Not a bar disk !\n");
		}
	return -1;
	}

*previous_sect=(offset+511)/512 + header_sect; /* è un multiplo di 512, ma non fidarsi è meglio */
/* se fosse previous_sect = 0 (se l'ultimo settore di un disco è un header...) allora
permettere di estrarre anche questo file !!*/

if( unbar_checksum(disk_buffer,-1,&exp_check) <0 ){
	if(verbose){
		printf("Bad volume header checksum ");
		if(expert_mode){
			if( print_if_string(GOT_EXPECTED,((barh_type*)disk_buffer)->chksum,8))
				printf("%lo)",exp_check);
			}
		vai_a_capo();
		if( ! ignore_errors ) return -1;
		}
	else
		return -1;
	}
return disk_n;
}

static Boolean chiedi_su_questo(this_floppy,previous_sect,name,p1,p2,p3)
/* presents the modal dialog "must I continue on this disk ?" */
short this_floppy;
sector_t previous_sect;
char*name;
Str255 p1,p2,p3;
{
enum {dOK=1,dNo};
switch(my_semimodal_dialog(129,titoli_si_no,2,p1,p2,p3)){
case dOK:
	if(this_floppy>1&&previous_sect>0){
		printf(in_Italia?"Presenti %ld settori di completamento di "
			:"%ld sectors from ",(long)previous_sect);
		if(name==NULL)
			printf(in_Italia?"un file precedente\n":"a previous file\n");
		else
			printf("%s\n",name);
		}
	return 1;
case dNo:
	return 0;
}
}

/*******************************/

void tar_check_floppy_swap(situation)
short situation;
/* check disk swap for GNU tar or multivolume POSIX tar archives.
Many comments are identical to those in bar_check_floppy_swap, hence they were not
translated */
{
/* in questa routine, floppy_n puo' valere solo 1 o 2, non vado a contare i floppy.
Però, se capita che un disco che non sia il primo abbia di nuovo un header di file
al settore 0, devo rimetterlo ad 1 perché non avrò l'header di continuazione
-- floppy_n may have only two values: 1 for disks without an 'M' continuation
header (such as the first disk) and 2 for a disk which has one (as most of the 
following disks)
*/

short i,this_floppy,accept_any_disk;
sector_t previous_sect,header_sect=0;
static char s1[]="Volume occupato completamente da parte di %s\n",
			s2[]="The whole volume is occupied by a part of %s\n";

if(situation>0){	/* 1 => il primo settore dell'archivio;
					0 => un altro settore richiesto da readblock (entro un file);
					-1 => un altro settore richiesto per leggere un header;
					-2 => il parametro di ingresso era 1 ma ho dovuto espellere
						il disco perché non buono */
	floppy_n=1;
	previousFormat=tar_unknown;
	ultimo_header_valido=false;
	header_sect=sect_n=di.archive_start_sector;
	if(fase==hack_listing){
		if(next_header_for_POSIX!=-1){
			sect_n=next_header_for_POSIX;	/* hacking() has already checked that's a good
								starting sector */
			return;
			}
		accept_any_disk= true;
		this_floppy=tar_check_settore0(header_sect,true);
		if(this_floppy<0)
			raise_error();
		if(this_floppy==2){
			sect_n= (untar_number(((tarh_type*)disk_buffer)->size,false)+511)>>9;
			if(sect_n>0)printf("%ld sectors from %s",	/* se il file è lunghissimo non
					è vero, non sono tutti qui ! Lo scrivo sotto */
				(long)sect_n,((tarh_type*)disk_buffer)->name);
			sect_n++;
			}
		if(hasVheader) sect_n++;
		if(sect_n>=di.sectors_on_floppy){
			printf(" (Partly on next volume)\n");
			longjmp(main_loop,-1);
			}
		vai_a_capo();
		return;
		}
	accept_any_disk= true;
	}
else if(sect_n<di.sectors_on_floppy || di.tipo_device==d_file)
	return;
else{
	accept_any_disk= listonly || situation!=0;
	floppy_n=2;
	sect_n -= di.sectors_on_floppy;	/* in modalità normale si ottiene sempre 0, in listonly no !*/
	if(situation==-1 && sect_n==0){		/* in questo caso l'header di continuazione non c'è
				-- if the last sectors of a file happens to be written in the last
				sector of a disk, the following disk has not a continuation header, hence
				it looks like the first disk of another archive */
		floppy_n=1;
		/*sect_n--; no, chiedo comunque il 0, sta poi alla gestione dell'inserzione
					prossimo disco saltare eventuali header */
		}
	if(fase==hack_listing){
		if(di.tipo_device==d_file){
			if(sect_n>0)
				printf(UNEXP);
			else
				printf(END_OF_FILE_CR);
			}
		else{
			if(sect_n>0)
				printf(SECT_ON_NEXT,(long)sect_n);
			printf(END_OF_VOL_CR);
			}
		longjmp(main_loop,-1);
		}
	}

for(;;){
	if(situation<=0){	/* il primo disco è già dentro, gli altri bisogna sostituirli a
						ciò che è dentro... */
		if(di.tipo_device==d_file) raise_error();
		diskEject(true);
		if(tar_version==tar_unknown &&previousFormat==tar_unknown){
			ParamText(in_Italia?
				"\pQuesto è un archivio multidisco ?":
				"\pIs this a multivolume archive ?",PNS,PNS,PNS);
			assegna_tar_version();
			}
		if(tar_version==tar_singlevol && previousFormat==tar_unknown)
			error_message(in_Italia?"Errore: raggiunta la fine del volume\n":
				"Error: end of volume was reached\n");

		do{
			if(situation==-2)
				i=inserzione_assicurata(
				in_Italia?"\pInserisci il disco in formato tar":
				"\pInsert the disk in tar format",accept_any_disk,false);
			else
				i=inserzione_assicurata(in_Italia?"\pInserisci il prossimo disco tar":
				"\pInsert next tar disk", accept_any_disk,true);
			}
		while(i!=0);
		}
	else
		situation=-2;	/* non posso lasciare 1 se resto entro il ciclo */

	if(situation!=-2 && (previousFormat==tar_POSIX ||
	   previousFormat==tar_unknown && tar_version==tar_POSIX) ){
		/* a multivolume POSIX disk has no header, hence I can't be sure that it's the right
		disk, I must rely on the current setting of the option */
		previousFormat=tar_POSIX;
		this_floppy=2;
		previous_sect=0;
		if(sect_n >= di.sectors_on_floppy){		/* al solito, può succedere per un List */
			sect_n -= di.sectors_on_floppy;
			printf(in_Italia? s1:s2,((tarh_type*)ultimo_header)->name);
			continue;		/* the disk must be ejected, continuing on next one */
			}
		/* if(qualche controllo che sia veramente POSIX.....) */
		return;
		}

	this_floppy= tar_check_settore0(header_sect,true);
	previous_sect=0;

	if(floppy_n==2 && this_floppy==2 && 
	   verifica_nome(hasVheader,&previous_sect)!=noErr ) this_floppy=3; /* any value 
	   			which can't be equal to floppy_n */
	if(this_floppy==floppy_n){		/* è stato inserito il disco voluto */

		if(hasVheader) sect_n++;		/* devo incrementare sect_n per saltare l'header di 
					volume, ed è più flessibile non farlo in tar_check_settore0,
					non posso farlo se il disco non viene accettato e l'ultima
					decisione su questo la prendo qui
					-- I could not do that in tar_check_settore0, since it can't
					know whether the disk is the right one or not */
		if(floppy_n==2) sect_n++;	/* salto anche l'header extra... */
		avail_sectors_for_file = di.sectors_on_floppy;
		if(hasVheader) avail_sectors_for_file--;		/* perché quei settori non sono */
		if(this_floppy>=2) avail_sectors_for_file--;	/*usati per il file */
		if(sect_n<di.sectors_on_floppy)
			return;

		/* può capitare durante il list di un file che risulta suddiviso in più
		di 2 dischi, allora chiede anche quelli intermedi ma al solo scopo di
		sapere quanti settori ci sono */
		printf(in_Italia? s1:s2,((tarh_type*)ultimo_header)->name);
		sect_n -= di.sectors_on_floppy;	/* gli incrementi a sect_n per gli header extra
									sono già stati effettuati OK */
		floppy_n=2;
		if(situation==-1 && sect_n==1)
			floppy_n=1;
		/* e proseguo, esattamente come se ci fosse stato un errore e fosse stato 
		inserito il disco sbagliato, solo che i dati di controllo sono stati
		aggiornati a pretendere il disco successivo */
		}
	else if(this_floppy<0){
		/* printf(in_Italia ? "Il disco non è in formato tar né bar\n" : 
			"Disk format is neither tar nor bar\n"); no, qualcosa viene scritto da
			bar_check_settore_zero (magari tramite check_error o untar_number) */
		if(situation==-2){
			raise_error();		/* l'utente può ridare il comando senza rimetterci nulla */
			}
		else
			printf(in_Italia?"Riprova !\n":"Try again !\n");	/* disco non tar, non c'è che riprovare... */
		}
	else{
		if(!accept_any_disk){
			beep_in_foreground();
			printf(in_Italia?"Errore, non è il volume successivo!\n":
			"Error, it\'s not next volume!\n");
			}
		else{
			if(this_floppy==1)
				previous_sect=0;
			else
				previous_sect= (untar_number(((tarh_type*)disk_buffer)->size,false)+511)>>9;
			if( chiedi_su_questo(this_floppy,previous_sect,((tarh_type*)disk_buffer)->name,
				situation==-2 ? (in_Italia?"\pInizio da questo volume ?":
					"\pMust I start from this volume ?") :
				(in_Italia?"\pContinuo su questo volume ?":
					"\pMust I continue on this volume ?") ,PNS,PNS) ){
				if(this_floppy==1){
					ultimo_header_valido=false;	/* tanto gestisco subito il primo buono */
					avail_sectors_for_file=di.sectors_on_floppy-sect_n;
					}
				else{
					long last_size;
					copia_ultimo_header(disk_buffer,hasVheader?2:1);
					last_offset=untar_number(((tarh_type*)disk_buffer)->offset,-1);
					last_size=untar_number(((tarh_type*)ultimo_header)->size,false);
					((tarh_type*)ultimo_header)->linkflag='\0';
					numstr (((tarh_type*)ultimo_header)->size,last_size+last_offset,12);
					}
				if(this_floppy==3) this_floppy=2;
				floppy_n=this_floppy;
				sect_n = previous_sect;
				if(hasVheader) sect_n++;
				if(this_floppy>=2) sect_n++;
				if(sect_n<di.sectors_on_floppy){
					if(this_floppy==1) printf(in_Italia ? "Parte finale del file %s\n":
						"Last part of file %s\n",((tarh_type*)ultimo_header)->name);
						/* if it's !=1, the message is in chiedi_su_questo */
					return;
					}
				sect_n-=di.sectors_on_floppy;
				if(this_floppy!=1) printf(in_Italia ? s1: s2, ((tarh_type*)ultimo_header)->name);
				}
			}
		}
	}
}

short tar_check_settore0(header_sect,verbose)
/* examine the first sector of a tar disk: either it's all zeros, or a normal
file header, or a continuation file header. If it's a 'V' volume header, then
check sector 1 too
*/
sector_t header_sect;
Boolean verbose;
{
short i;

hasVheader=0;
leggi_settore(header_sect,disk_buffer);
if(check_error()) return err_code;

if(check_all_zero(disk_buffer)) return 1;		/* un archivio tar vuoto... */
if( (i=untar_checksum(disk_buffer,-1,verbose)) < 0){
	if(i==-2||!verbose|| !ignore_errors) return -1;
	}

hasVheader = ((tarh_type*)disk_buffer)->linkflag=='V';
if(hasVheader){
	previousFormat=tar_GNU;
	disk_buffer[99]=0;
	if(verbose){
		if(fase==hack_listing){
			long cdate=untar_number(((tarh_type*)disk_buffer)->mtime,-1);
			if(cdate!=-1) print_one_date(cdate+unixtime,in_Italia?"Archivio creato ":
				"Archive created ");
			printf(in_Italia?"; i":"; t");
			}
		else
			printf(in_Italia?"I":"T");
		printf(in_Italia?"l nome del volume è %s\n":
		"he volume name is %s\n",disk_buffer);	/* stampa l'informazione contenuta nell'header ! 
		-- that's the string the user specified in the V option, plus the volume number */
		}
	leggi_settore(header_sect+1,disk_buffer);
	if(check_error()) return err_code;
	if(check_all_zero(disk_buffer)) return 1;
	if( (i=untar_checksum(disk_buffer,-1,verbose&&!ignore_errors)) < 0){
		if( (i==-2 || !ignore_errors) && verbose ) return -1;
			/* verbose is false if and only if I were called by identify format,
			which only wants to know if this is a tar archive, not if its sector 1
			is corrupted */
		}
	}

if(((tarh_type*)disk_buffer)->linkflag=='M'){
	previousFormat=tar_GNU;
	return 2;
	}
else
	return 1;
}

static OsErr verifica_nome(volume_header,previous_sect)
/* verify a continuation header against the normal header which was in the previous disk */
short volume_header;
sector_t *previous_sect;
{
long last_size,sizeleft,expected_offset;

if(!ultimo_header_valido){
	*previous_sect=0;
	return noErr;		/* non ho visto il disco precedente, perché
		sono alla prima inserzione */
	}
sizeleft=untar_number(((tarh_type*)disk_buffer)->size,false);
last_size=untar_number(((tarh_type*)ultimo_header)->size,false);
*previous_sect=(sizeleft+511)>>9;
if( strcmp(ultimo_header,disk_buffer) ) return -1;	/* il nome non coincide */

expected_offset=last_offset+((long)avail_sectors_for_file<<9);
last_offset=untar_number(((tarh_type*)disk_buffer)->offset,false);
/*printf("<%ld %ld,%ld,%ld,%ld>",(long)avail_sectors_for_file,expected_offset,last_offset,last_size,sizeleft);*/

if(expected_offset!=last_offset || last_size-last_offset!=sizeleft) return -1;

if(volume_header)
	(*previous_sect)++;

return noErr;
}

void delete_out_file(void);
void delete_out_file()
{
reinit_hd_buffering();
cancella_file_aperto(pb.ioParam.ioRefNum);
devo_chiudere_out=false;
}

void close_or_del_out_file()
{
if(devo_chiudere_out){
	if(del_incompl)
		delete_out_file();
	else{
		flush_hd_buffer();
		PBCloseSync(&pb);
		}
	devo_chiudere_out=false;
	}
}

/******************************************************************/

short filter_conf(EventRecord*);
static short filter_conf(theEvent)
EventRecord*theEvent;
{
/* the standard filter for semimodal makes CR equivalent to button 1,
this filter makes esc equivalent to button 2 */
#define ESC 27
if(theEvent->what==keyDown&&(char)theEvent->message==ESC)
	return 2;
return 0;
}


void check_confirmation()
/* handles the confirm saves dialog */
{
/* il nome del file DEVE stare in tarh.name, formato C
-- the file name MUST be a C string in tarh.name
 */
static Point wPos={-1,-1};
char buffer[270];

if(!confirm_saves||all_listonly||fase==hack_reading||
	fase==hack_listing||fase==selected_reading || di.file_format>ff_tarbar ) 
	return;
else{
	short item;
	enum {cdSave=1,cdSkip,cdSaveAll,cdSkipAll};


	do{
		my_c2pstr(full_name);
		item=semimodalDialog(136,&wPos,filter_conf,t_ita,6,"\pFile\312",	/* spazio 
								non interrompibile
								-- non-breakable space: if the file name is long,
								breaking between lines here is not a wise thing */
			full_name,NULL,
			teJustLeft,true,NULL);
		my_p2cstr(full_name);
		if(item==6){
			touch_if_needed();
			select_directory();
			}
		else if(item==7){
			extern long last_selection;
			last_selection = menuItemMess(fileID,fmAbort);
			raise_error();
			}
		}
	while(item>4);

	listonly= item==cdSkip || item==cdSkipAll;
	if(item==cdSaveAll || item==cdSkipAll)
		set_skip_all(1);
	}
}

void check_conf_dir(dname)
char*dname;
{
/* in confirm saves directories are extracted only if they already existed.
OK, that seems the opposite of any logical thing, but when extracting, for example,
dir/file if dir does not exist it's created. Hence, if I wish to extract files
which go to that directory it will be created anyway, hence not creating it
avoids an almost useless dialog, but I should warn the user
before placing files in a directory which already existed (that could cause a
chaotic mix of files). The simplest way to do that warning is calling create_directory...
*/
if(!confirm_saves||all_listonly||fase==hack_reading||
	fase==hack_listing||fase==selected_reading) 
	return;
{
CInfoPBRec cinfo;
short i;
cinfo.dirInfo.ioNamePtr= my_c2pstr(dname);
#ifdef USA_CURRVREFN
cinfo.dirInfo.ioVRefNum=curr_vrefnum;
#else
cinfo.dirInfo.ioVRefNum=0;
#endif
cinfo.dirInfo.ioFDirIndex=0;
cinfo.dirInfo.ioDrDirID=0;
listonly= PBGetCatInfoSync(&cinfo)!=noErr;
my_p2cstr(dname);
}
}

/********************************/
void my_itoa(val,buffer)
register long val;
char *buffer;
{
char buf[14];
Boolean nega;
register char *p=&buf[13];
if(nega= (val<0) )
	val= -val;
*p='\0';
do{
	*--p ='0'+(short)(val%10);
	val /= 10;
	}
while(val>0);
if(nega) *--p='-';
strcpy(buffer,p);
}

static short isMacBin2(buf)
/* checks whether the MacBinary header is a MacBinary II header;
returns 0 for non-MacBinary II
1 for Macbinary II
-1 for an unknown (future) release of MacBinary II that suntar can't decode,
-2 for CRC error
 */
register unsigned char *buf;
{
register short i;
/* can't declare a pointer to binh_type since it's misaligned by one byte */
if(buf[122]<129 || buf[123]<129) return 0;
if(buf[123]>129) return -1;
current_crc=0;
for(i=0;i<124;i++)
	CalcCRC(buf[i]);
CalcCRC(0);
CalcCRC(0);
if( current_crc != *(short*)&buf[124]) return -2;
return 1;
}

void clear_unused_fields()
{
/* leggere documentazione su MacBinary II, ma non azzero busy perché 
	nel System 7 ha cambiato significato
-- the file macbinary2-standard.txt, reporting the decisions of the people
who met to set the standard, explains:
>All Finder flags and information would be uploaded, however, a downloading
>program should clear the Finder flag bits of
>  0 - Set if file/folder is on the desktop (Finder 5.0 and later)
>  1 - bFOwnAppl (used internally)
>  8 - Inited (seen by Finder)
>  9 - Changed (used internally by Finder)
> 10 - Busy (copied from File System busy bit)

>Also, fdLocation and fdFldr should be zeroed

Bit	Meaning (from Technical Note 40)
0	Set if file/folder is on the desktop (Finder 5.0 and later)
1	bFOwnAppl (used internally)
2	reserved, currently unused
3	reserved, currently unused
4	bFNever (never SwitchLaunch) (not implemented)
5	bFAlways (always SwitchLaunch) 
6	Set if application is shared and is opened read-only (128K ROM only)
7	Set if file should be cached (not implemented)
8   Inited (seen by Finder)
9   Changed (used internally by Finder)
10  Busy (copied from File System busy bit)
11  NoCopy (not used in 5.0 and later, formerly called BOZO)
12  System (set if file is a system file)
13  HasBundle
14  Invisible
15  File Locked

However, in System 7 bit 10 means "hasCustomIcon" and it's better to preserve it.
It's unfortunate that the MacBinary standard did not include a field for "version
of Finder which created it", now that different versions of the Finder use the same bits
for different purposes: luckily, if that file does have a custom icon its resource
fork contains a 'ICN#' resource with ID -16455.
And bit 15 should not be cleared according to the standard, but in System 7 it means
"is an alias" and it's correct to clear it when transferring an old file to
system 7, if the file does not contain an 'alis' resource.
As a result, I MUST examine the resource fork of the file if either bit 10 or 15
are set.
Bit 11 is now "is stationery pad": it's better to preserve it anyway, since the old
meaning is obsolete since Finder 5.0, there is no particular contents to check for
and the user may change it by a Get Info
Bit 12 was "is System file" and is now "Name locked": since it would be useless
to download an old system file to a machine with System 7, under System 7 it's
better to preserve it, but under system 6 ?


A recent Technical Note by Apple (TB 09 - Finder Flags) says:
"The MacBinary II specification instructs downloading applications to
clear bits 0, 1, 8, 9, and 10 when downloading a file in MacBinary II format. 
Downloading applications should actually clear only bits 0 and 8, (the former
onDesk bit, and the hasBeenInited bit).  Clearing bits 1, 9, or 10 will result
in the file being interpreted incorrectly, since bit 1 is now used to store
color information, bit 9 tells the system if the file is a letter, and bit 10
informs the Finder if the file has a custom icon"
So, now I'm not clearing bits 1 and 9 too, hoping this won't cause problems
to people still using System 6 

*/

	macbinh.finfo.fdFlags &= ~ 0x101 /* was 0x303 till version 2.0.3 included */;
	*(long*)&macbinh.finfo.fdLocation=0;
	macbinh.finfo.fdFldr=0;

	if(macbinh.finfo.fdFlags & 0x8400){
		if(macbinh.rflen==0 || 
		   !(macbinh.finfo.fdFlags & 0x400)&&(macbinh.dflen>0||macbinh.rflen>4096) )
		   /* an alias file has no data fork and is not very big */
			macbinh.finfo.fdFlags &= ~0x8400;
		else{
			unsigned char name[64];
			short refnum,oldvRefNum,oldres;
			SetResLoad(false);		/* it should save the old status to restore
					it, but it should never be false... */
			if(GetVol (&name,&oldvRefNum)!=noErr) oldvRefNum=0;
 
  			SetVol(NULL,pb.fileParam.ioVRefNum);
  			oldres=CurResFile();
  			refnum= OpenResFile(pb.fileParam.ioNamePtr);
  			if(refnum==-1)
  				macbinh.finfo.fdFlags &= ~0x8400;	/* clear both flags */
  			else{
				if(macbinh.finfo.fdFlags & 0x8000){	/* alias bit */
					if(! Count1Resources ('alis') )
						macbinh.finfo.fdFlags &= ~0x8000;
					else if(!gHasResolveAlias){
						ParamText(in_Italia?
						"\pQuesto file è un alias, solo il System 7 può utilizzarlo":
						"\pThis file is an alias, only System 7 knows how to use it",PNS,PNS,PNS);
						my_alert();
						}
					}
				if(macbinh.finfo.fdFlags & 0x400){	/* "has custom icon" bit */
					Handle h = Get1Resource('ICN#',-16455);
					if(h!=NULL)
						ReleaseResource(h);
					else
						macbinh.finfo.fdFlags &= ~0x400;
					}
				CloseResFile(refnum);
				}
			if(oldvRefNum) SetVol (&name,oldvRefNum);
			UseResFile(oldres);
			SetResLoad(true);
			}
		}
}


static void make_badname(file_name,already_bad)
/* creates a new name for a file which caused a bad name error (some
versions of UNIX have a max name length which is greater than that of the HFS,
and it happens to see public domain Mac files with a very long name which 
would not be legal on a Macintosh)
*/
unsigned char *file_name;	/* it points to a Pascal string */
Boolean already_bad;	/* in that case, it already existed, hence... */
{
static short bad_names=0;
register char *p1,*p;
register short i;
short d;
char newname[64];
static char BadNameFolder[]=":bad names folder:";

#define MAX_NAME_LEN 31

strcpy(newname,BadNameFolder);

p1= file_name;
p=&p1[(unsigned char)*p1];	/* last character of original name */
p1++;						/* first character of original name */
while(*p==':' && p>p1) p--;		/* typically, at the end of a Mac name there may be */
i = *p!=':';	/* 0-1 */
while(p>p1 && p[-1]!=':' ){
	p--; i++;
	}
/* now i is the filename length: but it might be shorter than 31, meaning
that's a good name, if the directory name was longer than 31 */
if(i && i<=MAX_NAME_LEN-1 && !already_bad)
	p1=newname + sizeof(BadNameFolder)-1;
else{
	my_itoa((long)++bad_names,&newname[sizeof(BadNameFolder)-1]);
	d=strlen(&newname[sizeof(BadNameFolder)-1]);
	p1=&newname[sizeof(BadNameFolder)-1] + d;
	*p1++ ='-';	/* used as separator */
	d=MAX_NAME_LEN-1-d;
	/* *p1 ='\0'; */
	if(i>d ){
		p+=i-d;	/* the last part of name is kept, it's better to preserve
				the extensions and anyway the most informative part of the name
				is often at the end */
		i=d;	/*i-=i-d*/
		}
	}
while(i--){
	if(*p>=' ' && *p< 127) *p1++ = *p++;
	else if(*p) p++;
	}
*p1='\0';

if(!already_bad)		/* !!!!! stampa comunque solo il primo */
	printf(in_Italia?"%P: nome non valido, cambiato in %s\n":
		"%P: bad name, renamed to %s\n",file_name,newname);
else
	printf("Already used, trying with %s\n",newname);
/*SetHandleSize(file_name,strlen(newname)+1);*/
strcpy(file_name,newname);
my_c2pstr(file_name);
}

static Boolean too_long(fname)
register unsigned char *fname;
/* returns true if ANY of the components of the Mac pathname fname is longer then 31 */
{
register short i=0;
register unsigned char n=*fname++;
while(n--){
	if(*fname==':')
		i=0;
	else
		if(++i>31) return true;
	fname++;
	}
 return false;
}

static Boolean is_gif_file(buffer,fsize)
register char*buffer;
long fsize;
{
/* checks whether the file begins with 'GIF'
(really, the signature is GIF87a or GIF89a, but when there will be a GIF9xa
it will be better to recognize it too)
(This "privilege" is  because gif files are very popular, and since they are 
a non-Mac format usually they are downloaded as data only so they have no 
type and creator)
*/
static char h[]={'G','I','F'};
if (fsize>100 && !strncmp(h,buffer,(size_t)3) && 
    buffer[3]>='0' && buffer[3]<='9' && 
    buffer[4]>='0' && buffer[4]<='9' &&
    buffer[5]>='a' && buffer[5]<='z') return true;
return false;
/*	since a gif file has an internal signature, it's better to rely on it
than to look at the name...
short l;
l=strlen(tarh.name);
return (l >4 && !ci_strcmp(&tarh.name[l-4],".gif" );
*/
}

Boolean known_extension(void);
static Boolean known_extension()
{
extern short size_ext_table;
extern Handle h_type,h_creat,h_extens,h_comments,h_ascii;
short i, l=strlen(full_name);
register unsigned char*p=*h_extens+2;

for(i=0;i<size_ext_table;i++){
	register short lext=*p;
	if(lext && l>=lext && *(p+1)!=' '){
		register char*q1=(char*)(full_name)+l-lext;
		register char*q2=p+1;
		register char c;
		while(lext--){
			c=*q1++;
			/* I'm using very tricky code to perform a case-insensitive
			comparison: try to understand why it works */
			if(c != *q2 && ((c|=32)<'a' || c>'z' || c!=(*q2|32))) goto next;
					/* unfortunately continue would
					continue the while, but I must continue the for */
			q2++;
			}

		/*printf("identificato %P %d\n",p,i);*/
		HLock(h_comments);
		p=*h_comments+2;
		l=i;
		while(l--)
			p+=*p+1;
		printf(" %P",p);
		HUnlock(h_comments);
		binary=!*((*h_ascii)+i);
		i<<=2;
		mcopy(&filecreator,*h_creat+i,4);
		mcopy(&filetype,*h_type+i,4);
		if(filetype!='TEXT') binary=true;
		return true;
		}
next:
	p+=*p+1;
	}
return false;
}


void print_type_creator(t,c)
OSType t,c;
{
/* sorry, my printf doesn't accept %.4s */
printf(in_Italia?"tipo=\'":"type=\'");
print_chars(&t,4);		/* now that I've implemented that I could use %.4s,
						but why change something which works ? */
printf(in_Italia?"\' creatore=\'":"\' creator=\'");
print_chars(&c,4);
printf("\'");
}

void print_one_date(cdate,message)
long cdate;
char*message;
{
	char p[20];
	printf("%s",message);
	IUDateString(cdate,abbrevDate,p);
	printf("%P ",p);
	IUTimeString(cdate,true,p);
	printf("%P",p);
}



void cancella_file_aperto(ioRefNum)
short ioRefNum;
{
OSErr i;
ParamBlockRec pb;	/* per non sporcare la var globale... */
get_openfile_location(ioRefNum);
fillmem(&pb,0,sizeof(pb));
pb.ioParam.ioRefNum=ioRefNum;
PBCloseSync(&pb);
pb.fileParam.ioNamePtr=mac_file_name;
pb.fileParam.ioVRefNum=openfile_vrefnum;
pb.fileParam.ioDirID=openfile_dirID;
i=PBHDeleteSync(&pb);
/*printf("delete=%d\n",i);*/
}

short crea_e_controlla(already_pascal)
/* crea un file ma fa anche qualche controllo...;
ritorna come create_file
-- creates a file and does some checking; returns as create_file
*/
short already_pascal;
{
short i;

if(!already_pascal) my_c2pstr(full_name);

for(;;){
	if( create_file(full_name,fsWrPerm)) return 1;	/* Cancel... */

	i=controlla_spazio(macbinh.dflen,macbinh.rflen);
	if(i==1){	/* Skip */
		delete_out_file();	/* it exists but it's currently of 0 size */
		return 1;
		}
	if(i==2){	/* continue elsewhere */
		delete_out_file();		
		/* and retry... */
		}
	else{
		if(macbinh.finfo.fdFlags&0x4000){	/* bit "invisible" */
			if(my_semimodal_dialog(147,titoli_si_no,2,
				in_Italia?"\pFile invisibile: lo rendo visibile ?":
				"\pInvisible file: make it visible ?",PNS,PNS)==1)
				macbinh.finfo.fdFlags &= ~0x4000;
			}
		return 0;
		}
	}
}


short controlla_spazio(dlen,rlen)
long dlen,rlen;
/* checks that the destination volume has enough free space;
returns 0 if OK, 1 if not enough space and the user canceled
the operation, 2 if not enough space and the user selected another
volume for continuing
*/
{
long l;
short vrefnum;
Boolean prima_volta=true;
short i;

for(;;){
	if(pb.fileParam.ioVRefNum){
		vrefnum=pb.fileParam.ioVRefNum;
/*dp("vr=%d\n",vrefnum);*/
		}
	else{
		volumeParam param;
		char buffer[50];
		param.ioNamePtr=buffer;	/* or NULL, but what happens if some system bug
							(or a bad INIT) does not check for NULLs ? */
		if(PBGetVolSync (&param) !=noErr) return;
		vrefnum=param.ioVRefNum;
		/*dp("vr=%d %P\n",vrefnum,buffer);*/
		}
	if(GetVInfo (0,NULL,&vrefnum,&l)!=noErr)
		return 0;		/* the error might be not fatal, it's better to hope that
						anything is OK, this routine is only a check to prevent
						some errors but suntar still works without checking for
						space... */
/*dp("spazio libero=%ld (%ld da usare)\n",l,dlen+rlen);*/


/* by the way, GetVInfo is a dangerous routine, because:
a) HFS stores  the allocation block size in 32 bits (really too much, but that's not
   a problem) and absurdly the allocation blocks number in 16 bit (a little scarce today,
   absolutely insufficient in the near future), so that, no doubts, quickly there will
   be a HFS2 which will have a 32-bit free blocks number
b) GetVInfo is probably the only routine which is affected by the sizes of those
   fields, luckily it returns a long, obtained by multiplying the two numbers, but
   unfortunately it's "NOT in ROM": that is, it's part of the library linked to your
   program by the compiler, and that code can't be changed by a patch to the ROM.
   You must relink the program to update a "not in ROM" routine. Hence, in order
   to keep old applications working, Apple should patch PbGetVInfo, but they should
   know whether you need the allocation block size in order to multiply it by other
   numbers (and they can alter all numbers so that the products are as nearest as
   possible to the true values) or you need it itself (e.g. a compression program
   may need it to know whether the compressed files occupies less physical space
   than the original), and obviously in that case the true value should be returned.

hence, I don't know how the GetVInfo call should be performed to be compatible with
HFS2, and as far as I know there is no other way to get the free space but let the
system perform that multiplication (and know where to get the numbers and how many
bits they occupy)

However, Inside Mac IV gives this warning for PBGetVInfo (but not for PBHGetVInfo):
IOVNmAlBlks and ioVFrBlks, which are actually unsigned integers, are clipped to 31744
($7C00) regardless of the size of the volume.
*/
	/*printf("l=%ld\n",l);*/
	if( ((dlen+511)&~511L) + ((rlen+511)&~511L) <= l )
		return 0;	/* space is enough */
	else{
		if(!prima_volta){
			if(my_semimodal_dialog(147,titoli_si_no,2,
				in_Italia?"\pSpazio ancora insufficiente,\rproseguo comunque ?":
				"\pSpace is still insufficient,\rcontinue anyway ?",PNS,PNS)==1)
				return 0;
/*			else continue */
			}
		do{
			i=semimodalDialog(141,NULL,NULL,titoli_full,3,
			   in_Italia?"\pSpazio su disco insufficiente: puoi voler cancellare\r\
qualcosa prima di cliccare su un bottone\roppure puoi abortire il comando":
"\pInsufficient space on destination volume: you\r\
might wish to delete something before clicking a\r\
button or you may abort the current command",
			   NULL,NULL,teJustCenter,true,NULL);
			if(i==2){
			   	/* continue elsewhere */
			   	touch_if_needed();
				select_directory();
				cancella_file_aperto(pb.ioParam.ioRefNum);
				if(reply.good) return 2;
				}
			else if(i==3)	/* skip */
				return 1;
			else{
				/* continue here: maybe the user deleted some files */
				prima_volta=false;
				}
			}
		while(i==2);	/* if I'm here, it was cancelled */
		}
	}
}



static short gestisci_nome_doppio(hfname,fpb,duplicate)
/* handle a duplicate name error */
unsigned char* hfname;	/* Pascal string */
register FileParam *fpb;
Boolean duplicate;
{
short err,i,l;
WDPBRec param;
Str255 name;

l=hfname[0];
my_p2cstr(hfname);	/* this function was written to work on C strings */
settori_passati=0;
beep_in_foreground();
/*if(fase==hack_reading){
	error_message("File already existing !\n");
	}
*/
/* ci sono due modi di procurarsi le informazioni da mettere nelle due variabili 
che dicono dove lo standard file deve posizionarsi inizialmente:
1) come nel programma Apple, creare una WD con PBOpenWD e poi fare PBGetWDInfo
2) non creare alcuna WD, ma allora bisogna usare PBGetCatInfo per avere il DirID
   e PBGetWDInfo (sulla WD preesistente...) per avere il vRefNum del volume
   (cioè, della root)
per ora seguo il metodo 1, che funziona, ma quando avrò tempo preferirei passare al 2
-- I must oblige SFPutFile to show the folder where the file had to go, and that's
done by setting two low memory variables. But I must get the information in the
right format, and the simplest way to do that requires the creation of a working
directory (the other way looks even easier, but currently it does not work, at least
not always)
*/

for(i=l-1;i>=0 && hfname[i]!=':';i--)
	;
#if 1
/* ora i è -1 o punta ad un ':' */
if(i<=0){		/* un : in testa non conta... */
	err=GetVol (name,&param.ioVRefNum);
	/* name[0]=0; */
	}
else{
	mcopy(&name[1],hfname,i+1);	/* così tronco al : (che resta compreso)...*/
	name[0]=i+1;

#ifdef USA_CURRVREFN
	param.ioVRefNum=curr_vrefnum;
#else
	param.ioVRefNum=0;
#endif
	}
	param.ioNamePtr=(unsigned char*)name;
	param.ioWDProcID=signature;
	param.ioWDDirID=0;
	err=PBOpenWDSync(&param);		/* ho bisogno di una WD perché doForceDirectory
								è fatta a quel modo... */
	doForceDirectory(&param);
	register_WD(param.ioVRefNum);

#else
/* that's the second way: in this form it's worse, but, damn it, why should I be
obliged to do two calls to get both the dirID and the root vRefNum ? */

/* se mi decido per questo, togliere doForceDirectory da StdFile.c ! */
/*printf("giusti = %d %ld \n",-SFSaveDisk,CurDirStore);*/
{CInfoPBRec cinfo;

/*printf("fname=%s, name=%P\n",fname,name);*/

cinfo.dirInfo.ioNamePtr= (unsigned char*)name;
cinfo.dirInfo.ioVRefNum=curr_vrefnum;
cinfo.dirInfo.ioFDirIndex=0;
cinfo.dirInfo.ioDrDirID=0;
PBGetCatInfoSync(&cinfo);		/* get the dirID */

/*printf("result= %d attr=%x\n",cinfo.dirInfo.ioResult,cinfo.dirInfo.ioFlAttrib);
printf("cose che vedo =%d %ld %d\n",cinfo.dirInfo.ioVRefNum,cinfo.dirInfo.ioDrDirID,
	cinfo.dirInfo.ioFRefNum);*/
/*cinfo.dirInfo.ioDrDirID contiene il DirId giusto, ma il vRefNum giusto
non sta in nessun campo !*/
	param.ioNamePtr = NULL;
	param.ioVRefNum = 0;
	param.ioWDIndex = 0;
	param.ioWDProcID = 0;

	PBGetWDInfoSync(&param);		/* get the vrefnum */
/* ioVRefNum è quello che voglio, ma ovviamente non avendogli dato il path da lì
non posso avere alcun DirID, e anche a dare il path sembra che non si ottenga */
/*printf("stavolta ho %d %ld\n",param.ioVRefNum, param.ioWDDirID);*/

	CurDirStore = cinfo.dirInfo.ioDrParID  or cinfo.dirInfo.ioDrDirID ???????;
	SFSaveDisk = -param.ioVRefNum;
}
#endif
/*printf("settato %d %ld\n",SFSaveDisk,CurDirStore);*/

my_c2pstr(hfname+i+1);

while( SFPut_con_pausa(hfname+i+1,duplicate?
	(in_Italia?"\pIl file esiste già,\rsalvalo come:":
	"\pThe file is already existing,\rsave as:") :
	"\pFile could not be created,\rsave as:")){
	handle_pause();
	accept_abort_command();
	}
if(!reply.good) return 1;

	fpb->ioNamePtr = reply.fName;
	fpb->ioVRefNum = reply.vRefNum;
	fpb->ioFVersNum = 0;
	err=PBCreateSync(fpb) ;
	/*SetHandleSize(hfname,reply.fName[0]+1);*/
	pStrcpy(hfname,reply.fName);		/* devo salvarlo prima se no lo perdo */
	if(err==dupFNErr){
		if(open_overwrite(reply.fName,reply.vRefNum,&(fpb->ioFRefNum))==noErr)	/* ok il 
				codice che dice che esiste, siamo dopo un SFPutFile, ma bisogna 
				troncare il file... */
			FSClose(fpb->ioFRefNum);
		}

	else if(err!=noErr)
		pbsyserr(fpb);

	fpb->ioVRefNum = reply.vRefNum;
	fpb->ioFVersNum = 0;
	fpb->ioFDirIndex = 0;
	return 0;
}

static void skip_macbinII(size)
short size;
{
while(size>0){
	if(readblock(&tarh,128)) raise_error();
	size-=128;
	}
}


void get_openfile_location(ioRefNum)
short ioRefNum;
{
/* get the current name and location of the file, so that it may be
reopened even if the user has moved or renamed it */

FCBPBRec param;

fillmem(&param,0,sizeof(param));
/*param.ioVRefNum=0;*/
param.ioRefNum=ioRefNum;
/*param.ioFCBIndx=0;*/

param.ioNamePtr=mac_file_name;
if((err_code=PBGetFCBInfoSync(&param))==noErr){
	openfile_vrefnum=param.ioFCBVRefNum;
	openfile_dirID=param.ioFCBParID;
	/*pStrcpy(mac_file_name,param.ioNamePtr);*/
	}
else
	openfile_dirID=0;
/*
dp("err=%d(%d) ioV=%d ioR=%d volRN=%d dirID=%ld name=%lx:%P\n",
err_code,param.ioResult,param.ioVRefNum,param.ioRefNum,param.ioFCBVRefNum,param.ioFCBParID,
param.ioNamePtr,param.ioNamePtr);
*/

}

Boolean this_file_is_writable(short);
Boolean this_file_is_writable(refn)
short refn;
{

#if 0
addirittura dà errore, forse perché il file è aperto (da me) con diritti di scrittura
--I get an error, maybe because the file is opened (by me) with write permission 
{FileParam fpb;
get_openfile_location(refn);

fillmem(&fpb,0,sizeof(fpb));
/*fpb.ioFVersNum = 0;
fpb.ioFDirIndex = 0;*/
fpb.ioVRefNum=openfile_vrefnum;
fpb.ioNamePtr=mac_file_name;
fpb.ioDirID=openfile_dirID;
dp("provo FInfo per %P %d %ld\n",mac_file_name,openfile_vrefnum,openfile_dirID);
if (PBGetFInfoSync(&fpb)) {
dp("failed %d\n",fpb.ioResult);
return true;	/* it's not worth to issue a fatal error */
}
dp("protected=%d\n",fpb.ioFlAttrib&1);
if(fpb.ioFlAttrib&1) return false;	/* the file is protected */
}
{VolumeParam vpb;
vpb.ioNamePtr=NULL;
vpb.ioVRefNum=refn; /*openfile_vrefnum;*/
vpb.ioVolIndex=0;
if(PBGetVInfoSync(&vpb)) return true;
dp("vol locked=%x\n",vpb.ioVAtrb&0x8080);
return (vpb.ioVAtrb&0x8080)==0;		/* true if the volume is not locked */
}
#else
/* there is a simpler way: */

FCBPBRec param;

fillmem(&param,0,sizeof(param));
/*param.ioVRefNum=0;*/
param.ioRefNum=refn;
/*param.ioFCBIndx=0;*/

param.ioNamePtr=mac_file_name;
if((err_code=PBGetFCBInfoSync(&param))!=noErr) return true;

return (param.ioFCBFlags&0x100)!=0;	/* fcbMdRByt sta nel byte alto */

#endif
}

static void print_containing()
{
register short i;
i=0;
printf(in_Italia ? "Contenente " : "Containing ");
while(i<macbinh.nlen){
	if(macbinh.name[i]=='\r') /* it happens for icon files, and if you do a MacBinary
						write of a folder the icon file is saved */
		put_char(19);		/* diamond, in most fonts it's missing but that's OK
						for this purpose */
	else
		put_char(macbinh.name[i]);
	i++;
	}
printf(" (data %ld+res %ld bytes)", macbinh.dflen, macbinh.rflen);
}

void uudecode (fsize)
  long fsize;
{
short i;
void find_uu_start (void);

hqx_length=fsize;

find_uu_start();

if(expert_mode&&di.file_format<=ff_tarbar) printf(s_spazi);
printf(in_Italia ? "Contenente " : "Containing ");
print_fullname();

write_uu_fork();
termina_info_file();

}

#define FAST_GET_HQX_BYTE(b)	\
if(!more_in_bytes||(--hqx_length)<0)		\
	b=(unsigned char)get_hqx_byte(&dummy);	\
else							\
	b= (unsigned char)disk_buffer[511-(--more_in_bytes)];


void write_uu_fork()
/* visto che il chiamante non può fare un lookahead dei dati, ci deve
pensare lei a testare il tipo e quindi anche ad aprire il file (infatti
è chiamata anche in listonly)
--other formats perform some lookahead so that the write_xx_fork routine
is called after opening the output file. That does not happen for uudecode
hence I must create the file, and I do that only after knowing which
type&creator I must give to it
*/
{

unsigned char buffer[100];
register unsigned char*p, *hd_p;
register char b_0,b_1;
#define b_2 b_0	/* it's always better to avoid declaring too many register variables */
#define b_3 b_1
register long avail;
register unsigned char chars_to_decode;
Boolean dummy;
Boolean testato=false;
int iii;

chars_to_decode=0;
#define UUDEC(c)	(((c) - ' ') & 0x3F)

reinit_hd_buffering();
init_deDOSize();

for(;;){
	if(!chars_to_decode){		/* refill the input line */
		do{
			p=buffer-1;
			
			/*while( (*++p=get_hqx_byte(&dummy))!=CR && *p!=LF){
				if(*p<32 || *p >=127 || p>=buffer+sizeof(buffer)-1)
					goto uu_error;
				}*/

			for(;;){
				FAST_GET_HQX_BYTE(b_0);
				*++p=b_0;
				if(b_0==CR || b_0==LF) break;
				if(b_0<32 || b_0 >=127 || p>=buffer+sizeof(buffer)-1)
					goto uu_error;
				}
			*p='\n';
			}
		while(buffer[0]=='\n');	/* skip empty lines... */
		chars_to_decode=UUDEC(buffer[0]);	/* number of decoded bytes */
		if(chars_to_decode==0)
			break;		/* end of file reached */
		p=buffer+1;
		hd_p=hd_buffer+bytes_in_hd_buffer;
		avail=hd_buffer_size-bytes_in_hd_buffer;
		}
	if(!testato && bytes_in_hd_buffer>=512){
		/* the ouput file must be created before flushing the buffer
		towards it */
		test_for_uu();
		if(listonly || crea_e_controlla(0) )	/* operazione cancellata dall'utente */
			return;
		testato=true;
		}
	b_0=*p++;	/* 6 good bits */
	b_1=*p++;	/* 12 good bits */
	if(b_0=='\n'||b_1=='\n')
		goto uu_error;
	b_0=UUDEC(b_0);
	b_1=UUDEC(b_1);
	*hd_p++=b_0 << 2 | b_1 >> 4;		/* one byte is available */
	if(--chars_to_decode){	/* at least two decoded bytes */
		b_2=*p++;	/* 18 good bits */
		if(b_2=='\n')
			goto uu_error;
		b_2=UUDEC(b_2);
		*hd_p++=b_1 << 4 | b_2 >> 2;	/* two bytes are available */
		if(--chars_to_decode){
			chars_to_decode--;
			b_3=*p++;	/* 24 good bits */
			if(b_3=='\n')
				goto uu_error;
			b_3=UUDEC(b_3);
			*hd_p++ = b_2 << 6 | b_3;	/* three bytes are available */
			bytes_in_hd_buffer+=3;
			avail-=3;
			}
		else{
			bytes_in_hd_buffer+=2;
			avail-=2;
			}
	if(avail<=0){
		/* un valore negativo NON significa scarabocchiare perché init_hd_buffer
		alloca 4 bytes in più
		--since init_HD_buffer allocates 4 bytes over the correct size, avail<0 does
		not mean that I'm writing outside the allocated buffer
		*/
		if(!binary && !non_convertire)
			bytes_in_hd_buffer=macize_ASCII(hd_buffer,bytes_in_hd_buffer,0);
		if(bytes_in_hd_buffer>hd_buffer_size){
			/* faccio questo per scrivere comunque in blocchi di multipli di
			512 bytes allineati su multipli di 512 bytes, non ne sono sicuro
			ma credo che questo velocizzi la scrittura permettendo al file system
			di evitare di passare per i suoi buffer privati
			--I did not verify, but I think that writing in multiples of 512 bytes
			aligned to multiples of 512 bytes is faster, since the OS and the disk
			driver need not reorganize the data to fit disk sectors
			*/
			avail=bytes_in_hd_buffer-hd_buffer_size;
			bytes_in_hd_buffer=hd_buffer_size;
			if(flush_hd_buffer()) pbsyserr(&pb);

			write_hd(hd_buffer+hd_buffer_size,avail);
			}
		else
			if(flush_hd_buffer()) pbsyserr(&pb);
 
		hd_p=hd_buffer+bytes_in_hd_buffer;
		avail=hd_buffer_size-bytes_in_hd_buffer;
		}
		}
	else{
		bytes_in_hd_buffer+=1;
		avail-=1;
		}
	}
if(!testato){	/* file più corto di 512
				--the file is shorter than 512 bytes, I reached the end
				before creating the output file */
	test_for_uu();

	if(listonly || crea_e_controlla(0) )	/* operazione cancellata dall'utente */
		return;
	}
if(!binary && !non_convertire)
	bytes_in_hd_buffer=macize_ASCII(hd_buffer,bytes_in_hd_buffer,0);
if(flush_hd_buffer()) pbsyserr(&pb);

/* get_openfile_location(pb.ioParam.ioRefNum); useless */
PBCloseSync(&pb);
devo_chiudere_out=false;
return;

uu_error:
	flush_hd_buffer();
	error_message("error in uudecode\n");
#undef b_2
#undef b_3
}

static void test_for_uu()
/* a sub-function of the previous one: identifies the file type,
prints informations about it and sets type & creator to be used to
open the file */
{

macbinh.finfo.fdFlags=0;
macbinh.dflen=hqx_length- (hqx_length>>2); /* hqx_length*3/4, serve per crea_e_controlla */
macbinh.rflen=0;

binary=1;

if(!known_extension()){
	if(is_macbin(hd_buffer,bytes_in_hd_buffer,1)) {	/* I don't know
				the file size, hence I must relax the test */
		filecreator=macbin_creator;
		filetype='TEXT';
		printf(" (MacBinary)");
		}
	else{
		/* istructions copied from elsewhere */
 		short nLF,nCR;
  		short isText;

		macbinh.dflen=hqx_length- (hqx_length>>2); /* hqx_length*3/4 */
		macbinh.rflen=0;

  		isText=isASCII(hd_buffer,bytes_in_hd_buffer,&nLF,&nCR);
  	  	binary= ! (isText>0 && nLF>0);
  		filecreator= (isText>0) ? text_creator : '????';
  		filetype= (isText>0 || isText==-2) ? 'TEXT' : '????';
		if (isText>0){
			printf(in_Italia?TESTO_ASCII:ASCII_TEXT);
			if(!binary){
				if(nCR)
					printf(MSDOSTEXT);
				else
					printf(UNIXTEXT);
				}
			}
		else if(is_gif_file(hd_buffer,bytes_in_hd_buffer)){
			printf(COMPUSERVEGIF);
  			filecreator= gif_creator;
  			filetype='GIFf';
  			}
  		}
  	}

vai_a_capo();
}


void examine_POSIX_prefix(void);
static void examine_POSIX_prefix()
{
/* unfortunately the "prefix" field is in overlay with some fields of GNU tar
(see suntar.h). In theory the magic field should tell what's the version,
(it contains 'ustar  \0' or nothing for an old tar, 'ustar\0' followed by
a version (typically '00') for a recent tar/pax. However, I prefer not
to rely exclusively on that and examine directly the contents of the fields,
which are different enough to be distinguished without any risk of confusion
*/
if(tarh.POSIX_prefix[0]=='\0')
	full_name=strcpy(nam_buf,tarh.name);	/* no prefix */
else{
	/* in the GNU archives I've seen, atime and ctime were always empty. If
	they were filled, anyway they would contain two octal numbers, and that's
	different enough from a partial pathname */
	register char *p;
	register short i;
	Boolean has_prefix;
	if(!strcmp(tarh.magic,"ustar"))		/* i.e. without the two spaces */
		has_prefix=true;
	else{
		p=&tarh.atime[0];
		has_prefix=false;
		i=12+12;	/* sizeof(atime)+sizeof(ctime) */
		while(i--){
			if(*p!='\0'&&*p!=' '&&(*p<'0'||*p>'9')){
				has_prefix=true;
				break;
				}
			p++;
			}
		}
	if(has_prefix){
		/* it's a prefix: the full name is prefix/name, and since I've reserved
		room before the tarh.name field, I just copy the prefix */
		short lp, l=tarh.name[99]?100:strlen(tarh.name);
		tarh.POSIX_prefix[155]=0;	/* that's the first byte AFTER the prefix, which
			should be empty but could be filled, and prefix is not guaranteed to be
			zero-terminated... */
		lp=strlen(tarh.POSIX_prefix);
		full_name=nam_buf;
		/*full_pathname=NewHandle(l+lp+2);*/
		strncpy(nam_buf,tarh.name,100);
		*(full_name+l) = '/';
		mcopy(full_name+l+1,tarh.POSIX_prefix,lp+1);	/* including the '\0' */
		}
	else
		full_name=strcpy(nam_buf,tarh.name);	/* it's not a prefix... */
	}
}


void reinit_full_name()
{
if(full_name){
	full_name=NULL;
	}
}

#if 0
Handle string_to_handle(p)
char*p;
{
Handle h;
reinit_full_name();
h=NewHandle(strlen(p)+1);
strcpy(*h,p);
return h;
}
#endif

void print_fullname()
{
if(full_name!=NULL){
	printf("%s",full_name);
	}
}


void untar_long_name()
/* GNU tar 1.11 has introduced a new feature: long pathnames. It's more
powerful than the POSIX long pathnames, but:
a) Yet Another Incompatible Format
b) it's an huge conceptual change: a lot of rules about the tar format are no more
    valid (e.g. "a file is composed by one sector containing the header followed by
   the sectors with the contents" or "a pathname has a fixed maximum size") 
c) unlike POSIX, this feature is NOT thought to behave reasonably when extracting
   with a tar not supporting it: the 99 chars in the "regular" header are not
   always a valid pathname
d) it creates problems to suntar, which generally may extract starting from any
   file: if it sees the regular header but not the prefix, since the header has
   nothing special, suntar will behave exactly like a tar not supporting the feature
*/
{
Size namesize= untar_number(tarh.size,1);

/*reinit_full_name(); full_pathname=NewHandle(namesize+1);*/

full_name=nam_buf;
if(namesize>sizeof(nam_buf)-1){
	while(namesize>2*sizeof(nam_buf)-1){
		if (readblock(nam_buf, sizeof(nam_buf)))
			raise_error();
		namesize-=sizeof(nam_buf);
		}
	if (readblock(nam_buf, namesize-(sizeof(nam_buf)-1)))
		raise_error();
	namesize=sizeof(nam_buf)-1;
	}
if (readblock(nam_buf, namesize))
	raise_error();
nam_buf[namesize]=0;
}

static void untar_long_linkname()
/* Uh, probably that will never be met, not behaving well should not
be a problem, silently skip it (the only negative consequence will be
a truncated name in the "link ignored" message) */
{
sect_n+= (untar_number(tarh.size,1)+511) >>9;
}

void TouchDirectory(void);
/*OSErr*/ void TouchDirectory(/*short vRefNum, long dirID*/)
/* I've found this in the HyperCard stack "DevScraps", till now files
extracted by suntar appeared without an icon if the destination folder
was open in the Finder, and one had to Get Info or to close the folder
and reopen it. Otherwise, clicking on the file did NOT cause what
was expected, applications were not launched and so on.
This routine forces the Finder to update the icon */
{
	CInfoPBRec	info;
	Str63	name;
	/*OSErr	err;*/

	info.dirInfo.ioDrDirID = 0;	/* the default directory dirID */
	info.dirInfo.ioVRefNum = 0;	/* the default volume vRefNum */
	info.dirInfo.ioNamePtr = name;
	info.dirInfo.ioFDirIndex = -1;

	if (PBGetCatInfoSync(&info) == noErr) {
		info.dirInfo.ioCompletion = 0;
		info.dirInfo.ioDrDirID = info.dirInfo.ioDrParID;
		info.dirInfo.ioFDirIndex = 0;
		GetDateTime(&info.dirInfo.ioDrMdDat);
		/*err =*/ PBSetCatInfoSync(&info);
		}
}

void touch_if_needed()
{
if(need_a_touch){
	TouchDirectory();
	need_a_touch=false;
	}
}

/************* end of routines by Speranza **************************/

/************* Routines obtained by modifying ones by Zacharias **********
(that is, modifying their interface and purpose and not only their code)
 **********************************************************************/


/********************* hqx *******************************/

void untar_hqx (fsize)
  long fsize;
{
jmp_buf savebuf;
short i;

hqx_length=fsize;
mcopy(&savebuf,&main_loop,sizeof(jmp_buf));
if((i=setjmp(main_loop))<0 ){
	mcopy(&main_loop,&savebuf,sizeof(jmp_buf));
	if(i==-2)		/* raise_hqx_error... */
		return;
	raise_error();
	}

if( hqx_header() ){			/* con risultati in macbinh
							-- results in macbinh */
	mcopy(&main_loop,&savebuf,sizeof(jmp_buf));
	return;
	}

attacca_path_e_nome();

disable_autoflush(1);
if(expert_mode&&di.file_format<=ff_tarbar) printf(s_spazi);	/*per allinearsi col [sector xxxx] stampato
				presumibilmente alla riga prima
				-- to align with the [sector xxxx] printed in the previous line */
print_containing();
if(fase==hack_listing){
	printf("  ");
	print_type_creator(macbinh.finfo.fdType,macbinh.finfo.fdCreator);
	}
vai_a_capo();
if(macbinh.dflen+macbinh.rflen>=FLUSH_MIN_SIZE) flush_console();	/* somebody
	complained about the slowness of the operations with many small files, and
	buffering the printing to the console may speed up things up to 40%, but I
	can't leave the console unflushed during a long file... */

filecreator=filetype='????';

if(!listonly && ! crea_e_controlla(1) ){

	write_hqx_fork(macbinh.dflen);

	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsWrPerm;
	pb.ioParam.ioMisc = 0;
	if((pb.fileParam.ioDirID=openfile_dirID)!=0){
		pb.ioParam.ioVRefNum=openfile_vrefnum;
		pb.ioParam.ioNamePtr=mac_file_name;
		}
	if( PBHOpenRFSync(&pb)) pbsyserr(&pb);

	devo_chiudere_out=true;

	write_hqx_fork(macbinh.rflen);

	termina_info_file();
	set_binhex();
	}
hqx_end_of_file();
mcopy(&main_loop,&savebuf,sizeof(jmp_buf));

}

void write_hqx_fork(size)
  long size;
{
long bytes_to_read;

reinit_hd_buffering();
current_crc=0;
#if 0
while (size) {
	bytes_to_read= 512;
	if(bytes_to_read>size) bytes_to_read=size;
	read_hqx(&tarh,bytes_to_read);
	size -= bytes_to_read;
	if(write_hd((char*) &tarh, bytes_to_read) ) pbsyserr(&pb);
	}
if(flush_hd_buffer()) pbsyserr(&pb);

#else
/* ha un unico svantaggio: se il file è incompleto, non riesco ad ottenere
comunque la parte che c'è: risolto tramite un'istruzione in read_hqx che prima
di dare errore... */
while (size) {
	bytes_to_read= hd_buffer_size;
	if(bytes_to_read>size) bytes_to_read=size;
	read_hqx(hd_buffer,bytes_to_read);
	size -= bytes_to_read;
	bytes_in_hd_buffer=bytes_to_read;
	if(flush_hd_buffer()) pbsyserr(&pb);

	}
#endif
get_openfile_location(pb.ioParam.ioRefNum);
PBCloseSync(&pb);
devo_chiudere_out=false;
check_CRC();
}

void write_pit_fork(size)
  long size;
{
long bytes_to_read;

reinit_hd_buffering();

#if 0
while (size) {
	bytes_to_read= 512;
	if(bytes_to_read>size) bytes_to_read=size;
	get_pit_bytes(&tarh,bytes_to_read);

	size -= bytes_to_read;
	if(write_hd((char*) &tarh, bytes_to_read)) pbsyserr(&pb);
	}
if(flush_hd_buffer()) pbsyserr(&pb);
#else
while (size) {
	bytes_to_read= hd_buffer_size;
	if(bytes_to_read>size) bytes_to_read=size;
	/*printf("ora chiedo %ld bytes\n", bytes_to_read);*/
	flush_console();
	get_pit_bytes(hd_buffer,bytes_to_read);
	/*printf("ottenuti...\n");*/
	flush_console();
	size -= bytes_to_read;
	bytes_in_hd_buffer=bytes_to_read;
	if(flush_hd_buffer()) pbsyserr(&pb);

	}
#endif
get_openfile_location(pb.ioParam.ioRefNum);
PBCloseSync(&pb);
devo_chiudere_out=false;
}


void set_binhex ()
/* set the file informations from the BinHex header, almost identical to setmacbin */
{
FileParam fpb;
fpb.ioFVersNum = 0;
fpb.ioFDirIndex = 0;

if((fpb.ioDirID=openfile_dirID)!=0){
	fpb.ioVRefNum=openfile_vrefnum;
	fpb.ioNamePtr=mac_file_name;
	}
else{
	fpb.ioVRefNum=pb.fileParam.ioVRefNum;
	fpb.ioNamePtr=pb.fileParam.ioNamePtr;
	}

if (PBHGetFInfoSync(&fpb)) pbsyserr(&fpb);

clear_unused_fields();
mcopy(&fpb.ioFlFndrInfo, &macbinh.finfo, 10); /* type,creator e flags ma non il resto*/
if(di.file_format==ff_binhex){
	fpb.ioFlCrDat=macbinh.cdate;
	fpb.ioFlMdDat=macbinh.mdate;
	}
else if(file_date!=-1)
		fpb.ioFlCrDat = fpb.ioFlMdDat = file_date + unixtime;
fpb.ioDirID=openfile_dirID;	/* PBHSetFInfo writes the FlNum return value in that field ! */
if (PBHSetFInfoSync(&fpb)) pbsyserr(&fpb);
need_a_touch=true;

}


short untar_checksum(buffer,do_error,verbose)
register unsigned char *buffer;
short do_error;
Boolean verbose;
{
register short i;
register long chksum;
long hchksum;
#define tar_chksum_offset 148 /* (tarh.checksum-(char*)&tarh) */
hchksum = untar_number(((tarh_type*)buffer)->chksum,do_error);
if(hchksum==-1){
	if(verbose) printf("Empty header checksum\n");
	return -2;		/* testata non tar */
	}
chksum= ' ' * 8;	/* nel calcolo del checksum, il campo checksum era sostituito
					da spazi */
for (i=0; i < tar_chksum_offset; ++i)
	chksum += (unsigned char)*buffer++;
for (i+=8,buffer+=8; i < sizeof(tarh); ++i)
	chksum += (unsigned char)*buffer++;

if (chksum == hchksum) return 0;	/* tutto OK */
if(verbose){
	printf(BAD_HEADER_CHECS);
	if(expert_mode){
		if(((tarh_type*)disk_buffer)->linkflag!='V')
			if(print_if_string(GOT_EXPECTED,((tarh_type*)disk_buffer)->chksum,8))
				printf("%lo)",chksum);
		}
	vai_a_capo();
	}
return -1;
}

static short is_macbin(buf,fsize,relax_test)
char *buf;
long fsize;
short relax_test;
{

mcopy(&macbinh, &buf[1], sizeof(macbinh));

/* in the MacBinary format, the data and resource forks should occupy an
integer number of 128 bytes blocks (the size of XMODEM blocks): however, 
some programs save space by not extending the two forks, hence I must
accept both standard and "compact" formats
*/

if (buf[0] == 0 && macbinh.zero == 0 && macbinh.nlen < 64 && macbinh.nlen > 0 ){
	if(macbinh.dflen >= 0 && macbinh.rflen >= 0 ){
		short i=isMacBin2(buf);
		long expected_length_compact=macbinh.dflen +macbinh.rflen + 128L;
		long expected_length_standard=macbinsz(macbinh.dflen) + macbinsz(macbinh.rflen) + 128L;
		if( (MacBinaryII=i>0)){
/* The MacBinary || standard defines fields for the sizes of extra header info
and of comment:
>  Offset 099-Word, length of Get Info comment to be sent after the resource
>             fork (if implemented).
> *Offset 120-Word, Length of a secondary header.  If this is non-zero,
>             Skip this many bytes (rounded up to the next multiple of 128)
>             This is for future expansion only, when sending files with
>             MacBinary, this word should be zero.
 we've never seen files using those fields, nor versions of MacbinaryII
 bigger than 129, but it would not be honest to declare that suntar is
 MacBinaryII compatible without supporting them
*/
			mcopy(&mbIIsec_head_len,&macbinh.headlen[0],2);
			mbIIsec_head_len = macbinsz(mbIIsec_head_len);
			expected_length_standard += macbinh.gilen + mbIIsec_head_len;
			/* I believe that a MBII file can't be "compact", and surely it can't 
			be compact and have comment info or secondary header */
			}
		if(relax_test ||
			(fsize==expected_length_compact||fsize==expected_length_standard) &&
			i>=0 ){
			notTrueMacBinary = fsize<expected_length_standard;
			if(relax_test==ff_c_macbin&&fsize<expected_length_standard)
				notTrueMacBinary=true;

			if( MacBinaryII )
				macbinh.finfo.fdFlags = macbinh.finfo.fdFlags&0xFF00 |
						(unsigned char) macbinh.extra_fflags;
			/* else
				macbinh.finfo.fdFlags = macbinh.finfo.fdFlags&0xFF00; NO:
The first MacBinary standard did not store bits 7-0, stating that
the byte following bits 15-8 should be zeroed. But mtar did not zero it, and maybe
it's the same for other programs (but NOT for BinHex5.0), anyway suntar does not check
for them to be zero, and in a MacBinary (not MacBinary II) files which has a nonzero 
value in that byte it believes that it's containing the correct values of bits 0-7.
*/
			return 1;
			}
		}
	}
return 0;
}

short chkmacbin (fsize,relax_test)
/* tests whether the file is MacBinary */
  long fsize;
  short relax_test;
{
  char buf[128];

if (fsize == 0) return 0;
if (readblock(buf,128))	 /* devo leggerlo comunque, per uniformità */
	raise_error();
if (fsize < 128){
	unget_block();
	return 0;
	}
if(is_macbin(buf,fsize,relax_test)) return 1;
unget_block();
return 0;
}

void setdata ()
/* set modification date = creation date */
{
FileParam fpb;
fpb.ioFVersNum = 0;
fpb.ioFDirIndex = 0;

if((fpb.ioDirID=openfile_dirID)!=0){
	fpb.ioVRefNum=openfile_vrefnum;
	fpb.ioNamePtr=mac_file_name;
	}
else{
	fpb.ioVRefNum=pb.fileParam.ioVRefNum;
	fpb.ioNamePtr=pb.fileParam.ioNamePtr;
	}
if (PBHGetFInfoSync(&fpb)==noErr){	/* not to be able to set the date is NOT a big error */
	fpb.ioFlMdDat = fpb.ioFlCrDat;
	fpb.ioDirID=openfile_dirID;
	PBHSetFInfoSync(&fpb);
	}
}

static void attacca_path_e_nome()
{
register char *cp, *pn;
/* prendi il path del nome originale e sostituisci il nome file con il nome
contenuto nell'header MacBinary o BinHex contenuto in macbinh */
pn=full_name;
for(cp = pn+strlen(pn);
	 cp != pn && cp[-1] != ':'; --cp)
	 	;
/*SetHandleSize(full_pathname,cp-pn+macbinh.nlen+1);
cp=*full_pathname+(cp-pn);*/
#if 0
mcopy(cp, macbinh.name, macbinh.nlen);
cp[macbinh.nlen] = '\0';
#else
*cp='\0';
my_c2pstr(full_name);
pStrcat(full_name,&macbinh.nlen);
#endif
}



void copy_mac_file (name)
/* copies a Macintosh file (both data and resorce fork) to another place,
without any conversion
beware, "full_name" must contain the destination name
*/
unsigned char*name;
{
extern short dest_wdir;
OSErr err;
short org_wdir=curr_vrefnum;
ParamBlockRec par;

di.file_format=ff_macintosh;

/* macbinheader() si aspetta i dati in posti ben precisi, quando non ci
sono già devo metterceli
-- obviously I'm exploiting routines which already existed, such as
macbinheader(), which expect the data in a MacBinary format
*/
pStrcpy(&macbinh.nlen,name);
statf (name);
macbinheader();
SetVol(NULL,curr_vrefnum=dest_wdir);
filecreator=macbinh.finfo.fdCreator;
filetype=macbinh.finfo.fdType;

pStrcpy(full_name,name);
if( ! crea_e_controlla(1) ){
	writefork(512,macbinh.dflen, -1);
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsWrPerm;
	pb.ioParam.ioMisc = 0;
	if((pb.fileParam.ioDirID=openfile_dirID)!=0){
		pb.ioParam.ioVRefNum=openfile_vrefnum;
		pb.ioParam.ioNamePtr=mac_file_name;
		}
	if(macbinh.rflen){
		if( PBHOpenRFSync(&pb)) pbsyserr(&pb);

		devo_chiudere_out=true;
		/* anche il sorgente va riaperto sulla res fork, e sarebbe meglio usare
		anche qui get_location !!!!! e una PBH, ovviamente */
		get_openfile_location(di.m_vrefnum);
		FSClose(di.m_vrefnum);

		#if 1
		if((par.fileParam.ioDirID=openfile_dirID)!=0){
			par.ioParam.ioVRefNum=openfile_vrefnum;
			par.ioParam.ioNamePtr=mac_file_name;
			}
		else{
			par.fileParam.ioDirID=0;
			par.ioParam.ioVRefNum=org_wdir;
			par.ioParam.ioNamePtr=&macbinh.nlen;
			}
		par.ioParam.ioVersNum = 0;
		par.ioParam.ioPermssn = fsWrPerm;
		par.ioParam.ioMisc = 0;
		if( PBHOpenRFSync(&par)) pbsyserr(&par);

		di.m_vrefnum=par.ioParam.ioRefNum;
		#else
		if(err=OpenRF (&macbinh.nlen, org_wdir,&di.m_vrefnum)){
			pb.ioParam.ioResult=err;
			pb.ioParam.ioNamePtr=&macbinh.nlen;
			pbsyserr(&pb);
			}
		#endif
		end_of_file();
		invalid_buffers();
		sect_n=0;
		pb.ioParam.ioReqCount = 512;
		writefork(512,macbinh.rflen, -1);
		}
	setmacbin();
	}
}


/******* end of routines by Gail Zacharias, rewritten by Speranza ********/

/********* start of routines by Gail Zacharias, modified by Speranza *******/


void untar ()
{

long exp_check;
short i;

	disable_autoflush(1);
	more_in_bytes=0;	/* forse non necessario, ma prudente */

	mcopy(&tarh,&disk_buffer,sizeof(tarh));
	if(expert_mode && !full_name) print_sector_n(sect_n);
	sect_n++;
	if ( (i=untar_checksum(&disk_buffer[0],1,true)) != 0 ){
		if(i==-2 || !ignore_errors) raise_error();
		if(i==-1) printf(s_spazi);
		}
	copia_ultimo_header(disk_buffer,sect_n);	/* sect_n è già stato incrementato */
	last_offset=0;		/* l'offset è 0 perché non è un header 'M' */

	file_date= untar_number(&tarh.mtime,false);

	if(!full_name)	/* it may be nonzero only if the last header was a GNU
			extension 'L' */
		examine_POSIX_prefix();

	switch (tarh.linkflag) {
	case '\0': case '0': case '7':
is_file:	
		if ((full_name)[strlen(full_name)-1] == '/')
			untar_directory();
		else
			untar_file(untar_number(tarh.size,1));
		break;
	case '1': case '2':
		untar_link();
		break;
	case '5':
		untar_directory();
		break;
	case 'K':	/* new for GNU tar 1.11: long linkname */
		untar_long_linkname();
		break;
	case 'L':	/* new for GNU tar 1.11: long name */
		untar_long_name();
		break;
	case 'M':
		if(untar_number(&tarh.offset,-1)==0) goto is_file;		/* the file header
			was in the last sector of previous disk, hence this continuation
			header is followed by the whole contents of the file, so it may
			be extracted starting from here: all informations are available... */
		/* else continue */
	case 'V':
		error_message("Bad use of GNU extensions\n");
		break;
	default:
		error_message_1("Unknown header type (octal %o)\n", tarh.linkflag);
    }
}


void untar_file (fsize)
long fsize;
{
  short nLF,nCR;
  short isText;
  short isBinHex;
  short dostext=0;
  long cdate=untar_number(bar_archive?((barh_type*)disk_buffer)->mtime:
		((tarh_type*)disk_buffer)->mtime,-1);
	if(listonly!=2){
		/* 2 means Get File Info, which has already printed the file name and size */
		printf("File ");
		print_fullname();
		}
	if(!bar_archive){
		next_header_for_POSIX = sect_n + ((fsize+511)>>9);
		if(next_header_for_POSIX<di.sectors_on_floppy)
			next_header_for_POSIX=-1;
		else
			next_header_for_POSIX-=di.sectors_on_floppy;
		}
	if(sect_n==di.sectors_on_floppy){
		if(fase==hack_listing){
			if(di.tipo_device==d_file)
				printf(UNEXP);
			else
				printf(" (%ld bytes, on next volume)\n",fsize);
			/* for expert list of POSIX archives, I must let sect_n point to next
			file header... */
			enable_autoflush();
			full_name=NULL;
			longjmp(main_loop,-1);
			}
		else
			start_of_line();	/* ci saranno i messaggi di cambio disco...
								--some messages are arriving... */
		enable_autoflush();
		}
	if ( di.file_format==ff_macbin || di.file_format<=ff_tarbar && chkmacbin(fsize,0)){
		/*printf(" ");
		if(fase==hack_listing)*/
		if(listonly!=2) printf(" (%ld bytes) ",fsize);
		if(notTrueMacBinary) printf("compact ");
		printf("MacBinary");
		if(MacBinaryII) printf(" II");
		vai_a_capo();
		if(expert_mode&&di.file_format<=ff_tarbar) printf(s_spazi);
		print_containing();
		if(fase==hack_listing){
			printf("\n%s",s_spazi);
			print_type_creator(macbinh.finfo.fdType,macbinh.finfo.fdCreator);
			if(cdate!=-1) print_one_date(cdate+unixtime,in_Italia?"  creato ":"  created ");	/* di solito in expert mode non
					bado alla lingua, ma poi stampo una stringa del toolbox che lo fa...*/
			}
		vai_a_capo();
		if(macbinh.dflen+macbinh.rflen>=FLUSH_MIN_SIZE) flush_console();

		check_confirmation();	/* se ora andassi in non_convertire, nessun problema... */
		unix_to_mac();
		if(non_convertire&&di.file_format<=ff_tarbar){
  			unget_block();	/* i 128 bytes di intestazione... */
			filetype='TEXT';
			filecreator=macbin_creator;
			untar_data(fsize,0);
  			}
  		else{
			untar_macbin(fsize);
			}
		}
	else{
		if(listonly!=2) printf(" (%ld bytes)",fsize);	/* queste scritte devono stare prima della
					check_confirmation, ma del resto i test sulle conversioni vanno dopo
					e questo complica un po' le cose */
		if(di.file_format<=ff_tarbar)
			isBinHex= fsize>=20 ? is_hqx_name() : 0;
		else
			isBinHex=di.file_format==ff_binhex;
		if(isBinHex){
			printf(in_Italia?" testo BinHex":" BinHex text");
			binary=0;	/* per il caso poi non vada convertito */
  			filecreator= text_creator;
  			filetype='TEXT';
			}
		else{
			if(di.file_format<=ff_tarbar){
  				isText=isASCII(disk_buffer,fsize,&nLF,&nCR); /* grazie a chkmacbin, disk_buffer è pieno... */
  	  			binary= ! (isText>0 && nLF>0);
  	  			dostext= nLF&&nCR;
  	  			}
			else{
  				isText= di.file_format==ff_ASCII ? 1 : -1;	/* I'm not here in that case, but
  						who knows what may happen in the future... */
  				binary=false;
  				}

			if(!known_extension()){
  				filecreator= (isText>0) ? text_creator : '????';
  				filetype= (isText>0 || isText==-2) ? 'TEXT' : '????';
				if (isText>0){
					printf(in_Italia?TESTO_ASCII:ASCII_TEXT);
					if(!binary){
						if(nCR)
							printf(MSDOSTEXT);
						else
							printf(UNIXTEXT);
						}
					}
				else if(is_gif_file(disk_buffer,fsize)){
					printf(COMPUSERVEGIF);
  					filecreator= gif_creator;
  					filetype='GIFf';
  					}
				}
			else if(isText==0 && !listonly){
				binary=1;
				start_of_line();
				printf("This file is not ASCII\n");
				}
			}
		if(fase==hack_listing)
			if(cdate!=-1) print_one_date(cdate+unixtime,in_Italia?"  creato ":"  created ");

		vai_a_capo();
		if(fsize>=FLUSH_MIN_SIZE) flush_console();

		check_confirmation();	/* durante il quale posso anche cambiare preferences */
		unix_to_mac();

		if( (di.file_format==ff_binhex||!(disable_binhex&1)) && isBinHex==1 && 
			!(non_convertire && !listonly && di.file_format<=ff_tarbar ) ){
  			untar_hqx(fsize);
  			}
		else{
  			untar_data(fsize,dostext);
  		}
	}
	full_name=NULL;
}


void untar_data (fsize,dostext)
  long fsize;
  short dostext;
{

	macbinh.dflen=fsize;		/* crea_e_controlla expects some data here... */
	macbinh.rflen=0;
	macbinh.finfo.fdFlags=0;
	if(!listonly && ! crea_e_controlla(0) ){
		pb.ioParam.ioReqCount = sizeof(tarh);
		notTrueMacBinary=true;	/* writefork tests it... in practice its value is
			irrelevant, since extra bytes in the last sector are skipped anyway */
		writefork(512, fsize, binary || non_convertire&&di.file_format<=ff_tarbar ?
			-1 : dostext);
		if(save_modi_date) setdata();
		}
	else
		skip_file(fsize);
}

void untar_macbin (fsize)
  long fsize;
{

attacca_path_e_nome();

filecreator=filetype='????';	/* temporaneamente, ma in caso di errore resta così */


if(!listonly && ! crea_e_controlla(1) ){
	if(MacBinaryII) skip_macbinII(mbIIsec_head_len);	/* secondary header... */
	writefork(128,macbinh.dflen, -1);
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = fsWrPerm;
	pb.ioParam.ioMisc = 0;
	if((pb.fileParam.ioDirID=openfile_dirID)!=0){
		pb.ioParam.ioVRefNum=openfile_vrefnum;
		pb.ioParam.ioNamePtr=mac_file_name;
		}
	if(macbinh.rflen){
		if( PBHOpenRFSync(&pb)) pbsyserr(&pb);
		devo_chiudere_out=true;
		pb.ioParam.ioReqCount = 128;
		writefork(128,macbinh.rflen, -1);
		}
	if(MacBinaryII) skip_macbinII(macbinh.gilen);		/* comment */
	setmacbin();

	end_of_file();
	}
else
	skip_file(fsize);
}

void writefork(chunk_size, size, binp)
short chunk_size;
long size;
short binp;	/* -1 binary 0 UNIX-like text 1 MS-DOS-like text */
{
	short bytes_to_read;

	reinit_hd_buffering();
	init_deDOSize();

#if 0	/* old version... */
	while (size) {
		bytes_to_read= chunk_size;
		if(notTrueMacBinary && bytes_to_read>size) bytes_to_read=size;
		if (readblock(&tarh, bytes_to_read))
			 raise_error();
		if(bytes_to_read>size) bytes_to_read=size;	/* in true MacBinary, I may read
			from the archive more bytes than I write to the file */
		size -= bytes_to_read;
		if (binp>=0) bytes_to_read=macize_ASCII(&tarh,bytes_to_read,binp);

		if(write_hd((char*) &tarh, bytes_to_read)) pbsyserr(&pb);
		}
#else
	while(size){
		bytes_to_read=chunk_size;
		if(chunk_size==128 && size>=512 && more_in_bytes==0)
			bytes_to_read=512;	/* in such case there is no reason
					to avoid the optimization, the limitation to 128 bytes
					is effective only at the end of the fork */
		if(notTrueMacBinary && bytes_to_read>size) bytes_to_read=size;
		if(more_in_bytes || bytes_to_read!=512){		/* do it the old way; it must
					be inside the loop anyway because when extracting a MacBinary file
					I start with more_in_bytes=384, in three steps it decreases to 0
					and then I can use the optimized version */
			if (readblock(&tarh, bytes_to_read))
					raise_error();
			if(bytes_to_read>size) bytes_to_read=size;	/* in true MacBinary, I may read
				from the archive more bytes than I write to the file */
			size -= bytes_to_read;
			if (binp>=0) bytes_to_read=macize_ASCII(&tarh,bytes_to_read,binp);
			if(write_hd((char*) &tarh, bytes_to_read)) pbsyserr(&pb);
			}
		else{	/* optimized: rather than mcopy from the input buffer to disk_buffer,
				from it to tarh, from there to hd_buffer, do a single copy directly
				to hd_buffer, unless the data do not fit, in such case avoid at
				least one step */
			char *dest;
			short i;
			if(hd_buffer_size-bytes_in_hd_buffer>=512 && size>=512)
				dest=hd_buffer+bytes_in_hd_buffer;
			else
				dest=disk_buffer;
			/* this is obviosly a copy of part of the body of readblock */
			if(bar_archive)
				bar_check_floppy_swap(0);
			else
				tar_check_floppy_swap(0);
			leggi_settore(sect_n,dest);
 			if((i=check_error_and_events())<0) raise_error();
 			if(i>0){	/* "missing disk" was converted to a pause event, now the disk
 						should be in, but I must repeat the operation */
				leggi_settore(sect_n,dest);
 				if(check_error()) raise_error();
 				}
 			if(size<512)
				more_in_bytes=512-size;
			if(bytes_to_read>size) bytes_to_read=size;	/* in true MacBinary, I may read
				from the archive more bytes than I write to the file */
			size -= bytes_to_read;
			if (binp>=0) bytes_to_read=macize_ASCII(dest,bytes_to_read,binp);
			/* and this is part of the body of write_hd */
			if(dest!=disk_buffer){	/* copied directly to the out buffer */
				bytes_in_hd_buffer += bytes_to_read;
				if(bytes_in_hd_buffer==hd_buffer_size)
					if(flush_hd_buffer()) pbsyserr(&pb);
			}
			else{
				if(write_hd((char*) dest, bytes_to_read)) pbsyserr(&pb);
			}
			sect_n++;
			settori_passati++;
		}
	}
#endif
	if(flush_hd_buffer()) pbsyserr(&pb);

	get_openfile_location(pb.ioParam.ioRefNum);

	PBCloseSync(&pb);
	devo_chiudere_out=false;
}

void setmacbin ()
/* set the file informations from the MacBinary header */
{
FileParam fpb;
fpb.ioFVersNum = 0;
fpb.ioFDirIndex = 0;

if((fpb.ioDirID=openfile_dirID)!=0){
	fpb.ioVRefNum=openfile_vrefnum;
	fpb.ioNamePtr=mac_file_name;
	}
else{
	fpb.ioVRefNum=pb.fileParam.ioVRefNum;
	fpb.ioNamePtr=pb.fileParam.ioNamePtr;
	}

if (PBHGetFInfoSync(&fpb)) pbsyserr(&fpb);
clear_unused_fields();
mcopy(&fpb.ioFlFndrInfo, &macbinh.finfo, 10);
fpb.ioFlCrDat = macbinh.cdate;
fpb.ioFlMdDat = macbinh.mdate;

fpb.ioDirID=openfile_dirID;
if (PBHSetFInfoSync(&fpb)) pbsyserr(&fpb);
need_a_touch=true;
}

void untar_directory ()
{
  short len = strlen(full_name);

  if (len && (full_name)[len-1] != '/'){
  	/*SetHandleSize(full_pathname,len+2);*/
    strcpy(full_name+len, "/");	/* that is, strcat ! */
    }

  if(!bar_archive){
		if(sect_n<di.sectors_on_floppy)
			next_header_for_POSIX=-1;
		else
			next_header_for_POSIX=sect_n-di.sectors_on_floppy;
		}
  printf("Directory ");
  print_fullname();
  if(fase==hack_listing && file_date!=-1) print_one_date(file_date+unixtime,
  	in_Italia?"  creata ":"  created ");
  vai_a_capo();
  unix_to_mac();
  if (!strcmp(":",full_name )){
	full_name=NULL;
	return;
	}
  check_conf_dir(full_name);
  if(!listonly){
  	create_directory(full_name);
  	}
  full_name=NULL;
}



/*  fname may contain a filename after the last ':', it will be ignored */
OSErr create_directory (fname)
  char *fname;
{
  FileParam fpb;
  char name[256], *cp, *bp;
  OSErr i;

  cp = fname + strlen(fname);
  for (bp = fname; *bp == ':'; ++bp);

  fpb.ioNamePtr = (StringPtr) name;
  /* first loop: shorten the path until one directory is created: e.g. if
  d1 exists but d1/d2 does not, shorten d1/d2/d3/d4/filename to d1/d2
  because d1/d2/d3/d4 and d1/d2/d3 can't be created */
  do {
	while (cp[-1] != ':') --cp;
	if (cp == bp){ printf("%s - Bad directory name\n", fname); raise_error(); }
	strncpy(name+1, fname, name[0] = --cp - fname);

	retry:
#ifdef USA_CURRVREFN
	fpb.ioVRefNum = curr_vrefnum;
#else
	fpb.ioVRefNum = 0;
#endif

	fpb.ioDirID = 0;
	i=PBDirCreateSync(&fpb);
	if(i == bdNamErr){
		printf("%s - Bad directory name, not created\n", fname);
		return 1;
		}
	else if(i == dupFNErr){
		beep_in_foreground();
		if(fase==hack_reading){
			printf("Folder already existing\n");
			fpb.ioResult=noErr;
			return 0;
			}
		else{
			short item;
			do{
				item = my_semimodal_dialog(137,titoli_full,2,
					in_Italia?"\pLa cartella\312":"\pThe folder\312",name,in_Italia?
"\p\resiste già: prima di cliccare su un bottone puoi\rvoler cambiar nome o spostare quella preesistente\ro puoi abortire il comando":
"\p\ralready exists: before clicking a button, you might\rwish to rename, move or delete the old one, or\ryou may abort the command");
				if(item!=1){
					touch_if_needed();
					select_directory();
					if(reply.good)
						goto retry;	/* se il folder esisteva non può che essere l'ultimo
								nel path altrimenti non l'avrebbe accorciato, quindi non
								c'è motivo di ripristinare il nome */
					}
				}
			while(item!=1);
			return 0;		/* It's useless to continue with the second loop: since
							the path is not shortened when the prefix to the last
							component name exists, I may be here only with the
							original full path, so the second loop need not be executed,
							and the date assignment must not
							*/
			}
		}
  } while ( i== dirNFErr || i==fnfErr);

  /* second loop: one by one, reinsert in the path all the directories after
  the one that was created in the first loop and create them: in the above
  example, create d1/d2/d3 and then d1/d2/d3/d4 */
  if (fpb.ioResult) pbsyserr(&fpb);

  while (1) {
    while (*++cp != ':')
    	if (!*cp){
		/* new for suntar 1.3.2
		now, the last directory created is the originally requested directory:
		if the name was not followed by a file name, assign to it the creation date */
			CInfoPBRec cipb;
			if(file_date==-1 || fname[strlen(fname)-1]!=':' )
				 return 0;	/* the path contains a file name, hence the date
				 	is of the file, not of the directory just created */
			fillmem(&cipb,0,sizeof(cipb));
			cipb.dirInfo.ioNamePtr= (StringPtr) name;
			cipb.dirInfo.ioVRefNum=curr_vrefnum;
			cipb.dirInfo.ioFDirIndex=0;
			if(i=PBGetCatInfoSync (&cipb)) return;
  
			cipb.dirInfo.ioNamePtr= (StringPtr) name;
			cipb.dirInfo.ioVRefNum=curr_vrefnum;
			cipb.dirInfo.ioDrDirID=0;
			cipb.dirInfo.ioDrMdDat=cipb.dirInfo.ioDrCrDat=
				unixtime+file_date;
  			i=PBSetCatInfoSync (&cipb);

    		return 0;
		 	}
    strncpy(name+1, fname, name[0] = cp - fname);

#ifdef USA_CURRVREFN
	fpb.ioVRefNum = curr_vrefnum;
#else
	fpb.ioVRefNum = 0;
#endif

    fpb.ioDirID = 0;
    if (PBDirCreateSync(&fpb)) pbsyserr(&fpb);
    }
 return 0;
}

short create_file (orig_name,perm)
  unsigned char* orig_name;	/* Pascal string */
  short perm;
/* ritorna 1 se, causa nome doppio o bad name, il salvataggio è stato annullato
-- returns 1 if, due to a duplicate name, the user clicked on Cancel
*/
{
/*
this routine was heavily modified by us (Speranza).
OK, it's incoherent that for duplicate names a dialog appears
while bad names are renamed without asking to the user...
*/
	FileParam fpb;
	Str255 hfname;
	extern SysEnvRec	gMac;
	Boolean opened_OK=false;
	Boolean changed;
	Str255 name;
	Boolean already_bad=false;

	/* use a copy, in order to save the original name if the name is modified
	(e.g. if the file existed) */
	pStrcpy(hfname,orig_name);

	fillmem(&fpb,0,sizeof(fpb));
	fpb.ioNamePtr = hfname;

	while(!opened_OK){

	#ifdef USA_CURRVREFN
		fpb.ioVRefNum = curr_vrefnum;
	#else
		fpb.ioVRefNum = 0;
	#endif

		fpb.ioFVersNum = 0;
		if(!PBCreateSync(&fpb)) {
		#ifdef USA_CURRVREFN
			fpb.ioVRefNum = curr_vrefnum;
		#else
			fpb.ioVRefNum = 0;
		#endif

			fpb.ioFVersNum = 0;
			fpb.ioFDirIndex = 0;
			opened_OK=true;
			need_a_touch=false;	/* because the creation modified anyway the directory
					(well, not true for files nested within directories, but I don't
					want to be too precise) */
			}
		else{
/*printf("creo %P con %d\n",name,fpb.ioResult);*/
/* The new network software from Apple has a bug: with System 7.0 and 7.01, 
when the "File Sharing Extension" is active (on our Mac LC which is not 
connected to a network), PBCreate returns paramErr (error in parameter block) 
when it should return bdNamErr (bad name) and fnfErr (file not found) when 
it should return dirNFErr (folder not found).
  Since fnfErr was not expected, we handle it as a dirNFErr. However, thinking
that any paramErr is a misnamed bdNamErr would be dangerous, hence we check that
the name contains a subname which is longer than 31 characters (since unix_to_mac
translates any non-ASCII char and checks the placing of ':', there should
be no other possible cause for a bad name) before translating the err code.
*/
			if(fpb.ioResult==paramErr && too_long(hfname) ) fpb.ioResult=bdNamErr;

			changed=false;
			if(fpb.ioResult == dupFNErr && already_bad){
					fpb.ioResult=bdNamErr;	/* try again, by incrementing the number
										at start of the bad name... */
					changed=true;		/* per non soddisfare il test che segue */
					}

			if(fpb.ioResult == dupFNErr || fpb.ioResult==bdNamErr&&!changed&&already_bad){
				if( gestisci_nome_doppio(hfname,&fpb,fpb.ioResult == dupFNErr) ){
					/*mcopy(main_loop,mlcopy,sizeof(main_loop));*/
					return 1;	/* annullato...*/
					}
				already_bad=false;
				opened_OK=true;
				}
			else if(fpb.ioResult== bdNamErr){
				/* check_foreground(); no, ma... non lo so */
				if(!already_bad) SysBeep(5);
				make_badname(hfname,already_bad);	/* o anche qui proporre di cambiare nome !*/
				already_bad=true;
				fpb.ioNamePtr = hfname;
				}
			else if (fpb.ioResult == dirNFErr || fpb.ioResult==fnfErr){ /* System 7
					sometimes returns fnfErr when the folder does not exist */
					/* at least one directory specified in the path does not exist:
					create it and retry */
				my_p2cstr(hfname);
				if(create_directory(hfname)){	/* bad directory name: I'm not
						sure it may happen, but just in case... */
					my_c2pstr(hfname);
					make_badname(hfname,already_bad);
					already_bad=true;
					fpb.ioNamePtr = hfname;
					}
				else{
					my_c2pstr(hfname);
					fpb.ioNamePtr = hfname;
					}
				}
			else{
				pbsyserr(&fpb);
				}
			}
  		}

	pStrcpy(name,hfname);
	fpb.ioNamePtr = name;
	if (PBGetFInfoSync(&fpb)) pbsyserr(&fpb);	/* modifies the name parameter ! */

	pStrcpy(name,hfname);
	/*mcopy(main_loop,mlcopy,sizeof(main_loop));*/
	
	fpb.ioNamePtr = name;
	fpb.ioFlFndrInfo.fdType = filetype;
	fpb.ioFlFndrInfo.fdCreator = filecreator;
	if(di.file_format==ff_binhex)
		fpb.ioFlCrDat =macbinh.cdate;	/* non posso assegnare ora anche la data
			di modifica, perché sto per modificare il file: in macbinary e binhex
			lo faccio dopo, per il .info pazienza... Poi, è bene avere un'indicazione 
			che il salvataggio di un file non è stato completato (l'icona appare solo
			alla fine...)
			--there is no reason to set the modification date now, since I'm 
			going to modify the file ! That date is modified after closing the
			file, but not for the .info file. Also type and creator are set only
			when closing the file, so that the icon appears only when the file is 
			completed and may be opened by the application.
			*/
	else if(file_date!=-1 )
		fpb.ioFlCrDat = file_date + unixtime;

	if (PBSetFInfoSync(&fpb)) pbsyserr(&fpb);

	pb.ioParam.ioVRefNum = fpb.ioVRefNum;
	pb.ioParam.ioNamePtr = name;
	pb.ioParam.ioVersNum = 0;
	pb.ioParam.ioPermssn = perm;
	pb.ioParam.ioMisc = 0;
	pb.ioParam.ioRefNum = 0;
	if (PBOpenSync(&pb)) pbsyserr(&pb);
	devo_chiudere_out=true;
	return 0;
}

void untar_link ()
{
 printf("Link %s -> %.100s%s\n", full_name, tarh.linkname,
   		listonly?"":in_Italia?" (ignorato)":" (ignored)");
 full_name=NULL;

  if(!bar_archive){
		if(sect_n<di.sectors_on_floppy)
			next_header_for_POSIX=-1;
		else
			next_header_for_POSIX=sect_n-di.sectors_on_floppy;
		}
}

/* Convert Unix pathname to a mac pathname.  Do not allow absolute
   names - "/foo/bar/baz" is treated as if it were "foo/bar/baz".
*/
void unix_to_mac ()
{
  char buf[sizeof(nam_buf)+10];
  register unsigned char *cp, *bp, *op, c;

  cp=full_name;

  /*buf=NewHandle(strlen(cp)+10);a little more room for the extra ":" */
  bp=buf;

  *bp++ = ':';
  op = bp;
  while (1) {
    if (cp[0] == '/') ++cp;
    else if (cp[0] == '.' && cp[1] == '/') cp += 2;
    else if (cp[0] == '.' && cp[1] == '.' && cp[2] == '/') {
      if (op == bp) *bp++ = ':', op = bp;
      else for (--op; op != bp && op[-1] != ':'; --op);
      cp += 3;
    }
    else {
      while (c=*cp, c!=0 && c != '/'){
      	if(c== ':')
      		c= '/';
      	else if(c=='_' && !pres_underscore && !pres_all_chars)
      		c=' ';
      	else if(!pres_all_chars && (c<' '||c>126)) c=0xC9;	/* '…' */
      	*op++ = c;
      	cp++;
      	}
      if (!*cp++) break;
      *op++ = ':';
    }
  }
  *op = '\0';
  /*DisposHandle(full_pathname);*/
  strcpy(full_name,buf);
}

#if 0
/* Name is the user-specified name, file is the actual tarred file */
void match_file (name, file)
  char *name, *file;
{
  char *cp;
  if (!strdiff(name, file)) return 1;
  if ((cp = strrchr(file, ':')) && !strdiff(cp+1, name)) return 1;
  if (name[strlen(name)-1] == ':' && strdiff(name, file) >= 0) return 1;
  return 0;
}

/* return 0 if the same, 1 if s1 is a substring of s2, -1 otherwise */
strdiff (s1, s2)
  char *s1, *s2;
{
  while (*s1) {
    if (*s1 != *s2) {
      char c1 = *s1;
      if ('A' <= c1 && c1 <= 'Z') c1 |= 0x20;
      if ('a' > c1 || c1 > 'z' || c1 != (*s2 | 0x20)) return -1;
    }
    ++s1, ++s2;
  }
  return (*s2 ? 1 : 0);
}
#endif

long untar_number (cp,doerror)
  char *cp;
  short doerror;
{
  short neg = 0;
  long num = 0;
  while (*cp == ' ') cp++;
  if (*cp == '-') neg++, cp++;
  if('0' <= *cp && *cp <= '7'){
 	 while ('0' <= *cp && *cp <= '7') num = (num<<3) + (*cp++ - '0');
 	 if (neg) num = -num;
 	 }
  else
  	cp="A";		/* per fargli dare errore
  				-- in order to force the error */
  if(*cp!='\0'&&*cp!=' '){
  	if(doerror>=0)
  		printf(in_Italia?"Testata non in formato %car !\n":
  			"Error: not a %car header !\n",bar_archive ? 'b' : 't');
  	if(doerror>0)
  		raise_error();
  	else
  		return -1L;
  }
  return num;
}

void pbsyserr (fpb)
  ioParam *fpb;
{
  short err = fpb->ioResult;
  char *name = (char*) fpb->ioNamePtr;
  start_of_line();
  if (err == fnfErr) printf("%P - File not found\n", name);
  else if (err == nsvErr) printf("%P - No such volume\n", name);
  else if (err == tmfoErr) printf("%P - Too many files open\n", name);
  else if (err == permErr) printf("%P - Permissions error\n", name);
  else if (err == dupFNErr) printf("%P - Duplicate filename\n", name);
  else if (err == eofErr) printf("%P - Premature end of file\n", name);
  else if (err == dskFulErr) printf("%P - Disk full\n", name);
  else if (err == opWrErr) printf("%P - Already open with write permission\n",name);
  else printf("%P - Error #%d\n", name, err);
  raise_error();
}
