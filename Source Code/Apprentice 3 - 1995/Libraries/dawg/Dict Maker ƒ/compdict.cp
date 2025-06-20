//
// compdict - a program to construct a dictionary file for scrabble
//
// See the file dawg.h for the format of the dawg data structure.
//
// The wordlist MUST be in the form ([a-z]*\n)* and in STRICTLY
// increasing lexicographic order, or compdict will weird out
// without giving a proper error message.
//
#include "small_alloc.h"
#include "dawg.h"
#include "compdict.h"

extern short	gOutFile;
extern Rect		gStringRect;

char stringbuf[ 80];	// space for current string
char *string;			// marks END of current string
char *input;
char *endofinput;

unsigned long nodesused  = 1;
unsigned long edgesused  = 1;
unsigned long nodessaved = 0;
unsigned long edgessaved = 0;

unsigned long words_processed = 0;

node **table;
//
// The edges of a node structure are stored in the numedges following words of memory
//
unsigned long hashsize;

void do_it( char *the_input, unsigned long size)
{	
	input = the_input;
	endofinput = input + size;
	//
	// Allocate the hash table
	//
	hashsize = size >> kScaleBits;
	table = (struct node **)NewPtrClear( hashsize * sizeof( struct node *));
	//
	// last node of the dawg acts is the only 'zero' node
	//
	unsigned long new_node = 0;
	long count = sizeof( new_node);
	(void)FSWrite( gOutFile, &count, &new_node);
	//
	// Call makenode with null (relative to stringbuf) prefix;
	// Initialize string to null; Put index of start node on output
	//
	string = stringbuf;
	new_node = makenode( string);
	
	new_node |= FIRST;
	new_node |= LAST;
	count = sizeof( new_node);
	(void)FSWrite( gOutFile, &count, &new_node);
}
//
// Makenode takes a prefix (as position relative to stringbuf) and
// returns an index of the start node of a dawg that recognizes all
// words beginning with that prefix.  String is a pointer (relative
// to stringbuf) indicating how much of prefix is matched in the
// input.
//
unsigned long makenode( char *prefix)
{
	//
	// when edges is declared as 'unsigned long edges[ 256]' the program
	// crashes on a Mac with 'stack ran into heap' (at least when 'MaxApplZone'
	// gets called in the beginning of the program _and_ no special precautions
	// (i.e. calling of SetApplLimit) have been taken). We solved this problem
	// by not calling 'MaxApplZone' during Macintosh toolbox initialization.
	//
	// As an alternative
	//		unsigned long *edges = new unsigned long[ 256];
	// could be used with a 'delete' just before the return statement at the end
	// of this function.
	//
	unsigned long edges[ 256];
	unsigned long *edge = edges;

	while( prefix == string)				// More edges out of node
	{
		*edge++ = (*string++ = *input++);	// read char of input into string
		
		if( *input == 0)	// end of a word
		{
			edge[ -1] |= WORD;				// mark edge as being a word
			*string++ = *input++;			// Skip end of word
			if( input == endofinput)		// At end of input?
			{
				break;
			}
			//
			// skip part of next word, for as far as it equals the prefix of the
			// current word.
			//
			for( string = stringbuf; *string++ == *input++;){};
			--string; // Reset string
			--input;
			
			words_processed += 1;
			if( (words_processed % 20) == 0)
			{
				Str255 str;
				NumToString( words_processed, str);
				TETextBox( &str[ 1], str[ 0], &gStringRect, teJustRight);
			}
		}
		edge[ -1] |= makenode( prefix + 1);	// make dawg pointed to by this edge
	}
	const int numedges = edge - edges;
	
	if( numedges == 0)
	{
		return 0;			// Special node zero - no edges
	}
	edges[ 0] |= FIRST;		// Mark the first edge
	edge[ -1] |= LAST;		// Mark the last edge

	struct node *current_node = hash( edges, numedges);	// Look up node in hash table
	
	if( current_node->index)			// same as an existing node
	{
		edgessaved += numedges;
		nodessaved += 1;
	} else {				// a new node
		long count = sizeof( edge) * numedges;
		current_node->index = edgesused;	// enter node's index into table
		edgesused += numedges;
		nodesused += 1;
		//
		// Output the edges of this node
		//
		(void)FSWrite( gOutFile, &count, edges);
	}
	return (current_node->index) << INDEX;
}
//
//	Hash takes an array of edges (with a count) and maps it to a pointer
//	to a node, filling in the edge info if needed.  It uses simple
//	bucket chaining to keep a list of all nodes which hash to the same key.
//
struct node *hash( unsigned long *theEdges, int numedges)
{
	//
	// Cheesy signature method
	//
	unsigned long key = 0;
	for( int i = 0; i < numedges; i++)
	{
		key ^= (key << 3) ^ (key >> 1) ^ theEdges[ i];
	}
	key %= hashsize;
	//
	// Look for identical node in hash table
	//
	for( struct node *slot = table[ key]; slot; slot = slot->next)
	{
		if( numedges == slot->numedges)
		{
			unsigned long *slot_edges = (unsigned long *) (slot + 1);
			for( unsigned long i = 0; i < numedges; i++)
			{
				if( slot_edges[ i] != theEdges[ i])	// Some edge is different
				{
					break;
				}
			}
			if( i == numedges)	// all edges were the same
			{
				return slot;	// just return a pointer
			}
		}
	}
	//
	// Found an empty position; allocate and copy in edge info
	//
	slot = (struct node *) small_alloc(
				sizeof( struct node) + numedges * sizeof( unsigned long));
				
	slot->numedges = numedges;

	unsigned long *slot_edges = (unsigned long *)(slot + 1);

	for( i = 0; i < numedges; i++)
	{
		slot_edges[ i] = theEdges[ i];
	}
	//
	// Link this node into the bucket
	//
	slot->next = table[ key];
	table[ key] = slot;
	return slot;
}
