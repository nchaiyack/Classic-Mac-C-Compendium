/*
   Copyright (c) Michael S. Morton
	 1986, 1987, 1988, 1989, 1990, 1991, 1992, 1993
   All rights reserved.

   Permission is granted to anyone to use this software for any purpose on
   any computer, and to alter it and redistribute it freely, subject
   to the following restrictions:

   0. This software is provided "as is" and without any express or implied
      warranties, including, without limitation, the implied warranties of
      merchantability and fitness for a particular purpose.

   1. The author is not responsible for the consequences of the use of this
      software, no matter how awful, even if they arise from flaws in it.

   2. The origin of this software must not be misrepresented, either by
      explicit claim or by omission.  The credits must mention that this
      software is based on the algorithm as described in the November 1987
      issue of BYTE Magazine. The credits must appear in the About... dialog
      and any documentation.

   3. Altered versions must be plainly marked as such, and must not be
      misrepresented as being the original software.  The credits must appear
      in the About... dialog and any documentation.

   4. This software may not be sold except as part of a substantially
	 		different program.

	 5. The Boston Museum of Science is expressly forbidden to use this program
	 		for profit or to produce revenue.

   6. This notice may not be removed or altered.
*/

/*	[Set tabs to 2 characters.]

		This is the source for Ars Magna v1.1, a version with some improvements but
		at least one serious bug.  It was never widely released.

		This source is not up-to-date, except for this comment.

		I plan to return to work on anagram engines and user interfaces someday, but
		it's not going to be soon.  I've been thinking hard about user interfaces
		for this app since 1984 (the original app was done in '82), and haven't come
		up with anything good.  My hope is that various developers will experiment,
		steal each other's best ideas, and in general have a real good time.  I've
		already seen some people take Ars Magna's bad user interface and produce a
		worse one; get creative here, folks, will ya?

		A few disconnected ideas, more or less off the top of my head:

			* I want to be able to say "Show me everything with word ____"
			* Then, of course, the remaining mainstream anagrams omit that word
			*	Similarly, I want to be able to say "Show me everything with words ____
				and ____".
			* I want to be able to say "Don't ever show me this word during this session"
			* I want to be able to say "Don't ever show me this word, ever"
			* This version tries to do one-word anagrams, then two-word, etc.  It
				tends to lock up at the each length; there's some obvious pruning
				that can be done here.
			*	Reviewing the output can take hours, so:
				(1) the user should be able to do nearly everything with the keyboard
				(2) they should be able to save their complete search state
			* The app should handle various dictionary formats, and set one or more
				default dictionaries as a preference.
			* I've never been able to find foreign-language dictionaries; anyone
				know if there are public-domain ones around?
			*	Word order and search order are very important to producing good
				anagrams early in the search.  What are the possible orderings?
				Which help in which ways?  How should the user control them?
			* The list of eligible words should be displayed somewhere.  The user
				should be able to save it, specify hot words to try first, etc.
			*	There should be a way to show incomplete anagrams, with the user
				specifying the maximum number of unused letters to show (zero would
				make the app behave as it does now).  They should be able to rearrange
				unused letters under keyboard or mouse control.
			*	Once you've found an anagram which looks interesting, the app should
				let you rearrange them under keyboard or mouse control, or quickly
				display all orderings of the words.  Of course, a quick command should
				save an anagram to the list of cool anagrams.
			*	When listing anagrams, a change in a given column should be highlighted
				to wake up a bored user.  This version does this crudely with capitals
				when a word first appears in a column, as in this example:
						CLIMATE HO MR NO
						climate ho mr ON
						climate MR NO OH
						climate mr OH ON
			*	I hate retyping anagrams for posting or publication, so the app should:
				(1) Allow easy word-shuffling and punctuation and capitalization
						changes in the saved list of cool anagrams.
				(2) Quickly check a text file or the clipboard's contents to make
						sure they're right.
			*	You should be able to export the list of cool anagrams to the
				clipboard or a text file.
			* The current bit-banging scheme makes sure that an anagram contains
				exactly the right number of letters, by adding them up and checking
				that the count is exactly right.  This could be generalized to make
				sure the count is in a certain range for an attribute OTHER than a
				letter.  This is useful for:
				(a) I'm using several dictionaries, but only one is new, so show me
						only anagrams with one or more words from that dictionary.
				(b)	I want to see no more than 2 one-letter words.
				(c)	I want to see at least one word that I've marked as interesting.
			*	Or, if you want to use the counter scheme for "no more than one of"
				or "at least one of", you could use AND and OR instead of addition.
			*	I should be able to quickly count all the anagrams which will result
				from a partial anagram I've found, to help me decide when to explore.
			*	I've tried a NeXT-style browser interface and, at least the way I did
				it, it was lousy.  Not interactive enough.
			*	You might be able to prune or order anagrams on the basis of available
				or constructed information on word-pair frequencies in natural languages.
				This idea strikes me as promising, but I haven't done anything with it.
			*	Extra credit: Parse the anagrams as natural language to find good ones.

			Don't expect that doing all of the above will give you a good app.  This is
			just a laundry list, not an overall vision of a good UI.  I've talked to
			good anagrammers, and they work on paper and in their heads -- devising an
			interface which is easier for them, not just for a beginner, will be hard.

		If you develop an app, prototype or not, commercial or not, I'd be interested
		in seeing it and commenting on the UI.  If you feel like sharing the source,
		I'd be interested in that, too.

		Keep in touch,
				Mr. Machine Tool
				February, 1993

		Email:	Mike_Morton@Proponent.com
		Paper:	Mike Morton, POB 11299, Honolulu, HI  96828
*/

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "console.h"
#include "MacTypes.h"
#include "FileMgr.h"

