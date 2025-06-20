/*
 * main program to test softkiss device driver
 * by Aaron Wohl / N3LIW (aw0g+@andrew.cmu.edu) jul 1990
 * 6393 Penn Ave #303
 * Pittsburgh PA, 15206
 * work: (412)-268-5032
 * home: (412)-731-6159
 */

#define VERS "1.6"

#include <stdlib.h>
#include <stdio.h>
#include <OSChecks.h>
#include <ctype.h>

#define NIL 0L

#include "sfk_core.h"
#include "sfk_core_private.h"
#include "sfk_core_some_macs.h"
#include "sfk_die.h"
#include "driver_shell.h"

#define fatal_error(xx_arg) {printf xx_arg;fflush(stdout);exit(0);}
extern void exit(int);

int drvr_num= (-10);
static char cmd_buf[300];

#define DEFAULT_PORT (1)
static int last_port=DEFAULT_PORT;

static char fix_letter(uint8 in)
{
	return (0x7f&(in>>1));
}

/*
 * test circular buffer code
 */
#define BUF_SIZE (10)
static void cu_test(char *cmd)
{
	static sfk_circular_buffer cu;
	switch(*cmd++) {
	case 'i':
		sfk_cu_init(&cu,BUF_SIZE);
		break;
	case 'u':
		sfk_cu_uninit(&cu);
		break;
	case 's':
		printf("size=%ld write=%ld read=%ld\n",
			cu.cu_size,cu.cu_write,cu.cu_read);
		printf("wsize=%ld\n",sfk_cu_write_size(&cu));
		printf("rsize=%ld\n",sfk_cu_read_size(&cu));
		break;
	case 'w':
		{
			long slen=strlen(cmd);
			cmd[slen++]='\n';
			cmd[slen]=0;
			slen=sfk_cu_write(&cu,cmd,slen);
			printf("wrote %ld bytes\n",slen);
		}
		break;
	case 'r':
		{
			char buf[BUF_SIZE];
			long rlen=sfk_cu_read(&cu,buf,sizeof(buf));
			buf[rlen]=0;
			printf("read %ld bytes '%s'\n",rlen,buf);
		}
		break;
	default:
		printf("unknown circuilar buffer command\n");
	}
}

static void print_address(uint8 *addr)
{
	char buf[20];
	char *o=buf;
	char ch;
	int ssid= (0x0f&((*(addr+6))>>1));
	int len=6;
	while(len-- > 0) {
		ch=fix_letter(*addr++);
		if(ch==' ')break;
		*o++=ch;
	}
	*o=0;
	if(ssid!=0)
		sprintf(o,"-%d",ssid);
	printf("%s",buf);
}

static print_buf(uint8 *buf,int len)
{
	char out_buf[2200];
	char *o=out_buf;
	char ch;
	int left=len;
	while(left-- > 0) {
		ch= *buf++;
		ch&=0x7f;
		if((ch==0x7f)||(ch<' ')) {
			*o++='^';
			if(ch==0x07f)
				ch='?';
			else
				ch+='@';
		}
		*o++ =ch;
	}
	*o=0;
	printf("len=%d %s\n",len,out_buf);
}

static int do_ctl(sfk_io_record_pt cmd,char *in_text)
{
	char out_buf[4000];
	int failed=TRUE;
	static Handle driver_shell=0;
	out_buf[0]=0;
	if(driver_shell==0)
		driver_shell=sh_read_in_driver_shell();
	if(driver_shell==0) {
		printf("can't read in driver shell\n");
		return -1;
	}
	if(cmd->csCode==sfk_CMD_msg)
	  sfk_parse_from(cmd,in_text,strlen(in_text),out_buf,sizeof(out_buf),last_port);
	sfk_driver((CntrlParam *)cmd,0,sh_CONTROL,driver_shell);
	if(cmd->csCode==sfk_CMD_msg)
		last_port=cmd->do_this.cknd.sfk_msg.portno;
	if((cmd->do_this.sfk_hdr.sys_err_code!=0)||
	   (cmd->do_this.sfk_hdr.sfk_err_code!=0)) {
		if(cmd->do_this.sfk_hdr.sfk_err_code!=sfk_NOTHING_TO_READ)
	   		printf("ctl failed %d %d\n",
	   		cmd->do_this.sfk_hdr.sys_err_code,cmd->do_this.sfk_hdr.sfk_err_code);
	} else
		failed=FALSE;
	if(out_buf[0]!=0)
	  printf("ctloutbuf='%s'\n",out_buf);
	return failed;
}

static char *command_resp_names[4]={
 "V1",
 "R",
 "C",
 "V1"
};

