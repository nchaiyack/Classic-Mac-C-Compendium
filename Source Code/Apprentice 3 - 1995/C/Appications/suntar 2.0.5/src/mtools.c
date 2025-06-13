/*
 * Mtools package v.2.0.4
 *
 * Emmet P. Gray			US Army, HQ III Corps & Fort Hood
 * ...!uunet!uiucuxc!fthood!egray	Attn: AFZF-DE-ENV
 * fthood!egray@uxc.cso.uiuc.edu	Directorate of Engineering & Housing
 * 					Environmental Management Office
 * 					Fort Hood, TX 76544-5057
 */

/* a few routines were chosen and slightly adapted to become: */

/*******************************************************************************\

MS-DOS module

part of suntar, ©92 Sauro & Gabriele Speranza

\*******************************************************************************/

#include <string.h>
#include "suntar.h"
#include "windows.h"
#include "msdos.h"


/********************** mdir.c *************************/

#define LITTLEND_WORD(x) ( (*((unsigned char*)(x)+1)<<8)+ *(unsigned char*)(x) )

static unsigned short fat_len,num_clus,clus_size,curr_sector,fat_start;
Boolean old_msdos;

static unsigned char get_FAT_byte(offset)
unsigned short offset;
{
unsigned short sector=offset>>9;
if(sector!=curr_sector){
	leggi_settore(fat_start+sector,disk_buffer);
	if(err_code) longjmp(main_loop,-2);
	curr_sector=sector;
	}
return disk_buffer[offset&0x1FF];
}

unsigned short fat_decode(unsigned short);
unsigned short
fat_decode(num)
unsigned short num;
{
	unsigned short fat;
	unsigned short start;

	if (1 /*fat_bits == 12*/) {
		unsigned char byte_1, byte_2;
		/*
		 *	|    byte n     |   byte n+1    |   byte n+2    |
		 *	|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
		 *	| | | | | | | | | | | | | | | | | | | | | | | | |
		 *	| n+0.0 | n+0.5 | n+1.0 | n+1.5 | n+2.0 | n+2.5 |
		 *	    \_____  \____   \______/________/_____   /
		 *	      ____\______\________/   _____/  ____\_/
		 *	     /     \      \          /       /     \
		 *	| n+1.5 | n+0.0 | n+0.5 | n+2.0 | n+2.5 | n+1.0 |
		 *	|      FAT entry k      |    FAT entry k+1      |
		 */
					/* which bytes contain the entry */
		start = (num + num + num) / 2;
		if (start <= 2 || start + 1 > (fat_len * 512))
			return(1);

		byte_1=get_FAT_byte(start);
		byte_2=get_FAT_byte(start + 1);
					/* (odd) not on byte boundary */
		if (num & 1) 
			fat = (byte_2 << 4) + ((byte_1 & 0xf0) >> 4);
					/* (even) on byte boundary */
		else
			fat = ((byte_2 & 0xf) << 8) + byte_1;
		}
	else {
		/*
		 *	|    byte n     |   byte n+1    |
		 *	|7|6|5|4|3|2|1|0|7|6|5|4|3|2|1|0|
		 *	| | | | | | | | | | | | | | | | |
		 *	|         FAT entry k           |
		 */
		 #if 0
					/* which bytes contain the entry */
		start = num * 2;
		if (start <= 3 || start + 1 > (fat_len * 512))
			return(1);

		fat = get_FAT_byte(start)+(get_FAT_byte(start+1)<<8);
		#endif
		}
	return(fat);
}

/*
 * Get the amount of free space on the diskette
 */

static void
getfree(result)
unsigned short result[];
{
	register unsigned short i,j;
	unsigned short total,bad;
	extern unsigned short num_clus;

	total = 0;
	bad=0;
	for (i = 2; i < num_clus + 2; i++) {
					/* if fat_decode returns zero */
		if (!(j=fat_decode(i)))
			total += clus_size;
		else if(j==0xFF7)		/* FFF7 in 16-bit FATs */
			bad += clus_size;
		/*else printf("%d %x\n",i,fat_decode(i));*/
		}
	/*printf("settori liberi =%d difettosi=%d totali=%d\n",total,bad,num_clus*clus_size);*/

result[0]=total;
result[1]=bad;
result[2]=num_clus*clus_size;
}