/* Easily changeable constants: */
#define STRMAX			100								/* size of some strings */
#define MAXFILES		10								/* maximum number of dictionaries */
#define MAXMASKS		5									/* number of "masks" for bitbanging */
																			/* note that doanagram() depends on MAXMASKS */
#define DFTFILE			"anagram dict"		/* default dictionary name */
#define	WPTRDELTA		512								/* # of word ptrs to add */
#define TEXBSIZE		2048							/* size of a block of text */
#define DOTFREQ			25								/* anagrams per dot, when filing/counting */
#define DOTSPERLINE	40								/* dots per line for filing/counting */
#define PAGESIZE		22								/* lines between prompts */
#define WORDCHUNK		5000							/* size of chunks for words */

/* Not-so-easily-changeable things: */
#define mask		long									/* masks are stored in longwords */
#define maskwidth	8*sizeof(mask)			/* and their size is important */

/* Global stuff -- phrase information: */
char phrase [STRMAX];									/* the phrase to be anagrammed */
char origphrase [STRMAX];							/* unmunged copy of phrase */
short freqs [26];											/* frequency table for phrase */
short letmask [26], letbit [26];			/* mask, bit positions for each letter */
short letwidth[26];										/* width in mask of each letter */
short usedmasks;											/* count of used masks */
short minWords, maxWords;

mask oflodesc [MAXMASKS];							/* descriptor used to detect overflow */
mask phrasedesc [MAXMASKS];						/* descriptor for main phrase */
mask startdesc [MAXMASKS];						/* starting descriptor for search */

/* Global stuff -- Dictionary information */
short numdicts = 0;										/* number of dictionaries */
FILE *dicts [MAXFILES];								/* dictionary files */
short numwords;												/* number of eligible words */
short maxwords = 0;										/* number of words which fit in *wordlist */
char **wordlist = NULL;								/* words[0..maxwords-1],
																							with 0..numwords-1 used */
mask *worddescs;											/* descriptors [0..numwords-1] */
struct wchunk {
	struct wchunk *next;
	char text [WORDCHUNK];
};
char *nextwcp = NULL;									/* pointer to next char to fill */
short nextwleft = 0;									/* chars left in this block */
struct wchunk *curchunk = NULL;				/* pointer to current chunk */
struct wchunk *chunkhead = NULL;			/* pointer to first chunk */

/* Global stuff -- For printing anagrams */
char *anawords [20];									/* stacked words */
char **anaptr;												/* pointer to first unused slot */
FILE *anafile;												/* stdin for screen, output file, or NULL */
short silent;													/* 1 iff not printing to screen */
unsigned long anacount;								/* number of anagrams found */
unsigned long startt, endt, tottime;	/* timing information */
short paging;													/* pause after each screenful? */

enum {																/* types of help */
	dict1, dict1dft, dict2,
	anahelp,
	scf,
	overadd,
	quithelp,
	helppage, helppaused
};

#define DR register										/* Debugging Register */
#define pf printf

void dophrase (void);
void clean (char *s);
void cleannl (char *s);
void makefreqs (char *s);
short findbits (void);
short findbitwidth (short count);
void makeof (void);
short makedesc (char *str, mask desc[]);
short okword (char *str);
short getdicts (void);
short getwords (void);
short storeword (char *word);
void freewords (void);
short mycmp (char **s1p, char **s2p);
void sortwords (void);
void nodups (void);
void printwords (void);
void makeWrite (FILE *f);
short makemasks (void);
void freemasks (void);
short doanagrams (short curword, mask *curstate, short wordsLeft);
short printana (void);
short exitcheck (void);
void getoutput (void);
short openoutfile (char *name);
short getline (char *prompt, char *keywords, short keyneeded, char *dft, char *reprompt,
						short helpstate, char *response);
void dohelp (short state);
short keyfind (char *words, char *oneword);
short findstr (char *master, char *sub);
short strNequal (char *s1, char *s2, short size);
long msec (void);
void starttimer (void);
void stoptimer (void);
void inittimer (void);
void printtime (void);
void chkerr (short i);

void main (void)
{
	cgotoxy (0, 0, stdout);						/* don't scroll initial stuff */
	pf ("********************************************************************************\n");
	pf ("*      A R S  M A G N A (TM), an anagram generator, version 1.1 (24 Sep '90)   *\n");
	pf ("* Copyright 1986-1993 Michael Morton (aka "Mr. Machine Tool", "Harmonic motel")*\n");
	pf ("*                            All rights reserved.                              *\n");
	pf ("*    Based on my algorithm as described in the November 1987 issue of BYTE.    *\n");
	pf ("********************************************************************************\n");
	pf ("\n");
	pf ("QUICK...This program finds anagrams for a phrase or name.  You can just press\n");
	pf (" INFO   RETURN for most questions, or type "HELP" at any point for more info.\n");
	pf ("\n");

	numdicts = getdicts ();							/* just once: open up dictionaries */

	while (1)
	{
		printf ("\n");
		getline ("What's the phrase you'd like to anagram? ",
							"", 0, "", "\nWhat's the phrase you'd like to anagram? ",
							anahelp, phrase);
		strcpy (origphrase, phrase);			/* remember it, before we... */
		clean (phrase);										/* clean up the string */

		if (strlen (phrase) == 0)					/* nothing there? */
			printf ("Come on, don't be shy...  try something!\n");
		else dophrase ();

		if ((anafile != NULL) && (anafile != stdout))
			fclose (anafile);								/* heavy-handed to put this here... */
	}																		/* end of infinite main loop */
}																			/* end of main program */

