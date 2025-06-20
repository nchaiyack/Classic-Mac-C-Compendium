/***************************************************
*                                                  *
*  hash.c                                          *
*                                                  *
*  This contains all the routines to manage the    *
*  hash table.                                     *
*                                                  *
***************************************************/

#include <MacHeaders>
#include <string.h>
#include "hash.h"


#define DEBUG_HASH 0               /* != 0 for debugging purposes */

static int hash_add_node(HashNodePtr);
static int hash_get_key(char *);

static HashTablePtr gTable=NULL;

/**********************************************************
*                                                         *
*  hash_add_entry()                                       *
*                                                         *
*  This routine accepts the table to add a new node to.   *
*  The node's name, type, and line number are also passed *
*  here.                                                  *
*                                                         *
**********************************************************/
int hash_add_entry(char *name)
{
  HashNodePtr node;
  char *eqp;    /* ptr to equals sign/string after '=' sign */
  
  if ( (eqp = strchr(name, '=')) == NULL)
    return H_ERROR;
  node = (HashNodePtr) NewPtr( sizeof( HashNode));
  if (node == NULL)
    return H_ERROR;
  
  *eqp++ = '\000';   /* change '=' to string terminator; expr is divided now */
  node->name  = NewPtrClear( strlen(name)+1);
  if (node->name == NULL)
  {
  	DisposPtr( (Ptr)node);
  	return(H_ERROR);
  }
  node->value = NewPtrClear( strlen(eqp)+1);
  if (node->value == NULL)
  {
  	DisposPtr (node->name);
  	DisposPtr ((Ptr)node);
  	return(H_ERROR);
  }
  strcpy( node->name, name);
  strcpy( node->value, eqp);
  node->key = hash_get_key(node->name);
  node->next_node = NULL;
  return hash_add_node( node);
} /* hash_add_entry */


/******************************************************************
*                                                                 *
*  hash_add_node()                                                *
*                                                                 *
*  This routine accepts a pointer to a hash table and a pointer   *
*  to a hash table node and adds that node to the table.          *
*  Entries are stored in the buckets in an alphabetic order(A-Z). *
*                                                                 *
******************************************************************/
static int hash_add_node(HashNodePtr node)
{
  HashNodePtr here, last;
  
  if (gTable == NULL)
    gTable = (HashTablePtr)NewPtrClear(sizeof( HashTable));
  here = gTable->buckets[node->key];
  last=NULL;
  while (here != NULL)
  {
    int order = strcmp(here->name, node->name);
    
    if (order < 0)
    {
    	last = here;
    	here = here->next_node;
    }
    else
    {
    	if (order == 0)   /* then it's already in the table; redefine it */
    	{
    		node->next_node = here->next_node;
    		if (last == NULL)
    			gTable->buckets[node->key] = node;
    		else
    			last->next_node = node;
    		
    		DisposPtr(here->name);      /* delete the old definition */
    		DisposPtr(here->value);
    		DisposPtr((Ptr)here);
    		return(H_NOERR);
    	}
    	else
    		break;
    }
  } /* while */
  if (last == NULL)
  {
    node->next_node = gTable->buckets[node->key];
    gTable->buckets[node->key] = node;
  }
  else  /** put node after last if last != NULL */
  {
    node->next_node = last->next_node;
    last->next_node = node;
  }
  
  /** save the last referenced node in the table info for a quick lookup **/
  gTable->last_key  = node->key;
  gTable->last_node = node;
  return(H_NOERR);
} /* hash_add_node */


/***************************************************************
*                                                              *
*  hash_get_entry()                                            *
*                                                              *
*  This routine fetches the node that contains name from the   *
*  table pointed to by table.                                  *
*  It returns the pointer to the node if name is in the table. *
*  Otherwise, it returns NULL if name cannot be found.         *
*                                                              *
***************************************************************/
HashNodePtr hash_get_entry(char *name)
{
  HashNodePtr here;
  int key;
  
  key = hash_get_key(name);
  
  /** before we search the buckets, search the last accessed node. **/
  if (gTable->last_key == key)
    if ((gTable->last_node != NULL) && !strcmp(gTable->last_node->name, name))
      /** then the last accessed node is the one we want **/
      return(gTable->last_node);
  /** else search the buckets */

  here = gTable->buckets[key];
  while ((here!= NULL) && strcmp(here->name, name))
  {
    here = (HashNodePtr)here->next_node;
  }
  if (here != NULL)  /* then save this node in the last accessed fields */
  { gTable->last_node = here;
    gTable->last_key = key;
  }
  return(here);
} /* hash_get_entry */


/*****************************************************************
*                                                                *
*  hash_get_key()                                                *
*                                                                *
*  This routine calculates the hashing key for the string passed *
*  to it.  It returns the key.                                   *
*  This hashing algorithm is from Aho, Sethi, Ullman - Compilers *
*  Principles, Techniques and Tools  (The dragon book).          *
*                                                                *
*****************************************************************/
static int hash_get_key(char *s)
{
  char *p;
  unsigned long h=0, g;
  for (p=s; *p != '\0'; p++)
  {
    h = (h << 4) + *p;
    if ( g = h&0xf0000000)
    {
      h = h^(g>>12);
      h = h^g;
    }
  }
  return( h % HASH_SIZE);
} /* hash_get_key */


