 /*
 * SoftKiss command parser
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#define sfk_CORE_EXTERN
#include "sfk_core.h"
#include "sfk_core_private.h"
#include <ctype.h>

/*
 * define type of argument various commands can take
 */
#define SFK_CF_NO_ARG	(0)
#define SFK_CF_BOOL 	(1)
#define SFK_CF_INT 		(2)
#define SFK_CF_TEXT 	(3)
#define SFK_CF_ARG_MASK (3)

#define SFK_CF_IMMEDIATE  (8)		/*can do this command while online*/
#define SFK_CF_RO		 (16)		/*readonly variables*/
#define SFK_CF_HEX		 (32)		/*print value in hex*/

#define sfk_BE_not_found 0
#define sfk_BE_ambigous  1
#define sfk_BE_found	 2

#define FAKE_EOL_CH	(';')

#define SFK_TEXT_RESID (1024)
#define SFK_COMMAND_RESID (1025)

/*
 * information about each command, shared amongst all ports
 */
struct sfk_name_R {
	char *nm_text;		/*text of the command*/
	char *nm_help;		/*help message for command*/
	char *nm_default; 	/*default setting*/
	int32 nm_flags;		/*what kind of command it is*/
	int16 nm_val_num;	/*index into value table*/
};
typedef struct sfk_name_R sfk_name,*sfk_name_pt;

/*
 * globals for parseing commands
 */
static struct {
	sfk_name sfk_cmd_table[sfk_max_cmd_num];
	/*allcate text variables in here*/
	char string_space[SFK_NUM_TEXT*sfk_NUM_PORTS*sfk_STRING_SIZE];
	char *string_allocator;
	int sfk_cmd_table_size;
}sfk_cgl;

/*
 * fatal error
 */
void sfk_assert_fail(char *msg)
{
	unsigned char buf[256];
	unsigned short len=strlen(msg);
	len&=0xFF;
	buf[0]=len;
	memcpy(buf+1,msg,len);
	while(TRUE)
		DebugStr(buf);
}

/*
 * binary search the command table
 */
static int sfk_binsearch_cmd(char *look_me_up,int *insert_point)
{
	int high=sfk_cgl.sfk_cmd_table_size-1;
	int cur=0;
	int cval= 1;
	int slen=strlen(look_me_up);

	while(cur<=high) {
		cval=strncmp(look_me_up,sfk_cgl.sfk_cmd_table[cur].nm_text,slen);
		if(cval<=0)
			break;
		cur++;
	}
	*insert_point=cur;
	if((cval==0)&&
		(cur<high)&&
		(strncmp(look_me_up,sfk_cgl.sfk_cmd_table[cur+1].nm_text,slen)==0))
			return sfk_BE_ambigous;
	if(cval==0)
		return sfk_BE_found;
	return sfk_BE_not_found;
}

/*
 * allocate string space
 */
static char *sfk_alloc_string(int size)
{
	char *result=sfk_cgl.string_allocator;
	sfk_cgl.string_allocator+=size;
	return result;
}
/*
 * add one command to the command table
 */
static void sfk_add_command(char *cname,int cnum,int cflags,char *help_text,char *xadd_default)
{
	int ins_point;
	int i;
	sfk_name_pt nm;
	sfk_binsearch_cmd(cname,&ins_point);
	sfk_cgl.sfk_cmd_table_size++;
	for(i=(sfk_cgl.sfk_cmd_table_size-2);i>=ins_point;i--)
		sfk_cgl.sfk_cmd_table[i+1]=sfk_cgl.sfk_cmd_table[i];
	nm= &sfk_cgl.sfk_cmd_table[ins_point];
	nm->nm_text=cname;
	nm->nm_flags=cflags;
	nm->nm_val_num=cnum;
	nm->nm_help=help_text;
	if(xadd_default==0)
		nm->nm_default="";
	else
		nm->nm_default=xadd_default;
	/*
	 * if this variable neads space, allocate string space for each port
	 */
	if(((cflags&SFK_CF_IMMEDIATE)==0)&&
	   ((cflags&SFK_CF_ARG_MASK)==SFK_CF_TEXT))
		for(i=0;i<sfk_NUM_PORTS;i++)
			sfk_PN(i)->var_vals[cnum].x.nm_sval=
				sfk_alloc_string(sfk_STRING_SIZE);
}

/*
 * append a c string onto the end of a sfk_string
 * don't overflow the output string
 */