void dophrase ()
{
	short i;														/* counter for masks */
	char response [STRMAX];							/* dummy input buffer */
	short wordCount;
	short keynum;

	makefreqs (phrase);									/* build the frequency table */

	if (findbits () == 0)								/* choose bit fields; check size */
	{
		printf ("Sorry -- that phrase is too long to handle.\n");
		return;
	}

	getoutput();												/* where should we put this stuff? */

	if (anafile == stdout)
		paging = getline ("Want output to pause after each page [RETURN for YES]? ",
											"#no#yes", 1, "yes",
											"Please type YES or NO.  Want output to pause after each page? ",
											helppage, response);
	else paging = 0;										/* no paging if not printing */

	makeof ();													/* compute overflow and "full" descriptors */

	makedesc (phrase, phrasedesc);			/* make descriptor for phrase; ignore result (can't fail) */

	for (i = 0; i <= usedmasks; i++)		/* loop through all masks we used... */
		startdesc[i] =										/* and for each one, the starting descriptor... */
			phrasedesc[i];									/* ...is the full one */

	getwords ();												/* read in words for this phrase */
	if (numwords == 0) return;					/* bag it?  OK */
	sortwords ();												/* get 'em in the right order */
	nodups ();													/* eliminate duplicate words */
	printwords ();											/* dump 'em to a file */
	if (makemasks () == 0) return;			/* make masks for them all -- return on error */

	inittimer ();

	if (anafile != NULL)
		chkerr (fprintf (anafile, "\n\n*** anagrams for '%s' ***\n\n", origphrase));
	for (wordCount = 1; wordCount < 100; wordCount++)
	{
		anaptr = anawords;								/* point to base of stack */
		anacount = 0;											/* no anagrams found so far */
		doanagrams (0, startdesc, wordCount); /* print (or whatever) all anagrams */
		if (anafile != stdout) printf ("\n");	/* don't print after dots */
		printf ("Anagrams found with %d words: %ld.\n", wordCount, anacount);
		stoptimer ();
		keynum = getline ("   ...more (next size of anagram)... ",
			"#stop", 0, "", "   ...more (next size of anagram)... ",
			helppaused, response);
		starttimer ();
		if (keynum == 0)								/* bag this anagram? */
			wordCount = 100;							/* break the loop */
	}

	printtime ();

	freemasks ();												/* ditch the masks */
	freewords ();												/* ditch the words */
}																			/* end of dophrase() */

/* clean -- Clean up a string in place: map all letters to lowercase and
   discard everything else. */

void clean (s)
	DR char *s;													/* UPDATE: string to clean */
{
	DR char *in = s, *out = s;					/* reading, writing pointers */
	DR char c;													/* working copy of character */

	while (c = *in++)										/* loop through whole input string */
	{
		if ( (c>='A') && (c<='Z') )				/* uppercase alphabetic? */
			c -= ('A' - 'a');								/* yup: map it to lower */
		if ( (c>='a') && (c<='z') )				/* after mapping, is it lower case? */
			*out++ = c;											/* yup: store it */
	}																		/* end of loop mapping&discarding */
	*out++ = c;													/* store the final null */
}																			/* end of clean() */

/* cleannl -- Toss all newlines out of a string, in place. */

void cleannl (s)
	register char *s;
{
	register char *in = s;
	register char *out = s;
	register char c;

	while (c = *in++)
		if (c != '\n')
			*out++ = c;
	*out++ = '\0';
}

/* makefreqs -- Take a phrase and produce a frequency table from it.  The
   phrase has already been cleaned up, so we know it contains only the
   characters 'a'..'z'. */

void makefreqs (s)
	DR char *s;													/* INPUT: string to analyze */
	/* GLOBAL OUTPUT: frequency table */
{
	DR short i;													/* traditional loop index */

	for (i = 0; i<26; i++)							/* loop through and initialize... */
		freqs [i] = 0;										/* ...the frequency array */

	while (*s)													/* while there's more to the string... */
	{
		freqs [*s - 'a'] ++;							/* incremement slot for this letter */
		s++;															/* and skip to the next character */
	}																		/* end of loop through string */

}																			/* end of makefreqs() */


/* findbits -- Given the frequency table, find the bit position for each
   character found in the original phrase.  Nonexistent letters are
   ignored.  We may run out of room in the masks in doing this; we
   return 0 on failure. */

short findbits ()											/* zero <=> failure */
	/* GLOBAL INPUT: frequency table */
	/* GLOBAL OUTPUT: whichword[], whichbit[], count of used masks */
{
	short letter;												/* letter value (0..25) */
	short curword = 0, curbit = 0;			/* initial bit and word */
	short width;												/* bitwidth of letter's field */

	for (letter = 0; letter < 26; letter++)	/* loop through all letters */
	{
		if (freqs[letter] != 0)						/* any occurrences of this letter? */
		{																	/* yes: find where it'll go */
			width = findbitwidth (freqs [letter]); /* how much room does it need? */
			if (curbit+width > maskwidth) 	/* too wide for this word? */
			{																/* yes: have to kick into next word */
				curword++; curbit = 0;				/* go to start of next word */
				if (curword >= MAXMASKS) 			/* no more room? */
					return (0);									/* no more: report failure */
			}																/* end of kicking into next word */

			letmask [letter] = curword;			/* remember which word we go in */
			letbit [letter] = curbit;				/* and bit position in the word */
			letwidth [letter] = width;			/* and the width */
			curbit += width;								/* advance past this bit field */
		}																	/* end of handling letter found in phrase */
	}																		/* end of loop through A..Z */

	usedmasks = curword;								/* remember highest used mask */
	return (1);													/* indicate success */
}																			/* end of findbits() */

