#include "defs.h"


/* TABLE_SIZE is the number of entries in the symbol table. */
/* TABLE_SIZE must be a power of two.			    */

#define	TABLE_SIZE 1024


bucket **symbol_table;
bucket *first_symbol;
bucket *last_symbol;


int
hash(name)
char *name;
{
    register char *s;
    register int c, k;

    assert(name && *name);
    s = name;
    k = *s;
    while (c = *++s)
	k = (31*k + c) & (TABLE_SIZE - 1);

    return (k);
}


bucket *
make_bucket(name)
char *name;
{
    register bucket *bp;

    assert(name);
    bp = (bucket *) MALLOC(sizeof(bucket));
    if (bp == 0) no_space();
    bp->link = 0;
    bp->next = 0;
    bp->name = MALLOC(strlen(name) + 1);
    if (bp->name == 0) no_space();
    bp->tag = 0;
    bp->value = UNDEFINED;
    bp->index = 0;
    bp->prec = 0;
    bp-> class = UNKNOWN;
    bp->assoc = TOKEN;

//    if (bp->name == 0) no_space();
    strcpy(bp->name, name);

    return (bp);
}


bucket *
lookup(name)
char *name;
{
    register bucket *bp, **bpp;

    bpp = symbol_table + hash(name);
    bp = *bpp;

    while (bp)
    {
	if (strcmp(name, bp->name) == 0) return (bp);
	bpp = &bp->link;
	bp = *bpp;
    }

    *bpp = bp = make_bucket(name);
    last_symbol->next = bp;
    last_symbol = bp;

    return (bp);
}


create_symbol_table()
{
    register int i;
    register bucket *bp;

    symbol_table = (bucket **) MALLOC(TABLE_SIZE*sizeof(bucket *));
    if (symbol_table == 0) no_space();
    for (i = 0; i < TABLE_SIZE; i++)
	symbol_table[i] = 0;

    bp = make_bucket("error");
    bp->index = 1;
    bp->class = TERM;

    first_symbol = bp;
    last_symbol = bp;
    symbol_table[hash("error")] = bp;
}


free_symbol_table()
{
    FREE(symbol_table);
    symbol_table = 0;
}


free_symbols()
{
    register bucket *p, *q;

    for (p = first_symbol; p; p = q)
    {
	q = p->next;
	FREE(p);
    }
}


#if 0


// kTableLength is the number of entries in the symbol table.
// kTableLength must be a power of two.

enum {
	kTableLength	= 1024,
	kTableSize		= kTableLength * sizeof(CBucket*)
};

CBucket**	symbol_table;
CBucket*	first_symbol;
CBucket*	last_symbol;


char*
strdup (const char* src)
{
	char* dst = MALLOC(strlen(src) + 1);
	strcpy(dst, src);
	return dst;
}


static int
Hash (const char* name)
{
	assert(name != NULL && *name);
	const char* s = name;
	int k = *s;

	register int c;
	while (c = *++s)
		k = (31 * k + c) & (kTableLength - 1);

	return k;
}


CBucket::CBucket (const char* name)
{
	assert(name != NULL);
	fLink  =
	fNext  = NULL;
	fName  = strdup(name);
	fTag   = 0;
	fValue = UNDEFINED;
	fIndex = 0;
	fPrec  = 0;
	fClass = UNKNOWN;
	fAssoc = TOKEN;
}


CBucket*
Lookup (const char* name)
{
	CBucket** bpp = &symbol_table[Hash(name)];
	CBucket* bp = *bpp;

	while (bp) {
		if (strcmp(name, bp->fName) == 0)
			return bp;
		bpp = &bp->fLink;
		bp = *bpp;
	}

	*bpp = bp = new CBucket(name);
	last_symbol->fNext = bp;
	last_symbol = bp;

	return bp;
}


void
create_symbol_table (void)
{
    symbol_table = (CBucket**) CALLOC(kTableSize);

    CBucket* bp = new CBucket("error");
    bp->fIndex = 1;
    bp->fClass = TERM;

    first_symbol =
    last_symbol  =
    symbol_table[Hash("error")] = bp;
}


void
free_symbol_table (void)
{
    FREE(symbol_table);
    symbol_table = NULL;
}


void
free_symbols (void)
{
    for (CBucket* bp = first_symbol; bp; ) {
		CBucket* q = bp->fNext;
		delete bp;
		bp = q;
    }
}


#endif
