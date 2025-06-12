/*
 * dump protections from an applshare file system to a data file
 * By Aaron Wohl (aw0g+@andrew.cmu.edu)
 */

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <Packages.h>
#ifdef RUBBISH
not needed since it is in MacHeaders
#include <Files.h>
#endif
#define VER_MAJ (2)
#define VER_MIN (6)

#define ROOT_DIR_ID (2L)		/*directory id of the root directory of a volume*/
#define NIL 0L
extern void exit(int);

struct {
	FILE *outfile;
	FILE *historyfile;
	long num_desktop_files;	/*number of desktop files found*/
	long num_files;	/*number of directories seen on current volume*/
	long num_dirs;  /*number of directories seen on current volume*/
	long unbacked_files; /*number of files needing to be backed up*/
#define NO_OLDEST (0xffffffffL)
	unsigned long oldest_unbacked; /*write date of oldest unbacked up file*/
	double bytes_needing_saved; /*number of bytes that need to be saved*/
}gl;

#define EEXTERN
#include "pmCommon.h"


void die(char *txt,char *exit_text);
void fatal_error(char *txt,int oserr);

/*
 * convert a string to upper case
 */
void upcase(char *s);
void upcase(char *s)
{
	while((*s)!=0) {
		if(islower(*s))
			*s=toupper(*s);
		s++;
	}
}


/*
 * give a promt and wait for the given reply
 */
void readtoken(char *answer);
void readtoken(char *answer)
{
	char line_buf[ERR_BUF_SIZE];
	while(TRUE) {
		printf("\nEnter this in the log book then type in %s<return> to exit:",answer);
		if(read_line(line_buf,stdin))
			break;
		upcase(line_buf);
		if(strcmp(line_buf,answer)==0)
			break;
	}
}

/*
 * close a log file and make sure it was happy
 */
void finish_file(FILE **afile,char *aname);
void finish_file(FILE **afile,char *aname)
{
	FILE *f_to_close=(*afile);
	*afile=0;
	if(f_to_close==0)
		return;
	if(ferror(f_to_close)) {
		char errbuf[ERR_BUF_SIZE];
		sprintf(errbuf,"error writeing to output file '%s'",aname);
		fatal_error(errbuf,0);
	}
}

/*
 * fatal error handler
 */
void die(char *txt,char *exit_text)
{
	unsigned long now=time(0L);
	if(gl.outfile!=0) {
		fprintf(gl.outfile,"%s at %s",txt,ctime(&now));
		finish_file(&gl.outfile,"output dump file");
	}
	if(gl.historyfile!=0) {
		fprintf(gl.historyfile,"%s at %s",txt,ctime(&now));
		finish_file(&gl.historyfile,"history log file");
	}
	printf("%s at %s",txt,ctime(&now));
	readtoken(exit_text);
	exit(0);
}

/*
 * fatal error handler
 */
void fatal_error(char *txt,int oserr)
{
	char errbuf[ERR_BUF_SIZE];
	sprintf(errbuf,"*** Fatal error:%s, oserr=%d",txt,oserr);
	die(errbuf,"ERROR");
}

void file_error(char *fname,char *txt,int oserr);
void file_error(char *fname,char *txt,int oserr)
{
  char err_buf[ERR_BUF_SIZE];
  sprintf(err_buf,"\n?file '%s'\n?%s",fname,txt);
  fatal_error(err_buf,oserr);
}

/*
 * str_fname - convert a file name into a unix string
 */
char *str_fname(char *fin);
char *str_fname(fin)
char *fin;
{
	*(fin+1+((*fin)))=0;
	return fin+1;
}

/*
 * open the output log file
 */
void open_outfile(void);
void open_outfile()
{
	struct tm *t;
	unsigned long now=time(0L);
	char buf[100];
	t=localtime(&now);

	sprintf(buf,"protect.%02d%02d%02d.%02d%02d",
		t->tm_year,
		t->tm_mon+1,
		t->tm_mday,
		t->tm_hour,
		t->tm_min);
	printf(";protectomat dumping to '%s'\n",buf);
	gl.outfile=fopen(buf,"w");
	if(gl.outfile==0)
		fatal_error("can't open output file, (unix errno)",errno);

	sprintf(buf,"history.%02d%02",
		t->tm_year,
		t->tm_mon+1);
	printf(";protectomat history log to '%s'\n",buf);
	gl.historyfile=fopen(buf,"a");
	if(gl.historyfile==0)
		fatal_error("can't open history file, (unix errno)",errno);
	fprintf(gl.historyfile,"\nProtecomat starting at %s",ctime(&now));
}

/*
 * map a number to text
 */