/* findbitwidth -- Find the number of bits needed to store a single letter
   up to the specified frequency.  Our output looks like this:
   		Frequency:		Width (+ one overflow bit)
   		0				<undefined>
   		1				1 (+ 1)
   		2..3			2 (+ 1)
   		4..7			3 (+ 1)
   ...etc. */

short findbitwidth (count)						/* find width of field to hold "count" */
	DR short count;											/* INPUT: frequency of letter */
{
	DR short width = 0;									/* result */

	while (count != 0)									/* loop 'til all bits discarded */
	{
		width++;													/* counting the bits... */
		count >>= 1;											/* ...and chucking out one more */
	}																		/* end of loop counting bits */

	width ++;														/* and one more for the overflow */
	return (width);											/* that's the answer */
}																			/* end of findbitwidth() */


/* makeof -- Find the descriptors for the "overflow" and "full" descriptors.
   The former has the bit just ABOVE each bit field set.  ANDing with this
   set of masks will detect when the thing overflows.  The latter is the
   former minus one in each bit field -- an almost full field. */

void makeof ()
	/* GLOBAL INPUT: frequency table, letter information, count of masks */
	/* GLOBAL OUTPUT: oflodesc */
{
	short l;														/* letter number */
	short mnum, bnum, bwidth;						/* mask #, bit #, field width */
	mask onebit, ovbit;									/* bits for single letter, overflow */
	short i;														/* usual counter */

	for (i = 0; i <= usedmasks; i++)		/* clean out... */
		oflodesc [i] = 0;									/* ...each overflow mask */


	for (l = 0; l < 26; l++)						/* loop through letters */
		if (freqs [l] != 0)								/* any letters in the phrase */
		{
			bnum = letbit [l];							/* what's the bit # for this letter? */
			bwidth = letwidth [l];					/* how wide is the field? */
			mnum = letmask [l];							/* and which mask does field go in? */

			onebit = 1;											/* start with a right-aligned 1 */
			onebit <<= bnum;								/* ...align it with the field */
			ovbit = onebit << (bwidth-1);		/* and find where the overflow bit is */

			oflodesc [mnum] |= ovbit;				/* put the overflow bit in */
		}																	/* end of handling letter in phrase */
}																			/* end of makeof() */



/* makedesc -- Create the descriptor for a string.  If the string contains
   too many of any letter, we return zero to say so. */

short makedesc (str, desc)						/* returns zero <=> failure */
	register char *str;									/* INPUT: string to analyze */
	register mask desc[];								/* OUTPUT: descriptor for string */
	/* GLOBAL INPUT: frequency table, letter information */
{
	register short l;										/* letter number */
	register char c;										/* character from the string */
	short sfreqs [26];									/* string's frequency profile */
	register short i;										/* loop counter */
	register mask b;										/* a bit, for ORing into the desc */

	if (*str == '\0')										/* null string? */
		return (0);												/* no good */

	for (i = 0; i <= usedmasks; i++)		/* go through all used masks... */
		desc[i] = 0;											/* ...initializing their descriptors */

	for (l = 0; l < 26; l++)						/* loop through all letters... */
		sfreqs [l] = 0;										/* zeroing their frequency */

	while (c = *str++)									/* pick up all characters in str... */
		sfreqs [c - 'a'] ++;							/* ...tallying up their count */

	for (l = 0; l < 26; l++)						/* loop through all letters... */
		if (sfreqs [l] != 0)							/* did it occur in the string? */
		{
			if (sfreqs [l] > freqs[l])			/* did it occur more than in the phrase? */
				return (0);										/* yes: fail */
			b = sfreqs [l];									/* start with the count */
			b <<= letbit [l];								/* shift it into the field */
			desc [letmask [l]] +=	b;				/* now add it into the mask */
		}																	/* end of handling letter in str */

	return (1);													/* we're OK -- say so */
}																			/* end of makedesc */

/* okword -- See if a string is eligible for use in anagrams.
   We return zero if not. */

short okword (str)										/* returns zero <=> failure */
	register char *str;									/* INPUT: string to analyze */
	/* GLOBAL INPUT: frequency table */
{
	register short l;										/* letter number */
	register char c;										/* character from the string */
	short sfreqs [26];									/* string's frequency profile */

	if (*str == '\0')										/* null string? */
		return (0);												/* no good */

	for (l = 0; l < 26; l++)						/* loop through all letters... */
		sfreqs [l] = 0;										/* zeroing their frequency */

	while (c = *str++)									/* pick up all characters in str... */
	{
		c -= 'a';
		if (++sfreqs [c] > freqs[c])			/* ...tallying up their count */
			return (0);											/* ...and checking each one */
	}
	
	return (1);													/* we're OK -- say so */
}																			/* end of okword */

/* getdicts -- Get and open up at least one dictionary. */