static void read_input()
{
	uint8 buf[4000];
	uint8 *obuf;
	int len;
	char *via=" via ";
	int command_resp;
	sfk_io_record cmd;
	memset(buf,0,sizeof(buf));
	memset(&cmd,0,sizeof(cmd));
	sfk_INIT_CPB(cmd,drvr_num,sfk_CMD_read);
	cmd.do_this.cknd.sfk_rw.data= buf;
	buf[0]=0;
	if(do_ctl(&cmd,cmd_buf+1))
		return;
	printf("%d!",cmd.do_this.cknd.sfk_rw.portno);
	print_address(buf+7);
	printf(" > ");
	print_address(buf);
	printf(" ");
	obuf=buf+7;
	len=cmd.do_this.cknd.sfk_rw.cnt-14;
	while ((len > 0) && (((*(obuf+6))&1)==0)) { /*print vias*/
		obuf+=7;
		printf("%s",via);
		via=" ";
		len-=7;
		print_address(obuf);
	}
	command_resp=(((*(buf+6))>>6)&2)| /*dest bit*/
				(((*(buf+13))>>7)&1); /*src bit*/
	printf("%s ",command_resp_names[command_resp]);
	obuf+=7;
	{
		uint8 ctl= *obuf++;
		len--;
		if((ctl&1)==0) {
			uint8 pid=*obuf++;
			len--;
			printf("Ir(%d)%ss(%d) ",	/*I (info) frame*/
				(ctl>>5)&7,
				(((ctl&0x10)!=0)?"P":""),
				(ctl>>1)&7);
			if(pid==0xcc)
				printf("IP ");
			else if(pid==0xcd)
				printf("ARP ");
			else if(pid==0xf0)
				printf("nol3 ");
			else if((pid&0x30)==0x10)
				printf("ax25 ");
			else if((pid&0x30)==0x20)
				printf("ax25 ");
		} else if ((ctl&2)==0)	{		/*S frame*/
			char *skind=" S";
			switch((ctl>>2)&3) {
			case 0:
				skind="RR ";
				break;
			case 1:
				skind="RNR ";
				break;
			case 2:
				skind="REJ ";
				break;
			default:
				break;
			}
			printf("%sN(%d)%s ",skind,
				((ctl>>5)&7),
				(((ctl&0x10)!=0)?"P":""));
		} else {		/*U frame*/
			char *cmd_type;
			switch(ctl&0xec) {
			case 0x2c:
				cmd_type="SABM";
				break;
			case 0x40:
				cmd_type="DISC";
				break;
			case 0x0c:
				cmd_type="DM";
				break;
			case 0x60:
				cmd_type="UA";
				break;
			case 0x84:
				cmd_type="FRMR";
				break;
			case 0x00:
				cmd_type="UI";
				break;
			default:
				cmd_type="U?";
			}
			printf("%s ",cmd_type);
		}
		print_buf(obuf,len);
	}
}

static char get_cmd()
{
	switch(last_port) {
	case 0:
		printf("printer_");
		break;
	case 1:
		printf("modem_");
		break;
	case 2:
		printf("loopback_");
		break;
	default:
		printf("port_%d_",last_port);
		break;
	}
	printf("sfk> ");
	if(gets(cmd_buf)==NIL)
		return 0;
	return cmd_buf[0];
}

static void read_cmd(void)
{
	int have_wne=WNEIsImplemented();
	EventRecord theEvent;
	printf("click mouse to exit recieve\n");
	while (TRUE ) {
		if(have_wne)
			if(WaitNextEvent(keyDownMask,&theEvent,10,0))
				break;
		if(Button())
			break;
		read_input();
		}
}

/*
 * send the passed message to the driver
 */
static void send_msg(char *msg)
{
	sfk_io_record cmd;
	sfk_INIT_CPB(cmd,drvr_num,sfk_CMD_msg);
	do_ctl(&cmd,msg);
}

/*
 * transmit the passed message on the passed port
 */
static void xmit_msg(char *msg,int msg_len,int portno)
{
	sfk_io_record cmd;
	memset(&cmd,0,sizeof(cmd));
	sfk_INIT_CPB(cmd,drvr_num,sfk_CMD_write);
	cmd.do_this.cknd.sfk_rw.data= (void *)msg;
	cmd.do_this.cknd.sfk_rw.portno= portno;
	cmd.do_this.cknd.sfk_rw.cnt=msg_len;
	do_ctl(&cmd,msg);
}

#define AX(xx_ch) ((xx_ch)<<1)
#define BL AX(' ')

static char cq_to[]={
	AX('C'), AX('Q'), BL, BL, BL, BL, 0x60 /*v 2.0 resp, no reserved bits, 0 ssid*/
};

static char cq_from[]={
	BL, BL, BL, BL, BL, BL, 0xe1, /*V2.0 resp, no reserved bits end of addr*/
	0x03,	/*unnumbered information*/
	0xF0	/*no layer 3 protocol in an id packet*/
};

