#include "brlr grade 2 meat.h"
#include "brlr grade 1 meat.h"
#include "brlr conversion.h"
#include "util.h"
#include "window layer.h"
#include "brlr main window.h"
#include "program globals.h"

static	void DealWithNonAlpha(unsigned char *a, long len, long *offset);
static	short DealWithCapitals(unsigned char *a, long len, long offset);
static	Boolean DealWithWholeWord(unsigned char *a, long offset, short wordLength);
static	Boolean DealWithPartialWord(unsigned char *a, long offset, short wordLength,
			Boolean startOfWord, short *p);
static	void AddString(Str255 theStr);
static	void AddDots(Str31 dotStr);
static	Boolean MatchWord(Str255 oneStr, Str255 twoStr);
static	Boolean MatchPartial(unsigned char *a, short *foundLength, Str255 theStr);

static	Boolean			gSuppressSpace=FALSE;

void DealWithGrade2Text(WindowPtr theWindow, unsigned char *a, long len)
/* pass the destination window, a pointer to the text to convert, and the length of the
   text to convert.  If the text ptr is coming out of a TEHandle, make sure to lock down
   the TEHandle and the (**hTE).hText handle _before_ calling this function */
{
	long			offset;
	short			wordLength;
	short			partialLength;
	Boolean			startOfWord;
	
	if (len==0)
		return;
	gTheWindow=theWindow;
	
	gPostingEvents=TRUE;
	gSuppressSpace=FALSE;
	offset=0;
	
	while (offset<len)
	{
		DealWithNonAlpha(a, len, &offset);
		/* offset should now point to first letter in a word */
		if (offset<len)
		{
			wordLength=DealWithCapitals(a, len, offset);
			/* now offset should still point to first letter in a, but the word it
			   points to has been converted to lowercase; the length of the word is
			   returned and stored in wordLength */
			if (!DealWithWholeWord(a, offset, wordLength))
			{	/* didn't find whole word contraction, so check for partial ones */
				/* offset is still pointing to the first character in this word */
				startOfWord=TRUE;
				while (wordLength>0)
				{
					if (!DealWithPartialWord(a, offset, wordLength, startOfWord, &partialLength))
					{	/* didn't find partial word contraction; just add character
						   and go on to the next one */
						AddCharacter(a[offset++]);
						wordLength--;
					}
					else
					{	/* found partial contraction; update offset and wordLength */
						offset+=partialLength;
						wordLength-=partialLength;
					}
					startOfWord=FALSE;
				}
			}
			else
			{	/* found whole word contraction and dealt with it; we'll update the
				   offset ourselves to point to the next character after this word */
				offset+=wordLength;
			}
		}
	}
	
	gPostingEvents=FALSE;
}

static	void DealWithNonAlpha(unsigned char *a, long len, long *offset)
{
	unsigned char	theChar;
	
	theChar=a[*offset];
	if (gSuppressSpace)
	{	/* some whole word contractions require no space after them: by, to, into */
		if (theChar==' ')
			theChar=a[++(*offset)];
		gSuppressSpace=FALSE;
	}
	while (((*offset)<len) && (!(((theChar>='A') && (theChar<='Z')) || ((theChar>='a') && (theChar<='z')))))
	{
		AddCharacter(theChar);
		theChar=a[++(*offset)];
	}
}

