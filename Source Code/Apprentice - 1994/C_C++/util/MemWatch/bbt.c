#include "bmminclude.h"
/*
 * general purpose in memory btree interface
 * by Aaron Wohl (n3liw+@cmu.edu)
 * public domain no rights reserved
 */


#include "bbt.h"

#define STRINGIFY(xx_arg) #xx_arg

#ifdef THINK_C
#pragma options(!profile)

#define bbt_ASSERT(xx_eqn) \
	do { \
		if(!(xx_eqn)) { \
			 DebugStr("\poh barf assertion failed " STRINGIFY(xx_eqn)); \
		} \
	} while (0)
#endif

/*
 * compare each key foind with a depth first tree walk
 * each key should be greater or equal to the previous one
 */
msn_PRIVATE_PROC msn_VOID bbt_check_key(table,last_key,this_key)
	bbt_table_pt table;
	bbt_key_t **last_key;
	bbt_key_t this_key;
{
	if((*last_key)!=0)
		bbt_ASSERT(((*table->bbt_compare)(*last_key,this_key,table))<=0);
	(*last_key)=this_key;
}

/*
 * check a bbt tree see if it meets all the constraints
 */
msn_PRIVATE_PROC msn_VOID bbt_check_internal(table,anode,depth,nfound,last_key)
	bbt_table_pt table;
	bbt_node_pt anode;
	int depth;
	long *nfound;
	bbt_key_t **last_key;
{
	int i;
	if(anode!=table->bbt_root)
		bbt_ASSERT(anode->bbt_size>=table->bbt_min_degree);
	bbt_ASSERT(anode->bbt_size<=table->bbt_branch);
	if(anode->bbt_isleaf) {
		bbt_ASSERT(depth==table->bbt_depth);
		(*nfound)+=anode->bbt_size;
		for(i=0;i<anode->bbt_size;i++)
			bbt_check_key(table,last_key,anode->bbt_items[i].bbt_key);
	} else
		for(i=0;i<anode->bbt_size;i++) {
			bbt_check_internal(table,anode->bbt_items[i].x.bbt_down,depth+1,nfound,last_key);
			if(i<(anode->bbt_size-1))
				bbt_check_key(table,last_key,anode->bbt_items[i].bbt_key);
		}
}

/*
 * check a bbt tree see if it meets all the constraints
 */
msn_PUBLIC_PROC void bbt_check(table)
	bbt_table_pt table;
{
	long nfound=0;
	bbt_key_t *last_key=0;
	bbt_ASSERT(table->bbt_branch==(2*table->bbt_min_degree-1));
	bbt_ASSERT(table->bbt_root!=0);
	bbt_ASSERT(table->bbt_compare!=0);
	bbt_ASSERT(table->bbt_allocz!=0);
	bbt_ASSERT(table->bbt_free!=0);
	bbt_check_internal(table,table->bbt_root,0,&nfound,&last_key);
	bbt_ASSERT(nfound==table->bbt_tsize);
}

/*
 * compare keys which are really longs
 */
msn_PUBLIC_PROC int bbt_compare_longs(key1,key2,table)
	long key1;
	long key2;
	bbt_table_pt table;
{
	if(key1<key2)return -1;
	if(key1>key2)return 1;
	return 0;
}

/*
 * allocate a new node
 * it defaults to being a leaf
 */
msn_PRIVATE_PROC bbt_node_pt bbt_new_node(table)
	bbt_table_pt table;
{
	bbt_node_pt result=(*table->bbt_allocz)(table->bbt_mem_blk_size);
	result->bbt_isleaf=TRUE;
	return result;
}

/*
 * initialize a bbt table for ascii strings
 */
msn_PUBLIC_PROC msn_VOID bbt_init_ascii_bbt(table,allocator,freeor,min_degree)
	bbt_table_pt table;
	bbt_alloc_proc_t allocator;
	bbt_freeor_t freeor;
	long min_degree;
{
	bbt_init_bbt(table,allocator,freeor,min_degree);
	table->bbt_compare=(bbt_compare_t)&strcmp;
}

/*
 * initialize an arbitrary bbt table
 * a default compare routine is installed for keys of type long
 */
