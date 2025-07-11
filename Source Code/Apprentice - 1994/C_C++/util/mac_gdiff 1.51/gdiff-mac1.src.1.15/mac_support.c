/*
 * fake routines for the macintosh gnu support
 * Written by Aaron Wohl (aw0g+@andrew.cmu.edu)
 * Public domain
 */

#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <console.h>
#include <unix.h>
#include <fcntl.h>
#include <sys/dir.h>
#include <errno.h>
#include <string.h>

/*
 * calls that gnu diff refrences but which we don't implement
 */
#define BOGUS(xx_rtn) void xx_rtn() {fprintf(stderr,"unimplemented call to %s",#xx_rtn);exit(1);}

BOGUS(dup2)
BOGUS(pipe)
BOGUS(vfork)
BOGUS(wait)
BOGUS(fstat)

static int silent_quit=FALSE;	/*if true exit if no error*/

/*
 * at startup remember default volume
 */
struct {
	short vnum;
	unsigned char vname[256];
}ms;

/*
 * cache current volume
 */
void remember_volume()
{
	if(GetVol(ms.vname,&ms.vnum)!=0) {
		fprintf(stderr,"can't get current volume");
		exit(1);
	}
}

/*
 * cut off a trailing end of line
 */
static void chop(s)
char *s;
{
	s=strchr(s,'\n');
	if(s!=0)
		*s=0;
}

/*
 * read the next character from input string
 * update input, stop end end of input and
 * keep returning 0
 */
static char parse_getch(char **s)
{
	char ch;
	if(s==0)return 0;
	if(*s==0)return 0;
	if(**s==0)return 0;
	ch=*((*s)++);
	if(ch=='\t')
		ch=' ';
	return ch;
}

/*
 * break a command line up into tokens
 */
static int parse_command(char ***argv,char *in)
{
#define MAX_ARGS (30)
	char ch;
	int result=0;
	char *out=in;
	static char *real_argv[MAX_ARGS+1];
	*argv= real_argv;		/*set result string pointer*/
	while((ch=parse_getch(&in))!=0) {
		char quote_ch=' ';			/*fields break on spaces by default*/
		if(ch==' ')continue;	/*skip spaces between tokens*/
		real_argv[result++]=out; /*point to start of next field*/
		if(result>=MAX_ARGS) {
			fprintf(stderr,"too many input fields in command line");
			exit(1);
		}
		/*handle quoted strings*/
		if((ch=='"')||(ch=='\'')) {
			quote_ch=ch;
			ch=parse_getch(&in);
		}
		while((ch!=0)&&(ch!=quote_ch)) {
			*out++=ch;
			ch=parse_getch(&in);
		}
		*out++=0;
	}
	return result;
}

/*
 * get the first finder file if it exists
 */
static int read_1_finder_file(AppFile *afile)
{
	short doWhat;
	short fileCnt;
	int i;
	CountAppFiles(&doWhat,&fileCnt);
	if(fileCnt<1)return FALSE;		/*nothing to do*/
	GetAppFiles(1,afile);
	for(i=0;i<fileCnt;i++)
		ClrAppFiles(i);
	return TRUE;
}

/*
 * return volume name and directory number for a working directory
 */
static void get_voldir(short vnum,unsigned char *vname,long *dir_id)
{
	WDPBRec paramBlock;
	int iErr;
	memset(&paramBlock,0,sizeof(paramBlock));
	paramBlock.ioWDIndex=0;
	paramBlock.ioVRefNum=vnum;
	paramBlock.ioNamePtr=vname;
	paramBlock.ioWDVRefNum=vnum;
	iErr=PBGetWDInfoSync(&paramBlock);
	if(iErr!=0) {
		fprintf(stderr,"error reading finder working directory info %d\n",iErr);
		exit(1);
	}
	*dir_id=paramBlock.ioWDDirID;
}

#define ROOT_DIR (2L)
#define MAXPATHLEN (2000)

/*
 * prepend ins to outs
 */
char *str_prepend(char *outs,char *ins,int len)
{
	ins+=len-1;
	while(len-- >0)
		*--outs= *ins--;
	return outs;
}

/*
 * given a volume name return it's number
 */
