#include "defs.h"

extern short *itemset;
extern short *itemsetend;
extern unsigned *ruleset;

int nstates;
core *first_state;
shifts *first_shift;
reductions *first_reduction;

static core **state_set;
static core *this_state;
static core *last_state;
static shifts *last_shift;
static reductions *last_reduction;

static int nshifts;
static short *shift_symbol;

static short *redset;
static short *shiftset;

static short **kernel_base;
static short **kernel_end;
static short *kernel_items;

static void allocate_itemsets _P_((void));
static void allocate_storage _P_((void));
static void append_states _P_((void));
static void free_storage _P_((void));
static void generate_states _P_((void));
static int get_state _P_((int symbol));
static void initialize_states _P_((void));
static void new_itemsets _P_((void));
static core *new_state _P_((int symbol));
static void save_shifts _P_((void));
static void save_reductions _P_((void));
static void set_derives _P_((void));
static void set_nullable _P_((void));

#ifdef DEBUG
extern void print_derives _P_((void));
extern void show_cores _P_((void));
extern void show_ritems _P_((void));
extern void show_rrhs _P_((void));
extern void show_shifts _P_((void));
#endif

static void allocate_itemsets()
{
    register short *itemp;
    register short *item_end;
    register int symbol;
    register int i;
    register int count;
    register int max;
    register short *symbol_count;

    count = 0;
    symbol_count = NEW2(nsyms, short);

    item_end = ritem + nitems;
    for (itemp = ritem; itemp < item_end; itemp++)
    {
	symbol = *itemp;
	if (symbol >= 0)
	{
	    count++;
	    symbol_count[symbol]++;
	}
    }

    kernel_base = NEW2(nsyms, short *);
    kernel_items = NEW2(count, short);

    count = 0;
    max = 0;
    for (i = 0; i < nsyms; i++)
    {
	kernel_base[i] = kernel_items + count;
	count += symbol_count[i];
	if (max < symbol_count[i])
	    max = symbol_count[i];
    }

    shift_symbol = symbol_count;
    kernel_end = NEW2(nsyms, short *);
}


static void allocate_storage()
{
    allocate_itemsets();
    shiftset = NEW2(nsyms, short);
    redset = NEW2(nrules + 1, short);
    state_set = NEW2(nitems, core *);
}


static void append_states()
{
    register int i;
    register int j;
    register int symbol;

#ifdef	TRACE
    fprintf(stderr, "Entering append_states()\n");
#endif
    for (i = 1; i < nshifts; i++)
    {
	symbol = shift_symbol[i];
	j = i;
	while (j > 0 && shift_symbol[j - 1] > symbol)
	{
	    shift_symbol[j] = shift_symbol[j - 1];
	    j--;
	}
	shift_symbol[j] = symbol;
    }

    for (i = 0; i < nshifts; i++)
    {
	symbol = shift_symbol[i];
	shiftset[i] = get_state(symbol);
    }
}


static void free_storage()
{
    FREE(shift_symbol);
    FREE(redset);
    FREE(shiftset);
    FREE(kernel_base);
    FREE(kernel_end);
    FREE(kernel_items);
    FREE(state_set);
}



static void generate_states()
{
    allocate_storage();
    itemset = NEW2(nitems, short);
    ruleset = NEW2(WORDSIZE(nrules), unsigned);
    set_first_derives();
    initialize_states();

    while (this_state)
    {
	closure(this_state->items, this_state->nitems);
	save_reductions();
	new_itemsets();
	append_states();

	if (nshifts > 0)
	    save_shifts();

	this_state = this_state->next;
    }

    finalize_closure();
    free_storage();
}