msn_PUBLIC_PROC msn_VOID bbt_init_bbt(table,allocator,freeor,min_degree)
	bbt_table_pt table;
	bbt_alloc_proc_t allocator;
	bbt_freeor_t freeor;
	long min_degree;
{
  bbt_ASSERT(table!=0);
  memset(table,0,sizeof(*table));
  bbt_ASSERT(allocator!=0);
  table->bbt_allocz=allocator;
  bbt_ASSERT(freeor!=0);
  table->bbt_free=freeor;
  bbt_ASSERT(min_degree>=2);
  table->bbt_min_degree=min_degree;
  table->bbt_branch=2*min_degree-1;
  table->bbt_mem_blk_size=BBT_NODE_SIZE(table->bbt_branch);
  /*fill in a default compare routine for longs*/
  table->bbt_compare=(bbt_compare_t)&bbt_compare_longs;
  table->bbt_root=bbt_new_node(table);
}

/*
 * find the position of the passed key in this node, or where it would go
 * returns TRUE iff found
 * sets idx to position item should be in node
 */
msn_PRIVATE_PROC int bbt_find_in_node(anode,key,idx,table)
	bbt_node_pt anode;
	bbt_key_t key;
	msn_uint16 *idx;
	bbt_table_pt table;
{
	int i;
	int nsearch=anode->bbt_size;
	if(!anode->bbt_isleaf)
		nsearch--;		/*no key to check for last pointer*/
	for(i=0;i<nsearch;i++) {
		bbt_ent_pt here= &anode->bbt_items[i];
		int cval= (*table->bbt_compare)(here->bbt_key,key,table);
		(*idx)=i;
		if(cval==0)
			return 0;
		if(cval>0)
			return 1;
	}
	(*idx)=i;
	return 1;
}

/*
 * lookup a key in a btree
 * returns true if an item is found
 */
msn_PUBLIC_PROC int bbt_find(table,key,fvalue)
	bbt_table_pt table;
	bbt_key_t key;
	bbt_value_t *fvalue;
{
	bbt_node_pt this_node=table->bbt_root;
	msn_uint16 idx;
	int found;
	while(TRUE) {
		bbt_ASSERT(this_node!=0);
		found=bbt_find_in_node(this_node,key,&idx,table)==0;
		if(idx<table->bbt_branch)
			(*fvalue)=this_node->bbt_items[idx].x.bbt_value;
		if(this_node->bbt_isleaf)
			return found;
		this_node=this_node->bbt_items[idx].x.bbt_down;
	}
}

/*
 * return first key in this tree
 * returns true iff key was present
 */
msn_PRIVATE_PROC int bbt_first_internal(anode,key,fvalue)
	bbt_node_pt anode;
	bbt_key_t *key;
	bbt_value_t *fvalue;
{
	while(!anode->bbt_isleaf)
		anode=anode->bbt_items[0].x.bbt_down;
	if(anode->bbt_size==0)		/*any item here*/
		return FALSE;			/*no, tree is empty*/
	*key= anode->bbt_items[0].bbt_key;
	if(fvalue!=0)
		*fvalue= anode->bbt_items[0].x.bbt_value;
	return TRUE;
}

/*
 * return first key in table
 * returns true iff key was present
 */
msn_PUBLIC_PROC int bbt_first(table,key,fvalue)
	bbt_table_pt table;
	bbt_key_t *key;
	bbt_value_t *fvalue;
{
	bbt_first_internal(table->bbt_root,key,fvalue);
}

/*
 * return last key in this tree
 * returns true iff key was present
 */
msn_PRIVATE_PROC int bbt_last_internal(anode,key,fvalue)
	bbt_node_pt anode;
	bbt_key_t *key;
	bbt_value_t *fvalue;
{
	bbt_ASSERT(anode!=0);
	while(!anode->bbt_isleaf) {
		bbt_ASSERT(anode->bbt_size>0);
		anode=anode->bbt_items[anode->bbt_size-1].x.bbt_down;
		bbt_ASSERT(anode!=0);
	}
	if(anode->bbt_size==0)		/*any item here*/
		return FALSE;			/*no, tree is empty*/
	*key= anode->bbt_items[anode->bbt_size-1].bbt_key;
	if(fvalue!=0)
		*fvalue= anode->bbt_items[anode->bbt_size-1].x.bbt_value;
	return TRUE;
}

