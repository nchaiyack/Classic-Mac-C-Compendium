//
// Ratio of file size to hash table size (bitshifts)
//
#define kScaleBits	2

typedef struct node
{
	struct node *next;		// next node with same hash key
	unsigned long index;	// index of this node
	int numedges;			// number of edges
	//	short edges[];
} node;

unsigned long makenode( char *prefix);

unsigned long flength( char *filename);
char *snarf( char *filename, unsigned long length);

char *small_alloc( int nbytes);
struct node *hash( unsigned long *e, int n);

void do_it( char *input, unsigned long size);