void sfk_put_string(sfk_string_pt astr,char *add_on)
{
	int32 add_on_len=strlen(add_on)+1;
	int32 space_available=astr->sfk_in_cnt-astr->sfk_out_cnt;
	space_available=sfk_imax(0,sfk_imin(add_on_len,space_available));
	if(space_available<=0)
		return;
	memcpy(astr->sfk_str+astr->sfk_out_cnt,add_on,space_available);
	astr->sfk_out_cnt+=(space_available-1);
	*(astr->sfk_str+astr->sfk_out_cnt)=0;
}

/*
 * append a string onto command output
 */
void sfk_cmd_out(sfk_iio_pt cmd,char *add_on)
{
	sfk_put_string(&cmd->uio->do_this.cknd.sfk_msg.omsg,add_on);
}

/*
 * report a command failure
 */
void sfk_parse_fail(sfk_iio_pt cmd,int fail_code,char *extra)
{
#define xDEBUG_FAILURES
#ifdef DEBUG_FAILURES
  {
  	char buf[256];
  	strcat(buf+1,"cfail ");
  	strcat(buf+1,"|");
  	strcat(buf+1,SFK_TEXT(41));
  	strcat(buf+1,"|");
  	strcat(buf+1,SFK_TEXT(fail_code));
  	strcat(buf+1,"|");
  	strcat(buf+1,((extra==0)?"":extra));
  	buf[0]=strlen(buf+1);
	DebugStr((unsigned char *)buf);
  }
#endif
	sfk_cmd_out(cmd,SFK_TEXT(41));
	sfk_cmd_out(cmd,SFK_TEXT(fail_code));
	if(extra!=0)
		sfk_cmd_out(cmd,extra);
	sfk_control_fail(cmd,0,fail_code);
}

/*
 * parse a an ascii string as a hex number
 */
static long parse_hex(sfk_iio_pt cmd,char *s)
{
	long result=0;
	char *hex=sfk_map_text(65);
	char *next;
	char ch;
	while((ch= *s++)!=0) {
		if((next=strchr(hex,ch))==0)
			sfk_parse_fail(cmd,TEXT_NUM(1),s);
		result<<=4;
		result+=(next-hex);
	}
	return result;
}

/*
 * parse a integer set command
 */
static int32 sfk_parse_integer(sfk_iio_pt cmd,char *cmd_word)
{
	register int32 result=0;
	register char ch;
	register char *instr=cmd_word;
	while ((ch= *instr++)!=0) {
		if(!isdigit(ch))
			sfk_parse_fail(cmd,TEXT_NUM(1),cmd_word);
		result*=10;
		result+=ch-'0';
	}
	return result;
}

/*
 * parse a hex or decimal integer
 */
static int32 sfk_parse_hex_or_dec(sfk_iio_pt cmd,char *cmd_word)
{
	if((*cmd_word)!='$')
		return sfk_parse_integer(cmd,cmd_word);
	return parse_hex(cmd,cmd_word+1);
}

/*
 * setup debugging
 */
static void sfk_init_debugging(sfk_prt_pt p)
{
#ifdef DBO_ENABLED
	if(p->dbo.dbo_guard2==dbo_GUARD2)
		return;
	dbo_fopen(&p->dbo);
	p->dbo.just_kidding=(p->sfk_IVAR(debug_level)==0);
	p->dbo.win_br.h=8*40;
	switch(p->pnum) {
	  case sfk_PRINTER_PORT:
		p->dbo.win_tl.v=30;
		p->dbo.win_br.v=130;
		break;
	  case sfk_MODEM_PORT:
		p->dbo.win_tl.v=140;
		p->dbo.win_br.v=240;
		break;
	}
	dbo_fgoto(&p->dbo,0,0);
	dbo_fclear(&p->dbo);
#ifdef TEST_OUTPUT
	{
		int k;
		int j;
		for(k=0;k<10;k++)
			for(j=' ';j<0x7f;j++)
				dbo_fputc(&p->dbo,j);
	}
#endif
#endif
}

/*
 * reset port parameters to default values
 */
