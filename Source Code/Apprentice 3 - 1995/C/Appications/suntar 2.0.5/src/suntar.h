/*******************************************************************************\

include file

suntar 2.0, ©1991-95 Sauro & Gabriele Speranza

This program is public domain, feel free to use it or part of it for anything

\*******************************************************************************/
#include <setjmp.h>

#define dp printf	/* used for debugging, it's simpler to find&delete all */
				/* uses of dp rather than find all printf & select the good ones
				from the debugging ones */

#define V_13	/* no more, but it meant "do some things in the new way" */
#define V_20

#define hqxID	145
#define writeID 132
#define hackID	133
#define deviceID 136
#define tarPopupID 146
#define ntAPopupID 147

#define fmNew		1
#define fmEncode	2
#define fmGetInfo	3
#define fmOpen		4
#define fmOpenDev	5
#define fmClose		6
#define fmEject		6
#define fmDriveList 7

#define fmCopy		9
#define fmList		10
#define fmExtract 	11
#define fmSelect	12
#define fmFormat	14
#define fmPause		16
#define fmAbort		17
#define fmQuit		18
#define last_fm_command fmFormat

#define pmOptions	6
#define pmEnglish	11
#define pmConfirm	12
#define pmExpert	8
#define pmRareOpt	9
#define pmIgnore	13
#define pmNoConvers 14

#define wmAppend	1
#define wmCreateTar	2
#define wmCreateBar	3
#define wmDataFork 5
#define wmASCII 6
#define wmWriteMacBin 7
#define wmWriteSmart 8
#define wmWriteSmartMB 9
#define wmWriteTar 10
#define wmSetLabel 12
#define wmEndWrite 14


#define hmCreateLog 1
#define hmView 2
#define hmClear 3
#define hmSave 4
#define hmOverwrite 5
#define hmMac_ize 6
#define hmIBMize 7
#define hmFind 8
#define hmUntar 9
#define hmUnbar 10
#define hmReadStarting 11
#define hmList 12
#define hmAgain 13

#define bmDisable 1
#define bmShowExtr 3
#define bmSaveInfo 4
#define bmShowList 6

#define ff_tarbar 1
#define ff_ASCII 2
#define ff_macbin 3
#define ff_c_macbin 4
#define ff_binhex 5
#define ff_packit 6
#define ff_uuencode 7
#define ff_macintosh 8

#define SHOWINFO (all_listonly&&(disable_binhex&4) || (!all_listonly&&(disable_binhex&2) ))
#define SAVEINFO (!listonly&&((disable_binhex&9)==8 || di.file_format==ff_binhex && (disable_binhex&8)) )

/* this define is used to compare the return value of a menu selection 
with known menu items */
#define menuItemMess(ID,item) (((long)ID<<16)+item)

#define signature 'S691';	/* useless, since OpenWD usually ignores it... */

#ifndef LF
#define LF 	'\012'	/*	'\n' in Think C, '\r' in MPW	*/
#define CR 	'\015'	/*	'\r' in Think C, '\n' in MPW 	*/
#endif

#define highbyte(x)		(((unsigned char *) &(x))[0])
#define lowbyte(x)		(((unsigned char *) &(x))[1])

#define max_drive 4

typedef unsigned char byte;
typedef long sector_t;
#define SECTOR_T_SIZE 4

union dsk_buffer{
	unsigned char dsk_buf [512];
	long ldsk_buf[512/sizeof(long)];	/* just to be sure it's aligned, sometimes
			I do casts on pointers to portions of disk_buffer to access a short or long */
	};
#define disk_buffer dsk_buffer.dsk_buf