short getdicts ()											/* return count of opened dicts */
	/* GLOBAL OUTPUT: dicts[] */
{
	short count = 0;										/* initially none found */
	short done = 0;											/* done flag */
	short defaultavail;									/* default-file-failed flag */
	char dictname [STRMAX];							/* dictionary file name */
	FILE *testfile;											/* used for checking for default file */
	short helptype;

	/* is the default file available? */

	testfile = fopen (DFTFILE, "r");
	if (testfile == NULL)								/* blew it? */
		defaultavail = FALSE;							/* yup -- remember it's not there */
	else {															/* found it */
		defaultavail = TRUE;							/* remember this */
		fclose (testfile);								/* and chuck it */
	}																		/* end of successful find */

	while (! done)											/* loop 'til we're happy */
	{
		if (count == 0)										/* no files open yet? */
		{																	/* yes: special greeting */
			printf ("Name of dictionary file");
			helptype = dict1;								/* set help state */
			if (defaultavail)
			{
				printf (" [press RETURN to use '%s']", DFTFILE);
				helptype = dict1dft;
			}
		}
		else
		{
			printf ("Next dictionary name [press RETURN if no more]");
			helptype = dict2;
		}
		getline ("? ", "", 0, "", "Dictionary name? ", helptype, dictname); /* ask 'em */

		if (strlen (dictname) == 0)				/* just RETURN? */
		{																	/* handle untalkative input */
			if (count == 0)									/* no files yet? */
			{
				if (defaultavail == 0)				/* no files yet? */
					printf ("Come now, don't be shy.\n");	/* no default available */
				else strcpy (dictname, DFTFILE);	/* use default name */
			}
			else done = TRUE;								/* one or more files is OK */
		}																	/* end of handling RETURN */

		if (strlen (dictname) != 0)				/* did we read (or plug in) a name? */
		{																	/* we got a name... */
			testfile = fopen (dictname, "r"); /* ...try to open it */
			if (testfile == NULL)						/* failed? */
			printf ("Sorry, can't find the file '%s'.  Try again?\n", dictname);
			else dicts [count++] = testfile;	/* stack this dict; bump count */
		}																	/* end of processing input name */

		if (count >= MAXFILES)						/* full up? */
		{																	/* yes: force an exit */
			printf ("\nThat's enough files -- can't handle any more!\n");
			done = TRUE;
		}

	}																		/* end of looping 'til done */

	return (count);											/* say how many dictionaries we got */
}																			/* end of getdicts() */

/* getwords -- Read in all the usable words from all the dictionaries.
   We set the count of usable words, including zero if there are
   none.  If we run out of memory, we return zero.  Either way, we
   print a message so the caller doesn't have to. */

short getwords ()														/* we return the count of words */
{
	register short dnum;								/* dictionary number */
	register FILE *infile;							/* one dictionary's FILE */
	char word [STRMAX];									/* one word from the dictionary */
	char cleanword [STRMAX];						/* cleaned-up copy */
	register long readcount = 0;				/* count of words read */
	char lastline [STRMAX];							/* last line read */
	char inpline [STRMAX];							/* raw input line */
	char *inp;													/* input line pointer */
	short common;												/* count of letters in common */

	numwords = 0;												/* no words yet */
	for (dnum = 0; dnum < numdicts; dnum++)	/* loop through all dictionaries */
	{
		printf ("Reading dictionary...");
		infile = dicts [dnum];						/* grab this dictionary */
		fseek (infile, 0L, 0);						/* reset to start of file */
		while (1)													/* EOF is noticed in mid-loop */
		{
			if (fgets (inpline, STRMAX, infile) == NULL)	/* get a word */
				break;												/* if end-of-file, quit */
			common = 0;
			inp = inpline;									/* point to start of input line */
			while ( (*inp >= '0') && (*inp <= '9') ) /* process digits... */
				common = (common * 10) + (*inp++ - '0'); /* ...accumulate number */
			lastline [common] = '\0';				/* take first N chars of last line */
			strcat (lastline, inp);					/* and add the rest */

			strcpy (word, lastline);				/* now get the word */
			cleannl (word);									/* ditch the newline */
			strcpy (cleanword, word);				/* make a copy... */
			clean (cleanword);							/* ...and clean it up */
			readcount++;
			if ((readcount % 1000) == 0)
			{	printf (".");
				fflush (stdout);							/* don't let this line buffer up */
			}

			if (okword (cleanword) != 0)		/* can it be used in anagrams? */
			{																/* yes! */
				if (storeword (word) == 0)		/* store it; check for failure */
					return (0);									/* say we can't do it */
			}																/* end of handling useful word */
		}																	/* end of loop through one dict */
		printf ("\n");										/* ... left us in mid-line */
	}																		/* end of loop through dictionaries */

	if (numwords == 0)									/* NOTHING found? */
		printf ("Sorry -- absolutely NO usable words found!\n");
	else printf ("%d usable words found.\n", numwords);

}																			/* end of getwords () */

short storeword (word)
	register char *word;
{
	register char *memword;							/* allocated word */
	register short len = 1 + strlen (word);	/* size we need to store word */
	register short newmaxwords;					/* new array size */
	register short size;								/* size of new block */

	if (len > nextwleft)								/* no room in this chunk? */
	{
		struct wchunk *newchunk;

		newchunk = (struct wchunk *) malloc (sizeof (struct wchunk));
		if (newchunk == NULL)							/* blew it? */
		{
			printf ("Sorry -- not enough memory for this anagram!\n");
			freewords ();										/* discard accumulated words */
			return (0);											/* say so */
		}
		newchunk -> next = chunkhead;			/* make this... */
		chunkhead = newchunk;							/* ...the head chunk */
		curchunk = newchunk;							/* which (coincidentally) is the current one, too */
		nextwcp = newchunk -> text;				/* point to first usable character */
		nextwleft = WORDCHUNK;						/* remember available size */
	}

	memword = nextwcp;									/* point to first free character */
	nextwcp += len;											/* skip to next slot */
	nextwleft -= len;										/* and debit available stuff */

	strcpy (memword, word);							/* store the word in new chunk */

	if ((numwords+1) >= maxwords)				/* will this overflow the current list? */
	{
		newmaxwords = maxwords + WPTRDELTA;	/* jump to next size */
		size = newmaxwords * sizeof (char *); /* find new size */

		if (wordlist == NULL)
			wordlist = (char **) malloc (size); /* first time */
		else wordlist = (char **) realloc (wordlist, size); /* grow the block */

		if (wordlist == NULL)							/* blew it? */
		{																	/* looks that way */
			printf ("Sorry -- not enough memory for this anagram!\n");
			free (memword);									/* chuck word not yet in list */
			freewords ();										/* discard accumulated words */
			return (0);											/* say to give up */
		}

		maxwords = newmaxwords;						/* remember new maximum */
	}																		/* end of handling list overflow */

	wordlist [numwords++] = memword;		/* stack this word in the list */

	return (1);													/* the sweet smell of success */
}