static void sfk_reset_port(sfk_iio_pt cmd,sfk_prt_pt p)
{
  int i;
  sfk_name_pt nm;
  p->sfk_IVAR(port)=p->pnum;
  for(i=0;i<sfk_max_cmd_num;i++) {
  	nm= &sfk_cgl.sfk_cmd_table[i];
  	p->var_vals[nm->nm_val_num].changed=TRUE;
	switch(nm->nm_flags&SFK_CF_ARG_MASK) {
	    case SFK_CF_NO_ARG:
	       break;
	    case SFK_CF_BOOL:
	    case SFK_CF_INT:
	 	  p->var_vals[nm->nm_val_num].x.nm_ival=sfk_parse_hex_or_dec(cmd,nm->nm_default);
	      break;
	    case SFK_CF_TEXT:
		  strcpy(p->var_vals[nm->nm_val_num].x.nm_sval,nm->nm_default);
	      break;
	    }
  }
  sfk_init_debugging(p);
}

/*
 * map a port number to a name
 * drivers can't have static data point to other static data
 * so this can't be a simple array access
 */
static char *name_port(int n)
{
	switch(n) {
	case sfk_PRINTER_PORT:
		return SFK_TEXT(42);
	case sfk_MODEM_PORT:
		return SFK_TEXT(43);
	default:
		return SFK_TEXT(45);
	}
}

/*
 * reset settings of all ports
 */
static void sfk_reset_ports(sfk_iio_pt cmd)
{
  int i;
  for(i=0;i<sfk_NUM_PORTS;i++) {
    sfk_prt_pt p=sfk_PN(i);
    p->pnum=i;
    p->port_name=name_port(i);
	sfk_reset_port(cmd,p);
  }
}

/*
 * define the command procedures
 */
#define SFK_CMD(xx_cmd_name,xx_cmd_arg_type,xx_cmd_help,xx_default) \
	void sfk_cmd_proc_##xx_cmd_name(void);
#include "sfk_core_command.h"
#undef SFK_CMD

/*
 * setup string space allocator
 */
static void sfk_init_string_allocator()
{
	sfk_cgl.string_allocator=sfk_cgl.string_space;
}

/*
 * allocate queue memory now in case we are under finder
 */
static void sfk_alloc_mem(sfk_iio_pt cmd)
{
  int i;
  for(i=0;i<sfk_NUM_PORTS;i++) {
    sfk_prt_pt p=sfk_PN(i);
	sfk_soft_tnc_install(cmd,p);
	sfk_allocate_queue_memory(cmd,p);
	sfk_allocate_soft_tnc_memory(cmd,p);
  }
}

/*
 * map a failure code to it's text
 */
char *sfk_map_text(int text_code)
{
	return (char *)
	  (sfk_gl.dt_text.tptr+(*(sfk_gl.dt_text.tstrs+text_code)));
}

/*
 * read in in map text
 */
static void init_map_text(sfk_iio_pt cmd)
{
	if(!sfk_dt_load(SFK_TEXT_RESID,&sfk_gl.dt_text,sfk_malloc,sfk_free))
		sfk_parse_fail(cmd,TEXT_NUM(64),0);
}

/*
 * split an input string
 */
static int split(char **dst,int limit,char *in)
{
	int result=0;
	while(result<limit) {
		*dst++= in;
		result++;
		if((in=strchr(in,'|'))==0)
			break;
		*in++=0;
	}
	return result;
}

/*
 * add the command table
 */
static void add_commands(sfk_iio_pt cmd)
{
	dyn_text dt;				/*dt text*/
	int i;
	if(!sfk_dt_load(SFK_COMMAND_RESID,&dt,sfk_malloc,sfk_free))
		sfk_parse_fail(cmd,TEXT_NUM(64),0);
	for(i=1;i<=dt.nstrs;i++){
		char *args[6];
		long nm_flags;
		if(split(args,6,(char *)(dt.tptr+(*(dt.tstrs+i))))!=4)
			sfk_parse_fail(cmd,TEXT_NUM(64),0);
		nm_flags=parse_hex(cmd,args[3]);
		sfk_add_command(args[0],i-1,nm_flags,args[1],args[2]);
	}
	sfk_dt_free(&dt);
}

/*
 * update version string
 */
static void upd_version(void)
{
	Handle vers=GetResource('vers',1);
	int len;
	char *vers_str;
	if(vers==0) {	//hmm, can't get the real version
		strcpy(sfk_gl.vers,__DATE__);
		return;
	}
	vers_str= (*vers)+6;
  	len= (*vers_str++)&0x0F;
  	memcpy(sfk_gl.vers,vers_str,len);
  	sfk_gl.vers[len]=0;
}

