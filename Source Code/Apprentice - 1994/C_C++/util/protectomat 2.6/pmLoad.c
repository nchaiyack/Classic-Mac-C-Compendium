/*
 * appleshare protection restore program by
 * Aaron Wohl (aw0g+@andrew.cmu.edu)
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

#define VER_MAJ (2)
#define VER_MIN (6)
#define NIL 0L
extern void exit(int);
FILE *infile;
#define BOGUS_ID (-1)
#define BOGUS_PROTECTION (-1)

#define LOG_NAME "protectoload.log"

#define EEXTERN
#include "pmCommon.h"

long num_warnings=0;
FILE *outfile;

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
 * map a group or username to a number
 */
long map_name_to_num(cache_entry_pt acache,char *match_name,int avol,int kind,char *pure_path);
long map_name_to_num(acache,match_name,avol,kind,pure_path)
register cache_entry_pt acache;
int avol;
char *match_name;
int kind;
char *pure_path;
{
	HParamBlockRec cinfo;
	char xxname[300];
	int os;
	int slen;
	int csize=0;
	while(acache->text[0]!=0) {
		if(strcmp(match_name,&acache->text[1])==0)
			return acache->num;
		acache++;
		csize++;
	}
	if(csize>CACHE_SIZE) {
		printf("fatal - cash size exeeded, recompile protectomat with larger CACHE_SIZE",0);
		fprintf(outfile,"fatal - cash size exeeded, recompile protectomat with larger CACHE_SIZE",0);
		exit(1);
	}

	/*
	 * not found so add it
	 */
	slen=strlen(match_name);
	memcpy(&acache->text[1],match_name,slen);
	acache->text[0]=slen;
	acache->text[slen+1]=0;
	memset(&cinfo,0,sizeof(cinfo));
	cinfo.objParam.ioObjType=kind;
	cinfo.objParam.ioObjNamePtr=(ProcPtr)acache->text;
	cinfo.objParam.ioVRefNum=avol;
	memset(xxname,0,sizeof(xxname));
	cinfo.objParam.ioNamePtr=(StringPtr)xxname;
	os=PBHMapName(&cinfo,FALSE);
	if(os!=0) {
		printf("warning:get PBHMapName os err=%d on %s\n",os,pure_path);
		fprintf(outfile,"warning:get PBHMapName os err=%d on %s\n",os,pure_path);
		num_warnings++;
		return BOGUS_ID;
	}
	if((acache->num=cinfo.objParam.ioObjID)==0) {
		printf("warning:name '%s' on path '%s' does not exist\n",match_name,pure_path);
		fprintf(outfile,"warning:name '%s' on path '%s' does not exist\n",match_name,pure_path);
		num_warnings++;
		return BOGUS_ID;
	}
	return acache->num;
}

/*
 * restore one directories protection
 */
void set_1_dir(char *fname,long avnum,long dir_id,long user_num,long group_num,long protection_num,char *pure_path);
void set_1_dir(fname,avnum,dir_id,user_num,group_num,protection_num,pure_path)
char *fname;
long avnum;
long dir_id;
long user_num;
long group_num;
long protection_num;
char *pure_path;
{
	int os;
	HParamBlockRec cinfo;
	int flen;
	char aname[300];
	char xxname[300];
	memset(aname,0,sizeof(aname));

	flen=strlen(fname);
	if(flen>250) {
		printf("warning:user or group name too long '%s'\n",pure_path);
		fprintf(outfile,"warning:user or group name too long '%s'\n",pure_path);
		num_warnings++;
		return;
	}
	xxname[1]=':';
	strcpy(&xxname[2],fname);
	xxname[0]=flen+1;

	memset(&cinfo,0,sizeof(cinfo));

	cinfo.accessParam.ioVRefNum=avnum;
	cinfo.fileParam.ioDirID=dir_id;
	cinfo.accessParam.ioNamePtr=(StringPtr)xxname;
	cinfo.accessParam.ioACOwnerID=user_num;
	cinfo.accessParam.ioACGroupID=group_num;
	cinfo.accessParam.ioACAccess=protection_num;

	printf("user:%ld group:%ld prot:%lx - %s\n",user_num,group_num,protection_num,fname);
	os=PBHSetDirAccess(&cinfo,FALSE);
	if(os!=0) {
		printf("warning:set access err=%d %s\n",os,pure_path);
		fprintf(outfile,"warning:set access err=%d %s\n",os,pure_path);
		num_warnings++;
		return;
	}
}

/*
 * parse a protection
 */
long parse_protection(char *protection);
long parse_protection(char *protection)
{
	long result=0;
	char ch;
	char *scn=protection;
	while((ch = (*scn++))!=0) {
	 long one_byte=0;
	 long shift;
	  switch(ch) {
	  case 'o':
	    shift=0;
	    break;
	  case 'g':
	    shift=8;
	    break;
	  case 'e':
	    shift=16;
	    break;
	  default:
	  	goto barfo_protection;
	  }
	if((*scn++)!='(')
	  	goto barfo_protection;
	while((ch = *scn++)!=')') {
	  switch(ch) {
	    case 0:
	  	  goto barfo_protection;
	    case 'r':
		  one_byte|=2;
	      break;
	    case 'w':
		  one_byte|=4;
	      break;
	    case 's':
		  one_byte|=1;
	      break;
	    default:
	  	  goto barfo_protection;
	    }
      }
	 result|=(one_byte<<shift);
	}

	return result;

barfo_protection:
	printf("warning:bogus protection %s\n",protection);
	fprintf(outfile,"warning:bogus protection %s\n",protection);
	num_warnings++;
	return BOGUS_PROTECTION;
}

