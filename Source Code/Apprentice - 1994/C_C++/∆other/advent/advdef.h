/*	header ADVDEF.H						*
 *	WARNING: GLOBAL variable allocations for adventure	*/


boolean gaveup;			/* TRUE if he quits early	 */
/*
  Database variables
*/
FILE *fd1, *fd2, *fd3, *fd4;

/*
  English variables
*/
char *vtxt[MAXWORDS], *iotxt[MAXITEMS], *otxt[MAXITEMS];
int verbs[MAXWORDS], objs[MAXITEMS], iobjs[MAXITEMS];
int vrbx, objx, iobx;
int verb, object, motion, iobj, prep;
char word1[WORDSIZE], word2[WORDSIZE];
boolean newtravel, is_wiz;

/*
  Play variables
*/
int plac[MAXOBJ];		/* initial location	 */
int fixd[MAXOBJ];
struct {
  int turns;
  int loc, oldloc, oldloc2, newloc;	/* location variables */
  long loc_attrib[MAXLOC];	/* location status	 */
  int place[MAXOBJ];		/* object location	 */
  int fixed[MAXOBJ];		/* second object loc	 */
  int weight[MAXOBJ];
  int atloc[MAXLOC];
  int link[MAXOBJ * 2];
  int holder[MAXOBJ];
  int hlink[MAXOBJ];
  int visited[MAXLOC];		/* >0 if has been here	 */
  int prop[MAXOBJ];		/* status of object	 */
  long obj_state[MAXOBJ];
  long points[MAXOBJ];
  int hinted[HNTMAX+1];
  int hints[HNTMAX+1][5];
  int hintlc[HNTMAX+1];
  int tally, tally2;		/* item counts		 */
  int limit;			/* time limit		 */
  int lmwarn;			/* lamp warning flag	 */
  int wzdark, closing, closed;	/* game state flags	 */
  int holding;			/* count of held items	 */
  int detail;			/* LOOK count		 */
  int knfloc;			/* knife location	 */
  int clock, clock2, panic;	/* timing variables	 */
  int dloc[DWARFMAX+1];		/* dwarf locations	 */
  int dflag;			/* dwarf flag		 */
  int dseen[DWARFMAX+1];	/* dwarf seen flag	 */
  int odloc[DWARFMAX+1];	/* dwarf old locations	 */
  int daltloc;			/* alternate appearance	 */
  int dkill;			/* dwarves killed	 */
  int chloc, chloc2;		/* chest locations	 */
  int bonus;			/* to pass to end	 */
  int numdie;			/* number of deaths	 */
  int foobar;			/* fee fie foe foo...	 */
  int combo;			/* combination for safe  */
  boolean terse;
  int abbnum;
  int health;
  int chase;
  boolean flag239;


  int lastglob;			/* to get space req.	 */
} g;