static int nasty_chars(char *s)
{
	char ch;
	while((ch=*s++)!=0) {
		if(ch==' ')
			continue;
		if(ch<0) return TRUE;
		if(!isalnum(ch)) return TRUE;
	}
	return FALSE;
}

static void set_from()
{
	char *s=cmd_buf+1;
	int clen=strlen(s);
	char ch;
	unsigned char *out= (unsigned char *)cq_from;
	if(clen<1) {
		printf("from argument too short.  usage example: Fw1aw\n");
		return;
	}
	if(clen>6) {
		printf("from argument too long.  usage example: Fw1aw\n");
		return;
	}
	if(nasty_chars(s)) {
		printf("illegal character in from.  usage example: Fw1aw\n");
		return;
	}
	printf("new from '%s'\n",s);
	memset(out,BL,6);	/*perhaps previous id was shorter*/
	while(clen-- > 0) {
		ch= *s++;
		if(islower(ch))
			ch=toupper(ch);
		*out++ = AX(ch);
	}
}

struct out_buf_R {
	char *ostr;
	int max_len;
	int siz;
};
typedef struct out_buf_R out_buf,*out_buf_pt;

static append_output(out_buf_pt ob,char *txt,int len)
{
	if(ob->siz+len>ob->max_len) {
		printf("send string output buffer overflow, max size=%d\n",ob->max_len);
		exit(1);
	}
	memcpy(ob->ostr+ob->siz,txt,len);
	ob->siz+=len;
}

/*
 * transmit the passed text
 */
static int xmit_text(char *send_me)
{
	char buf[512];
	out_buf ob;
	ob.siz=0;
	ob.max_len=sizeof(buf);
	ob.ostr=buf;
	if(cq_from[0]==BL) {
		printf("set the callsign to send from with the F command\n");
		return TRUE;
	}
	printf("sending '%s'\n",send_me);
	append_output(&ob,cq_to,sizeof(cq_to));
	append_output(&ob,cq_from,sizeof(cq_from));
	append_output(&ob,send_me,strlen(send_me));
	xmit_msg(ob.ostr,ob.siz,last_port);
	return FALSE;
}

/*
 * send a message
 * when transmision begins, turn off interupts for long enough to
 * miss transmiting an outgoing character.  This is to test
 * for how someone having interupts off for too long is handled
 */
#define CHARS_TO_SEND_BEFORE_LOCKOUT (14)
long random_global;
static void isend(void)
{
    sfk_prt_pt p;
	if(xmit_text("softkiss interupt lockout test message 0123456789abcdefghijklmnopqrstuvwxyz"))
		return;
    p=sfk_PN(last_port);
    // wait for xmit to start
   while(p->out_count<CHARS_TO_SEND_BEFORE_LOCKOUT)
    	if(Button()) {
    		printf("isend abort by click\n");
    		return;
    		}
    sfk_SCC_interupts_off;
    {
    	long delay=0;
    	long delay2;
    	for(delay2=0;delay2<0x00300000;delay2++)
    			random_global++;
    }
	sfk_SCC_interupts_on;
	printf("woke up from lockout\n");
}

/*
 * send a test message
 */
static void xsend(void)
{
	if(strlen(cmd_buf+1)<1) {
		printf("send with string to send. example: Shello bob\n");
		return;
	}
	xmit_text(cmd_buf+1);
}

static void dump_regs()
{
	sfk_prt_pt p=sfk_PN(last_port);
	int i;
	for(i=0;i<NUM_SCC_REGS;i++) {
		printf("[%02d]=%02x ",i,p->scc_reg_cache[i]);
		if((i&7)==7)
		  printf("\n");
	}
}

static char *pr_bool(int a)
{
	if(a) return "";
	return "not ";
}

static void print_available()
{
	printf("_SerialPower %savailable\n",pr_bool(serial_power_implemented()));
	printf("_IdleState %savailable\n",pr_bool(sleep_state_implemented()));
	printf("_MemoryDispatch %savailable\n",pr_bool(is_vm_manager_present()));
	printf("_CacheFlushTrap %savailable\n",pr_bool(is_cache_manager_present()));
	printf("Time manager version %d\n",time_manager_version());
}

#define RES_FILE "\pboot:new_res"
#define SOUT_SIZE (60000)
#define FailOSErr(xx_test) \
	do { if((xx_test)!=0) fatal_error(("os call failed "));} while(0)

static void FailResError(void)
{
	FailOSErr(ResError());
}

int sfk_cmd_dump(int idx,char **nm_text,char **nm_help,
	char **nm_default,long *nm_flags,short *nm_val_num);

unsigned short scnt;
unsigned char *sbuf;
unsigned char *sout;
char **shandle;