#ifdef THINK_C_5
/* Damn it ! Think C 5 has changed the rules for assigning offsets to
struct fields (thanks to Basil Duval for reporting the problem to us).
Really that's not a bug, the assignment of offsets to struct fields is
an implementation choice which the compiler may arbitrarily alter, but
that's absolutely bad when the struct describes something which exists
outside the program (in this case, also in a different machine than the
current one, with different operating system etc.), so that the internal
structure of the struct cannot be arbitrarily altered.
  The solution is to alter the declarations in such a way that
ThC5 assigns the correct offsets (hoping that the rules won't be
changed again !). It would be more portable not to use a struct at all,
but a set of #define's which take pointers to an array of char, add an
offset and cast to the proper type, but that would require many
changes and would greatly decrease the readability of the source code:
e.g. MODE(tarh) rather than tarh.mode ...
 Think of what would happen if the structs used by the ToolBox could
not be represented by struct declarations: you would become crazy
adjusting your programs...
 Really, there is a pragma to tell Think C 5 to work in the right
way, "pragma options(!align_arrays)" but I did not succeed to use it,
getting strange results
*/


typedef struct tarh_type{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char linkflag;
	char linkname[100-1];	/* if the size is not odd, THC5 places a padding byte before
							it so it's aligned to an even address => catastrophe ! */
	char lastbyte1;			/* but obviously the last byte must be present ! */
	char magic[8-1];
	char lastbyte2;
	char uname[32-1];
	char lastbyte3;
	char gname[32-1];
	char lastbyte4;
	char devmajor[8-1];
	char lastbyte5;
	char devminor[8-1];
	char lastbyte6;
	char atime[12-1];
	char lastbyte7;
	char ctime[12-1];
	char lastbyte8;
	char offset[12-1];
	char lastbyte9;
	char longnames[4-1];
	char lastbyte10;
	char fill[127];
} tarh_type;

typedef struct barh_type{
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char rdev[8];
	char linkflag;
	char bar_magic[2-1];
	char lastbyte11;
	char volume_num[4-1];
	char lastbyte12;
	char compressed;
	char cdate[12];
	char name[100];
	char linkname[100];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char fill[148];
} barh_type;

extern struct binh_type{
	/*byte version;*/
	unsigned char nlen;
	char name[63];
	FInfo finfo;
	char protected;
	char zero;
	long dflen;
	long rflen;
	long cdate;
	long mdate;
	short gilen;
	char extra_fflags;
	char unused[14-1];
	char lastbyte13;
	char packlen[4-1];
	char lastbyte14;
	char headlen[2-1];
	char lastbyte15;
	unsigned char uploadvers;
	unsigned char readvers;
	char crc[2-1];
	char lastbyte16;
	char padding[2-1];
	char lastbyte17;
} macbinh;



#else
/* no problem, use the original declarations */

typedef struct tarh_type{
	char name[100];
	char mode[8];
	char uid[8];
	char gid[8];
	char size[12];
	char mtime[12];
	char chksum[8];
	char linkflag;
	char linkname[100];
	char magic[8];
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	#if 1	/* GNU use of the remaining space */
		/* these following fields were added by JF for gnu */
		/* and are NOT standard */
		char	atime[12];
		char	ctime[12];
		char	offset[12];
		char	longnames[4];
		char 	fill[127];	/* well, really there are other fields, but not interesting */
	#else	/* POSIX use of the remaining space */
		char prefix[155];
		char fill[12];
	#endif
		
} tarh_type;

typedef struct barh_type{	/* dalla documentazione UNIX 
							-- from the bar(5) page of man */
	char mode[8];
	char uid[8];		/* in ottale nell'header di file, in decimale in quello di volume 
						--in octal in file headers, decimal in volume headers */
	char gid[8];		/* idem */
	char size[12];		/* in ottale; nel volume header, è il numero di bytes restanti del 
						file a metà tra due dischetti (sempre multiplo di 512)
						-- in octal; in the volume header, it's the number of bytes 
						remaining from a previous file (always multiple of 512) */
	char mtime[12];		/* in ottale, secondi dal 1 gen 1970 
						-- in octal, seconds since 1 jan 1970 */
	char chksum[8];		/* in ottale */
	char rdev[8];		/* ??? */
	char linkflag;
	char bar_magic[2];	/* 'V\0', solo nell'header di volume 
						-- 'V\0', only in the volume header */
	char volume_num[4];	/* decimale... */
	char compressed;	/* '0', solo nel volume header 
						-- '0', only in the volume header */
	char cdate[12];		/* nel volume header, dice quando si e' creato l'archivio: YYMMDDHHmm\0 
						-- in the volume header, archive creation date: YYMMDDHHmm\0 */
	char name[100];
	char linkname[100];	/* in file normali tutto 0, come i seguenti...
						però, non è così: il nome del link parte immediatamente
						dopo il \0 che chiude il nome, non ad un offset fisso 
						-- see comments in the untar.c file */
	char uname[32];
	char gname[32];
	char devmajor[8];
	char devminor[8];
	char fill[148];
} barh_type;