static	short DealWithCapitals(unsigned char *a, long len, long offset)
/* checks for presence of single capital letter or all caps, then sets chars to lowercase */
{
	short			i;
	Boolean			firstCaps, allCaps;
	short			wordLength;
	unsigned char	theChar;
	
	wordLength=0;
	theChar=a[offset];
	while ((offset+wordLength<len) &&
			(((theChar>='A') && (theChar<='Z')) || ((theChar>='a') && (theChar<='z'))))
	{
		wordLength++;
		theChar=a[offset+wordLength];
	}
	
	allCaps=TRUE;
	for (i=0; ((i<wordLength) && (allCaps)); i++)
		if ((a[offset+i]>='a') && (a[offset+i]<='z'))
			allCaps=FALSE;
	
	if (!allCaps)
	{
		firstCaps=((a[offset]>='A') && (a[offset]<='Z'));
	}
	
	if (firstCaps || allCaps)
		AddDots("\p6");
	if ((allCaps) && (wordLength>1))
		AddDots("\p6");
	
	for (i=0; i<wordLength; i++)
		if ((a[offset+i]>='A') && (a[offset+i]<='Z'))
			a[offset+i]|=0x20;
	
	return wordLength;
}

static	Boolean DealWithWholeWord(unsigned char *a, long offset, short wordLength)
{
	Str255			theStr;
	
	theStr[0]=wordLength;
	Mymemcpy((Ptr)&theStr[1], (Ptr)&a[offset], (wordLength>255) ? 256 : wordLength+1);
	
	if (MatchWord(theStr, "\pbut")) AddCharacter('b');
	else if (MatchWord(theStr, "\pcan")) AddCharacter('c');
	else if (MatchWord(theStr, "\pdo")) AddCharacter('d');
	else if (MatchWord(theStr, "\pevery")) AddCharacter('e');
	else if (MatchWord(theStr, "\pfrom")) AddCharacter('f');
	else if (MatchWord(theStr, "\pgo")) AddCharacter('g');
	else if (MatchWord(theStr, "\phave")) AddCharacter('h');
	else if (MatchWord(theStr, "\pjust")) AddCharacter('j');
	else if (MatchWord(theStr, "\pknowledge")) AddCharacter('k');
	else if (MatchWord(theStr, "\plike")) AddCharacter('l');
	else if (MatchWord(theStr, "\pmore")) AddCharacter('m');
	else if (MatchWord(theStr, "\pnot")) AddCharacter('n');
	else if (MatchWord(theStr, "\ppeople")) AddCharacter('p');
	else if (MatchWord(theStr, "\pquite")) AddCharacter('q');
	else if (MatchWord(theStr, "\prather")) AddCharacter('r');
	else if (MatchWord(theStr, "\pso")) AddCharacter('s');
	else if (MatchWord(theStr, "\pthat")) AddCharacter('t');
	else if (MatchWord(theStr, "\pus")) AddCharacter('u');
	else if (MatchWord(theStr, "\pvery")) AddCharacter('v');
	else if (MatchWord(theStr, "\pwill")) AddCharacter('w');
	else if (MatchWord(theStr, "\pit")) AddCharacter('x');
	else if (MatchWord(theStr, "\pyou")) AddCharacter('y');
	else if (MatchWord(theStr, "\pas")) AddCharacter('z');
	else if (MatchWord(theStr, "\pwas")) AddDots("\p356");
	else if (MatchWord(theStr, "\pwere")) AddDots("\p2356");
	else if (MatchWord(theStr, "\phis")) AddDots("\p236");
	else if (MatchWord(theStr, "\pby"))
	{
		AddDots("\p356");
		gSuppressSpace=TRUE;
	}
	else if (MatchWord(theStr, "\pto"))
	{
		AddDots("\p235");
		gSuppressSpace=TRUE;
	}
	else if (MatchWord(theStr, "\pinto"))
	{
		AddDots("\p35 235");
		gSuppressSpace=TRUE;
	}
	else if (MatchWord(theStr, "\pchild")) AddDots("\p16");
	else if (MatchWord(theStr, "\pshall")) AddDots("\p146");
	else if (MatchWord(theStr, "\pthis")) AddDots("\p1456");
	else if (MatchWord(theStr, "\pwhich")) AddDots("\p156");
	else if (MatchWord(theStr, "\pour")) AddDots("\p1256");
	else if (MatchWord(theStr, "\pstill")) AddDots("\p34");
	else if (MatchWord(theStr, "\penough")) AddDots("\p26");
	else if (MatchWord(theStr, "\pabout")) AddString("\pab");
	else if (MatchWord(theStr, "\pabove")) AddString("\pabv");
	else if (MatchWord(theStr, "\paccording")) AddString("\pac");
	else if (MatchWord(theStr, "\pacross")) AddString("\pacr");
	else if (MatchWord(theStr, "\pafter")) AddString("\paf");
	else if (MatchWord(theStr, "\pafternoon")) AddString("\pafn");
	else if (MatchWord(theStr, "\pafterward")) AddString("\pafw");
	else if (MatchWord(theStr, "\pagain")) AddString("\pag");
	else if (MatchWord(theStr, "\pagainst")) AddDots("\p1 1245 34");
	else if (MatchWord(theStr, "\palmost")) AddString("\palm");
	else if (MatchWord(theStr, "\palready")) AddString("\palr");
	else if (MatchWord(theStr, "\palso")) AddString("\pal");
	else if (MatchWord(theStr, "\palthough")) AddDots("\p1 123 1456");
	else if (MatchWord(theStr, "\paltogether")) AddString("\palt");
	else if (MatchWord(theStr, "\palways")) AddString("\palw");
	else if (MatchWord(theStr, "\pbecause")) AddDots("\p23 14");
	else if (MatchWord(theStr, "\pbefore")) AddDots("\p23 124");
	else if (MatchWord(theStr, "\pbehind")) AddDots("\p23 125");
	else if (MatchWord(theStr, "\pbelow")) AddDots("\p23 123");
	else if (MatchWord(theStr, "\pbeneath")) AddDots("\p23 1345");
	else if (MatchWord(theStr, "\pbeside")) AddDots("\p23 234");
	else if (MatchWord(theStr, "\pbetween")) AddDots("\p23 2345");
	else if (MatchWord(theStr, "\pbeyond")) AddDots("\p23 13456");
	else if (MatchWord(theStr, "\pblind")) AddString("\pbl");
	else if (MatchWord(theStr, "\pbraille")) AddString("\pbrl");
	else if (MatchWord(theStr, "\pchildren")) AddDots("\p16 1345");
	else if (MatchWord(theStr, "\pconceive")) AddDots("\p25 14 1236");
	else if (MatchWord(theStr, "\pconceiving")) AddDots("\p25 14 1236 1245");
	else if (MatchWord(theStr, "\pdeclare")) AddString("\pdcl");
	else if (MatchWord(theStr, "\peither")) AddString("\pei");
	else if (MatchWord(theStr, "\pfirst")) AddDots("\p124 34");
	else if (MatchWord(theStr, "\pfriend")) AddString("\pfr");
	else if (MatchWord(theStr, "\pgood")) AddString("\pgd");
	else if (MatchWord(theStr, "\pgreat")) AddString("\pgrt");
	else if (MatchWord(theStr, "\pherself")) AddDots("\p125 12456 124");
	else if (MatchWord(theStr, "\phim")) AddString("\phm");
	else if (MatchWord(theStr, "\phimself")) AddString("\phmf");
	else if (MatchWord(theStr, "\pimmediate")) AddString("\pimm");
	else if (MatchWord(theStr, "\pits")) AddString("\pxs");
	else if (MatchWord(theStr, "\pitself")) AddString("\pxf");
	else if (MatchWord(theStr, "\pletter")) AddString("\plr");
	else if (MatchWord(theStr, "\plittle")) AddString("\pll");
	else if (MatchWord(theStr, "\pmuch")) AddDots("\p134 16");
	else if (MatchWord(theStr, "\pmust")) AddDots("\p134 34");
	else if (MatchWord(theStr, "\pmyself")) AddString("\pmyf");
	else if (MatchWord(theStr, "\pnecessary")) AddString("\pnec");
	else if (MatchWord(theStr, "\pneither")) AddString("\pnei");
//	else if (MatchWord(theStr, "\po'clock")) AddString("\po'c");
	else if (MatchWord(theStr, "\poneself")) AddDots("\p5 135 124");
	else if (MatchWord(theStr, "\pourselves")) AddDots("\p1256 1235 1236 234");
	else if (MatchWord(theStr, "\ppaid")) AddString("\ppd");
	else if (MatchWord(theStr, "\pperceive")) AddDots("\p1234 12456 14 1236");
	else if (MatchWord(theStr, "\pperceiving")) AddDots("\p1234 12456 14 1236 1245");
	else if (MatchWord(theStr, "\pperhaps")) AddDots("\p1234 12456 125");
	else if (MatchWord(theStr, "\pquick")) AddString("\pqk");
	else if (MatchWord(theStr, "\preceive")) AddString("\prcv");
	else if (MatchWord(theStr, "\preceiving")) AddString("\prcvg");
	else if (MatchWord(theStr, "\prejoice")) AddString("\prjc");
	else if (MatchWord(theStr, "\prejoicing")) AddString("\prjcg");
	else if (MatchWord(theStr, "\psaid")) AddString("\psd");
	else if (MatchWord(theStr, "\pshould")) AddDots("\p146 145");
	else if (MatchWord(theStr, "\psuch")) AddDots("\p234 16");
	else if (MatchWord(theStr, "\pthemselves")) AddDots("\p2346 134 1236 234");
	else if (MatchWord(theStr, "\pthyself")) AddDots("\p1456 13456 124");
	else if (MatchWord(theStr, "\ptoday")) AddString("\ptd");
	else if (MatchWord(theStr, "\ptogether")) AddString("\ptgr");
	else if (MatchWord(theStr, "\ptomorrow")) AddString("\ptm");
	else if (MatchWord(theStr, "\ptonight")) AddString("\ptn");
	else if (MatchWord(theStr, "\pwould")) AddString("\pwd");
	else if (MatchWord(theStr, "\pyour")) AddString("\pyr");
	else if (MatchWord(theStr, "\pyourself")) AddString("\pyrf");
	else if (MatchWord(theStr, "\pyourselves")) AddString("\pyrvs");
	else if (MatchWord(theStr, "\pbeen")) AddDots("\p12 15 26");
	else return FALSE;
	
	return TRUE;
}

