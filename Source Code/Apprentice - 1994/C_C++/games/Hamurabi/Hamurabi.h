/* defines */

#define False 0
#define True 1
#define NIL -1

/* INITIAL VALUES */
#define INITPEOPLE 100
#define INITLAND 1000
#define INITSTORED 2800
#define INITSTARVED 0
#define INITNEWBIES 5
#define INITYIELD 3
#define INITRATS 200

#define PLAGUECHANCE 15 /* percent chance of plague */
#define RNGLOST 5 /* range of random number for lost grain */
#define RNGNEWBIE 5 /* range of random number for newbie calc */
#define ACREperFARMER 10 /* farmers needed to raise one acre */
#define ACREperGRAIN 2 /* seed grain it takes for one acre */
#define GRAINperMAN 20 /* amount of grain a man eats in a year */
#define RNGLANDPRICE 10 /* range of rnd # for land  price */
#define ADDLANDPRICE 17 /* add to base rnd# for land price */
#define RNGYIELD 5 /* range of rnd # for yield */
#define ADDYIELD 0 /* add to base rnd for yield */
#define STARVELIMIT 45 /* maximum % of pop you can starve */
#define LANDATTRACT 20 /* Amount land attracts newbies*/
#define GRAINATTRACT 1 /* Amount stored grain attracts newbies */

/* -------------------- Useful Macros ----------------- */
#define abs(X) ((X) < 0 ? -(X) : (X))
#define min(X1,X2) ((X1) < (X2) ? (X1) : (X2))
#define max(X1,X2) ((X1) > (X2) ? (X1) : (X2))

/* ------------------------------------------------------ */

void InitGame(void);
int CalcStarved(int grain);
int CalcHarvest(int seed, int yield, int farmers);
int CalcLostGrain(void);
int CalcLandPrice(void);
int CalcYield(void);
int CalcNewbies(void);
void RandomEvent(void);
int GetFeedGrain(void);
int GetTradeLand(int price);
int GetPlantAcres(void);
void Report(void);
void Impeach(void);
int RandRng(int Range);
void InitMac(void);
int GetInt(char * prompt);
/* ------------------------------------------------------ */
