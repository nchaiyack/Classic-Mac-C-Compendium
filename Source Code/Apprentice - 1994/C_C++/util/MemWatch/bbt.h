/*
 * general purpose in memory btree interface by Aaron Wohl (aw0g@andrew.cmu.edu)
 * public domain no rights reserved
 */

#ifndef _H_bbt
#define _H_bbt

#ifndef _H_bbt_comp
#include "bbt_comp.h"
#endif _H_bbt_comp

/*
 * type of a btree key (opaque)
 */
typedef msn_VOID_PTR bbt_key_t;


/*
 * type of a btree value (opaque)
 */
typedef msn_VOID_PTR bbt_value_t;

/*
 * a btree item containing a key and value pair
 */
struct bbt_ent_R {
  bbt_key_t bbt_key;			/*key to look up this datum*/
  union {						/*value of item*/
 	struct bbt_node_R *bbt_down; /*pointer to lower level node if not a leaf*/
    bbt_value_t bbt_value;		/*value if leaf and value is pointer*/
    long long_bbt_value;		/*value if leaf and value is number*/
  }x;
};
typedef struct bbt_ent_R bbt_ent;
typedef bbt_ent *bbt_ent_pt;

/*
 * one node in a btree
 * NOTE: actual length has room for bbt_branch items
 */
struct bbt_node_R {
  msn_uint16	bbt_size;		/*count of entries here*/
  msn_uint16	bbt_isleaf;		/*true if this is a leaf node*/
  bbt_ent		bbt_items[1];
};
typedef struct bbt_node_R bbt_node;
typedef bbt_node *bbt_node_pt;

/*
 * amount of space in bytes needed for a given branching factor
 */
#define BBT_NODE_SIZE(BBT_BRANCH) \
	(sizeof(bbt_node)+(sizeof(bbt_ent)*(BBT_BRANCH-1)))

/*
 * compare btree entries
 * returns:
 * -1 if key1<key2
 *  0 if key1==key2
 *  1 if key1>key2
 */
typedef int (*bbt_compare_t)msn_ANSI_ARG((bbt_key_t key1,bbt_key_t key2,struct bbt_table_R*));

/*
 * an btree space allocator
 */
typedef msn_VOID_PTR (*bbt_alloc_proc_t)msn_ANSI_ARG((long));

/*
 * an btree space free
 */
typedef msn_VOID (*bbt_freeor_t)msn_ANSI_ARG((msn_VOID_PTR));

/*
 * an entire btree
 */
struct bbt_table_R {
  long		bbt_mem_blk_size;	/*size in bytes of a node block*/
  long		bbt_min_degree;		/*minimum degree of a non root node*/
  long      bbt_branch;			/*max number of branches in each node*/
  long		bbt_tsize;			/*number of items in table*/
  long		bbt_depth;			/*depth of tree*/
  bbt_node_pt  bbt_root;		/*root of btree*/
  bbt_compare_t	bbt_compare;	/*routine to compare entries*/
  bbt_alloc_proc_t bbt_allocz;	/*storage allocator*/
  bbt_freeor_t	bbt_free;		/*free routine*/
};
typedef struct bbt_table_R bbt_table,*bbt_table_pt;

/*
 * for a given number of bytes determine wich 
 * bbt_min_degree should be used to fill up a block this
 * size
 */
msn_PUBLIC_PROC long bbt_find_min_degree(long mem_size);

/*
 * lookup a key in a btree
 * returns true if an item is found
 */
msn_PUBLIC_PROC int bbt_find msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_key_t key,
	bbt_value_t *fvalue));

/*
 * insert an item into a btree leaf
 * can also be used to replace a value field of ane existing entry
 */
msn_PUBLIC_PROC msn_VOID bbt_insert msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_key_t key,
	bbt_value_t fvalue));

/*
 * remove a key from a btree
 * returns true iff key was present
 */
msn_PUBLIC_PROC int bbt_delete msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_key_t key));

/*
 * find the next item in a btree after this_key
 * returns true iff an item was found
 */
msn_PUBLIC_PROC int bbt_next msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_key_t this_key,
	bbt_key_t *next_key,
	bbt_value_t *next_value));

/*
 * find the prevous item in a btree after this_key
 * returns true iff an item was found
 */
msn_PUBLIC_PROC int bbt_prev msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_key_t this_key,
	bbt_key_t *next_key,
	bbt_value_t *next_value));

/*
 * initialize an arbitrary bbt table
 * a default compare routine is installed for keys of type long
 */
msn_PUBLIC_PROC msn_VOID bbt_init_bbt msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_alloc_proc_t allocator,
	bbt_freeor_t freeor,
	long min_degree));

/*
 * initialize a bbt table for ascii strings
 */
msn_PUBLIC_PROC msn_VOID bbt_init_ascii_bbt msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_alloc_proc_t allocator,
	bbt_freeor_t freeor,
	long min_degree));

/*
 * compare keys which are really longs
 */
msn_PUBLIC_PROC int bbt_compare_longs(long key1,long key2,bbt_table_pt table);

/*
 * return first key in table
 * returns true iff key was present
 */
msn_PUBLIC_PROC int bbt_first msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_key_t *key,
	bbt_value_t *fvalue));

/*
 * return last key in table
 * returns true iff key was present
 */
msn_PUBLIC_PROC int bbt_last msn_ANSI_ARG((
	bbt_table_pt table,
	bbt_key_t *key,
	bbt_value_t *fvalue));

/*
 * check a bbt tree see if it meets all the constraints
 */
msn_PUBLIC_PROC void bbt_check msn_ANSI_ARG((
	bbt_table_pt table));
#endif