/*
 * return last key in table
 * returns true iff key was present
 */
msn_PUBLIC_PROC int bbt_last(table,key,fvalue)
	bbt_table_pt table;
	bbt_key_t *key;
	bbt_value_t *fvalue;
{
	bbt_last_internal(table->bbt_root,key,fvalue);
}

/*
 * lookup this_key then return the next or prevous item from where it is or would be
 */
msn_PRIVATE_PROC int bbt_step_internal(table,this_node,this_key,next_key,next_value,direction)
	bbt_node_pt this_node;
	bbt_table_pt table;
	bbt_key_t this_key;
	bbt_key_t *next_key;
	bbt_value_t *next_value;
	int direction;
{
	msn_uint16 idx;
	int found;
	bbt_ASSERT(this_node!=0);
	found=bbt_find_in_node(this_node,this_key,&idx,table)==0;
	if(this_node->bbt_isleaf) {
		if(found)			/*exact match so skip to following item*/
			idx+=direction;
		if((idx>=this_node->bbt_size)||(idx<0)) /*is following item present in this leaf*/
			return FALSE;	/*no, depend on call from parent to find next leaf item*/
		*next_key= this_node->bbt_items[idx].bbt_key;
		if(next_value!=0)	/*if are interested in value of next item*/
			*next_value= this_node->bbt_items[idx].x.bbt_value;
		return TRUE;
	}
	if(bbt_step_internal(table,this_node->bbt_items[idx].x.bbt_down,this_key,next_key,next_value,direction))
		return TRUE;
	idx+=direction;
	if((idx>=this_node->bbt_size)||(idx<0))
		return FALSE;
	if(direction>0)
		return bbt_first_internal(this_node->bbt_items[idx].x.bbt_down,next_key,next_value);
	else
		return bbt_last_internal(this_node->bbt_items[idx].x.bbt_down,next_key,next_value);
}

/*
 * find the next item in a btree after this_key
 * returns true iff an item was found
 */
msn_PUBLIC_PROC int bbt_next(table,this_key,next_key,next_value)
	bbt_table_pt table;
	bbt_key_t this_key;
	bbt_key_t *next_key;
	bbt_value_t *next_value;
{
	return bbt_step_internal(table,table->bbt_root,this_key,next_key,next_value,1);
}

/*
 * find the prevous item in a btree after this_key
 * returns true iff an item was found
 */
msn_PUBLIC_PROC int bbt_prev(table,this_key,next_key,next_value)
	bbt_table_pt table;
	bbt_key_t this_key;
	bbt_key_t *next_key;
	bbt_value_t *next_value;
{
	return bbt_step_internal(table,table->bbt_root,this_key,next_key,next_value,-1);
}

/*
 * insert an item into a btree leaf
 */
msn_PRIVATE_PROC int bbt_insert_no_split(table,key,fvalue,anode,idx)
	bbt_table_pt table;
	bbt_key_t key;
	bbt_value_t fvalue;
	bbt_node_pt anode;
	int idx;
{
	int j;
	for(j=anode->bbt_size-1;j>=idx;j--)
		anode->bbt_items[j+1]=anode->bbt_items[j];
	anode->bbt_items[idx].bbt_key=key;
	anode->bbt_items[idx].x.bbt_value=fvalue;
	anode->bbt_size++;
	if(anode->bbt_isleaf)
		table->bbt_tsize++;
	return FALSE;
}

/*
 * return the last key in anode
 */
msn_PRIVATE_PROC bbt_key_t *bbt_max_key(anode)
	bbt_node_pt anode;
{
	return &anode->bbt_items[anode->bbt_size-1].bbt_key;
}

/*
 * insert an item into a btree node that needs to be split
 */