char *map_num_to_text(char * fname,cache_entry_pt acache,long an_id,int avol,int kind);
char *map_num_to_text(fname,acache,an_id,avol,kind)
char *fname;
register cache_entry_pt acache;
long an_id;
int avol;
int kind;
{
	HParamBlockRec cinfo;
	char xxname[300];
	int os;
	int csize=0;
	while(acache->text[0]!=0) {
		if(acache->num==an_id)
			return &acache->text[1];
		acache++;
		csize++;
	}

	if(csize>CACHE_SIZE)
		file_error(fname,"cash size exeeded, recompile protectomat with larger CACHE_SIZE",0);

	/*
	 * not found so add it
	 */
	acache->num=an_id;
	memset(&cinfo,0,sizeof(cinfo));
	cinfo.objParam.ioObjType=kind;
	/*Type here should really be Ptr but File.h has it as ProcPtr*/
	cinfo.objParam.ioObjNamePtr=(ProcPtr)acache->text;
	cinfo.objParam.ioObjID=an_id;
	cinfo.objParam.ioVRefNum=avol;
	xxname[0]=0;
	cinfo.objParam.ioNamePtr=(StringPtr)xxname;
	os=PBHMapID(&cinfo,FALSE);
	if(os!=0) {
		char errbuf[ERR_BUF_SIZE];
		sprintf(errbuf,"error mapping id %ld kind=%d to text PBHMapID\n",an_id,kind);
		file_error(fname,errbuf,os);
	}
	return str_fname(acache->text);
}

/*
 * print out a protection in human readable form
 */
char *print_1_prot(char *dst,long prot,char pname);
char *print_1_prot(char *dst,long prot,char pname)
{
	prot&=0x7;		/*only interested in access bits*/
	if(prot==0)		/*if no access no output*/
		return dst;
	*dst++ = pname;
	*dst++ = '(';
	if((prot&4)!=0)
		*dst++ = 'w';
	if((prot&2)!=0)
		*dst++ = 'r';
	if((prot&1)!=0)
		*dst++ = 's';
	*dst++=')';
	return dst;
}

/*
 * print out protections in human readable form
 */
void print_prot_names(char *dst,long prot);
void print_prot_names(char *dst,long prot)
{
	dst=print_1_prot(dst,prot,'o');
	dst=print_1_prot(dst,prot>>8,'g');
	dst=print_1_prot(dst,prot>>16,'e');
	*dst=0;
}

/*
 * print the protection of one directory
 */
void get_1_dir(char *fname,int avnum,long dir_id,StringPtr aname);
void get_1_dir(fname,avnum,dir_id,aname)
char *fname;
int avnum;
long dir_id;
StringPtr aname;
{
	int os;
	HParamBlockRec cinfo;
	char prot_names[100];

	char xxname[300];
	memcpy(xxname,aname,aname[0]&0x0FFL);

	memset(&cinfo,0,sizeof(cinfo));

	cinfo.accessParam.ioVRefNum=avnum;
	cinfo.accessParam.ioNamePtr=aname;
	cinfo.fileParam.ioDirID=dir_id;
	os=PBHGetDirAccess(&cinfo,FALSE);
	if(os!=0)
		file_error(fname,"can't lookup access",os);
	cinfo.accessParam.ioACAccess&= 0x00FFFFFFL;
	print_prot_names(prot_names,cinfo.accessParam.ioACAccess);
	fprintf(gl.outfile,
		"=%s,%s,%s,%s\n",
		map_num_to_text(fname,user_cache,cinfo.accessParam.ioACOwnerID,avnum,1),
		map_num_to_text(fname,group_cache,cinfo.accessParam.ioACGroupID,avnum,2),
		prot_names,
		fname);
}

char *mac_ctime(long atime);
char *mac_ctime(long atime)
{
	static Str255 result;
	Str255 ttime;
	IUDateString(atime,shortDate,result);
	result[(result[0]+1)&0xff]=0;
	IUTimeString(atime,FALSE,ttime);
	ttime[(ttime[0]+1)&0xff]=0;
	strcat((char *)&result[1]," ");
	strcat((char *)&result[1],(char *)&ttime[1]);
	return (char *)&result[1];
}

void remember_file_info(CInfoPBPtr cinfo,char *fname);
void remember_file_info(CInfoPBPtr cinfo,char *fname)
{
	register unsigned long mt=cinfo->hFileInfo.ioFlMdDat;
	register unsigned long bt=cinfo->hFileInfo.ioFlBkDat;
	register unsigned long ct=cinfo->hFileInfo.ioFlMdDat;
	if(mt<ct)
		mt=ct;
	if(mt>bt) {
	  gl.unbacked_files++;
	  gl.bytes_needing_saved+=cinfo->hFileInfo.ioFlLgLen;
	  if(mt<gl.oldest_unbacked) {
		gl.oldest_unbacked=mt;
#ifdef DEBUGGING
		fprintf(stdout,"file %s\n",fname);
		fprintf(stdout,"mod time = %s\n",mac_ctime(mt));
		fprintf(stdout,"bak time = %s\n",mac_ctime(bt));
		fprintf(stdout,"dirty %s\n",fname);
#endif
		}
	}
}