short visita_FAT_msdos(unsigned short *);
short visita_FAT_msdos(result)
unsigned short result[];
{
short dir_len,dir_start;
jmp_buf temp;
	mcopy(temp,main_loop,sizeof(main_loop));
	leggi_settore(0,disk_buffer);
	if(err_code || setjmp(main_loop) <0){
		mcopy(main_loop,temp,sizeof(main_loop));
		return -1;
		}
	if (old_msdos) {
		/* there may be a number of formats (see init.c in mtools), but
		only one is not totally obsolete */
		dir_start = 5;
		dir_len = 7;
		clus_size = 2;
		fat_len = 2;
		num_clus = 354;
		fat_start = 1;
		}
	else{
		clus_size = disk_buffer[13];
		fat_len = LITTLEND_WORD(&disk_buffer[22]);
		dir_len = (LITTLEND_WORD(&disk_buffer[17]) * MDIR_SIZE + 512-1) >>9;
		fat_start = LITTLEND_WORD(&disk_buffer[14]);
		dir_start = fat_start + (disk_buffer[16] * fat_len);
		num_clus = (unsigned int) (LITTLEND_WORD(&disk_buffer[19]) - dir_start - dir_len) / clus_size;
		}
/*printf("dir_start,clus_size,n_sectors=%d %d %d\n",dir_start,clus_size,LITTLEND_WORD(&disk_buffer[19]) );
printf("fat_len,dir_len,fat_start,num_clus= %d %d %d %d\n",fat_len,dir_len,fat_start,num_clus);*/

curr_sector=-1;
getfree(result);
mcopy(main_loop,temp,sizeof(main_loop));
return 0;
}


#define WRITE_WORD(ind,val) write_word((char*)&ind,val)

static void write_word(ind,val)
/* all numbers used by MS-DOS are stored in a little-endian fashion */
char *ind;
short val;
{
*ind++=lowbyte(val);
*ind=highbyte(val);
}

short msdos_logical_format(sector_t);
short msdos_logical_format(tot_sectors)
sector_t tot_sectors;
{
	short i, c, fat_len, dir_len, clus_size;
	short num_clus;
	sector_t n_sect;
	unsigned char media;
	short tracks,heads,sectors;

{
struct bootsector boot;	/* cosi' va in overlay con buf */

	tracks = 80;
	heads = 2;

	switch (tot_sectors) {
		case 720:		/* 40t * 2h * 9s = 360k */
			media = 0xfd;
			clus_size = 2;
			dir_len = 7;
			fat_len = 2;
			tracks=40;
			sectors=9;
			break;
		case 1440:		/* 80t * 2h * 9s = 720k */
			media = 0xf9;
			clus_size = 2;
			dir_len = 7;
			fat_len = 3;
			sectors=9;
			break;
		case 2400:		/* 80t * 2h * 15s = 1.2m */
			media = 0xf9;
			clus_size = 1;
			dir_len = 14;
			fat_len = 7;
			sectors=15;
			break;
		case 2880:		/* 80t * 2h * 18s = 1.44m */
			media = 0xf0;
			clus_size = 1;
			dir_len = 14;
			fat_len = 9;
			sectors=18;
			break;
		default:		/* a non-standard format */
			printf("Error: unknown disk size\n");
			return;
		}
					/* the boot sector */
	fillmem((char *) &boot, '\0', MSECTOR_SIZE);
	boot.jump[0] = 0xeb;
	boot.jump[1] = 0x44;
	boot.jump[2] = 0x90;
	strncpy((char *) boot.banner, "Mtools  ", 8);
	WRITE_WORD(boot.secsiz,512);
	boot.clsiz = (unsigned char) clus_size;
	WRITE_WORD(boot.nrsvsect,1);
	boot.nfat = 2;
	WRITE_WORD(boot.dirents, dir_len * 16);
	WRITE_WORD(boot.psect, (short)tot_sectors);
	boot.descr = media;
	WRITE_WORD(boot.fatlen, fat_len);
	WRITE_WORD(boot.nsect, sectors);
	WRITE_WORD(boot.nheads, heads);

	boot.jump[510]=0x55;	/* in teoria bisognerebbe farlo solo se il disco */
	boot.jump[511]=0xAA;	/* è boot-tabile, ma Access PC non riconosce un
							disco come MS-DOS se non c'è questa coppia di valori...*/

					/* write the boot */
	scrivi_settore((n_sect=0), (char *) &boot);
	if(err_code) return err_code;
}{
unsigned char buf[MSECTOR_SIZE];	/* cosi' va in overlay con boot */
struct directory dir;

					/* first fat */
	fillmem((char *) buf, '\0', MSECTOR_SIZE);
	buf[0] = media;
	buf[1] = 0xff;
	buf[2] = 0xff;
	scrivi_settore(++n_sect, (char *) buf);
	if(err_code) return err_code;
	fillmem((char *) buf, '\0', MSECTOR_SIZE);
	for (i = 1; i < fat_len; i++){
		scrivi_settore(++n_sect, (char *) buf);
		if(err_code) return err_code;
		}
					/* second fat */
	buf[0] = media;
	buf[1] = 0xff;
	buf[2] = 0xff;
	scrivi_settore(++n_sect, (char *) buf);
	if(err_code) return err_code;
	fillmem((char *) buf, '\0', MSECTOR_SIZE);
	for (i = 1; i < fat_len; i++){
		scrivi_settore(++n_sect, (char *) buf);
		if(err_code) return err_code;
		}
					/* the root directory */
	scrivi_settore(++n_sect, (char *) buf);
	if(err_code) return err_code;
	fillmem((char *) buf, '\0', MSECTOR_SIZE);
	for (i = 1; i < dir_len; i++){
		scrivi_settore(++n_sect, (char *) buf);
		if(err_code) return err_code;
		}
	flush_buffers(0);
	return err_code;
}
}