static int vol_name_to_vnum(unsigned char *vname)
{
	int iErr;
	ParamBlockRec paramBlock;
	paramBlock.volumeParam.ioVolIndex= -1;	/*lookup byname*/
	paramBlock.volumeParam.ioNamePtr=vname;
	iErr = PBGetVInfoSync(&paramBlock);
	if(iErr!=0) {
		fprintf(stderr,"error reading finder file volume info %d\n",iErr);
		exit(1);
	}
	return paramBlock.volumeParam.ioVRefNum;
}

/*
 * convert mac finder info to a unix happy file name
 */
static void AppFile_to_fname(AppFile *afile,char *fname)
{
	unsigned char vol_name[32];
	long dir_id;
	char result[MAXPATHLEN];
	char *out=result+MAXPATHLEN-1;
	int vnum;
	*out=0;
	get_voldir(afile->vRefNum,(unsigned char *)vol_name,&dir_id);
printf("orig volname '%#s'",vol_name);
	if(vol_name[vol_name[0]]!=':') {
		vol_name[0]++;
		vol_name[vol_name[0]]=':';
printf("colonized volname '%#s'",vol_name);
	}
	vnum=vol_name_to_vnum((unsigned char*)vol_name);
	out=str_prepend(out,(char *)(afile->fName+1),afile->fName[0]);
	
	/*
	 * work up the directory tree till we get to the root directory
	 */
	while(dir_id!=ROOT_DIR) {
		CInfoPBRec pb;
		int iErr;
		unsigned char pfname[256];
		pfname[0]=0;
		memset(&pb,0,sizeof(pb));
		pb.hFileInfo.ioNamePtr=pfname;
		pb.hFileInfo.ioVRefNum=vnum;
		pb.hFileInfo.ioDirID=dir_id;
		pb.hFileInfo.ioFDirIndex= -1;	/*ask about the file itself*/
		iErr = PBGetCatInfoSync(&pb);
		if(iErr!=0) {
			fprintf(stderr,"can't open input file supplied by the finder\n");
			exit(1);
		}
		if((*out)!=0)
			*--out=':';
		out=str_prepend(out,(char*)(pfname+1),pfname[0]);
		dir_id=pb.hFileInfo.ioFlParID;
	}
	if((*out)!=':')
		*--out=':';
	out=str_prepend(out,(char *)(vol_name+1),vol_name[0]);
	strcpy(fname,out);
}

/*
 * read in the input file and return it as a string
 * or nil if it does not exist
 */
static char *slurp_input(void)
{
	AppFile fileStuff;
	char fname[MAXPATHLEN];
	struct stat st;
	char *result;
	FILE *infile;
	long in_buf_size;
	if(!read_1_finder_file(&fileStuff))
		return 0;
	AppFile_to_fname(&fileStuff,fname);
	printf("reading command line from '%s'\n",fname);
	if(stat(fname,&st)!=0) {
		fprintf(stderr,"can't get size of input file '%s'",fname);
		exit(1);
	}
	in_buf_size=st.st_size+1;
	if((result=malloc(in_buf_size))==0) {
		fprintf(stderr,"out of memory reading finder input file");
		exit(1);
	}
	if((infile=fopen(fname,"r"))==0)  {
		fprintf(stderr,"error opening finder input file errno=%d",errno);
		exit(1);
	}
	if(fread(result,1,st.st_size,infile)!=st.st_size) {
		fprintf(stderr,"error reading finder input file");
		exit(1);
	}
	if(fclose(infile)!=0) {
		fprintf(stderr,"error closing finder input file");
		exit(1);
	}
	*(result+st.st_size)=0;
	return result;
}

/*
 * pick off output file redirect if there is one
 */
void get_output_file(char *redirect)
{
	int echo_also=TRUE;
	if(*redirect=='+') {
		redirect++;
		echo_also=FALSE;
		printf("echo to console disabled\n");
	}
	if(*redirect=='!') {
		redirect++;
		silent_quit=TRUE;
		printf("silent quit enabled\n");
	}
	while(*redirect==' ')
		redirect++;
	fprintf(stdout,"output to '%s'\n",redirect);
	fflush(stdout);
	if(echo_also)
		cecho2file(redirect,0,stdout);
	else
		if(freopen(redirect,"w",stdout)==0) {
			fprintf(stderr,"error opening log file errno=%d",errno);
			exit(1);
		}
}

/*
 * see if we where opened with an input file from the finder
 */