static void dump_res(void)
{
	int idx=0;
	char *nm_text;
	char *nm_help;
	char *nm_default;
	long nm_flags;
	char buf[1000];
	short nm_val_num;
	int len;
	short imap[300];
	int i;
	memset(imap,-1,sizeof(imap));
	while(sfk_cmd_dump(idx,&nm_text,&nm_help,&nm_default,&nm_flags,&nm_val_num)) {
		if(imap[nm_val_num]!= -1)
			fatal_error(("command table duplicated with %s",nm_text));
		imap[nm_val_num]=idx;
		idx++;
	}
	printf("command table size=%d\n",idx);
	for(i=0;i<idx;i++) {
		if(imap[i]<0)
			fatal_error(("command table empty at %d %d",i,imap[i]));
		if(!sfk_cmd_dump(imap[i],&nm_text,&nm_help,&nm_default,&nm_flags,&nm_val_num))
			fatal_error(("command table botched"));
		sprintf(buf,"%s|%s|%s|%lx",
			nm_text,nm_help,nm_default,nm_flags);
		len=strlen(buf);
		scnt++;
		*sout++=len;
		memcpy(sout,buf,len);
		sout+=len;
	}
}

static void dump_commands()
{
	int res_file_num=0;
	scnt=0;
	shandle=NewHandle(SOUT_SIZE);
	if(shandle==0)
		fatal_error(("allocate handle failed"));
	HLock(shandle);
	sbuf=(unsigned char*)StripAddress(*shandle);
	sout=sbuf+2;
	dump_res();
	*((unsigned short *)sbuf)=scnt;

	FSDelete(RES_FILE,0);
	FailOSErr(Create(RES_FILE,0,'Doug','RSRC'));
	CreateResFile(RES_FILE);
	FailResError();
	if((res_file_num=OpenResFile((unsigned char *)StripAddress(RES_FILE)))== -1)
		FailResError();
	UseResFile(res_file_num);
	FailResError();

	HUnlock(shandle);
	SetHandleSize(shandle,sout-sbuf);
	FailResError();
	AddResource(shandle,'STR#',1025,"\pCommand Table");
	printf("ResError()=%d\n",ResError());
	CloseResFile(res_file_num);
	FailResError();
}

void cmd_loop(void);
void cmd_loop()
{
	sfk_io_record cmd;
	while(TRUE) {
		char ch=get_cmd();
		switch(ch) {
		case 'W':
			dump_commands();
			break;
		case 'C':
			cu_test(cmd_buf+1);
			break;
		case 'A':
			print_available();
		case 'Q':
			return;
		case 'T':
			printf("tick command system\n");
			sfk_INIT_CPB(cmd,drvr_num,sfk_CMD_run);
			do_ctl(&cmd,"");
			break;
		case 'S':
			xsend();
			break;
		case 'I':
			isend();
			break;
		case 'F':
			set_from();
			break;
		case 'R':
			read_cmd();
			break;
		case 'D':
		{
			
				while(!Button())
					printf("roll(%ld)=%ld\n",1000L,die(1000L));
		}
			break;			
 		default:
 			send_msg(cmd_buf);
		}
	}
}

static void cleanup()
{
	send_msg("shutdown");
}

static void open_res_file(void)
{
	short resnum=OpenResFile("\psfk_drvr.prj.rsrc");
	if(resnum== -1) {
		printf("can't find resource file\n");
		exit(1);
	}
	UseResFile(resnum);
	FailOSErr(ResError());
}

int main()
{
	SetApplLimit(CurStackBase-70000L);
	printf("SoftKiss test client test jig built on %s %s\n",__DATE__,__TIME__);
	printf(" by Aaron Wohl / n3liw / aw0g+@andrew.cmu.edu / (412)-731-6159\n");
	printf("Stackspace=%ld bytes\n",((long)StackSpace()));
	printf(" test jig commands:\n");
	printf(" Q - quit\n");
	printf(" R - recieve, click mouse to exit\n");
	printf(" F - set from example Fw1aw\n");
	printf(" I - test interupt lockout during xmit\n");
	printf(" S<data> - send data then recieve example: Shello how are you?\n");
	printf(" all other commands passed as text to device driver\n");
	printf("common commands:\n");
	printf(" port printer - select printer port (port modem selects modem)\n");
	printf(" online true -- turn on port with current setting\n");
	printf(" online false -- turn off port\n");
	printf(" kiss true -- enable kiss mode on current port\n");
	printf(" help -- driver help file\n");
	printf(" display [variable] -- show value or all variables\n");
	printf("hold mouse down during output to pause\n");
	printf("version %s\n",VERS);
	_atexit(cleanup);
	open_res_file();
	cmd_loop();
	exit(0);
}
