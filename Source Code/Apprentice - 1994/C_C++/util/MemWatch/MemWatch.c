#include "bmminclude.h"
#if MEM_DEBUG>0

/*
 * AAMemWatch.c - watch memory allocation freeing
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */

#pragma options(!profile,!force_frame)

#include "MemWatch.h"
#include "MemWatchInternal.h"
#include "bbt.h"
#include "MemConfig.h"
#include "Traps.h"
#include "OSChecks.h"
#include <stdio.h>

/*
 * algolw like assert statement to check for assumptions
 */
#define MEM_ASSERT(xx_bool,xx_str) \
	do { \
		if(!(xx_bool)) while(TRUE) \
			DebugStr(xx_str); \
	} while(0)

/*
 * linked list of bbt_mem_entry blocks as a free list
 */
struct mem_free_blk_R {
	struct mem_free_blk_R *next;
};
typedef struct mem_free_blk_R mem_free_blk,*mem_free_blk_pt;

/*
 * globals for memory watching
 */
struct {
	long original_new_ptr;
	long original_new_handle;
	long original_DisposPtr;
	long original_DisposHandle;
	mem_free_blk_pt free_list;
	unsigned long	create_serial_num;	/*number of blocks created since startup*/
	long	mem_blocks_used;
	long	mem_blocks_total;
	bbt_table by_addr;					/*tree of memory blocks sorted by memory address*/
	bbt_table by_time;					/*tree of memory blocks sorted by time they where created*/
	int	recursive_call;					/*detect rentry*/
	long num_recursive_call;			/*number of times it happened*/
} mgl;

#define MEM_ENABLED (mgl.original_new_ptr!=0)

/*
 * set up the free list of blocks to record memory usage
 * this free list is also used for the btree blocks
 */
static void mem_add_storage(long num_blocks)
{
	int i;
	mem_entry_pt space=(mem_entry_pt)NewPtrClear(num_blocks*sizeof(mem_entry));
	MEM_ASSERT((space!=0),"\pMEM can allocate enough memory");
	mgl.mem_blocks_total+=num_blocks;
	for(i=0;i<num_blocks;i++) {
		((mem_free_blk_pt)space)->next=mgl.free_list;
		mgl.free_list=((mem_free_blk_pt)space);
		space++;
	}
}

/*
 * return the number of unalocated memory recording blocks
 */
long mem_blocks_left(void)
{
	return mgl.mem_blocks_total-mgl.mem_blocks_used;
}

/*
 * is this address in the application heap
 */
int mem_in_my_heap(unsigned long addr)
{
	if(addr>=((unsigned long)GetApplLimit()))
		return FALSE;
	if(addr<(((unsigned long)ApplicZone())+sizeof(THz)))
		return FALSE;
	return TRUE;
}

/*
 * return the by creation time table of current memory blocks
 */
bbt_table_pt mem_get_by_time(void)
{
	return &mgl.by_time;
}


/*
 * return the by address table of current memory blocks
 */
bbt_table_pt mem_get_by_addr(void)
{
	return &mgl.by_addr;
}

/*
 * option a4_globals is set by the thinkc compiler when
 * making a driver or code resource
 */
#if __option(a4_globals)
#define amw_GLOBAL_REG a4
#else
#define amw_GLOBAL_REG a5
#endif

#define MAGIC_UNPATCHED_NUMBER 0x12345678