msn_PRIVATE_PROC int bbt_insert_split(table,add_to_me,anode,idx,add_to_parent)
	bbt_table_pt table;
	bbt_ent_pt add_to_me;
	bbt_node_pt anode;
	int idx;
	bbt_ent_pt add_to_parent;
{
	bbt_node_pt new_left=bbt_new_node(table);
	int src;
	int asrc;
	bbt_ent_pt src_pt;
	bbt_ent_pt dst_pt;
	new_left->bbt_isleaf=anode->bbt_isleaf;
	new_left->bbt_size=table->bbt_min_degree;
	anode->bbt_size=table->bbt_min_degree;
	/* copy items from source to split up node */
	asrc=0;
	dst_pt=new_left->bbt_items;
	for(src=0;src<=table->bbt_branch;src++) {
		/*intermix intem to insert in ourselfs at the right point in source stream*/
		if(src!=idx)
			src_pt= &anode->bbt_items[asrc++];
		else
			src_pt=add_to_me;
		if(src==table->bbt_min_degree)
			dst_pt=anode->bbt_items;
		*dst_pt++ = *src_pt;
	}
	add_to_parent->x.bbt_down=new_left;
	{
		/*since new_left was just created and it's rightmost ent is known to have*/
		/*a valid key, we get it here to pass up to our parent*/
		bbt_key_t *max=bbt_max_key(new_left);
		add_to_parent->bbt_key= (*max);
		if(new_left->bbt_isleaf)
			table->bbt_tsize++;
		else
			*max=0;				/*for debugging clear unused key in last node*/
	}
	return TRUE;
}

/*
 * is this node full?
 * bbt_size is the number of keys
 * leaf nodes have keys for each data item
 * non leaf nodes have keys "between" down pointers
 * that is there is an extra down pointer at the right
 * end without a key
 */
msn_PRIVATE_PROC int bbt_is_node_full(table,anode)
	bbt_table_pt table;
	bbt_node_pt anode;
{
	bbt_ASSERT((anode->bbt_size>=0)&&(anode->bbt_size<=table->bbt_branch));
	return anode->bbt_size==table->bbt_branch;
}

/*
 * insert an item into a btree leaf
 */
msn_PRIVATE_PROC int bbt_insert_internal(table,key,fvalue,anode,add_to_parent)
	bbt_table_pt table;
	bbt_key_t key;
	bbt_value_t fvalue;
	bbt_node_pt anode;
	bbt_ent_pt add_to_parent;
{
	msn_uint16 idx;
	int found=(bbt_find_in_node(anode,key,&idx,table)==0);
	bbt_ent add_to_me;
	int new_child;
	if(anode->bbt_isleaf) {
		if(found)	{				/*updating an existing value, no insert*/
			anode->bbt_items[idx].bbt_key=key;
			anode->bbt_items[idx].x.bbt_value=fvalue;
			return FALSE;
		}
		new_child=TRUE;
		add_to_me.bbt_key=key;
		add_to_me.x.bbt_value=fvalue;
	} else
		new_child=bbt_insert_internal(table,key,fvalue,anode->bbt_items[idx].x.bbt_down,&add_to_me);
	/*the key has been added to our children which may have split because of it*/
	/*we now add that item to ourselfs*/
	if(!new_child)		/*if nothing to add to ourselfs*/
		return FALSE;	/*we did not split*/
	if(bbt_is_node_full(table,anode))
		return bbt_insert_split(table,&add_to_me,anode,idx,add_to_parent);
	else
		return bbt_insert_no_split(table,add_to_me.bbt_key,add_to_me.x.bbt_value,anode,idx);
}

/*
 * insert an item into a btree leaf
 * can also be used to replace a value field of ane existing entry
 */
msn_PUBLIC_PROC msn_VOID bbt_insert(table,key,fvalue)
	bbt_table_pt table;
	bbt_key_t key;
	bbt_value_t fvalue;
{
	register bbt_node_pt oroot=table->bbt_root;
	bbt_ent add_to_me;
	bbt_ASSERT(oroot!=0);
	bbt_ASSERT(oroot->bbt_size>=0);
	bbt_ASSERT(oroot->bbt_size<=table->bbt_branch);
	if(bbt_insert_internal(table,key,fvalue,oroot,&add_to_me)) { /*split root*/
		bbt_node_pt nroot=bbt_new_node(table);
		nroot->bbt_isleaf=FALSE;
		nroot->bbt_size=2;
		nroot->bbt_items[0]=add_to_me;
		nroot->bbt_items[1].x.bbt_down=oroot;
		nroot->bbt_items[1].bbt_key=(bbt_key_t) 0; /*mark for debugging*/
		table->bbt_root=nroot;
		table->bbt_depth++;						/*tree is one level deeper*/
	}
}