static	Boolean DealWithPartialWord(unsigned char *a, long offset, short wordLength,
	Boolean startOfWord, short *p)
{
	unsigned char	*b;
	
	b=(unsigned char*)&a[offset];
	*p=0;
	
	if (wordLength>=9)
	{
		if (MatchPartial(b, p, "\pcharacter")) AddDots("\p5 16");
	}
	if ((wordLength>=8) && ((*p)==0))
	{
		if (MatchPartial(b, p, "\pquestion")) AddDots("\p5 12345");
	}
	if ((wordLength>=7) && ((*p)==0))
	{
		if (MatchPartial(b, p, "\pthrough")) AddDots("\p5 1456");
	}
	if ((wordLength>=6) && ((*p)==0))
	{
		if (MatchPartial(b, p, "\pfather")) AddDots("\p5 124");
		else if (MatchPartial(b, p, "\pmother")) AddDots("\p5 134");
		else if (MatchPartial(b, p, "\pcannot")) AddDots("\p456 14");
		else if (MatchPartial(b, p, "\pspirit")) AddDots("\p456 234");
	}
	if ((wordLength>=5) && ((*p)==0))
	{
		if (MatchPartial(b, p, "\pation")) AddDots("\p6 1345");
		else if (MatchPartial(b, p, "\pright")) AddDots("\p5 1235");
		else if (MatchPartial(b, p, "\punder")) AddDots("\p5 136");
		else if (MatchPartial(b, p, "\pyoung")) AddDots("\p5 13456");
		else if (MatchPartial(b, p, "\pthere")) AddDots("\p5 2346");
		else if (MatchPartial(b, p, "\pwhere")) AddDots("\p5 156");
		else if (MatchPartial(b, p, "\pought")) AddDots("\p5 1256");
		else if (MatchPartial(b, p, "\pthese")) AddDots("\p45 2346");
		else if (MatchPartial(b, p, "\pthose")) AddDots("\p45 1456");
		else if (MatchPartial(b, p, "\pwhose")) AddDots("\p45 156");
		else if (MatchPartial(b, p, "\pworld")) AddDots("\p456 2456");
		else if (MatchPartial(b, p, "\ptheir")) AddDots("\p456 2346");
	}
	if ((wordLength>=4) && ((*p)==0))
	{
		if (MatchPartial(b, p, "\pwith")) AddDots("\p23456");
		else if (MatchPartial(b, p, "\pound")) AddDots("\p46 145");
		else if (MatchPartial(b, p, "\pance")) AddDots("\p46 15");
		else if (MatchPartial(b, p, "\psion")) AddDots("\p46 1345");
		else if (MatchPartial(b, p, "\pless")) AddDots("\p46 234");
		else if (MatchPartial(b, p, "\pount")) AddDots("\p46 2345");
		else if (MatchPartial(b, p, "\pence")) AddDots("\p56 15");
		else if (MatchPartial(b, p, "\ption")) AddDots("\p56 1345");
		else if (MatchPartial(b, p, "\pness")) AddDots("\p56 234");
		else if (MatchPartial(b, p, "\pment")) AddDots("\p56 2345");
		else if (MatchPartial(b, p, "\pally")) AddDots("\p6 13456");
		else if (MatchPartial(b, p, "\pever")) AddDots("\p5 15");
		else if (MatchPartial(b, p, "\phere")) AddDots("\p5 125");
		else if (MatchPartial(b, p, "\pknow")) AddDots("\p5 13");
		else if (MatchPartial(b, p, "\plord")) AddDots("\p5 123");
		else if (MatchPartial(b, p, "\pname")) AddDots("\p5 1345");
		else if (MatchPartial(b, p, "\ppart")) AddDots("\p5 1234");
		else if (MatchPartial(b, p, "\psome")) AddDots("\p5 234");
		else if (MatchPartial(b, p, "\ptime")) AddDots("\p5 2345");
		else if (MatchPartial(b, p, "\pword")) AddDots("\p5 2456");
		else if (MatchPartial(b, p, "\pupon")) AddDots("\p45 136");
		else if (MatchPartial(b, p, "\pmany")) AddDots("\p456 134");
	}
	if ((wordLength>=3) && ((*p)==0))
	{
		if (MatchPartial(b, p, "\pong")) AddDots("\p56 1245");
		else if (MatchPartial(b, p, "\pful")) AddDots("\p56 123");
		else if (MatchPartial(b, p, "\pily")) AddDots("\p56 13456");
		else if (MatchPartial(b, p, "\pday")) AddDots("\p5 145");
		else if (MatchPartial(b, p, "\pone")) AddDots("\p5 135");
		else if (MatchPartial(b, p, "\phad")) AddDots("\p456 125");
		else if (MatchPartial(b, p, "\ping")) AddDots("\p346");
		else if (MatchPartial(b, p, "\pble")) AddDots("\p3456");
		else if (MatchPartial(b, p, "\pcon")) AddDots("\p25");
		else if (MatchPartial(b, p, "\pcom")) AddDots("\p36");
		else if (MatchPartial(b, p, "\pdis")) AddDots("\p256");
		else if (MatchPartial(b, p, "\pand")) AddDots("\p12346");
		else if (MatchPartial(b, p, "\pfor")) AddDots("\p123456");
		else if (MatchPartial(b, p, "\pthe")) AddDots("\p2346");
		else if (MatchPartial(b, p, "\pear")) AddDots("\p15 345");	/* precedence */
		else if (MatchPartial(b, p, "\pain")) AddString("\pain");	/* don't use 'in' contraction */
		else if (MatchPartial(b, p, "\pein")) AddString("\pein");
		else if (MatchPartial(b, p, "\poin")) AddString("\poin");
	}
	if ((wordLength>=2) && ((*p)==0))
	{
		if (MatchPartial(b, p, "\pgh")) AddDots("\p126");
		else if (MatchPartial(b, p, "\ped")) AddDots("\p1246");
		else if (MatchPartial(b, p, "\per")) AddDots("\p12456");
		else if (MatchPartial(b, p, "\pow")) AddDots("\p246");
		else if (MatchPartial(b, p, "\par")) AddDots("\p345");
		else if ((startOfWord) && (MatchPartial(b, p, "\pbe"))) AddDots("\p23");
		else if ((startOfWord) && (MatchPartial(b, p, "\pin"))) AddDots("\p35");
		else if ((wordLength>2) && (MatchPartial(b, p, "\pea"))) AddDots("\p2");
		else if ((!startOfWord) && (MatchPartial(b, p, "\pbb"))) AddDots("\p23");
		else if ((!startOfWord) && (MatchPartial(b, p, "\pcc"))) AddDots("\p25");
		else if ((!startOfWord) && (MatchPartial(b, p, "\pdd"))) AddDots("\p256");
		else if ((!startOfWord) && (MatchPartial(b, p, "\pff"))) AddDots("\p235");
		else if ((!startOfWord) && (MatchPartial(b, p, "\pgg"))) AddDots("\p2356");
		else if (MatchPartial(b, p, "\pof")) AddDots("\p12356");
		else if (MatchPartial(b, p, "\pch")) AddDots("\p16");
		else if (MatchPartial(b, p, "\psh")) AddDots("\p146");
		else if (MatchPartial(b, p, "\pth")) AddDots("\p1456");
		else if (MatchPartial(b, p, "\pwh")) AddDots("\p156");
		else if (MatchPartial(b, p, "\pou")) AddDots("\p1256");
		else if (MatchPartial(b, p, "\pst")) AddDots("\p34");
		else if (MatchPartial(b, p, "\pen")) AddDots("\p26");
	}
	
	return ((*p)!=0);
}

