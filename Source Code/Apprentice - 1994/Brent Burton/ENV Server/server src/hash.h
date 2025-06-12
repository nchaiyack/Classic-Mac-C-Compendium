/********************************************************
*                                                       *
*  hash.h                                               *
*                                                       *
*  header for the hash table routines.                  *
*  The only module that                                 *
*  should be using this file is the symbol.[ch] module. *
*                                                       *
********************************************************/

#ifndef HASH_H
#define HASH_H


/********************************************/
/* macros for sizes and errors **************/
/********************************************/
#define HASH_SIZE   211

#define H_NOERR       0
#define H_ERROR       1

typedef struct _hash_node {
  char	*name;                /* environment variable name */
  char  *value;               /* environment variable value */
  int	key;                  /* search key of this item    */
  struct _hash_node *next_node;
} HashNode, *HashNodePtr;


typedef HashNodePtr Buckets[HASH_SIZE];

typedef struct _hash_table {
	int         last_key;    /* info for last node accessed/retrieved */
	HashNodePtr last_node;
	Buckets     buckets;
} HashTable, *HashTablePtr;

/******************
**   functions
******************/

int          hash_add_entry( char*);   /* returns F/T for success/failure */
HashNodePtr  hash_get_entry( char*);   /* fetch the info */

#endif /* HASH_H */




