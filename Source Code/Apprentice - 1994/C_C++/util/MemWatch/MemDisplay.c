#include "bmminclude.h"
#if MEM_DEBUG>0
/*
 * AAMemDisplay.c - display MemWatch tables to a string
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#pragma options(!profile)

#include "MemWatchInternal.h"
#include "bbt.h"
#include <ctype.h>
#include <stdio.h>

/*
 * wrap output lines at this width
 */
#define LINE_WRAP (60)

/*
 * append the recent call names together
 */
static void get_recent_text(char *start_of_line,unsigned char **src)
{
	int i;
	char *sep="";
	char *dst=start_of_line;
	*dst=0;
	for(i=0;i<MEM_RECENT_SIZE;i++) {
		unsigned char *astr= *src++;
		int len;
		if(astr==0)
			break;
		if((dst-start_of_line)>LINE_WRAP) {
			sep="\n    ";
			start_of_line=dst;
		}
		strcat(dst,sep);
		dst+=strlen(dst);
		sep=",";
		len= *astr++;
		memcpy(dst,astr,len);
		dst+=len;
		*dst=0;
	}
	*dst=0;
}

/*
 * is this a valid routine name?
 */
static int nice_name(char *src,int len)
{
	unsigned char ch;
	if(len<1)
		return FALSE;
	while(len--> 0) {
		ch= *src++;
		if(isalnum(ch)||(ch=='_')||(ch=='.')||(ch=='$'))
			continue;
		return FALSE;
	}
	return TRUE;
}

/*
 * search machine code looking for the end of a routine followed by
 * the debugging name of the routine
 */
static unsigned char *find_routine_end(unsigned char *addr)
{
	int max_search=4000;	/*give up if can't find it in this many bytes*/
	unsigned char ch;
	int len;
	while(max_search-->0) {
		ch= *addr++;
		if(ch!=0x4e)		/*byte of return instruction?*/
			continue;		/*no*/
		if((*addr)!=0x75)	/*other byte of return instruction?*/
			continue;		/*no*/
		addr++;
		len= (*addr)&0x7f;	/*get string length*/
		if((*(addr+len+1))!=0) /*string null terminated?*/
			continue;
		if(!nice_name((char *)(addr+1),len))
			continue;
		return addr;
	}
	return 0;
}

/*
 * search backwards in machine code for the start of a routine
 */
static unsigned char *find_routine_start(unsigned char *addr)
{
	int max_search=4000;	/*give up if can't find it in this many bytes*/
	unsigned char ch;
	int len;
	while(max_search-->0) {
		ch= *addr--;
		if((((unsigned long)addr)&1)!=0)
			continue;
		if(ch!=0x56)
			continue;
		if((*addr)!=0x4e)
			continue;
		if((*(addr-1))!=0)
			continue;
		if((*(addr-2))!=0)
			continue;
		return addr;
	}
	return 0;
}

/*
 * try to turn a memory address into a routine name+offset
 * returns TRUE on success
 */
static int try_textify_code_address(char *dst,unsigned long addr)
{
	unsigned char *routine_name;
	unsigned char *routine_start;
	int len;
	if(!mem_in_my_heap(addr))
		return FALSE;
	if((routine_name=find_routine_end((unsigned char *)addr))==0)
		return FALSE;
	routine_start=find_routine_start((unsigned char *)addr);
	len= (*routine_name++) & 0x7F;
	memcpy(dst,routine_name,len);
	dst+=len;
	*dst=0;
	if(routine_start!=0)
		sprintf(dst,"+%lx",((unsigned char *)addr)-routine_start);
	return TRUE;
}

/*
 * turn a memory address into a routine name+offset
 */
static void textify_code_address(char *dst,unsigned long addr)
{
	if(!try_textify_code_address(dst,addr))
		sprintf(dst,"%lx",addr);
}

/*
 * print the names of the calling routines
 */
static get_call_stack(char *start_of_line,unsigned char **src)
{
	int i;
	unsigned long addr;
	char *sep="";
	char *dst=start_of_line;
	*dst=0;
	for(i=0;i<MEM_STACK_SIZE;i++) {
		addr= (unsigned long)(*src++);
		if(addr==0)
			break;
		if((dst-start_of_line)>LINE_WRAP) {
			sep="\n    ";
			start_of_line=dst;
		}
		strcat(dst,sep);
		dst+=strlen(dst);
		textify_code_address(dst,addr);
		dst+=strlen(dst);
		sep=",";
	}
}

#define MEM_LOW_BYTE(xx_zz) ((int)((xx_zz)&0x0FF))

/*
 * write one memory block to a text buffer
 */