/*
 * clear the right hand key of non leaf nodes for debugging
 */
msn_PRIVATE_PROC msn_VOID bbt_clear_right_key(anode)
	bbt_node_pt anode;
{
	if(anode->bbt_isleaf||(anode->bbt_size==0))
		return;
	anode->bbt_items[anode->bbt_size-1].bbt_key=0; /*mark last key for debugging*/
}
		
/*
 * temporarily set the rightmost key
 */
msn_PRIVATE_PROC msn_VOID bbt_set_right_key(anode)
	bbt_node_pt anode;
{
	bbt_key_t lkey;
	if(anode->bbt_isleaf||(anode->bbt_size==0))
		return;
	bbt_last_internal(anode,&lkey,0L);
	anode->bbt_items[anode->bbt_size-1].bbt_key=lkey;
}

#define BBT_LAST_KEY(xx_size) (((xx_size)==0)?0:(xx_size-1))

/*
 * fix the key to a child node
 */
msn_PRIVATE_PROC msn_VOID bbt_fix_key(anode,idx)
	bbt_node_pt anode;
	msn_uint16 idx;
{
	bbt_node_pt down=anode->bbt_items[idx].x.bbt_down;
	bbt_ASSERT(!anode->bbt_isleaf);
	if(idx==(anode->bbt_size-1))
		return;
	while(!down->bbt_isleaf)
		down=down->bbt_items[BBT_LAST_KEY(down->bbt_size)].x.bbt_down;
	anode->bbt_items[idx].bbt_key=down->bbt_items[BBT_LAST_KEY(down->bbt_size)].bbt_key;
}

/*
 * loose empty child
 */
msn_PRIVATE_PROC msn_VOID bbt_loose_empty_child(table,parent,sidx,small)
	bbt_table_pt table;
	bbt_node_pt parent;
	msn_uint16 sidx;
	bbt_node_pt small;
{
	(*table->bbt_free)(small);
	parent->bbt_size--;
	for(;sidx<parent->bbt_size;sidx++)
		parent->bbt_items[sidx]=parent->bbt_items[sidx+1];
	memset(&parent->bbt_items[parent->bbt_size],0,sizeof(bbt_ent));
}		

/*
 * rotate one item from src on right to dst on left
 */
msn_PRIVATE_PROC msn_VOID bbt_rotate_left(src,dst,cnt)
	bbt_node_pt src;
	bbt_node_pt dst;
	int cnt;
{
	int i;
	bbt_set_right_key(dst);
	for(i=0;i<cnt;i++)
		dst->bbt_items[dst->bbt_size++]=src->bbt_items[i];
	src->bbt_size-=cnt;
	for(i=0;i<src->bbt_size;i++)
		src->bbt_items[i]=src->bbt_items[i+cnt];
	bbt_clear_right_key(dst);
}

/*
 * rotate one item from src on left to dst on right
 */
msn_PRIVATE_PROC msn_VOID bbt_rotate_right(src,dst,cnt)
	bbt_node_pt src;
	bbt_node_pt dst;
	int cnt;
{
	int i;
	dst->bbt_size+=cnt;
	for(i=(dst->bbt_size-1);i>=cnt;i--)
		dst->bbt_items[i]=dst->bbt_items[i-cnt];
	bbt_set_right_key(src);
	for(i=0;i<cnt;i++)
		dst->bbt_items[i]=src->bbt_items[(src->bbt_size-cnt)+i];
	src->bbt_size-=cnt;
	bbt_clear_right_key(src);
}