static int get_state(symbol)
int symbol;
{
    register int key;
    register short *isp1;
    register short *isp2;
    register short *iend;
    register core *sp;
    register int found;
    register int n;

#ifdef	TRACE
    fprintf(stderr, "Entering get_state(%d)\n", symbol);
#endif

    isp1 = kernel_base[symbol];
    iend = kernel_end[symbol];
    n = iend - isp1;

    key = *isp1;
    assert(0 <= key && key < nitems);
    sp = state_set[key];
    if (sp)
    {
	found = 0;
	while (!found)
	{
	    if (sp->nitems == n)
	    {
		found = 1;
		isp1 = kernel_base[symbol];
		isp2 = sp->items;

		while (found && isp1 < iend)
		{
		    if (*isp1++ != *isp2++)
			found = 0;
		}
	    }

	    if (!found)
	    {
		if (sp->link)
		{
		    sp = sp->link;
		}
		else
		{
		    sp = sp->link = new_state(symbol);
		    found = 1;
		}
	    }
	}
    }
    else
    {
	state_set[key] = sp = new_state(symbol);
    }

    return (sp->number);
}



static void initialize_states()
{
    register int i;
    register short *start_derives;
    register core *p;

    start_derives = derives[start_symbol];
    for (i = 0; start_derives[i] >= 0; ++i)
	continue;

    p = (core *) MALLOC(sizeof(core) + i*sizeof(short));

    p->next = 0;
    p->link = 0;
    p->number = 0;
    p->accessing_symbol = 0;
    p->nitems = i;

    for (i = 0;	 start_derives[i] >= 0; ++i)
	p->items[i] = rrhs[start_derives[i]];

    first_state = last_state = this_state = p;
    nstates = 1;
}


static void new_itemsets()
{
    register int i;
    register int shiftcount;
    register short *isp;
    register short *ksp;
    register int symbol;

    for (i = 0; i < nsyms; i++)
	kernel_end[i] = 0;

    shiftcount = 0;
    isp = itemset;
    while (isp < itemsetend)
    {
	i = *isp++;
	symbol = ritem[i];
	if (symbol > 0)
	{
	    ksp = kernel_end[symbol];
	    if (!ksp)
	    {
		shift_symbol[shiftcount++] = symbol;
		ksp = kernel_base[symbol];
	    }

	    *ksp++ = i + 1;
	    kernel_end[symbol] = ksp;
	}
    }

    nshifts = shiftcount;
}



static core *new_state(symbol)
int symbol;
{
    register int n;
    register core *p;
    register short *isp1;
    register short *isp2;
    register short *iend;

#ifdef	TRACE
    fprintf(stderr, "Entering new_state(%d)\n", symbol);
#endif

    if (nstates >= MAXSHORT)
	fatal("too many states");

    isp1 = kernel_base[symbol];
    iend = kernel_end[symbol];
    n = iend - isp1;

    p = (core *) allocate((unsigned) (sizeof(core) + (n - 1) * sizeof(short)));
    p->accessing_symbol = symbol;
    p->number = nstates;
    p->nitems = n;

    isp2 = p->items;
    while (isp1 < iend)
	*isp2++ = *isp1++;

    last_state->next = p;
    last_state = p;

    nstates++;

    return (p);
}


#ifdef DEBUG

void show_cores()
{
    core *p;
    int i, j, k, n;
    int itemno;

    k = 0;
    for (p = first_state; p; ++k, p = p->next)
    {
	if (k) printf("\n");
	printf("state %d, number = %d, accessing symbol = %s\n",
		k, p->number, symbol_name[p->accessing_symbol]);
	n = p->nitems;
	for (i = 0; i < n; ++i)
	{
	    itemno = p->items[i];
	    printf("%4d  ", itemno);
	    j = itemno;
	    while (ritem[j] >= 0) ++j;
	    printf("%s :", symbol_name[rlhs[-ritem[j]]]);
	    j = rrhs[-ritem[j]];
	    while (j < itemno)
		printf(" %s", symbol_name[ritem[j++]]);
	    printf(" .");
	    while (ritem[j] >= 0)
		printf(" %s", symbol_name[ritem[j++]]);
	    printf("\n");
	    fflush(stdout);
	}
    }
}


void show_ritems()
{
    int i;

    for (i = 0; i < nitems; ++i)
	printf("ritem[%d] = %d\n", i, ritem[i]);
}


void show_rrhs()
{
    int i;

    for (i = 0; i < nrules; ++i)
	printf("rrhs[%d] = %d\n", i, rrhs[i]);
}