void mem_dump_block(char *dst,mem_entry_pt ablock,long *tot_size)
{
	long block_size;
	char buf[256*MEM_STACK_SIZE];
	char *cname;
	if((ablock->mem_flags&MFL_isptr)!=0)
		block_size=GetPtrSize((Ptr)ablock->mem_addr);
	else
		block_size=GetHandleSize((Handle)ablock->mem_addr);
	(*tot_size)+=block_size;
	sprintf(dst,"%s %lx size=%ld created at %ld",
		((ablock->mem_flags&MFL_isptr)?"Pointer":"Handle"),
		ablock->mem_addr,
		block_size,
		ablock->mem_create_time);
	dst+=strlen(dst);
	if((!(ablock->mem_flags&MFL_isptr))&&mem_in_my_heap(ablock->mem_addr)) {
		unsigned char rName[256];
		short rID;
		ResType rType;
		sprintf(dst," @%lx",*((long*)(ablock->mem_addr)));
		dst+=strlen(dst);
		GetResInfo((Handle)ablock->mem_addr,&rID,&rType,rName);
		if(ResError()==0) {
			sprintf(dst,"  id=%d res='%c%c%c%c'",rID,
				MEM_LOW_BYTE(rType>>24),
				MEM_LOW_BYTE(rType>>16),
				MEM_LOW_BYTE(rType>> 8),
				MEM_LOW_BYTE(rType    ));
			dst+=strlen(dst);
		}
		
	}
	strcat(dst,"\n");
	dst+=strlen(dst);
	if(ablock->mem_flags&MFL_isobject) {
		cname=class_name((CObject*)ablock->mem_addr);
		sprintf(dst,"  object class %s\n",cname);
	} else {
		get_recent_text(buf,ablock->rp);
		sprintf(dst,"  recent - %s\n",buf);
	}
	dst+=strlen(dst);
	get_call_stack(buf,ablock->stackp);
	sprintf(dst,"  stack - %s\n",buf);
	dst+=strlen(dst);
	if(ablock->owner!=0)
		sprintf(dst,"  owner is block %lx\n",ablock->owner);
	dst+=strlen(dst);
	if(ablock->desc!=0)
		sprintf(dst,"  block is a '%s'\n",ablock->desc);
	dst+=strlen(dst);
}

/*
 * set a description of block if it is not already set
 */
void mem_try_describe(void *blk,char *describe,unsigned long desc_type)
{
	mem_entry_pt arecord;
	if(blk==0)return;
	if(!bbt_find(mem_get_by_addr(),(bbt_key_t)blk,(bbt_value_t*)&arecord))
		return;				/*we don't know about the block*/
	if(arecord->desc!=0)	/*already have a description?*/
		return;				/*yes, so go with that*/
	arecord->desc=describe;
	arecord->desc_type=desc_type;
}

/*
 * describe memory used by window controls
 */
static void mem_describe_controls(ControlHandle actl)
{
	while(actl!=0) {
		mem_try_describe(actl,"control",'JUNK');
		mem_try_describe((*actl)->contrlDefProc,"control def",'JUNK');
		actl=(*actl)->nextControl;
	}
}

/*
 * describe heap objects found on the window list
 */
static void mem_describe_window_stuff(void)
{
	WindowPeek awin=WindowList;
	while(awin!=0) {
		mem_try_describe(awin,"window",'JUNK');
		mem_try_describe(awin->strucRgn,"window content region+frame",'RGNN');
		mem_try_describe(awin->contRgn,"window content region",'RGNN');
		mem_try_describe(awin->updateRgn,"window update region",'RGNN');
		mem_try_describe(awin->titleHandle,"window title",'RGNN');
		mem_try_describe(awin->windowPic,"window picture",'PICT');
		mem_try_describe(awin->port.visRgn,"window visRgn",'RGNN');
		mem_try_describe(awin->port.clipRgn,"window clipRgn",'RGNN');
		mem_try_describe(awin->port.visRgn,"window visrgn",'RGNN');
		mem_try_describe(awin->port.picSave,"window open picture",'JUNK');
		mem_try_describe(awin->port.rgnSave,"window open region",'JUNK');
		mem_try_describe(awin->port.polySave,"window open polygon",'JUNK');
		mem_try_describe(awin->port.portBits.baseAddr,"window bitmap",'JUNK');
		mem_try_describe(awin->dataHandle,"window zoom state",'JUNK');
		mem_describe_controls(awin->controlList);
		awin=awin->nextWindow;
	}
}

/*
 * describe heap objects found on the window color window list
 */
static void mem_describe_color_window_stuff(void)
{
	AuxWinHandle awin=AuxWinHead;
	while(awin!=0) {
		mem_try_describe((*awin)->awCTable,"auxwindow color table",'JUNK');
		mem_try_describe((*awin)->dialogCItem,"auxwindow dialog item",'JUNK');
		mem_try_describe((*awin)->awReserved,"auxwindow reserved item",'JUNK');
		awin=(*awin)->awNext;
	}
}

/*
 * describe heap objects found on the window color window list
 */
static void mem_describe_color_control_stuff(void)
{
	AuxCtlHandle actl=AuxCtlHead;
	while(actl!=0) {
		mem_try_describe((*actl)->acOwner,"color control",'JUNK');
		mem_try_describe((*actl)->acCTable,"color control color table",'JUNK');
		actl=(AuxCtlHandle)(*actl)->acNext;
	}
}

/*
 * some low memory variables give hints as to what heap objects are
 */
void mem_make_up_descriptions(void)
{
	mem_try_describe(DAStrings[0],"ParamText0",'PSTR');
	mem_try_describe(DAStrings[1],"ParamText1",'PSTR');
	mem_try_describe(DAStrings[2],"ParamText2",'PSTR');
	mem_try_describe(DAStrings[3],"ParamText3",'PSTR');
	mem_try_describe(DeskPattern,"DeskPattern",'PATT');
	mem_try_describe(GrayRgn,"GrayRgn",'RGNN');
	mem_try_describe(SaveVisRgn,"SaveVisRgn",'RGNN');
	mem_try_describe(AppParmHandle,"AppParmHandle",'JUNK');
	mem_try_describe(MenuCInfo,"MenuCInfo - color menu table",'JUNK');
	mem_try_describe(MenuList,"MenuList",'JUNK');  /*??? find rest of list*/
	mem_try_describe(TEScrpHandle,"text edit scrap",'TEXT');
	mem_try_describe(TopMapHndl,"TopMapHndl",'JUNK');  /*??? find rest of list*/
	mem_describe_window_stuff();
	mem_describe_color_window_stuff();
	mem_describe_color_control_stuff();
}


#endif