void freewords ()
{
	struct wchunk *p;

	for (p = chunkhead; p = p -> next; p != NULL)
		free (p);

	nextwcp = NULL;
	nextwleft = 0;											/* force allocation next time */
	curchunk = NULL;
	chunkhead = NULL;
}																			/* end of freewords() */


short mycmp (s1p, s2p)
	char **s1p, **s2p;
{
	register char *s1 = *s1p;
	register char *s2 = *s2p;
	register short i;

	i = strlen (s2) - strlen (s1);	/* if S2 is longer, S1 is greater */

	if (i != 0)
		return (i);
	return (strcmp (s1, s2));
}

void sortwords ()
{
	printf ("Sorting words..."); inittimer ();
	qsort ((char *) wordlist, numwords, 4, /*mystrcmp*/mycmp);
	printf ("\n");
}

void nodups ()
{
	register short wordnum = 0;
	register short move;

	while (wordnum < numwords-1)
	{
		if (strcmp (wordlist [wordnum], wordlist [wordnum+1]) != 0 )
			wordnum ++;										/* different: go to next word */
		else {													/* same: have to move down */

			for (move = wordnum; move < numwords - 1; move++)
				wordlist [move] = wordlist [move+1];
			-- numwords;
		}
	}
}

void printwords ()
{	char response [STRMAX];
	FILE *f;
	register short wordnum = 0;

	printf ("What file do you want the word list in [RETURN if none]? ");
	gets (response);									/* get a line of input */
	if (strlen (response) == 0)				/* pressed RETURN? */
		return;													/* yes: bag it */

	f = fopen (response, "a");				/* open, append */
	if (f == NULL)
	{	printf ("Can't output to that file; sorry!");
		return;
	}

	fseek (f, 0L, 2);									/* reset to the end */
	while (wordnum < numwords)
	{	fprintf (f, "%s\r", wordlist [wordnum]);
		++wordnum;
	}

	/* makeWrite (f); */										/* make it MacWrite-able */
	fclose (f);
}

void makeWrite (f)
	FILE *f;
{	short refnum;
	FInfo info;
	Str255 s;

	refnum = f->refnum;
	GetFInfo (&s, refnum, &info);
	info.fdType = 'TEXT';
	info.fdCreator = 'MACA';
	SetFInfo (&s, refnum, &info);
}

short makemasks ()
	/* GLOBAL INPUT: numwords, wordlist */
	/* GLOBAL OUTPUT: worddescs[] */
{
	short i;
	short onesize, size;
	mask *descp;
	char wordcopy [STRMAX];

	onesize = (usedmasks+1) * sizeof (mask);
	size = numwords * onesize;					/* compute size of all descriptors */

	worddescs = (mask *) malloc (size);	/* find space for it */
	if (worddescs == NULL)							/* blew it? */
		return (0);

	descp = worddescs;									/* point to the first descriptor slot */
	for (i = 0; i < numwords; i++)			/* loop through every word... */
	{
		strcpy (wordcopy, wordlist [i]);	/* ...copy the word */
		clean (wordcopy);									/* ...clean it up */
		makedesc (wordcopy, descp);				/* ...and store each one's descriptor */
		descp += (usedmasks+1);						/* ...and bump to next slot */
	}
	return (1);													/* no memory problems */
}																			/* end of makemasks() */

void freemasks ()
{
	free (worddescs);
}

short doanagrams (curword, curstate, wordCount)	/* return 0 to halt search */
	register short curword;							/* current word number */
	register mask *curstate;						/* descriptor for current state */
	short wordCount;										/* words left to stack before we print anagram */
{
	mask newdesc [MAXMASKS];						/* new descriptor */
	register mask newmask;							/* one mask from descriptor */
	register mask *curdesc;							/* current word's descriptor */
	register long overflow;							/* does this combination overflow? */
	register long bitsleft;							/* is this combination full? */

	curdesc = &worddescs [curword * (usedmasks+1)];

	wordCount--;												/* count the word we're going to stack */
	while (curword < numwords)					/* loop through all words after this one */
	{

#define CASE(MNUM)																													\
		newmask = curstate [MNUM] - curdesc [MNUM];	/* subtract from anagram */	\
		if (overflow =										/* remember if there was oflo... */		\
			(newmask & oflodesc [MNUM]))		/* and, if so, bag it */							\
				break; 												/* note it; break out */							\
		newdesc [MNUM] = newmask;					/* it's OK; store it */								\
		bitsleft |= newmask;							/* note if done */

		bitsleft = 0;											/* assume complete 'til we see otherwise */
		overflow = 0;											/* no overflow 'til we've seen it */
		switch (usedmasks)
		{
			case 5:	CASE(5)
			case 4:	CASE(4)
			case 3:	CASE(3)
			case 2:	CASE(2)
			case 1:	CASE(1)
			case 0:	CASE(0)
		}

		if (! overflow)
		{
			*anaptr++ = wordlist [curword]; /* save this word */

			if ((! bitsleft)								/* no bits left? */
					&& (wordCount == 0))				/* ...and time to print? */
			{
				if (printana () == 0)					/* print it; do they want us to stop? */
				  return (0);									/* yes: stop */
			}
			else if (wordCount)							/* dive deeper only if caller wants more words */
			{
				if (doanagrams (curword, newdesc, wordCount) == 0)
					return (0);
			}

			--anaptr;												/* discard the word from the stack */
		}

		curword++;												/* next word number */
		curdesc += (usedmasks+1);					/* next word's mask */
	}

  return (1);													/* continue the search */
}																			/* end of doanagrams() */