extern struct binh_type{
	/*byte version;*/ 		/* the first byte is present in the MacBinary file but is
							NOT copied to this struct, so that the most commonly used
							fields start at an even address */
	unsigned char nlen;		/* Length of filename. */
	char name[63];			/* Filename (only 1st nlen are significant) */
	FInfo finfo;			/* type, creator, flags, zero1 (flags 0-7 must be cleared),
							point in folder, enclosing folder */
	char protected;			/* = 1 for protected file, 0 otherwise */
	char zero;				/* must be 0 */
	long dflen;				/* Data Fork length (bytes) */
	long rflen;				/* Resource Fork length */
	long cdate;				/* File's creation date */
	long mdate;				/* File's "last modified" date */
	short gilen;			/* GetInfo message length (MacBinary II only) */
	char extra_fflags;		/* Finder flags, bits 0-7 (MacBinary II only) */
	char unused[14];
	char packlen[4];		/* length of total files when unpacked */
	char headlen[2];		/* length of secondary header (MacBinary II only) */
	unsigned char uploadvers;		/* Version of MacBinary II that the uploading program is written for */
	unsigned char readvers;			/* Minimum MacBinary II version needed to read this file */
	char crc[2];			/* CRC of the previous 124 bytes */
	char padding[2];		/* two trailing unused bytes */
} macbinh;

#endif

extern tarh_type tarh;
#define POSIX_prefix atime

extern short err_code;		/* error code from read_sectors and write_sectors */

extern Boolean non_convertire;	/* disable conversions */
extern OSType text_creator;
extern Boolean ignore_errors,in_Italia,expert_mode,confirm_saves,bar_archive,multiVolume;
extern unsigned char listonly;
extern unsigned short max_hqx_header;
extern unsigned char previousFormat;
extern short info_file_open;
extern short info_file;
extern short disable_binhex;
extern unsigned short current_crc;
extern short show_info;
extern Boolean save_info;

extern short list_buf_size;
extern long hqx_length;
extern sector_t sect_n;

extern SFReply reply;
extern Point where[];
extern char *full_name,nam_buf[];
extern MenuHandle myMenus[4],hqxM;
extern jmp_buf main_loop;
extern union dsk_buffer dsk_buffer;
extern char ultimo_header[184];	/* non tutto, solo la parte che tiene i campi 
		importanti: lo uso sia in lettura sia in scrittura
		-- see in untar.c and tar.c */
extern Boolean ultimo_disco_espulso,ultimo_header_valido;
extern sector_t avail_sectors_for_file;
extern long last_offset;
extern Boolean gHasResolveAlias;
extern Boolean gHasVirtualMemory;
extern Boolean hasVheader;
extern Boolean inf_is_open;
extern short inf_refn;
extern short floppy_n;
extern sector_t settori_passati;
extern short curr_vrefnum;