/*
 * merge siblings to keep B+ tree minimum branching contstraints
 * if two nodes don't have enough data to make the minimum
 * then they must fit in one node
 * T = minimum branching
 * 2 = minimum value of T
 * 2*T-1 = maximum branching
 * 2T-1 = minimum size of two nodes one that is now two small
 * 3T-1 = upper bound on size of two nodes one of which is one too small
 * As two nodes 2T items are required if only 2T-1 are present then shuffle
 * them into one node with 2T-1 which is less than 2T (max branch) which is guarenteed to fit
 * If more thatn 2T-1 items are present then move ("rotate") one item from the larger node
 * to the smaller one
 */
msn_PRIVATE_PROC msn_VOID bbt_handle_shrinkage(table,parent,sidx)
	bbt_table_pt table;
	bbt_node_pt parent;
	msn_uint16 sidx;
{
	bbt_node_pt big;
	bbt_node_pt small;
	msn_uint16 bidx;
	int nrotate;
	bidx=sidx-1;
	if(sidx==0) /*child has no big sibling so use small*/
		bidx=1;
	bbt_ASSERT((sidx>=0)&&(sidx<parent->bbt_size));
	bbt_ASSERT((bidx>=0)&&(bidx<parent->bbt_size));
	big=parent->bbt_items[bidx].x.bbt_down;
	small=parent->bbt_items[sidx].x.bbt_down;
	bbt_ASSERT(big->bbt_size>0);
	/*normally last key is not filled in, make it valid since it will be in the middle*/
	/*or copied into the small node*/
	nrotate=1;
	if((big->bbt_size+small->bbt_size)<=table->bbt_branch)
		nrotate=big->bbt_size;
	if(sidx<bidx)
		bbt_rotate_left(big,small,nrotate);
	else
		bbt_rotate_right(big,small,nrotate);
	bbt_fix_key(parent,sidx);
	if(big->bbt_size<=0)
		bbt_loose_empty_child(table,parent,bidx,big);
	else
		bbt_fix_key(parent,bidx);
}

/*
 * remove a key from a btree
 * returns true iff key was present
 */
msn_PRIVATE_PROC int bbt_delete_internal(table,anode,key)
	bbt_table_pt table;
	bbt_node_pt anode;
	bbt_key_t key;
{
	msn_uint16 idx;
	bbt_node_pt child;
	int found=(bbt_find_in_node(anode,key,&idx,table)==0);
	if(anode->bbt_isleaf) {
		int i;
		if(!found)
			return FALSE;
		anode->bbt_size--;
		table->bbt_tsize--;
		for(i=idx;i<anode->bbt_size;i++)
			anode->bbt_items[i]=anode->bbt_items[i+1];
		return TRUE;
	}
	child=anode->bbt_items[idx].x.bbt_down;
	if(!bbt_delete_internal(table,child,key)) /*let child node delete item*/
		return FALSE;
	if(child->bbt_size>=table->bbt_min_degree)
		return TRUE;				/*this node meets the requirements*/
	/*one of our immediate children got to be too small*/
	/*we need to repackage the children to maintain B+ tree minimum*/
	/*node size requirements.*/
	bbt_handle_shrinkage(table,anode,idx);
	return TRUE;
}

/*
 * remove a key from a btree
 * returns true iff key was present
 */
msn_PUBLIC_PROC int bbt_delete(table,key)
	bbt_table_pt table;
	bbt_key_t key;
{
	register bbt_node_pt r;
	if(!bbt_delete_internal(table,table->bbt_root,key))
		return FALSE;
	/*something was deleted, clean up root if needed*/
	r=table->bbt_root;
	if(r->bbt_isleaf || (r->bbt_size>1))
		return TRUE;		/*node is valid as is*/
	table->bbt_root=r->bbt_items[0].x.bbt_down;
	table->bbt_depth--;
	(*table->bbt_free)(r);
	return TRUE;
}

/*
 * for a given number of bytes determine wich 
 * bbt_min_degree should be used to fill up a block this
 * size
 */
msn_PUBLIC_PROC long bbt_find_min_degree(long mem_size)
{
	mem_size-=BBT_NODE_SIZE(0);	/*subtrack off size of header*/
	mem_size/=sizeof(bbt_ent);	/*get number of branches*/
	mem_size=(mem_size+1)/2;	/*reverse computation of branch factor*/
	bbt_ASSERT(mem_size>=2);	/*must have at least branching of two*/
	return mem_size;
}