/*
 * convert a vnum to text
 */
void parse_1_file(long avnum,long dir_id,char *user_name,char *group_name,char *protection,char *mac_path,char *pure_path);
void parse_1_file(avnum,dir_id,user_name,group_name,protection,mac_path,pure_path)
long avnum;
long dir_id;
char *user_name;
char *group_name;
char *protection;
char *mac_path;
char *pure_path;
{
	long user_num=map_name_to_num(user_cache,user_name,avnum,3,pure_path);
	long group_num=map_name_to_num(group_cache,group_name,avnum,4,pure_path);
	long protection_num=parse_protection(protection);
	if(protection_num==BOGUS_PROTECTION)
		return;
	if((user_num==BOGUS_ID)||
	   (group_num==BOGUS_ID))
	   	return;
	set_1_dir(mac_path,avnum,dir_id,user_num,group_num,protection_num,pure_path);
}

/*
 * convert a vnum to text
 */
void read_input_file(void);
void read_input_file()
{
	char apath[ERR_BUF_SIZE];
	char pure_copy[ERR_BUF_SIZE];
	char prev_vol[300];
	long cur_vol_num=BAD_VOL_NUM;
	char *s;
	char *group_name;
	char *user_name;
	char *protection;
	char *mac_path;
	char *mac_vol;
	char ch;
	prev_vol[0]=0;
	while(!read_line(apath,infile)) {
		strcpy(pure_copy,apath);
		s=apath;
		ch= *s++;
		if(ch==0)
			continue;
		if(ch==';')
			continue;
		if(ch != '=') {
			printf("warning:bogus = '%s'\n",pure_copy);
			fprintf(outfile,"warning:bogus = '%s'\n",pure_copy);
			num_warnings++;
			continue;
		}
		user_name=s;
		group_name=strchr(user_name,',');
		if(group_name==NIL) {
			printf("warning:bogus group name '%s'\n",pure_copy);
			fprintf(outfile,"warning:bogus group name '%s'\n",pure_copy);
			num_warnings++;
			continue;
		}
		*group_name++ =0;

		protection=strchr(group_name,',');
		if(protection==NIL) {
			printf("warning:bogus protection '%s'\n",pure_copy);
			fprintf(outfile,"warning:bogus protection '%s'\n",pure_copy);
			num_warnings++;
			continue;
		}
		*protection++ =0;

		mac_vol=strchr(protection,',');
		if(mac_vol==NIL) {
			printf("warning:bogus volume name '%s'\n",pure_copy);
			fprintf(outfile,"warning:bogus volume name '%s'\n",pure_copy);
			num_warnings++;
			continue;
		}
		*mac_vol++ =0;

		mac_path=strchr(mac_vol,':');
		if(mac_path==NIL) {
			printf("warning:bogus file name '%s'\n",pure_copy);
			fprintf(outfile,"warning:bogus file name '%s'\n",pure_copy);
			num_warnings++;
			continue;
		}
		*mac_path++ =0;

		if(strcmp(mac_vol,prev_vol)!=0) {
			strcpy(prev_vol,mac_vol);
			cur_vol_num=get_vol_id(mac_vol);
		}

		if(cur_vol_num==BAD_VOL_NUM)
			continue;
		parse_1_file(cur_vol_num,2L /*root*/ ,user_name,group_name,protection,mac_path,pure_copy);
	}
}

void open_files(void);
void open_files()
{
	char apath[ERR_BUF_SIZE];
	while(TRUE){
	  printf ("Enter file of protection to restore > ");
	  if(read_line(apath,stdin))
	  	exit(1);
	  printf("opening '%s' for input\n",apath);
	  infile=fopen(apath,"r");
      if(infile!=0)
      	break;
      printf("can't open '%s' because errno=%d, reenter file name\n",apath,errno);
	}
	outfile=fopen(LOG_NAME,"w");
	if(outfile==0) {
		printf("fatal:can't open log file %s\n",LOG_NAME);
		exit(1);
	}
	{
		unsigned long now=time(0L);
		fprintf(outfile,"Protectomat load of %s starting at %s",apath,ctime(&now));
		printf("Protectomat load of %s starting at %s",apath,ctime(&now));
	}
}


int main(argc,argv)
int argc;
char **argv;
{
	printf(";protectomat - load appleshare protections v%d.%d built on %s %s\n",
		VER_MAJ,VER_MIN,__DATE__,__TIME__);
	allocate_cache_entries();
	open_files();
	read_input_file();
	fclose(infile);
	{
		unsigned long now=time(0L);
		fprintf(outfile,"Protectomat load finished with %ld warnings at %s",num_warnings,ctime(&now));
	    printf("Protectomat load finished with %ld warnings at %s",num_warnings,ctime(&now));
		fclose(outfile);
	}
	return 0;
}