enum tipo_fase {
/* the "fase" (phase) variable always remembers what suntar is currently doing, so that
some decisions may be performed by any function, not only by the higher levels of 
the functions hierarchy */
non_faccio_nulla,		/* nessun comando in corso di esecuzione, in attesa di uno 
						-- no command is being executed, waiting for one */
ricevuto_comando,		/* ho ricevuto un comando ma sono ancora nei preliminari 
						(aspetta_inserzione...) e posso ancora uscirne premendo un
						bottone di annulla 
						-- received a command, but I am in the preliminary phase
						of its execution (disk insertion dialog box...) hence the
						command may still be cancelled by clicking on a button */
paused,					/* tra Pause e Resume... */
in_writing,				/* sono nel menù di writing, pronto a ricevere comandi 
						-- ready to accept commands from the Write menu */
reading_sect_n,			/* sto leggendo un numero di settore dalla console
						-- in expert mode, reading from the console */
initializing,			/* sto eseguendo il comando di inizializzare un disco: a
						differenza dagli altri casi, la fase diventa questa prima 
						di chiamare aspetta_inserzione
						-- initializing a disk: in this case, the phase includes 
						the preliminaries which otherwise are ricevuto_comando
						*/
writing_disk,			/* sto realmente scrivendo su floppy disk, ma potrei
						anche essere in pausa per attesa disco successivo o per il
						comando "Pause": se sono chiamato da MainEvent posso usare la
						variabile sto_lavorando per sapere se non sono né in pausa
						né in semimodal e finestra_sm_aperta per sapere se sono
						in semimodal: assolutamente non assegnare quelle variabili!
						-- writing to a floppy disk, or maybe paused or waiting for 
						another disk: the two variables sto_lavorando (I'm working) and 
						finestra_sm_aperta (semimodal window is open) may be consulted to
						distinguish the three cases
						*/
reading_disk,			/* sto realmente leggendo da floppy disk, ma come sopra 
						-- as above... */
hack_listing,			/* sto eseguendo un expert list
						-- executing the expert list command
						*/
hack_reading,			/* sto eseguendo un untar at sector o unbar at sector
						-- executing untar at sector or unbar at sector
						*/
selected_reading,		/* sto leggendo col comando Extract selected files
						-- reading by "Extract selected files" */
drag_and_drop			/* suntar is not Apple Event aware, so the Finder sends
						drag&drop files as "Open" menu items. In this only case
						suntar accepts a second Open before the previous command
						was serviced */
};
extern enum tipo_fase fase;

enum os_type {unknown_os,mac_MFS,mac_HFS,msdos};
extern struct disk_info{
	sector_t sectors_on_floppy;
	short drive_number;	/* 0 when nothing is open or a file is open */
	short m_vrefnum;	/* used only when a driver or a file is open */
	byte signature_avail;	/* meaningful only when drive_number != 0:
							0: not available
							1: available
							2: can't verify a signature on this kind of device
							(the signature is currently a copy of sector 0, it's
							used to verify that the user has not inadvertently
							ejected the disk and inserted another one in the same
							drive)
							*/
	byte file_format;	/* !=0 solo se tipo_device==d_file, può valere ff_tarbar,
						ff_macbin etc.
						-- nonzero only if tipo_device==d_file
						*/
	Boolean isEjectable;
	Boolean writing_allowed;
	byte mounting_state;	/* used only for d_driver opened by "open device"
							and for d_scsi with DeviceType!=TAPE_UNIT */
#define unmountable 0 /* because there is no device driver for it */
#define not_mounted 1
#define mounted 2
#define mounted_as_startup 3

	enum{d_notopen,d_driver,d_file,d_scsi} tipo_device;
	enum os_type os;
	sector_t archive_start_sector;	/* usually 0 */
	union {
		/* the following fields are useful only for device drivers */
		struct{
			short disk_code;	/* as the return value of PBMountVol, but often it's
							computed by suntar itself (almost unused) */
			char opened_by_insertion;		/* 0=false, 1=true, 2=no, but
								later there was a disk insertion in that drive */
			Boolean is_not_initialized;		/* really, it's meaningful only
					for the .Sony driver, since there is not a standard way to
					know that and the error codes which typically mean "not
					initialized" are very low-level codes related to the GCR
					recording method */
		/* the following fields are useful only if the disk must be initialized */
			sector_t max_format;
			short format_index,
				supports_720K;
			} fi;

		/* the following fields are useful only for files */
		sector_t file_current_s;	/* current read/write position */

		/* the following fields are useful only for SCSI devices */
		struct{
			long supported_group0,supported_group1;
			long block_size;
			unsigned short n_sectors_per_block;	/* =block_size/512 */
			byte DeviceType;	/* copy of the response to INQUIRY */
#define TAPE_UNIT 1
#define PRINTER_UNIT 2

			/* the following fields are useful only for SCSI direct-access devices */
				/* nothing */
			/* the following fields are useful only for SCSI tape devices */
			char block_mode;
#define variable_blocks 0
#define fixed_blocks 1
#define software_fixed_blocks 2
#define firmware_fixed_blocks 3
			sector_t current_tape_block;	/* number of bytes from the start of the
							tape to here, divided by 512 */
			long tape_block_number;			/* number of blocks, if the block size
						is fixed it may be computed from the previous field,
						but often the block size is not fixed */
			}si;
		}v;
	} di;