/****************** match.c ***********************/

/* this was in the only file of mtools which has no copyright notice,
hence probably it's a non-copyrighted routine by somebody else */

/*
 * Do shell-style pattern matching for '?', '\', '[..]', and '*' wildcards.
 * Returns 1 if match, 0 if not.
 * modifications by G. Speranza: returns 2 if the match was not exact but
 * required the expansion of ?, * or [..], useful to catch cases when there
 * must be at most one match
 * Second modification: the original routine handled incorrectly the *, if
 * after it there was immediately a ? or [ or (later) another *. E.g *[ac]
 * and a*bc did not match axxbyybc. Now such cases are handled correctly.
 */

short match(char *,char *);
#include <string.h>

short
match(s, p)
register char *s, *p;
{
	Boolean matched, reverse;
	char first, last;
	short used_pattern=0;

	for (; *p != '\0'; s++, p++) {
		switch (*p) {
			case '?':	/* match any one character */
				if (*s == '\0')
					return(0);
				used_pattern=1;
				break;
			case '*':	/* match everything */
				used_pattern=1;
				while (*p == '*')
					p++;

					/* if last char in pattern */
				if (*p == '\0')
					return(2);

					/* se dopo l'asterisco NON ci sono altri
					asterischi, allora conta il numero di
					caratteri nel pattern e posiziona la
					stringa in prova ad altrettanti posti
					dalla fine, a meno che questo comporti
					il tornare indietro */

				if(!strchr(p,'*')){
					register char *q=p;
					short n_after=0;
					while(*q){
						if(*q=='\\'&& q[1])
							q++;
						else if(*q=='[')
							while(q[1] && *q!=']')
								q++;
						n_after++;
						q++;
						}
					if((n_after-=strlen(s))>0)
						return 0;
					s -= n_after;		
					p--; s--;	/* ora vado al controllo
						di ciclo che li incrementa
						entrambi */
					}
				else{
					/* questa e' in grado di gestire
					correttamente tutti i casi, ma essendo
					molto inefficiente tanto vale che nel
					caso di cui sopra non venga usata
					-- this loop may handle correctly all uses of '*', but
					since it's rather inefficient, it's better to prefer the
					method above when there is no need for the full generality
					of a nondeterministic search  */
					while(*s){
						if(!match(s,p))
							s++;
						else
							return 2;
						}
					return 0;
					}
				break;
			case '[':	 /* match range of characters */
				used_pattern=1;
				first = '\0';
				matched = 0;
				reverse = 0;
				while (*++p != ']') {
					if (*p == '^') {
						reverse = 1;
						p++;
					}
					first = *p;
					if (first == ']' || first == '\0')
						return(0);

					/* if 2nd char is '-' */
					if (*(p + 1) == '-') {
						p++;
					/* set last to 3rd char ... */
						last = *++p;
						if (last == ']' || last == '\0')
							return(0);
					/* test the range of values */
						if (*s >= first && *s <= last) {
							matched = 1;
							p++;
							break;
						}
						return(0);
					}
					if (*s == *p)
						matched = 1;
				}
				if (matched && reverse)
					return(0);
				if (!matched)
					return(0);
				break;
			case '\\':	/* Literal match with next character */
				p++;
				/* fall thru */
			default:
				if (*s != *p)
					return(0);
				break;
		}
	}
					/* string ended prematurely ? */
	if (*s != '\0')
		return(0);
	else
		return(1+used_pattern);
}