static char *lastanawords [20];

short printana ()											/* return 0 to stop current anagram */
{
	register char **p;
	char response [STRMAX];							/* dummy buffer for answer */
	short keynum;												/* keyword number from getline() */
	Boolean caps;

	++anacount;													/* that's one more anagram */

	if (anafile != stdout)							/* not printing to screen? */
	{
		if ((anacount % DOTFREQ) == 0)		/* every once in a while... */
		{
			printf (".");										/* remind them we're here */
			if (exitcheck () == 0)					/* time to stop THIS anagram? */
				return (0);										/* yes */
			if ((anacount % (DOTFREQ * DOTSPERLINE)) == 0)
			{
				printf ("anagram #%ld: ", anacount);
				for (p = anawords; p < anaptr; p++)
					printf ("%s ", *p);
				printf ("\n");
			}
		}
	}

	if (anafile == NULL) return (1);		/* no file?  no more to do here */

	if (! paging)												/* no way to stop? */
	{
		if (exitcheck () == 0)						/* time to stop THIS anagram? */
				return (0);										/* yes */
	}
	else {
		if ((anacount % PAGESIZE) == 0)
		{
			stoptimer ();
			keynum = getline ("   ...more... ",
				"#stop", 0, "", "   ...more... ",
				helppaused, response);
			starttimer ();
			if (keynum == 0)								/* bag this anagram? */
				return (0);										/* yup */
		}
	}

	/* Print the damned anagram already: */
	caps = false;
	for (p = anawords; p < anaptr; p++)
	{	if (lastanawords [p-anawords] != *p)	/* different word than showed up last time? */
			caps = true;
		if (caps)
		{	char buf [100];
			register char *bp;
			strcpy (buf, *p);
			bp = buf;
			while (*bp)
			{	if ((*bp >= 'a') && (*bp <= 'z'))
					*bp += ('A'-'a');
				bp++;
			}
			chkerr (fprintf (anafile, "%s ", buf));
		}
		else chkerr (fprintf (anafile, "%s ", *p));
		lastanawords [p-anawords] = *p;
	}
	chkerr (fprintf (anafile, "\n"));

	return (1);
}

short exitcheck ()										/* return 0 to stop this anagram */
{
	short c;
	short stopped = 0;
	short keynum;
	char str [STRMAX];

#if 0
	while (kbhit ())
		if (getchar () == ' ')
			stopped = 1;
#endif

	if (! stopped) return (1);					/* continue */

	stoptimer ();
	keynum = getline (
		"\nType STOP for the next anagram, QUIT to leave the program,\nor anything else to continue? ",
		"#stop", 0, "", 
		"Type STOP for the next anagram, QUIT to leave the program,\nor anything else to continue? ",
		quithelp, str);

	starttimer ();
	if (keynum == 0)										/* typed STOP? */
		return (0);												/* yes: stop */
	else return (1);										/* no: continue */
}

void getoutput ()
	/* GLOBAL OUTPUT: anafile, silent */
{
	char response [STRMAX];
	short done = 0;
	short keywd;

	printf ("Print to the screen, print to a file, or just count anagrams?\n");
	while (! done)
	{
		keywd = getline ("Type COUNT or a filename [or press RETURN for SCREEN]? ",
						"#screen#count", 0, "screen",
						"Type COUNT or a filename [or press RETURN for SCREEN]? ",
						scf, response);

		if (keywd == 0)
		{																	/* handle SCREEN */
			anafile = stdout;								/* output to screen */
			done = TRUE;
		}
		else if (keywd == 1)							/* handle COUNT */
		{
			anafile = NULL;									/* no output */
			done = TRUE;
		}
		else done = openoutfile (response);	/* see if we can output */
	}																		/* end of loop 'til done */
}

short openoutfile (name)
	char *name;
{
	short keywd;
	char response [STRMAX];
	char *opentype;

	opentype = "w";											/* default open type */
	anafile = fopen (name, "r");
	if (anafile != NULL)								/* got it? */
	{
		fseek (anafile, 0L, 2);						/* reset to the end */
		if (ftell (anafile) != 0L)				/* position != 0?  (ie, not empty?) */
		{
			printf ("'%s' isn't empty.  Want to OVERWRITE or ADD to it [press RETURN to ADD]? ", name);
			keywd = getline ("", "#overwrite#add", 1, "add",
											"Please type OVERWRITE or ADD? ", overadd, response);
			if (keywd == 0)
				opentype = "w";								/* overwrite */
			else opentype = "a";						/* add */
		}
		fclose (anafile);
	}

	anafile = fopen (name, opentype);		/* try to open it as desired */

	if (anafile == NULL)								/* blew it? */
	{
		printf ("Sorry -- can't write to '%s'.  Try again?\n", name);
		return (0);
	}
	return (1);													/* success */
}