enum formats{unreadable_disk,tar_format,bar_format,unknown_format};

typedef struct {
	OSType tx_cr;
	#define text_creator options.tx_cr
	short d_back;	/* delay when going to background during a disk write */
	/*#define delay_back options.d_back no more used */
	unsigned short m_to_beep;
	#define min_to_beep options.m_to_beep
#ifdef V_122
	unsigned char opt_bytes[12];
#else
	unsigned char opt_bytes[18];
	short fuso_orario;
#endif
	#define tar_version options.opt_bytes[0]			/* popup */

#define tar_unknown 0
#define tar_singlevol 1
#define tar_GNU 2
#define tar_POSIX 3

	#define accetta_GCR options.opt_bytes[1]
	#define resolve_aliases options.opt_bytes[2]
	#define crea_mac_dir options.opt_bytes[3]
	#define non_text_ASCII options.opt_bytes[4]		/* was radio buttons, now is popup */
	#define save_modi_date options.opt_bytes[5]
	#define useSys7_SF options.opt_bytes[6] /* no more used, System 7 has been out for a lot */
	#define del_incompl options.opt_bytes[7]
	#define trunc_14 options.opt_bytes[8]
	#define pres_underscore !options.opt_bytes[9]
	#define suppress_shell_chars !options.opt_bytes[10]
	#define verify_writes options.opt_bytes[11]
	#define smallFilesAreASCII options.opt_bytes[12]
	#define use_long_paths options.opt_bytes[13]
	#define pres_all_chars options.opt_bytes[14]
	#define separate_prefs opt_bytes[15]	/* !!! anche last_check_option !!! */

#define first_check_option 1
#define last_check_option  15
#define skip_check_option1   4	/* it's NOT a checkbox */
#define skip_check_option2   6	/* it's not a checkbox */

	#ifdef V_13		/* new field added in the 1.3; I preferred not to alter
					the layout of the previous fields in case somebody moves
					his old OPTs resource to the new version
					*/
	OSType gif_cr;
	#define gif_creator options.gif_cr
	#endif
	} options_type;
extern options_type options;

#ifdef THINK_C_5
#define OsErr OSErr
#endif

/* prototypes for untar.c */