void warn_desktop(char *fname);
void warn_desktop(char *fname)
{
	fprintf(gl.outfile,";Danger, desktop file found - %s\n",fname);
	fprintf(gl.historyfile,";Danger, desktop file found - %s\n",fname);
	fprintf(stdout,";Danger, desktop file found - %s\n",fname);
	gl.num_desktop_files++;
}

/*
 * convert a vnum to text
 */
void scan_dir(char *fname,char *fsuffix,int avnum,long dir_id,int offset);
void scan_dir(fname,fsuffix,avnum,dir_id,offset)
char *fname;
char *fsuffix;
int avnum;
long dir_id;
int offset;
{
	int os;
	CInfoPBRec cinfo;
	char aname[300];
	int findex=offset;
	*fsuffix++ = ':';
	while(TRUE) {
		*fsuffix=0;
		memset(&cinfo,0,sizeof(cinfo));
		aname[0]=0;
		cinfo.hFileInfo.ioVRefNum=avnum;
		cinfo.hFileInfo.ioNamePtr=(StringPtr)aname;
		cinfo.hFileInfo.ioFDirIndex=findex++;
		cinfo.hFileInfo.ioDirID=dir_id;
		os=PBGetCatInfo(&cinfo,FALSE);
		if(os!=0) {
			if(os!= -43)
				file_error(fname,"can't step to next file",os);
			break;
		}
		strcpy(fsuffix,str_fname(aname));
		if(strcmp(fsuffix,"Desktop")==0)
			warn_desktop(fname);
		if((cinfo.hFileInfo.ioFlAttrib&ioDirMask)==0) {
			gl.num_files++;
			remember_file_info(&cinfo,fname);
			continue;
		}
		gl.num_dirs++;
		get_1_dir(fname,avnum,dir_id,(StringPtr)aname);
		if(cinfo.hFileInfo.ioDirID!=2)
		 scan_dir(fname,fsuffix+strlen(fsuffix),avnum,cinfo.hFileInfo.ioDirID,1);
	}
}

void log_backup_info(FILE *of,char *vname);
void log_backup_info(FILE *of,char *vname)
{
	unsigned long now;
	long ksaved=((gl.bytes_needing_saved)/1024.0);
	now=time(0L);
	fprintf(of,";volume '%s' contained %ld directories, %ld files %s",
			vname,
			gl.num_dirs,
			gl.num_files,
			ctime(&now));
	if(gl.unbacked_files==0)
	  fprintf(of,";volume '%s' allfiles are currently backed up.\n",vname);
	else
	  fprintf(of,";volume '%s' had %ld files needing to be backed up.\n"
	  				   "; oldest dirty file was written on %s\n"
	  				   "; %ldK data bytes need to be saved\n",
			vname,
			gl.unbacked_files,
			mac_ctime(gl.oldest_unbacked),
			ksaved);
}

/*
 * select an input path
 */
void get_source_path(void);
void get_source_path()
{
	FILE *vfile;
	char vname[ERR_BUF_SIZE];
	int vname_len;
	long vol_num;
	open_outfile();
	if((vfile=fopen("volume_list","r"))==0)
		fatal_error("can't open volume_list (errno)",errno);
	while(!read_line(vname,vfile)) {
		unsigned long now;
		now=time(0L);
		gl.num_files=0;
		gl.num_dirs=0;
		gl.oldest_unbacked=NO_OLDEST;
		gl.unbacked_files=0;
		gl.bytes_needing_saved=0.0;
		fprintf(gl.outfile,";dumping volume '%s' at %s",vname,ctime(&now));
		fprintf(gl.historyfile,";dumping volume '%s' at %s",vname,ctime(&now));
		printf(";dumping volume '%s' at %s",vname,ctime(&now));
		vname_len=strlen(vname);
		if((vol_num=get_vol_id(vname))==BAD_VOL_NUM)
			fatal_error("can't map directory id to name",0);
		scan_dir(vname,vname+vname_len,vol_num,ROOT_DIR_ID,0);
		vname[vname_len]=0;
		log_backup_info(gl.outfile,vname);
		log_backup_info(gl.historyfile,vname);
		log_backup_info(stdout,vname);
		if(gl.num_dirs<2)
			fatal_error("volume didn't have any directories",0);
	}
	if(gl.num_desktop_files!=0)
		fatal_error("Desktop files exist on appleshare volumes",0);
	fclose(vfile);
}

int main(argc,argv)
int argc;
char **argv;
{
	printf(";pmDump - dump appleshare protections v%d.%d built on %s %s\n",
		VER_MAJ,VER_MIN,__DATE__,__TIME__);
	printf(";send bug reports to Aaron Wohl (aw0g+@andrew.cmu.edu)\n");
	allocate_cache_entries();
	printf(";reading list of volumes to dump from file volume_info\n");
	get_source_path();
	die("Protectomat finished normaly","OK");
}