short getline (prompt, keywords, keyneeded, dft, reprompt,
						helpstate, response)
	char *prompt;												/* INPUT: prompt */
	char *keywords;											/* INPUT: keywords, like "#a#b#c" */
	short keyneeded;										/* INPUT: insist on a keyword? */
	char *dft;													/* INPUT: default response */
	char *reprompt;											/* INPUT: repeated prompt */
	short helpstate;										/* INPUT: help state */
	char *response;											/* OUTPUT: response buffer */
{
	short done = 0;											/* flag to exit loop */
	short keyindex;											/* index of word in #string */

	printf ("%s", prompt);							/* prompt 'em */
	while (! done)
	{
		gets (response);									/* get a line of input */
		if (strlen (response) == 0)				/* pressed RETURN? */
			strcpy (response, dft);					/* yes: supply default */

		keyindex = keyfind ("#quit#help", response); /* see if it's a standard one */
		if (keyindex >= 0)								/* found something? */
		{																	/* handle QUIT or HELP */
			if (keyindex == 0)							/* QUIT? */
			{
				printf ("\n\n");
				exit (0);
			}
			dohelp (helpstate);							/* nope: HELP */
		}																	/* end of standard keywords */
		else {														/* not standard */
			keyindex = keyfind (keywords, response); /* look it up */

			if ( (keyindex >= 0)						/* found something... */
				|| (! keyneeded) )						/* ...or didn't need it */
					done = TRUE;								/* then we're satisfied */
		}																	/* end of parsing response */

		if (! done) printf ("%s", reprompt);	/* about to loop? be pushy */
	}																		/* end of loop 'til done */

	return (keyindex);									/* return keyword number or -1 */
}																			/* end of getline() */

void dohelp (state)
	short state;
{
	printf ("\n");
	if (state == anahelp)
	{
		printf ("  Enter a word, name or phrase.  This program will print out all\n");
		printf ("  the combinations of words which can be made by rearranging the\n");
		printf ("  letters in the phrase you type in.  Very short phrases won't\n");
		printf ("  have many good anagrams, while long phrases may produce too\n");
		printf ("  many to sift through.\n");
	}
	else if (state == scf)
	{
		printf ("  You can print anagrams on the screen, store them in a file,\n");
		printf ("  or just count them without printing them anywhere.  To print\n");
		printf ("  them on the screen, type SCREEN or just press RETURN.  To count\n");
		printf ("  them, type COUNT.  To send them to a file, type the file's name.\n");
	}
	else if (state == overadd)
	{
		printf ("  You can replace the current contents of that document with the\n");
		printf ("  anagrams, or you can add them after the old contents.  To replace,\N");
		printf ("  type OVERWRITE.  To add, type ADD.\n");
	}
	else if (state == quithelp)
	{
		printf ("  When you press the space bar, it suspends anagram finding\n");
		printf ("  in case you want to quit.  To leave the program, type QUIT.\n");
		printf ("  To do another anagram, type STOP.  To resume, type anything else.\n");
	}
	else if (state == dict1)
	{
		printf ("  Enter the name of a dictionary file you'd like to use.  Usually,\n");
		printf ("  '%s' is used, but it doesn't seem to be available.\n", DFTFILE);
		printf ("  Any text-only file containing one word per line will do.\n");
	}
	else if (state == dict1dft)
	{
		printf ("  Enter the name of a dictionary file you'd like to use.  Press\n");
		printf ("  RETURN to use '%s', the default file.\n", DFTFILE);
	}
	else if (state == dict2)
	{
		printf ("  Enter the name of another dictionary file, or press RETURN\n");
		printf ("  if you don't want to enter any more.\n");
	}
	else if (state == helppage)
	{
		printf ("  Normally output to the screen rushes by unless you press the\n");
		printf ("  mouse button to freeze it or hit the space bar to suspend it.\n");
		printf ("  If you ask for 'paged' output, it'll stop after each screenful.\n");
	}
	else if (state == helppaused)
	{
		printf ("  Press RETURN to resume reading anagrams, STOP to do a different\n");
		printf ("  anagram, or QUIT to stop.\n");
	}
	else printf ("Sorry -- no help available here.\n");

	if (state != quithelp)
		printf ("  At any time, you can leave by typing QUIT.\n");
	printf ("\n");
}

short keyfind (words, oneword)
	char *words;
	char *oneword;
{
	short cindex;
	char wordcopy [STRMAX], lookupword [STRMAX];
	short wordnum;

	strcpy (wordcopy, oneword);
	clean (wordcopy);
	if (strlen (wordcopy) < 1) return (-1);	/* can't find nothing */
	strcpy (lookupword, "#");
	strcat (lookupword, wordcopy);			/* construct "#word" */

	cindex = findstr (words, lookupword);	/* look up "#word" in "#a#b... */
	if (cindex == -1)										/* not found? */
		return (-1);											/* say so */

	wordnum = 0;												/* initially we're at word zero */
	while (cindex-- > 0)								/* loop up to "#" in "#word" in str */
		if (*words++ == '#')							/* is there a "#"? */
			++wordnum;											/* yes: that's another word */

	return (wordnum);
}

short findstr (master, sub)
	char *master;
	char *sub;
{
	short result = 0;
	short subsize = strlen (sub);

	while (*master)
	{
		if (strNequal (master, sub, subsize) == 1)	/* match? */
			return (result);								/* yup: say where */
		++result;
		++master;
	}

	return (-1);												/* no match */
}

short strNequal (s1, s2, size)
	register char *s1, *s2;
	register short size;
{	while (size--)
		if (*s1++ != *s2++) return 0;			/* not equal */
	return 1;														/* equal */
}

long msec ()
{
	return (TickCount () * (1000/60));	/* convert from 1/60s to ms */
}

void starttimer ()
{
	startt = msec ();
}

void stoptimer ()
{
	endt = msec ();
	tottime += (endt-startt);
}

void inittimer ()
{
	tottime = 0;
	starttimer ();
}

void printtime ()
{
	stoptimer ();
	printf ("Computing time: %ld.%03ld seconds.  ",
									tottime/1000L, (tottime % 1000));
}

void chkerr (i)
	short i;
{
	if (i == EOF)
	{
		printf ("Sorry!  Error in writing to output file.  Perhaps your disk is full?\n");
		exit (0);
	}
}