void show_shifts()
{
    shifts *p;
    int i, j, k;

    k = 0;
    for (p = first_shift; p; ++k, p = p->next)
    {
	if (k) printf("\n");
	printf("shift %d, number = %d, nshifts = %d\n", k, p->number,
		p->nshifts);
	j = p->nshifts;
	for (i = 0; i < j; ++i)
	    printf("\t%d\n", p->shift[i]);
    }
}
#endif


static void save_shifts()
{
    register shifts *p;
    register short *sp1;
    register short *sp2;
    register short *send;

    p = (shifts *) allocate((unsigned) (sizeof(shifts) +
			(nshifts - 1) * sizeof(short)));

    p->number = this_state->number;
    p->nshifts = nshifts;

    sp1 = shiftset;
    sp2 = p->shift;
    send = shiftset + nshifts;

    while (sp1 < send)
	*sp2++ = *sp1++;

    if (last_shift)
    {
	last_shift->next = p;
	last_shift = p;
    }
    else
    {
	first_shift = p;
	last_shift = p;
    }
}



static void save_reductions()
{
    register short *isp;
    register short *rp1;
    register short *rp2;
    register int item;
    register int count;
    register reductions *p;
    register short *rend;

    count = 0;
    for (isp = itemset; isp < itemsetend; isp++)
    {
	item = ritem[*isp];
	if (item < 0)
	{
	    redset[count++] = -item;
	}
    }

    if (count)
    {
	p = (reductions *) allocate((unsigned) (sizeof(reductions) +
					(count - 1) * sizeof(short)));

	p->number = this_state->number;
	p->nreds = count;

	rp1 = redset;
	rp2 = p->rules;
	rend = rp1 + count;

	while (rp1 < rend)
	    *rp2++ = *rp1++;

	if (last_reduction)
	{
	    last_reduction->next = p;
	    last_reduction = p;
	}
	else
	{
	    first_reduction = p;
	    last_reduction = p;
	}
    }
}


static void set_derives()
{
    register int i, k;
    register int lhs;
    register short *rules;

    derives = NEW2(nsyms, short *);
    rules = NEW2(nvars + nrules, short);

    k = 0;
    for (lhs = start_symbol; lhs < nsyms; lhs++)
    {
	derives[lhs] = rules + k;
	for (i = 0; i < nrules; i++)
	{
	    if (rlhs[i] == lhs)
	    {
		rules[k] = i;
		k++;
	    }
	}
	rules[k] = -1;
	k++;
    }

#ifdef	DEBUG
    print_derives();
#endif
}

#if __STDC__
void free_derives(void)
#else
void free_derives()
#endif
{
    FREE(derives[start_symbol]);
    FREE(derives);
}

#ifdef	DEBUG
static void print_derives()
{
    register int i;
    register short *sp;

    printf("\nDERIVES\n\n");

    for (i = start_symbol; i < nsyms; i++)
    {
	printf("%s derives ", symbol_name[i]);
	for (sp = derives[i]; *sp >= 0; sp++)
	{
	    printf("  %d", *sp);
	}
	putchar('\n');
    }

    putchar('\n');
}
#endif


static void set_nullable()
{
    register int i, j;
    register int empty;
    int done;

    nullable = MALLOC(nsyms);

    for (i = 0; i < nsyms; ++i)
	nullable[i] = 0;

    done = 0;
    while (!done)
    {
	done = 1;
	for (i = 1; i < nitems; i++)
	{
	    empty = 1;
	    while ((j = ritem[i]) >= 0)
	    {
		if (!nullable[j])
		    empty = 0;
		++i;
	    }
	    if (empty)
	    {
		j = rlhs[-j];
		if (!nullable[j])
		{
		    nullable[j] = 1;
		    done = 0;
		}
	    }
	}
    }

#ifdef DEBUG
    for (i = 0; i < nsyms; i++)
    {
	if (nullable[i])
	    printf("%s is nullable\n", symbol_name[i]);
	else
	    printf("%s is not nullable\n", symbol_name[i]);
    }
#endif
}


#if __STDC__
void free_nullable(void)
#else
void free_nullable()
#endif
{
    FREE(nullable);
}


void lr0()
{
    set_derives();
    set_nullable();
    generate_states();
}