#define amw_SETUP_GLOBAL_REG \
	asm { jsr amw_find_address } \
	asm { move.l amw_GLOBAL_REG,-(a7) } \
	asm { move.l #MAGIC_UNPATCHED_NUMBER,amw_GLOBAL_REG } /*patch at runtime*/

#define amw_RESTORE_GLOBAL_REG \
	asm { move.l (a7)+,amw_GLOBAL_REG }

/*
 * helper routine to help locate where code is actually
 * located.  when you take the address of code with a & you
 * get the jump table address
 * returns to callers caller with result in a0
 */
static void amw_find_address()
{
	asm { move.l (a7)+,a0 }
}

/*
 * return the contents of the global register (A5 or A4)
 */
static long get_global_reg()
{
	asm { move.l amw_GLOBAL_REG,d0 }
}

/*
 * patch in global register to interupt routines
 */
static void patch_in_global(void *iroutine)
{
	long *patch_point=((long *)(((char *)iroutine)+4));
	MEM_ASSERT(((*patch_point)==MAGIC_UNPATCHED_NUMBER),"\pMEM patchin failed");
	(*patch_point)=get_global_reg();
}

/*
 * initialize tables that can't be done as static initialization
 * in a think code segment.  patch global register into driver routines
 */
static void *mk_int(void *iroutine)
{
	void (*rtn)(void)=iroutine;
	/*call the routine to ask it where it really lives*/
	(*rtn)();
	asm { move.l a0,iroutine }
	patch_in_global(iroutine);
	return iroutine;
}


/*
 * allocate a memory block to record heap allocation
 * or as a btree entry
 */
static void *MEM_alloc(long size)
{
	mem_free_blk_pt result;
	if(mgl.free_list==0)
		mem_add_storage(MEM_ADDITIONAL_HEAP_BLOCKS);
	result=mgl.free_list;
	MEM_ASSERT((result!=0),"\pMEM free list garbled");
	MEM_ASSERT((size<=sizeof(mem_entry)),
		"\pMEM asked for incorrect block size");
	mgl.free_list=result->next;
	mgl.mem_blocks_used++;
	memset(result,0,sizeof(mem_entry));
	return result;
}

/*
 * free a block allocated with MEM_alloc
 */
static void MEM_free(void *blk)
{
	((mem_free_blk_pt)blk)->next=mgl.free_list;
	mgl.free_list=blk;
	mgl.mem_blocks_used--;
	MEM_ASSERT(mgl.mem_blocks_used>=0,"\pMEM free count not garbled");
}

/*
 * return the current stack pointer
 */
static unsigned long get_sp(void)
{
	asm {
		move.l sp,d0
	}
}

/*
 * is this address in the stack area of the current application?
 */
static int in_stack(unsigned long addr)
{
	if(addr<get_sp())		/*benith current bottom?*/
		return FALSE;		/*yes, not in stack*/
	if(addr>((unsigned long)CurStackBase))	/*above top?*/
		return FALSE;		/*yes, not in stack*/
	return TRUE;
}

/*
 * follow the a6 stack frame linkage to get the return addresses of our callers
 */
static void mem_get_stack_frame(unsigned char **dst,int depth,void *a6_link)
{
	while(depth-->0) {
		unsigned long a6=(long)a6_link;
		if((a6&1)!=0)		/*don't access odd stack addresses*/
			break;
		if(!in_stack(a6))
			break;
		a6_link=(void*)(*((long*)a6));		/*chase chain up to next stack frame*/
		*dst++= (void*)(*((long*)(a6+4)));	/*get return pc for this layer*/
	}
}

/*
 * return the current memory block creation serial number
 * it is a count of the number of blocks created since startup
 */
unsigned long mem_get_epoch(void)
{
	return mgl.create_serial_num;
}

/*
 * if there is a certain kind of block you are interested in
 * change this code to breakpoint on it
 */
static void check_break_point(mem_entry_pt nblk)
{
#ifdef XXX
	if(last_proc_is("\pCPrinter::OpenPrintMgr") &&
		((nblk->mem_flags&MFL_isptr)==0) &&
		(GetHandleSize((Handle)nblk->mem_addr)==4))
			DebugStr("\pmemory allocation breakpoint");
#endif
}

/*
 * record the allocation of a new block
 */
static record_new_mem(int is_ptr,void *blk,void *a6_link)
{
	mem_entry_pt nblk=MEM_alloc(sizeof(*nblk));
	nblk->mem_addr=(long)blk;
	nblk->mem_create_time=mgl.create_serial_num++;
	if(is_ptr)
		nblk->mem_flags|=MFL_isptr;
	mem_get_recent_procs(nblk->rp,MEM_RECENT_SIZE);
	mem_get_stack_frame(nblk->stackp,MEM_STACK_SIZE,a6_link);
	bbt_insert(&mgl.by_addr,(bbt_key_t)blk,(bbt_key_t)nblk);
	bbt_insert(&mgl.by_time,(bbt_key_t)nblk->mem_create_time,(bbt_key_t)nblk);
	check_break_point(nblk);
}

/*
 * clean up on destruction of a memory block
 */
static record_free_mem(int is_ptr,void *blk)
{
	mem_entry_pt arecord;
	if(!bbt_find(&mgl.by_addr,(bbt_key_t)blk,(bbt_value_t*)&arecord)) {
		// DebugStr("\pMEM pointer being freed does not exist");
		return;
	}
	MEM_ASSERT(bbt_delete(&mgl.by_addr,(bbt_key_t)blk),"\pmemory blocks being deleted where created/addr");
	MEM_ASSERT(bbt_delete(&mgl.by_time,(bbt_key_t)arecord->mem_create_time),"\pmemory blocks being deleted where created/time");
	MEM_free(arecord);
}

/*
 * record the birth of a new object
 * called with c linkage
 */
void mem_record_birth_c(CObject *new_object)
{
	mem_entry_pt arecord;
	if(!MEM_ENABLED)
		return;
	if(!bbt_find(&mgl.by_addr,(bbt_key_t)new_object,(bbt_value_t*)&arecord)) {
		DebugStr("\pMEM can't find memory block used by a new object");
		return;
	}
	MEM_ASSERT(((arecord->mem_flags&MFL_isptr)==0),"\pMEM objects are handles");
	MEM_ASSERT((new_object==((void *)arecord->mem_addr)),"\pMEM new object is valid");
	arecord->mem_flags|=MFL_isobject;
	arecord->class_name=class_name(new_object);
}

/*
 * called by changed line in new.c after object is created
 * called with new object in a0
 */
void mem_record_birth(void);
void mem_record_birth(void)
{
	asm {
		movem.l	d0-d4/a0-a2,-(sp)
		move.l d7,-(sp)
		jsr mem_record_birth_c
		addq.l #4,sp
		movem.l	(sp)+,d0-d4/a0-a2
	}
}

/*
 * give some commentary for what a memory block is
 * called by user code
 */
static void describe_block(const void *mem_blk,const CObject *owner,const char *description,int isptr)
{
	mem_entry_pt arecord;
	if(mem_blk==0)
		return;
	if(!bbt_find(&mgl.by_addr,(bbt_key_t)mem_blk,(bbt_value_t*)&arecord)) {
		DebugStr("\pMEM pointer being described does not exist");
		return;
	}
	arecord->owner=(CObject*)owner;
	arecord->desc=(char*)description;
	if(((isptr^arecord->mem_flags)&1)!=0)
		DebugStr("\pMEM mem_describe_handle/_ptr block is not expected type");
}

/*
 * describe what a handle is
 */
void mem_describe_handle(const void *mem_blk,const CObject *owner,const char *description)
{
	if(!MEM_ENABLED)
		return;
	describe_block(mem_blk,owner,description,FALSE);
}

/*
 * describe what a pointer is is
 */
void mem_describe_ptr(const void *mem_blk,const CObject *owner,const char *description)
{
	if(!MEM_ENABLED)
		return;
	describe_block(mem_blk,owner,description,TRUE);
}

/*
 * record something happening to a block of memory
 * possible values of action:
 * NH - new handle
 * NP - new pointer
 * DH - dispose handle
 * DP - dispose handle
 * This is called from inside a system call patch
 * So no segment faults and no memory allocation/freeing allowed
 */
static void record_mem(short action,void *mem_block,void *a6_link)
{
	short ch=action;
	void *blk=mem_block;
	if(!mem_in_my_heap((unsigned long)mem_block))
		return;
	if(mgl.recursive_call) {		//allow segfaults we cause to allocate memory
		mgl.num_recursive_call++;
		return;
	}
	mgl.recursive_call=TRUE;
	switch(ch) {
	case 'NH':
		record_new_mem(FALSE,blk,a6_link);
		break;
	case 'NP':
		record_new_mem(TRUE,blk,a6_link);
		break;
	case 'DH':
		record_free_mem(FALSE,blk);
		break;
	case 'DP':
		record_free_mem(TRUE,blk);
		break;
	}
	mgl.recursive_call=FALSE;
}

/*
 * called from system call dispatcher
 */
static void NewPtr_Patch(void)
{
	amw_SETUP_GLOBAL_REG;
	asm {
		move.l a0,-(sp)
		move.l a0,-(sp)
		move.l a0,-(sp)
		lea @after,a0
		move.l a0,8(sp)
		move.l mgl.original_new_ptr,a0
		move.l a0,4(sp)
		move.l (sp)+,a0
		rts
@after
		movem.l	d0-d4/a0-a4,-(sp)
		move.l	a6,-(sp)
		move.l	a0,-(sp)
		move.w	#'NP',-(sp)
		jsr		record_mem
		add.l	#10,sp
		movem.l (sp)+,d0-d4/a0-a4
	}
	amw_RESTORE_GLOBAL_REG;
}

/*
 * called from system call dispatcher
 */
static void NewHandle_Patch(void)
{
	amw_SETUP_GLOBAL_REG;
	asm {
		move.l a0,-(sp)
		move.l a0,-(sp)
		move.l a0,-(sp)
		lea @after,a0
		move.l a0,8(sp)
		move.l mgl.original_new_handle,a0
		move.l a0,4(sp)
		move.l (sp)+,a0
		rts
@after
		movem.l	d0-d4/a0-a4,-(sp)
		move.l	a6,-(sp)
		move.l	a0,-(sp)
		move.w	#'NH',-(sp)
		jsr		record_mem
		add.l	#10,sp
		movem.l (sp)+,d0-d4/a0-a4
	}
	amw_RESTORE_GLOBAL_REG;
}

/*
 * free a handle
 */
static void DisposHandle_Patch(void)
{
	amw_SETUP_GLOBAL_REG;
	asm {
		movem.l	d0-d4/a0-a4,-(sp)
		move.l	a6,-(sp)
		move.l	a0,-(sp)
		move.w	#'DH',-(sp)
		jsr		record_mem
		add.l	#10,sp
		movem.l (sp)+,d0-d4/a0-a4

		move.l a0,-(sp)
		move.l mgl.original_DisposHandle,a0
		move.l 4(sp),amw_GLOBAL_REG
		move.l a0,4(sp)
		move.l (sp)+,a0
		rts
		// never gets here
	}
	amw_RESTORE_GLOBAL_REG;
}

/*
 * free a handle
 */
static void DisposPtr_Patch(void)
{
	amw_SETUP_GLOBAL_REG;
	asm {
		movem.l	d0-d4/a0-a4,-(sp)
		move.l	a6,-(sp)
		move.l	a0,-(sp)
		move.w	#'DP',-(sp)
		jsr		record_mem
		add.l	#10,sp
		movem.l (sp)+,d0-d4/a0-a4

		move.l a0,-(sp)
		move.l mgl.original_DisposPtr,a0
		move.l 4(sp),amw_GLOBAL_REG
		move.l a0,4(sp)
		move.l (sp)+,a0
		rts
		// never gets here
	}
	amw_RESTORE_GLOBAL_REG;
}

struct patch_info_R {
	short	trap_word;
	void	(*new_address);
	long	*old_address;
	int		trap_type;
};
typedef struct patch_info_R patch_info,*patch_info_pt;

static patch_info patches[]={
	{_NewPtr,NewPtr_Patch,&mgl.original_new_ptr},
	{_NewHandle,NewHandle_Patch,&mgl.original_new_handle},
	{_DisposPtr,DisposPtr_Patch,&mgl.original_DisposPtr},
	{_DisposHandle,DisposHandle_Patch,&mgl.original_DisposHandle},
};
static int patches_size=(sizeof(patches)/sizeof(patches[0]));

/*
 * remove memory allocation watching hooks
 */
void mem_RemovePatch(void)
{
	int i;
	patch_info_pt pats=patches;
	for(i=0;i<patches_size;i++,pats++) {
		if((*(pats->old_address))==0)
			continue;
		NSetTrapAddress(*(pats->old_address),pats->trap_word,pats->trap_type);
		*(pats->old_address)=0;
	}
}

/*
 * figure out if this is a toolbox trap or an os trap
 */
static short DeduceTrap(short theTrap)
{
	return ((theTrap&0x800)!= 0)?ToolTrap:OSTrap;
}

/*
 * install memory watching hooks
 */
static void ReallyInstallMemWatchPatche()
{
	int i;
	patch_info_pt pats=patches;
	for(i=0;i<patches_size;i++,pats++) {
		long new_trap_location=(long)mk_int(pats->new_address);
		pats->trap_type=DeduceTrap(pats->trap_word);
		if(pats->trap_type==ToolTrap)
			pats->trap_word&=0x7FF;
		*(pats->old_address)=NGetTrapAddress(pats->trap_word,pats->trap_type);
		NSetTrapAddress(new_trap_location,pats->trap_word,pats->trap_type);
	}
	FlushCache();
}

/*
 * init the tables for recording memory allocations
 */
static void mem_init_btrees(void)
{
	long degree=bbt_find_min_degree(sizeof(mem_entry));
	bbt_init_bbt(&mgl.by_addr,MEM_alloc,MEM_free,degree);
	bbt_init_bbt(&mgl.by_time,MEM_alloc,MEM_free,degree);
}

/*
 * get any segments we refrence in and loaded
 */
static void mem_load_segments(void)
{
	int temp1;
	int temp2;
	char buf[100];
	FlushCache();		//get oschecks loaded
	memcpy(&temp1,&temp2,sizeof(temp1)); //get memcpy laoded
	sprintf(buf,"foo");	//get sprintf loaded
}

/*
 * install memory watching hooks
 */
void mem_InstallPatch(void)
{
	if(MEM_ENABLED)			//are the patches already installed?
		return;				//yes, so already done
	mem_load_segments();		//force in anything we need
	mem_add_storage(MEM_INITIAL_HEAP_BLOCKS);
	mem_init_btrees();
	ReallyInstallMemWatchPatche();
}

/*
 * record of recently entered proceedures
 */
static unsigned long last_proc=0;
static mem_recent_procs rps[MEM_NUM_LAST_PROCS];

/*
 * check to see if the passed pstring is the last entered proceedure
 */
int mem_last_proc_is(unsigned char *s)
{
	if(last_proc>MEM_NUM_LAST_PROCS)
		return FALSE;
	return (memcmp(s,rps[last_proc],*s)==0);
}


/*
 * called by think c procedure entry code for each entered proceedure
 */
void _profile_(unsigned char *proc_name);
void _profile_(unsigned char *proc_name)
{
	last_proc++;
	if(last_proc>=MEM_NUM_LAST_PROCS)
		last_proc=0;
	rps[last_proc]=proc_name;
}

/*
 * return a record of the recently entered proceedures
 */
void mem_get_recent_procs(mem_recent_procs_pt rprocs,unsigned long max_procs)
{
	long src;
	unsigned long dst;
	if(last_proc>=MEM_NUM_LAST_PROCS)
		last_proc=0;
	for(dst=0,src=last_proc;dst<max_procs;dst++,src--) {
		if(src<0)
			src=MEM_NUM_LAST_PROCS-1;
		rprocs[dst]=rps[src];
	}
}

/*
 * is memory debugging on?
 */
int mem_is_debug_on(void)
{
	return (mgl.original_DisposHandle!=0);
}
#endif