char *my_p2cstr(unsigned char*);
unsigned char*my_c2pstr(char*);
void mcopy(char *,char *,unsigned short);
void init_hd_buffering(void);
OSErr write_hd(char*,long);
OSErr flush_hd_buffer(void);
void connect_to_hd_buffering(short);
short readblock(char*,short);
void unget_block(void);
void unget_char(void);
void end_of_file(void);
void skip_file(long);
Boolean is_uu_name(void);
void uudecode(long);
void stampa_buffer(sector_t,unsigned char *,Boolean);
short isASCII(char*,long,short*,short*);
Boolean print_if_string(const char*,char*,short);
void my_untar(Boolean);
void my_unbar(Boolean);
short unbar_checksum(unsigned char *,short,long *);
unsigned char guess_bar_linkflag(void);
void reinit_full_name(void);
void unbar(void);
void untar_file (long);
short chkmacbin (long ,short);
void bar_check_floppy_swap(short);
void tar_check_floppy_swap(short);
short bar_check_settore0(sector_t,sector_t*,Boolean);
short tar_check_settore0(sector_t,Boolean);
void setmacbin (void);
void copia_ultimo_header(char*,sector_t);
void close_or_del_out_file(void);
void my_itoa(long,char *);
Handle string_to_handle(char*);
void clear_unused_fields(void);
void print_type_creator(OSType,OSType);
void print_one_date(long,char*);
short controlla_spazio(long,long);
short crea_e_controlla(short);
void get_openfile_location(short);
void write_hqx_fork(long);
void write_pit_fork(long);
short untar_checksum(unsigned char*,short,Boolean);
void copy_mac_file(unsigned char*);
void untar (void);
short create_file (unsigned char*,short);
void unix_to_mac (void);
long untar_number (char*,short);
void pbsyserr (IOParam *);

/* prototypes for suntar.c */
void chiudi_archivio(void);
void copia_da_disco_a_file(void);
void estrai_da_floppy(void);
void scrivi_su_floppy(void);
void disk_initialize(void);
enum formats identify_format(void);
short apri_file(char *,short *);
OSErr new_text_file(short*);
OSErr create_overwrite(OSType,OSType,short *);
short mac_fread(char*,short,short);
/* short mac_fwrite(char*,short,short); no more used */
OSErr open_overwrite (char *,short,short *);
Boolean my_disk_is_in(void);
void handle_pause(void);
void check_events(Boolean);
short check_error_and_events(void);
void accept_abort_command(void);
Boolean is_abort_command(void);
Boolean is_pause_command(void);
short aspetta_inserzione(const char*,short);
/* masks for the second parameter: */
#define examine_disk 1
#define button_is_stop 2

void esamina_disco(void);
void list_conciso(void);
void check_foreground(void);
void beep_in_foreground(void);
void about_box(void);
Boolean check_all_zero(char *);
void print_info(char *,long);
void raise_error(void);
void error_message(char *);
void error_message_1(char *,int);
short check_error(void);
void disk_error_message(void);
void write_error_message(void);
void hacking(short);
short read_sect_n(sector_t *);
short my_atoi(char*,short*);
short my_atol(char*,long*);
short untar_dec_number(char*,short);
void init_deDOSize(void);
long macize_ASCII(char*,long,Boolean);
void print_sector_n(sector_t);
void stampa_info_bar(short);
void DriveList(void);
void fine_lavoro(void);
void set_skip_all(short);
void prefs_da_risorsa(void);
void load_options(void);
void save_options(Handle,...);
void flush_all(void);
void stampa_descrizione_stato(void);
Boolean check_startup(short);
void new_ddriver_menu(void);


/* prototypes for diskdriver.c */
void stampa_driver_info(void);
void stampa_volume_info(void);
void disk_format(Boolean);
short testa_stato(short*,Boolean);
void stampa_descrizione_stato(void);
short is_wrprot(void);
enum os_type riconosci_disco_mac(unsigned char*);
void identifica_hardware(void);
void init_buffering(void);
void reinit_buffering(long);
void check_allocated(void*);
void flush_buffers(short);
void invalid_buffers(void);
void invalid_after(sector_t);
sector_t get_first(short*);
OSErr find_driver(void);
short fast_macize(Str255);
Boolean not_my_disk(void);
void store_disk_signature(void);

Boolean dirty_buffers(void);
void leggi_settore(sector_t,char*);
void scrivi_settore(sector_t,char*);
void diskEject(Boolean);
short read_sectors(sector_t,char*,short);
void write_sectors(sector_t,char*,short);
void read_one_sector(sector_t,char *,short);
OSErr ID_to_drive_number(short,short*);

/* prototypes for mtools.c */
short visita_FAT_msdos(unsigned short*);
short msdos_logical_format(sector_t);
short match(char *,char *);