int finder_command(char ***argv)
{
	char *input=slurp_input();
	char *redirect;
	int result;
	if(input==0)
		return -1;		/*no input file to read*/
	chop(input);
	fprintf(stdout,"command '%s'\n",input);
	fflush(stdout);
	redirect=strrchr(input,'>');
	if(redirect!=0)
		*redirect++=0;
	result=parse_command(argv,input);
	if(redirect!=0)
		get_output_file(redirect);
	return result;
}

int main(int argc,char **argv)
{	
	extern char *version_string;
	int rc;
	_fmode=O_TEXT;
	remember_volume();
	printf("gnu diff %s %s %s\n",version_string,__DATE__,__TIME__);
	printf("mac port by Aaron Wohl (aw0g+@andrew.cmu.edu)\n");
	if((argc=finder_command(&argv))<0)
		argc=ccommand(&argv);
	if(silent_quit)
		console_options.pause_atexit=FALSE;
	rc=gdiff_main(argc,argv);
	fprintf(stdout,"exit code=%d\n",rc);
	fflush(stdout);
	exit(rc);
}

/*
 * get info about a file
 */
int stat(char *fname,struct stat *st)
{
	unsigned char pfname[256];
	CInfoPBRec pb;
	int iErr;
	strcpy((char*)(pfname+1),fname);
	pfname[0]=strlen(fname);
	memset(&pb,0,sizeof(pb));
	pb.hFileInfo.ioNamePtr=pfname;
	pb.hFileInfo.ioVRefNum=ms.vnum;
	iErr = PBGetCatInfo(&pb,FALSE);
	if(iErr!=0) {
		errno=ENOENT;
		return errno;
	}
	st->st_size=pb.hFileInfo.ioFlLgLen;
	st->st_dev=pb.hFileInfo.ioVRefNum;
	st->st_ino=pb.hFileInfo.ioDirID;
	st->st_mtime=pb.hFileInfo.ioFlCrDat;
	if(pb.hFileInfo.ioFlAttrib&16)
		st->st_mode=S_IFDIR;
	else
		st->st_mode=S_IFREG;
	return 0;
}

/*
 * find the volume for this file
 */
static int pick_vnum(char *fname)
{
	unsigned char vname[256];
	char *s=strchr(fname,':');
	long vol_len;
	if((s==0)||(s==fname))	/*if subdir or no disk use default*/
		return ms.vnum;
	vol_len=(s-fname)+1;
	memcpy(vname+1,fname,vol_len);
	vname[0]=vol_len;
	return vol_name_to_vnum(vname);
}

/*
 * open a directory file
 */
DIR_pt opendir(char *fname)
{
	unsigned char pfname[256];
	CInfoPBRec pb;
	int iErr;
	DIR_pt result;
	strcpy((char *)(pfname+1),fname);
	pfname[0]=strlen(fname);
	memset(&pb,0,sizeof(pb));
	pb.hFileInfo.ioNamePtr=pfname;
	pb.hFileInfo.ioVRefNum=0;
	iErr = PBGetCatInfo(&pb,FALSE);
	if(iErr!=0) {
		errno=ENOENT;
		return 0L;
	}
	result=malloc(sizeof(*result));
	result->vnum=pick_vnum(fname);
	result->dirid=pb.hFileInfo.ioDirID;
	result->voffset=1;
	return result;
}

/*
 * open a directory file
 */
direct_pt readdir(DIR_pt adir)
{
	CInfoPBRec cinfo;
	int os;
	memset(&cinfo,0,sizeof(cinfo));
	adir->dr.d_name[0]=0;
	cinfo.hFileInfo.ioVRefNum=adir->vnum;
	cinfo.hFileInfo.ioNamePtr=(StringPtr)adir->dr.d_name;
	cinfo.hFileInfo.ioFDirIndex=adir->voffset++;
	cinfo.hFileInfo.ioDirID=adir->dirid;
	os=PBGetCatInfo(&cinfo,FALSE);
	if(os!=0) {
		errno=ENOENT;
		return 0L;
	}
	PtoCstr((unsigned char *)adir->dr.d_name);
	return &adir->dr;
}


/*
 * close a directory file
 */
int closedir(DIR_pt adir)
{
	if(adir==0)return -1;
	free(adir);
	return 0;
}

