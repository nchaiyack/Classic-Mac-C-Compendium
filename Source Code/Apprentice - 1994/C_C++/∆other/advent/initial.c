/*
  Initialization of adventure play variables
*/
#include	<string.h>
#include	<stdio.h>
#include	"advent.h"		/* #define preprocessor equates	 */
#include	"advdec.h"

_PROTOTYPE(static void scanint, (int *, char *));
_PROTOTYPE(static void scanlong, (long *, char *));

/*
  Initialize integer arrays with sscanf
*/
static void scanint(pi, str)
int *pi;
char *str;
{

    while (*str) {
	if ((sscanf(str, "%d,", pi++)) < 1)
	    bug(41);			/* failed before EOS			 */
	while (*str++ != ',')		/* advance str pointer	 */
	    ;
    }
    return;
}

/*
  Initialize long integer arrays with sscanf
*/
static void scanlong(pi, str)
long *pi;
char *str;
{

    while (*str) {
	if ((sscanf(str, "%ld,", pi++)) < 1)
	    bug(41);			/* failed before EOS	 */
	while (*str++ != ',')		/* advance str pointer	 */
	    ;
    }
    return;
}

/*
  Initialization of adventure play variables
*/
void initialize()
{
    int i;

    g.turns = 0;

    /* Initialize location status array */
    scanlong(&g.loc_attrib[0], "     0,    73,    65,    73,    73,");
    scanlong(&g.loc_attrib[5], "    65,    65,    73,  1601,    33,");
    scanlong(&g.loc_attrib[10], "    32,    32,    32,  1824,    32,");
    scanlong(&g.loc_attrib[15], "     0,     2,     0,     0,  2816,");
    scanlong(&g.loc_attrib[20], "     2,     2,     2,     0,    10,");
    scanlong(&g.loc_attrib[25], "     0,     2,     0,     0,     0,");
    scanlong(&g.loc_attrib[30], "     0,     2,     2,     8,     0,");
    scanlong(&g.loc_attrib[35], "     0,     0,     0,     8,     0,");
    scanlong(&g.loc_attrib[40], "     2,     0,   256,   256,   256,");
    scanlong(&g.loc_attrib[45], "   256,   272,   272,   272,   256,");
    scanlong(&g.loc_attrib[50], "   256,     0,   256,   256,   272,");
    scanlong(&g.loc_attrib[55], "   256,   272,     0,    16,     2,");
    scanlong(&g.loc_attrib[60], "     0,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[65], "     0,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[70], "     0,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[75], "     0,     0,     0,     0,     2,");
    scanlong(&g.loc_attrib[80], "   256,   256,   272,     0,     0,");
    scanlong(&g.loc_attrib[85], "    16,   272,     0,     0,     2,");
    scanlong(&g.loc_attrib[90], "     2,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[95], "     8,     0,     0,  1280,   513,");
    scanlong(&g.loc_attrib[100], "   513,   512,     0,     0,     0,");
    scanlong(&g.loc_attrib[105], "     0,     0,     0,   768,     0,");
    scanlong(&g.loc_attrib[110], "     0,     0,     0,     8,     0,");
    scanlong(&g.loc_attrib[115], "     1,     1,     0,     0,     0,");
    scanlong(&g.loc_attrib[120], "     0,     0,    16,    16,    16,");
    scanlong(&g.loc_attrib[125], "    16,    17,    16,    16,    16,");
    scanlong(&g.loc_attrib[130], "    16,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[135], "     0,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[140], "     0,  1040,    16,     0,     0,");
    scanlong(&g.loc_attrib[145], "     2,    65,    65,    65,    65,");
    scanlong(&g.loc_attrib[150], "    65,    65,    65,    65,    65,");
    scanlong(&g.loc_attrib[155], "    65,     3,  2625,  2113,    65,");
    scanlong(&g.loc_attrib[160], "    65,     3,     3,     3,     3,");
    scanlong(&g.loc_attrib[165], "     3,    41,    41,     9,     9,");
    scanlong(&g.loc_attrib[170], "     0,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[175], "     0,     0,     0,     2,     0,");
    scanlong(&g.loc_attrib[180], "     0,     2,     0,     0,     0,");
    scanlong(&g.loc_attrib[185], "     0,     0,     0,     0,    16,");
    scanlong(&g.loc_attrib[190], "     0,     0,     9,     0,     0,");
    scanlong(&g.loc_attrib[195], "     0,     0,     0,     9,     2,");
    scanlong(&g.loc_attrib[200], "     1,     1,  2304,     0,     0,");
    scanlong(&g.loc_attrib[205], "     0,     8,     8,     8,     0,");
    scanlong(&g.loc_attrib[210], "     0,     0,     1,     0,     9,");
    scanlong(&g.loc_attrib[215], "     0,     0,    12,     0,     0,");
    scanlong(&g.loc_attrib[220], "     0,     0,     0,     0,     0,");
    scanlong(&g.loc_attrib[225], "     0,     2,  2625,    73,    73,");
    scanlong(&g.loc_attrib[230], "     0,     2,     2,     2,     2,");
    scanlong(&g.loc_attrib[235], "     0,     0,     2,    65,  3137,");
    scanlong(&g.loc_attrib[240], "    65,    65,    65,    65,    65,");
    scanlong(&g.loc_attrib[245], "    65,    65,    65,    65,");

    /* Initialize object locations */
    memset(plac, 0, (sizeof(int)) * MAXOBJ);
    scanint(&plac[2], "3,8,10,11,0,14,0,148,96,");
    scanint(&plac[11], "19,17,101,103,0,106,0,0,238,238,");
    scanint(&plac[21], "117,190,109,25,23,111,35,0,97,");
    scanint(&plac[31], "119,0,117,0,130,0,126,140,0,96,");
    scanint(&plac[41], "94,158,160,188,0,155,174,166,228,18,");
    scanint(&plac[51], "204,27,29,30,0,92,168,97,100,101,");
    scanint(&plac[61], "0,119,127,130,141,144,205,28,182,225,");
    scanint(&plac[71], "230,0,147,241,248,");
    scanint(&plac[87], "193,102,0,159,");
    scanint(&plac[91], "141,172,188,189,0,0,193,227,174,");
    scanint(&plac[101], "13,238,217,171,0,146,159,3,");
    scanint(&plac[113], "3,180,39,5,0,110,169,200,");

    /* Initialize second (fixed) locations */
    memset(fixd, 0, (sizeof(int)) * MAXOBJ);
    scanint(&fixd[3], "9,0,0,0,15,");
    scanint(&fixd[11], "-1,27,-1,0,0,0,-1,");
    scanint(&fixd[21], "122,235,-1,-1,67,-1,110,0,-1,-1,");
    scanint(&fixd[31], "121,0,122,0,-1,-1,-1,-1,0,-1,");
    scanint(&fixd[41], "-1,166,167,189,0,0,-1,0,229,");
    scanint(&fixd[62], "121,0,-1,");
    scanint(&fixd[87], "-1,194,-1,");
    scanint(&fixd[91], "-1,174,-1,-1,0,0,-1,-1,-1,");
    scanint(&fixd[103], "-1,-1,-1,0,");
    scanint(&fixd[112], "-1,0,0,-1,-1,");

    scanint(&g.weight[1], "0,1,0,1,2,2,0,0,2,1,");
    scanint(&g.weight[11], "0,0,0,7,7,1,0,0,2,1,");
    scanint(&g.weight[21], "0,0,0,0,0,0,0,3,0,0,");
    scanint(&g.weight[31], "0,0,0,0,0,0,0,0,3,0,");
    scanint(&g.weight[41], "0,0,0,0,0,1,1,0,0,6,");
    scanint(&g.weight[51], "1,2,2,3,5,4,3,2,3,4,");
    scanint(&g.weight[61], "1,3,1,3,2,1,1,2,2,2,");
    scanint(&g.weight[71], "3,1,1,3,1,0,0,0,0,0,");
    scanint(&g.weight[81], "2,2,2,2,2,2,0,0,0,1,");
    scanint(&g.weight[91], "0,0,0,0,3,2,0,0,0,0,");
    scanint(&g.weight[101], "2,1,0,0,0,1,1,2,0,3,");
    scanint(&g.weight[111], "3,0,1,1,0,0,0,3,1,2,");

    memset(g.atloc, 0, (sizeof(int)) * MAXOBJ);
    memset(g.holder, 0, (sizeof(int)) * MAXOBJ);
    memset(g.hlink, 0, (sizeof(int)) * MAXOBJ);
    memset(g.link, 0, (sizeof(int)) * MAXOBJ * 2);

    for (i = MAXOBJ; i > 0; i--) {
	g.fixed[i] = fixd[i];
	if (fixd[i] > 0) {
	    drop(i + MAXOBJ, fixd[i]);
	    drop(i, plac[i]);
	}
	if ((plac[i] != 0) && (fixd[i] <= 0))
	    drop(i, plac[i]);
    }

    memset(g.points, 0, (sizeof(long)) * MAXOBJ);
    scanlong(&g.points[16], "1000108,");
    scanlong(&g.points[47], "-3000112,0,0,-2000055,-2000112,-2000112,");
    scanlong(&g.points[53], "-1000112,-5000112,5000003,-3000112,-2000055,");
    scanlong(&g.points[58], "2000003,-3000112,-4000112,-4000112,3000003,");
    scanlong(&g.points[63], "-1000112,-4000112,-4000112,-2000112,-3000112,");
    scanlong(&g.points[68], "-1000112,-1000112,-2000112,-3012055,-4000112,");
    scanlong(&g.points[73], "-1000112, -5000112, -5000112,");
    scanlong(&g.points[110], "-2000112,");
    scanlong(&g.points[119], "-4000118,-2000112,");

    /* Initialize various flags and other variables */
    scanlong(&g.obj_state[0], "     0,     0, 32800,    26, 32770,");
    scanlong(&g.obj_state[5], "    32,    32,  8192,     0,     0,");
    scanlong(&g.obj_state[10], "    32,   512,     0,   256,   770,");
    scanlong(&g.obj_state[15], "   770,   288,   512,     0,   160,");
    scanlong(&g.obj_state[20], " 32802,     0,     0,     0,   128,");
    scanlong(&g.obj_state[25], "     0,     0,     0,    32,  8192,");
    scanlong(&g.obj_state[30], "   512,   512,     0,   512,     0,");
    scanlong(&g.obj_state[35], "   512,   256,     0, 32768,  8224,");
    scanlong(&g.obj_state[40], "     0,    18,    26,    26,     2,");
    scanlong(&g.obj_state[45], "     0,  8320, 18464, 32768,     0,");
    scanlong(&g.obj_state[50], " 16384, 16416, 16416, 26656, 24608,");
    scanlong(&g.obj_state[55], " 49240, 24608, 16384, 49184, 16416,");
    scanlong(&g.obj_state[60], " 16416, 16416, 16384, 16544, 16442,");
    scanlong(&g.obj_state[65], " 16416, 18464, 26656, 16416, 16416,");
    scanlong(&g.obj_state[70], " 49184, 49154, 18464, 18464, 16416,");
    scanlong(&g.obj_state[75], " 16416,     0,     0,     0,     0,");
    scanlong(&g.obj_state[80], "     0,     0,     0,     0,     0,");
    scanlong(&g.obj_state[85], "     0,     0,  8704,     0,     0,");
    scanlong(&g.obj_state[90], "     0,     0,     0,     0, 32768,");
    scanlong(&g.obj_state[95], "     0,   128,     0,     0,     0,");
    scanlong(&g.obj_state[100], "     0,   160,  8224,     0,     0,");
    scanlong(&g.obj_state[105], "     0,  8352,  8352, 32870,     0,");
    scanlong(&g.obj_state[110], " 16674,   258, 32858,   288,    32,");
    scanlong(&g.obj_state[115], "   256,     0,     0, 32866, 16416,");
    scanlong(&g.obj_state[120], " 16416,     0,     0,");

    /* Initialize hint section */
    memset(g.hinted, 0, (sizeof(int)) * HNTMAX);
    memset(g.hints, 0, (sizeof(int)) * HNTMAX * 5);
    scanint(&g.hints[1][1], "9999,4,0,0,");
    scanint(&g.hints[2][1], "9999,10,0,0,");
    scanint(&g.hints[3][1], "9999,5,0,0,");
    scanint(&g.hints[7][1], "15,4,176,177,");
    scanint(&g.hints[8][1], "8,5,178,179,");
    scanint(&g.hints[9][1], "13,3,180,181,");
    scanint(&g.hints[10][1], "6,10,211,212,");
    scanint(&g.hints[11][1], "6,5,213,214,");
    scanint(&g.hints[12][1], "4,2,62,63,");
    scanint(&g.hints[13][1], "5,2,18,19,");
    scanint(&g.hints[14][1], "4,2,62,233,");
    scanint(&g.hints[15][1], "6,5,274,275,");
    scanint(&g.hints[16][1], "10,5,289,290,");
    scanint(&g.hints[17][1], "8,2,20,21,");
    scanint(&g.hints[18][1], "5,2,404,405,");

    memset(g.prop, 0, (sizeof(int)) * MAXOBJ);
    for (i = 1; i <= MAXOBJ; i++) {
	if (treasr(i)) {
	    g.prop[i] = -1;
	    g.tally++;
	}
    }
    g.wzdark = g.closed = g.closing = g.holding = g.detail = 0;
    g.tally2 = 0;
    g.newloc = 1;
    g.loc = g.oldloc = g.oldloc2 = 3;
    g.knfloc = 0;
    g.chloc = 114;
    g.chloc2 = 140;
    scanint(g.dloc, "0,19,27,33,44,64,114,");
    scanint(g.odloc, "0,0,0,0,0,0,0,");
    g.dkill = 0;
    scanint(g.dseen, "0,0,0,0,0,0,0,");
    g.clock = 30;
    g.clock2 = 50;
    g.panic = 0;
    g.bonus = 0;
    g.numdie = 0;
    g.daltloc = 18;
    g.lmwarn = 0;
    g.foobar = 0;
    g.dflag = 0;
    g.abbnum = 5;			/* start out in  unbrief mode */
    g.health = 100;
    newtravel = TRUE;
    g.place[BOOK] = -1;
    insert(BOOK, SAFE);
    g.place[WATER] = -1;
    insert(WATER, BOTTLE);
    g.prop[BOTTLE] = 8;
    g.flg239 = FALSE;
    return;
}