static	void AddString(Str255 theStr)
{
	short			i;
	
	for (i=1; i<=theStr[0]; i++)
		AddCharacter(theStr[i]);
}

static	void AddDots(Str31 dotStr)
{
	Str31			oneStr;
	short			i;
	
	i=1;
	oneStr[0]=0x00;
	while (i<=dotStr[0])
	{
		if (dotStr[i]==' ')
		{
			KeyPressedInMainWindow(gTheWindow, Dots(oneStr));
			oneStr[0]=0x00;
		}
		else oneStr[++oneStr[0]]=dotStr[i];
		i++;
	}
	
	if (oneStr[0]!=0x00)
	{
		KeyPressedInMainWindow(gTheWindow, Dots(oneStr));
	}
}

static	Boolean MatchWord(Str255 oneStr, Str255 twoStr)
{
	short			i;
	
	i=0;
	while ((i<=oneStr[0]) && (i<=twoStr[0]))
	{
		if (oneStr[i]!=twoStr[i])
			return FALSE;
		i++;
	}
	
	return TRUE;
}

static	Boolean MatchPartial(unsigned char *a, short *foundLength, Str255 theStr)
/* guaranteed that a contains at least theStr[0] valid characters */
/* if match is found, foundLength contains length of match on exit (theStr[0]) */
/* if match is found, returns TRUE */
{
	short			i;
	
	for (i=0; i<theStr[0]; i++)
		if (a[i]!=theStr[i+1])
			return FALSE;
	
	*foundLength=theStr[0];
	return TRUE;
}