/* prototypes for tar.c */
void fillmem(char*,char,short);
void reset_sector_count(void);
short warning_400_800(void);
void printf_protetto(void);
short warning_first_write(short,Boolean*);
void de_Mac_ize(char*);
void bar_checksum(char *);
void writeblock(char*,short);
unsigned char get_linkflag(long *,short);
void svuota_buffer_macbin(void);
void cerca_fine(void);
void svuota_buffer(void);
void azzera_settore(void);
void tronca_archivio(void);
Boolean compare_mem(char*,char*,short);
void print_ready(void);
void check_and_eject(void);
void check_wr_err(void);
void my_tar(short);
void tar_file_or_folder (unsigned char *);
void close_input_files(void);
void FillMacBin2Fields(void);
void assegna_tar_version(void);
void controlla_che_ci_stia(sector_t);
void macbinheader(void);
void numstr (char *,long,short);
void statf (unsigned char *);

/* prototypes for dialogs.c */
void options_box(void);
void new_list_dialog(short,WindowPtr,Boolean);
void append_to_list(char*,short);
void dispose_list_dialog(void);
void select_and_extract(void);
void select_and_write(short);
void my_alert(void);
short my_modal_dialog(short,unsigned char **,short);
short my_semimodal_dialog(short,char **,short,Str255,Str255,Str255);
short my_edt_dialog(short,unsigned char*,short,const unsigned char*);
short dialogo_con_radio(short);
void scegli_device(void);
void SetCheckBox(DialogPtr,short);
void set_item_text(DialogPtr,short,const unsigned char *);
void get_item_text(DialogPtr,short,unsigned char *);

/* prototypes for dehqx.c */
void close_info_file(void);
short ci_strcmp(char*,char*);
short is_hqx_name(void);
void check_CRC(void);
short hqx_header(void);
void FindStart (void);
void init_hqx_tab(void);
void read_hqx(char *,long);
short read_3_hqx(unsigned char *,short);
unsigned char get_hqx_byte(Boolean *);
void open_info_file(void);
void flush_hqx_header(short);
void hqx_end_of_file(void);
void CalcCRC(char);

/* prototypes for unpit.c */
void my_unpit(void);
void get_pit_bytes(char*,long);
void deall_tree(void);

/* prototypes for folderSTDfile.c */
void my_SF_Put(unsigned char *,unsigned char *);
void my_SF_Get(short,SFTypeList);
short get_file_or_folder(Str255,short,SFTypeList*);
void select_directory(void);
/*Boolean get_file_with_checkbox(Boolean *);*/
Boolean SFGet_with_popup(void);
Boolean SFPut_with_popup(unsigned char *);
Boolean SFPut_con_pausa(unsigned char *,char*);
void close_all_open_WD(void);
void register_WD(short);

/* prototypes for StdFile.c & selectbutton.c */
void pStrcat(unsigned char *,unsigned char *);
void pStrcpy(unsigned char *,unsigned char *);
OSErr PathNameFromDirID(long,short,char*);
void doForceDirectory(WDPBRec*);
void SetRadioButton(DialogPtr,short,short);
void SelectButton(ControlHandle);
void OutlineControl(ControlHandle);

/* prototypes for semimodal.c */
short semimodalDialog(short,Point *,short (*)(EventRecord*),char **,short,
	Str255,Str255,Str255,short,Boolean,void(*)(WindowRecord*));
void close_semimodal(void);

/* prototypes for SCSI.c */
void scan_SCSI_bus(void);
OSErr test_SCSI_device(short*,short);
short read_scsi_random(short,sector_t,char*,short);
short read_scsi_tape(short,sector_t*,char*,short);
void write_scsi_random(short,sector_t,char*,short);
void write_scsi_tape(short,sector_t,char*,short);
void open_SCSI_device(short);
void close_SCSI_device(void);
void handle_device_menu(short);
void adjust_random_access_menu(void);
void use_fixed_sizes(void);
void start_appending(void);
int archive_rewind(void);

/* prototypes for loadpref.c */
void eredita_risorse(SFReply *);