/*
 * add all the commands to the command table
 * Note: This is done as code as think c code resources/drivers
 * can't have static initializers that point to addresses ie
 */
void sfk_init_commands(sfk_iio_pt cmd)
{
	init_map_text(cmd);
	sfk_init_string_allocator();
#define xCOMMANDS_FROM_H_FILE_NOT_RESOURCE
#ifdef COMMANDS_FROM_H_FILE_NOT_RESOURCE
#define SFK_CMD(xx_cmd_name,xx_cmd_arg_type,xx_cmd_help,xx_default) \
	sfk_add_command( #xx_cmd_name,sfk_cval_##xx_cmd_name,xx_cmd_arg_type,xx_cmd_help,xx_default);
#include "sfk_core_command.h"
#undef SFK_CMD
#else
	add_commands(cmd);
#endif
	sfk_reset_ports(cmd);
	sfk_patch_in_global_data();
	upd_version();
	sfk_alloc_mem(cmd);
}

/*
 * get a new current character
 */
static void sfk_nextch(sfk_iio_pt cmd)
{
  if(cmd->pqx.eof)
  	sfk_parse_fail(cmd,TEXT_NUM(2),0);
  if(cmd->uio->do_this.cknd.sfk_msg.imsg.sfk_out_cnt>=cmd->uio->do_this.cknd.sfk_msg.imsg.sfk_in_cnt)
    cmd->pqx.eof=TRUE;
  if(cmd->pqx.eof) return;
  cmd->pqx.ch= *cmd->uio->do_this.cknd.sfk_msg.imsg.sfk_str++;
  cmd->uio->do_this.cknd.sfk_msg.imsg.sfk_out_cnt++;
}

/*
 * is the current character white space?
 */
static int sfk_is_white_space(sfk_iio_pt cmd)
{
	return ((cmd->pqx.ch==' ')||(cmd->pqx.ch=='\t'));
}

/*
 * is the current character end of line
 */
static int sfk_is_eol(sfk_iio_pt cmd)
{
	return ((cmd->pqx.ch=='\n') || (cmd->pqx.ch==FAKE_EOL_CH));
}

/*
 * skip white space
 */
static void sfk_skip_white_space(sfk_iio_pt cmd)
{
  while(!cmd->pqx.eof) {
    if(!sfk_is_white_space(cmd))
      break;
    sfk_nextch(cmd);
  }
}

/*
 * say if a character is part of a token
 */
static int sfk_token_ch(char ch)
{
	return ((!(ch==FAKE_EOL_CH))&&(ch>' ')&&(ch<0x7f));
}

/*
 * parse one word from the input stream
 */
static void sfk_parse_word(sfk_iio_pt cmd,char *aword)
{
	int word_len=0;
	char ch;
	if(cmd->pqx.eof)
		sfk_parse_fail(cmd,TEXT_NUM(3),0);
	do {
		if(!sfk_token_ch(cmd->pqx.ch))
			sfk_parse_fail(cmd,TEXT_NUM(4),0);
		if(word_len>=(sfk_MAX_TOKEN-1))
			sfk_parse_fail(cmd,TEXT_NUM(5),0);
		*aword++=cmd->pqx.ch;
 		sfk_nextch(cmd);
	} while(sfk_token_ch(cmd->pqx.ch)&&(!cmd->pqx.eof));
	*aword=0;
	if(cmd->pqx.eof||sfk_is_white_space(cmd)||sfk_is_eol(cmd))
		return;
	sfk_parse_fail(cmd,TEXT_NUM(6),0);
}

/*
 * parse end of input line
 */
static void sfk_parse_eol(sfk_iio_pt cmd)
{
  sfk_skip_white_space(cmd);
  if(cmd->pqx.eof)
	return;
  if(!sfk_is_eol(cmd))
 	 sfk_parse_fail(cmd,TEXT_NUM(7),0);
  sfk_nextch(cmd);	/*consume the end of line character*/
}

/*
 * case insensitive compare of a string vs a keyword
 * if no characters in string then do match
 */
static int strmatch(const char *astr,const char *key_word)
{
	register char  ach;
	while((ach=(*astr++))!=0) {
		if(isupper(ach))
			ach=tolower(ach);
		if (ach == (*key_word++))
			continue;
		return FALSE;
	}
	return true;
}

/*
 * case insensitive compare of a string vs a keyword
 * if no characters in string then don't match
 */
static int strmatch_some(const char *astr,const char *key_word)
{
	if((*astr)==0)
		return FALSE;
	return strmatch(astr,key_word);
}
/*
 * parse a boolean set command
 */
static int sfk_parse_bool(sfk_iio_pt cmd,char *cmd_word)
{
	int wlen=strlen(cmd_word);
	if(strmatch_some(cmd_word,SFK_TEXT(46)))
		return TRUE;
	if(strmatch_some(cmd_word,SFK_TEXT(47)))
		return FALSE;
	sfk_parse_fail(cmd,TEXT_NUM(8),cmd_word);
	SFK_NEVER_RETURN(0);
}

/*
 * return command table entry to dump to resource fork
 * used in test program
 */
int sfk_cmd_dump(int idx,char **nm_text,char **nm_help,
	char **nm_default,long *nm_flags,short *nm_val_num)
{
	if(idx>=sfk_cgl.sfk_cmd_table_size)
		return FALSE;
	*nm_text=sfk_cgl.sfk_cmd_table[idx].nm_text;
	*nm_help=sfk_cgl.sfk_cmd_table[idx].nm_help;
	*nm_default=sfk_cgl.sfk_cmd_table[idx].nm_default;
	*nm_flags=sfk_cgl.sfk_cmd_table[idx].nm_flags;
	*nm_val_num=sfk_cgl.sfk_cmd_table[idx].nm_val_num;
	return TRUE;
}

/*
 * give help command
 */
static void sfk_help(sfk_iio_pt cmd,char *on_what)
{
	int high=sfk_cgl.sfk_cmd_table_size-1;
	int cur=0;
	SFK_UNUSED_ARG(on_what);
	while(cur<=high) {
		sfk_cmd_out(cmd,sfk_cgl.sfk_cmd_table[cur].nm_text);
		sfk_cmd_out(cmd,SFK_TEXT(48));
		sfk_cmd_out(cmd,sfk_cgl.sfk_cmd_table[cur].nm_help);
		sfk_cmd_out(cmd,SFK_TEXT(58));
		cur++;
	}
}

/*
 * convert an unsigned number to text
 */
static char *textify_unisgned_number(char *out,long anum)
{
	long remainder=anum%10;
	anum/=10;
	if(anum!=0)
		out=textify_unisgned_number(out,anum);
	*out++ ='0'+remainder;
	return out;
}

/*
 * convert a number to text
 */
static void textify_number(char *out,long anum)
{
	if(anum<0) {
		*out++='-';
		anum= -anum;
	}
	*textify_unisgned_number(out,anum)=0;
}

/*
 * ask a command to display itself
 */
static void display_cmd(sfk_iio_pt cmd,int cmd_num)
{
	switch(cmd_num) {
	case sfk_CMD_NUM(port):
		sfk_cmd_out(cmd,cmd->pqx.p->port_name);
		break;
	case sfk_CMD_NUM(display):
	case sfk_CMD_NUM(help):
	default:
    	sfk_cmd_out(cmd,SFK_TEXT(49));
	}
}

/*
 * display as text the value of a variable
 */
static void sfk_display_nm(sfk_iio_pt cmd,sfk_name_pt cur_nm)
{
  sfk_cmd_out(cmd,cur_nm->nm_text);
  if(cur_nm->nm_flags&SFK_CF_RO)
  	sfk_cmd_out(cmd,SFK_TEXT(50));
  sfk_cmd_out(cmd,SFK_TEXT(51));
  switch(cur_nm->nm_flags&SFK_CF_ARG_MASK) {
    case SFK_CF_NO_ARG:
      sfk_parse_fail(cmd,TEXT_NUM(9),cur_nm->nm_text);
       break;
    case SFK_CF_BOOL:
 	  if(sfk_PN(cmd->pqx.p->pnum)->var_vals[cur_nm->nm_val_num].x.nm_ival)
 	  	sfk_cmd_out(cmd,SFK_TEXT(46));
 	  else
 	  	sfk_cmd_out(cmd,SFK_TEXT(47));
      break;
    case SFK_CF_INT:
      {
      	char buf[60];
      	textify_number(buf,sfk_PN(cmd->pqx.p->pnum)->var_vals[cur_nm->nm_val_num].x.nm_ival);
	  	sfk_cmd_out(cmd,buf);
      }
      break;
    case SFK_CF_TEXT:
      if((cur_nm->nm_flags&SFK_CF_IMMEDIATE)!=0)
        display_cmd(cmd,cur_nm->nm_val_num);
       else
 	  	sfk_cmd_out(cmd,sfk_PN(cmd->pqx.p->pnum)->var_vals[cur_nm->nm_val_num].x.nm_sval);
      break;
    }
  sfk_cmd_out(cmd,SFK_TEXT(58));
}

/*
 * display the value of variables
 */
static void sfk_display(sfk_iio_pt cmd,char *vnames)
{
  int cmd_num=0;
  sfk_name_pt cur_nm;
  if(vnames[0]!=0)
  switch(sfk_binsearch_cmd(vnames,&cmd_num)) {
 	case sfk_BE_not_found:
	  sfk_parse_fail(cmd,TEXT_NUM(10),vnames);
	  break;		//never executed
	case sfk_BE_ambigous:
	  break;
	case sfk_BE_found:
	  break;
  }
  cur_nm= &sfk_cgl.sfk_cmd_table[cmd_num];
  while(strmatch(vnames,cur_nm->nm_text)&&
  	(cur_nm < &sfk_cgl.sfk_cmd_table[sfk_cgl.sfk_cmd_table_size]))
  	sfk_display_nm(cmd,cur_nm++);
}

/*
 * validate and select the requested port
 */
static void sfk_set_port(sfk_iio_pt cmd,int portno)
{
  if(portno== (-1))		/*-1 means leave port set as it is*/
  	portno=cmd->pqx.p->pnum;
  if(portno>=sfk_NUM_PORTS)
    sfk_parse_fail(cmd,TEXT_NUM(11),0);
  cmd->pqx.p=sfk_PN(portno);
}

/*
 * fail if a port is not selected
 */
static void sfk_need_port(sfk_iio_pt cmd)
{
  if((cmd->pqx.p == 0)||(cmd->pqx.p->pnum<0)||(cmd->pqx.p->pnum>=sfk_NUM_PORTS))
    sfk_parse_fail(cmd,TEXT_NUM(12),0);
}

/*
 * parse the name of a port
 */
static long parse_port(sfk_iio_pt cmd,char *arg_word)
{
  int i;
  if(isdigit(*arg_word))
    return sfk_parse_hex_or_dec(cmd,arg_word);
  for(i=0;i<sfk_NUM_PORTS;i++) {
    sfk_prt_pt p=sfk_PN(i);
    if(strmatch_some(arg_word,p->port_name))
    	return p->pnum;
  }
  sfk_parse_fail(cmd,TEXT_NUM(13),arg_word);
  SFK_NEVER_RETURN(0);
}

/*
 * set a non immediate value
 */
static void perform_immediate(sfk_iio_pt cmd,char *arg_word,int32 ival,sfk_name_pt cur_nm)
{
	switch(cur_nm->nm_val_num) {
	case sfk_CMD_NUM(help):
		sfk_help(cmd,arg_word);
		break;
	case sfk_CMD_NUM(display):
		sfk_display(cmd,arg_word);
		break;
	case sfk_CMD_NUM(port):
		{
		  int nval=parse_port(cmd,arg_word);
		  sfk_set_port(cmd,nval);
		}
		break;
	case sfk_CMD_NUM(debug_level):
		sfk_need_port(cmd);
		cmd->pqx.p->sfk_IVAR(debug_level)=ival;
#ifdef DBO_ENABLED
		cmd->pqx.p->dbo.just_kidding=(ival==0);
		dbo_fclear(&cmd->pqx.p->dbo);
#endif
		break;
	case sfk_CMD_NUM(online):
		sfk_need_port(cmd);
		sfk_change_online(cmd,ival);
		break;
	case sfk_CMD_NUM(kiss):
		sfk_need_port(cmd);
		sfk_change_kiss(cmd,ival);
		break;
	case sfk_CMD_NUM(soft_tnc):
		sfk_need_port(cmd);
		sfk_change_soft_tnc(cmd,ival);
		break;
	case sfk_CMD_NUM(shutdown):
		sfk_shutdown(cmd);
		break;
	default:     
		sfk_parse_fail(cmd,TEXT_NUM(14),cur_nm->nm_text);
	}
}

/*
 * set a non immediate value
 */
static void set_non_immediate(sfk_iio_pt cmd,char *arg_word,int32 ival,sfk_name_pt cur_nm)
{
  sfk_need_port(cmd);
  if(cur_nm->nm_flags&SFK_CF_RO)
    sfk_parse_fail(cmd,TEXT_NUM(15),cur_nm->nm_text);
  if(cmd->pqx.p->sfk_IVAR(online)!=0)
    sfk_parse_fail(cmd,TEXT_NUM(16),cur_nm->nm_text);
  sfk_PN(cmd->pqx.p->pnum)->var_vals[cur_nm->nm_val_num].changed=TRUE;
  switch(cur_nm->nm_flags&SFK_CF_ARG_MASK) {
    case SFK_CF_NO_ARG:
      if(arg_word[0]!=0)
		sfk_parse_fail(cmd,TEXT_NUM(17),arg_word);
      break;
    case SFK_CF_BOOL:
 	  sfk_PN(cmd->pqx.p->pnum)->var_vals[cur_nm->nm_val_num].x.nm_ival=ival;
      break;
    case SFK_CF_INT:
 	  sfk_PN(cmd->pqx.p->pnum)->var_vals[cur_nm->nm_val_num].x.nm_ival=ival;
      break;
    case SFK_CF_TEXT:
      if(strlen(arg_word)>=sfk_STRING_SIZE)
  		sfk_parse_fail(cmd,TEXT_NUM(18),arg_word);
	  strcpy(sfk_PN(cmd->pqx.p->pnum)->var_vals[cur_nm->nm_val_num].x.nm_sval,arg_word);
      break;
    }
  }

/*
 * parse and execute one line of input from the command object
 */
static void sfk_parse_1_command(sfk_iio_pt cmd)
{
  char cmd_word[sfk_MAX_TOKEN];
  char arg_word[sfk_MAX_TOKEN];
  sfk_name_pt cur_nm;
  int cmd_num;
  int32 ival=0;

  sfk_skip_white_space(cmd);
  sfk_parse_word(cmd,cmd_word);
  sfk_skip_white_space(cmd);
  switch(sfk_binsearch_cmd(cmd_word,&cmd_num)) {
 	case sfk_BE_not_found:
	  sfk_parse_fail(cmd,TEXT_NUM(19),cmd_word);
	  break;		//never gets executed
	case sfk_BE_ambigous:
	  sfk_parse_fail(cmd,TEXT_NUM(20),cmd_word);
	  break;		//never gets executed
	case sfk_BE_found:
	  break;
  }
  cur_nm= &sfk_cgl.sfk_cmd_table[cmd_num];
  arg_word[0]=0;
  if(!(cmd->pqx.eof || sfk_is_eol(cmd)))
  	sfk_parse_word(cmd,arg_word);
  else if((cur_nm->nm_flags&SFK_CF_IMMEDIATE)==0) {
	//if no argument pretend it is a display command
  	sfk_display_nm(cmd,cur_nm);
  	return;
  }
  switch(cur_nm->nm_flags&SFK_CF_ARG_MASK) {
	case SFK_CF_NO_ARG:
	  if(arg_word[0]!=0)
		sfk_parse_fail(cmd,TEXT_NUM(21),arg_word);
      break;
	case SFK_CF_BOOL:
      ival=sfk_parse_bool(cmd,arg_word);
      break;
	case SFK_CF_INT:
      ival=sfk_parse_hex_or_dec(cmd,arg_word);
      break;
	case SFK_CF_TEXT:
      break;
   }
  if((cur_nm->nm_flags&SFK_CF_IMMEDIATE)==0)
	set_non_immediate(cmd,arg_word,ival,cur_nm);
  else
	perform_immediate(cmd,arg_word,ival,cur_nm);
  sfk_parse_eol(cmd);
}

/*
 * parse and execute the passed commands
 */
void sfk_parse_command(sfk_iio_pt cmd)
{
  cmd->pqx.eof=FALSE;
  cmd->pqx.p=0;
  sfk_set_port(cmd,cmd->uio->do_this.cknd.sfk_msg.portno);
  sfk_nextch(cmd);
  while(!cmd->pqx.eof) {		/*do each line till eof or error*/
  	sfk_parse_1_command(cmd);
  	if(cmd->pqx.p!=0)			/*update selected port even if error later*/
  		cmd->uio->do_this.cknd.sfk_msg.portno=cmd->pqx.p->pnum;
  }
}
