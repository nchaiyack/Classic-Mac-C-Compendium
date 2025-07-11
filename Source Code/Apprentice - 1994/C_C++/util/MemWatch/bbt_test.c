#include "bmminclude.h"
/*
 * test bbt
 */

#include "Bbbt.h"
#include <stdio.h>
#include <stdlib.h>

static msn_VOID_PTR aloc_mem(long msize)
{
	void *result=malloc(msize);
	memset(result,0,msize);
}

static void free_mem(void *ptr)
{
	free(ptr);
}

msn_PRIVATE_PROC msn_VOID bbt_dump(anode,depth)
	bbt_node_pt anode;
	int depth;
{
	int i;
	printf("node depth %d at %lx leaf=%d\n",depth,anode,anode->bbt_isleaf);
	for(i=0;i<anode->bbt_size;i++)
		printf("  key %ld value %lx %ld\n",
			anode->bbt_items[i].bbt_key,
			anode->bbt_items[i].x.bbt_value,
			anode->bbt_items[i].x.bbt_value);
	if(!anode->bbt_isleaf)
		for(i=0;i<anode->bbt_size;i++)
			bbt_dump(anode->bbt_items[i].x.bbt_down,depth+1);
}

/*
 * insert an item into a btree leaf
 */
msn_PRIVATE_PROC msn_VOID bbt_xinsert(table,key)
	bbt_table_pt table;
	bbt_key_t key;
{
	bbt_value_t fvalue=((bbt_value_t)((long)key* -10));
	printf("insert key %ld value %ld\n",
		key,fvalue);
	bbt_insert(table,key,fvalue);
	bbt_check(table);
#ifdef RUBBISH
	{
		bbt_key_t key;
		bbt_value_t lvalue;
		int fresult;
		key=0;
		lvalue=0;
		fresult=bbt_first(table,&key,&lvalue);
		printf("first found=%d key=%ld lvalue=%ld\n",fresult,key,lvalue);
	}
#endif
}

/*
 * delete a btree item for testing
 */
msn_PRIVATE_PROC msn_VOID bbt_xdelete(table,key)
	bbt_table_pt table;
	bbt_key_t key;
{
	int was_present;
	printf("delete key %ld\n",key);
	was_present=bbt_delete(table,key);
	printf("was_present %d\n",was_present);
	// bbt_dump(table->bbt_root,0);
	if(!was_present) {
		printf("xdelete test couldn't find key %l\n",key);
		exit(1);
	}
	bbt_check(table);
return;
	{
		char buf[1000];
		gets(buf);
	}
}


static int test_data[]={
	1,10,100,50,150,74,120,5,7,0,3,11,15,-1,-10,-2
};
static int test_data_size=sizeof(test_data)/sizeof(test_data[0]);

void main()
{
	bbt_table table;
	int i;
	printf("bbt_test\n");
	bbt_init_bbt(&table,aloc_mem,free_mem,2);
	bbt_dump(table.bbt_root,0);
	for(i=0;i<test_data_size;i++)
		bbt_xinsert(&table,(bbt_key_t)test_data[i]);
	for(i=0;i<test_data_size;i++)
		bbt_xdelete(&table,(bbt_key_t)test_data[i]);

}