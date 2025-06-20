
//============================================================================
//----------------------------------------------------------------------------
//									Enemy.c
//----------------------------------------------------------------------------
//============================================================================

// This file contains all enemy related functions (enemy "AI").  It handles�
// the enemy decision making proccess, moves the enemies, etc.

#include "Externs.h"


#define kEnemyImpulse			8

#define kOwlMaxHVel				96
#define kOwlMaxVVel				320
#define kOwlHeightSmell			96
#define kOwlFlapImpulse			32

#define kWolfMaxHVel			128
#define kWolfMaxVVel			400
#define kWolfHeightSmell		160
#define kWolfFlapImpulse		48

#define kJackalMaxHVel			192
#define kJackalMaxVVel			512
#define kJackalHeightSmell		240
#define kJackalFlapImpulse		72


Boolean SetEnemyInitialLocation (Rect *);
void SetEnemyAttributes (short);
short AssignNewAltitude (void);
void InitEnemy (short, Boolean);
void CheckEnemyPlatformHit (short);
void CheckEnemyRoofCollision (short);
void HandleIdleEnemies (short);
void HandleFlyingEnemies (short);
void HandleWalkingEnemy (short);
void HandleSpawningEnemy (short);
void HandleFallingEnemy (short);
void HandleEggEnemy (short);
void ResolveEnemyPlayerHit (short);


handInfo	theHand;
eyeInfo		theEye;
Rect		grabZone;
short		deadEnemies, spawnedEnemies, numEnemiesThisLevel, numOwls;

extern	playerType	thePlayer;
extern	enemyType	theEnemies[kMaxEnemies];
extern	Rect		platformRects[6], enemyInitRects[5];
extern	long		theScore;
extern	short		numLedges, lightningCount, numEnemies, countDownTimer;
extern	short		levelOn, lightH, lightV;
extern	Boolean		evenFrame, doEnemyFlapSound, doEnemyScrapeSound;


//==============================================================  Functions
//--------------------------------------------------------------  SetEnemyInitialLocation

// When a new enemy is about to be "born", this function is called to determine�
// the enemies starting location.  The only thing important here is that the enemy�
// appears on a valid platform for the particular level we're on.  As well, which�
// platform he (it) appears on should be random.

Boolean SetEnemyInitialLocation (Rect *theRect)
{
	short		where, possibilities;
	Boolean		facing;
	
	possibilities = numLedges - 1;		// Determine number of valid platforms.
	where = RandomInt(possibilities);	// Choose one at random.
	*theRect = enemyInitRects[where];	// Initially place enemy at default location.
	
	switch (where)						// Determine if enemy facing left or right.
	{									// It depends upon which platform they're on.
		case 0:							// These are the left-most platforms.
		case 2:
		facing = TRUE;					// Enemy will face right.
		break;
		
		case 3:							// Special case for the center platform.
		if (RandomInt(2) == 0)			// Enemy randomly faces either left or right.
			facing = TRUE;
		else
			facing = FALSE;
		break;
		
		default:						// Catch remaining (right-most) platforms.
		facing = FALSE;					// Enemy will face left.
		break;
	}
	
	if ((levelOn % 5) == 4)				// Handle special case for Egg Wave
	{									// Re-define enemy bounds.
		theRect->left += 12 + RandomInt(48) - 24;
		theRect->right = theRect->left + 24;
		theRect->top = theRect->bottom - 24;
	}
	
	return (facing);
}

//--------------------------------------------------------------  SetEnemyAttributes

// Depending upon the type of enemy this function is passed (there are three�
// types of sphinx enemies), this function sets up that enemies various�
// attributes - such as maximum vertical velocity, etc.

void SetEnemyAttributes (short i)
{
	short		h;
											// Point enemy toward center of screen.
	h = (theEnemies[i].dest.left + theEnemies[i].dest.right) >> 1;
	if (h < 320)							// If enemy in left half of screen�
		theEnemies[i].facingRight = TRUE;	// the enemy will face to the right.
	else									// Otherwise, if in right half of screen�
		theEnemies[i].facingRight = FALSE;	// face to the left.
	
	switch (theEnemies[i].kind)				// Okay, depending upon what "kind" of enemy�
	{										// we're dealing with....
		case kOwl:							// The owl is the simplest (wimpiest) enemy.
		if (theEnemies[i].facingRight)		// Choose which graphic to use.
			theEnemies[i].srcNum = 0;
		else
			theEnemies[i].srcNum = 2;
											// Set owl's velocity limitations.
		theEnemies[i].maxHVel = kOwlMaxHVel;
		theEnemies[i].maxVVel = kOwlMaxVVel;
											// This is the distance within which he will�
											// pursue the player (it's strictly Y distance).
		theEnemies[i].heightSmell = kOwlHeightSmell;
											// This is how powerful the owl's "flap" is.
		theEnemies[i].flapImpulse = kOwlFlapImpulse;
		break;
		
		case kWolf:							// The wolf sphinx is of medium difficulty.
		if (theEnemies[i].facingRight)		// Choose which graphic to use.
			theEnemies[i].srcNum = 4;
		else
			theEnemies[i].srcNum = 6;
											// Set wolf's velocity limitations.
		theEnemies[i].maxHVel = kWolfMaxHVel;
		theEnemies[i].maxVVel = kWolfMaxVVel;
											// This is the distance within which he will�
											// pursue the player (it's strictly Y distance).
		theEnemies[i].heightSmell = kWolfHeightSmell;
											// This is how powerful the wolf's "flap" is.
		theEnemies[i].flapImpulse = kWolfFlapImpulse;
		break;
		
		case kJackal:						// The jackal is the swiftest, toughest enemy.
		if (theEnemies[i].facingRight)		// Choose which graphic to use.
			theEnemies[i].srcNum = 8;
		else
			theEnemies[i].srcNum = 10;
											// Set jackal's velocity limitations.
		theEnemies[i].maxHVel = kJackalMaxHVel;
		theEnemies[i].maxVVel = kJackalMaxVVel;
											// This is the distance within which he will�
											// pursue the player (it's strictly Y distance).
		theEnemies[i].heightSmell = kJackalHeightSmell;
											// This is how powerful the jackal's "flap" is.
		theEnemies[i].flapImpulse = kJackalFlapImpulse;
		break;
	}
}

//--------------------------------------------------------------  AssignNewAltitude

// The sphinxes "patrol" specific altitudes in the arena.  After wrapping around�
// the screen a few times, they randomly select a new altitude to patrol (this�
// keeps the player from finding a "safe" place to stand.  This function chooses�
// a new altitude for the enemy to patrol.

short AssignNewAltitude (void)
{
	short		which, altitude;
	
	which = RandomInt(4);		// There are only 4 "patrol altitudes".
	switch (which)				// Depending on which random number came up�
	{
		case 0:					// This is just below the ceiling.
		altitude = 65 << 4;
		break;
		
		case 1:					// This is below the top platforms but above the�
		altitude = 150 << 4;	// center platform.
		break;
		
		case 2:					// This is just below the center platform.
		altitude = 245 << 4;
		break;
		
		case 3:					// This is striahgt across the lava pit.
		altitude = 384 << 4;
		break;
	}
	
	return (altitude);
}

//--------------------------------------------------------------  InitEnemy

// This resets an enemies info.  It is called when a new enemy is to be born.
// It is called if an egg is about to hatch, if a new level has begun, or if�
// if it is simply time to add a new enemy.

void InitEnemy (short i, Boolean reincarnated)
{
	Boolean		facing;
	
	if (spawnedEnemies < numEnemiesThisLevel)	// New enemy to appear (in other words�
	{											// this enemy is not hatched).
												// Call function to set new location.
		facing = SetEnemyInitialLocation(&theEnemies[i].dest);
		theEnemies[i].wasDest = theEnemies[i].dest;
		theEnemies[i].h = theEnemies[i].dest.left << 4;
		theEnemies[i].v = theEnemies[i].dest.top << 4;
		theEnemies[i].wasH = theEnemies[i].h;	// Reset "old locations" variables.
		theEnemies[i].wasV = theEnemies[i].v;
												// Assign the "patrol altitude".
		theEnemies[i].targetAlt = theEnemies[i].v - (40 << 4);
		theEnemies[i].hVel = 0;					// Zero velocity vraiables.
		theEnemies[i].vVel = 0;
		theEnemies[i].pass = 0;					// Zero number of times wrapped around.
		if ((levelOn % 5) == 4)					// If this is an Egg Wave�
			theEnemies[i].mode = kEggTimer;		// set enemy in "wait to hatch" mode.
		else									// Otherwise, just sut enemy in�
			theEnemies[i].mode = kIdle;			// idle mode.
		if (i < numOwls)						// Determine what kind of enemy.
			theEnemies[i].kind = kOwl;
		else if (i > (numOwls + 6))	
			theEnemies[i].kind = kJackal;
		else
			theEnemies[i].kind = kWolf;
		theEnemies[i].facingRight = facing;
		SetEnemyAttributes(i);					// Initialize enemy attributes.
		
		if (reincarnated)						// If this is an egg that will hatch�
			theEnemies[i].frame = RandomInt(48) + 8 + (numOwls * 32);
		else
			theEnemies[i].frame = RandomInt(48) + 32 + (64 * i) + (numOwls * 32);
		
		if ((levelOn % 5) == 4)					// If this is an Egg Wave
			theEnemies[i].kind--;				// Decrement "kind" (since it's incremented�
												// when they hatch).
		spawnedEnemies++;						// Keep track of number of enemies active.
	}
}

//--------------------------------------------------------------  GenerateEnemies

// This function is called only for a new level.  It goes through and�
// intializes a whole host of enemies in one go.

void GenerateEnemies (void)
{
	short		i;
	
	if ((levelOn % 5) == 4)			// If this is an Egg Wave�
	{
		numEnemies = kMaxEnemies;	// we insist upon the maximum number of enemies.
		numEnemiesThisLevel = numEnemies;
	}
	else							// If not an egg wave, use a formula to determine�
	{								// the max number of enemies that are to be active.
		numEnemies = ((levelOn / 5) + 2) * 2;
		if (numEnemies > kMaxEnemies)
			numEnemies = kMaxEnemies;
		numEnemiesThisLevel = numEnemies * 2;
	}
	
	deadEnemies = 0;				// No dead enemies yet.
	
									// Use formula to determine the number of owls�
									// to appear.  This number goes down as the levels�
									// increase.  It is used not merely to determine�
									// how many owls are to appear, but also how many�
									// of the more advanced enemies.  For example, when�
									// numOwls goes down to zero, all the enemies will�
									// be of the more advanced breed (wolves and jackals).
	numOwls = 4 - ((levelOn + 2) / 5);
	if (numOwls < 0)
		numOwls = 0;
	
	spawnedEnemies = 0;				// No enemies have been "born" yet.
									// Go through and set up all the enemies.
	for (i = 0; i < numEnemies; i++)
		InitEnemy(i, FALSE);
}

//--------------------------------------------------------------  CheckEnemyPlatformHit

// This is the enemy counterpart to a similarly named function that tests for�
// player collsions with the platforms.

void CheckEnemyPlatformHit (short h)
{
	Rect		hRect, vRect, whoCares;
	short		i, offset;
	
	for (i = 0; i < numLedges; i++)					// Test all platforms.
	{												// Do a simple bounds test.
		if (SectRect(&theEnemies[h].dest, &platformRects[i], &whoCares))
		{											// If the enemy has hit the platform�
			hRect.left = theEnemies[h].dest.left;	// Determine if enemy hit platform sides.
			hRect.right = theEnemies[h].dest.right;
			hRect.top = theEnemies[h].wasDest.top;
			hRect.bottom = theEnemies[h].wasDest.bottom;
													// Test this new special rect to see if�
													// the enemy hit on of the platform sides.
			if (SectRect(&hRect, &platformRects[i], &whoCares))
			{										// If enemy hit from side, see which side.
													// We handle left and right seperatrely�
													// so that there's no ambiguity as to�
													// what the new velocity and location�
													// of the enemy is.  If we did not do it�
													// this way, there is the chance that an�
													// enemy get's "stuck" on the edge of�
													// a platform (due to round-off errors).
				if (theEnemies[h].h > theEnemies[h].wasH)
				{									// Enemy was moving right (hit left side).
					offset = theEnemies[h].dest.right - platformRects[i].left;
													// Slide enemy "off" platform.
					theEnemies[h].dest.left -= offset;
					theEnemies[h].dest.right -= offset;
					theEnemies[h].h = theEnemies[h].dest.left << 4;
					theEnemies[h].wasH = theEnemies[h].h;
													// Bounce enemy (negate velocity).
					if (theEnemies[h].hVel > 0)
						theEnemies[h].hVel = -(theEnemies[h].hVel >> 1);
					else
						theEnemies[h].hVel = theEnemies[h].hVel >> 1;
				}
				if (theEnemies[h].h < theEnemies[h].wasH)
				{									// Enemy was moving left (hit right side).
					offset = platformRects[i].right - theEnemies[h].dest.left;
													// Slide enemy "off" platform.
					theEnemies[h].dest.left += offset;
					theEnemies[h].dest.right += offset;
					theEnemies[h].h = theEnemies[h].dest.left << 4;
					theEnemies[h].wasH = theEnemies[h].h;
													// Bounce enemy (negate velocity).
					if (theEnemies[h].hVel < 0)
						theEnemies[h].hVel = -(theEnemies[h].hVel >> 1);
					else
						theEnemies[h].hVel = theEnemies[h].hVel >> 1;
				}
				doEnemyScrapeSound = TRUE;			// Play a collision sound.
													// Flip enemy to face opposite direction.
				theEnemies[h].facingRight = !theEnemies[h].facingRight;
			}
			else									// Enemy didn't hit from side.
			{										// See if enemy hit top/bottom.
				vRect.left = theEnemies[h].wasDest.left;
				vRect.right = theEnemies[h].wasDest.right;
				vRect.top = theEnemies[h].dest.top;
				vRect.bottom = theEnemies[h].dest.bottom;
													// Special "test rect" for top/bottom hit.
				if (SectRect(&vRect, &platformRects[i], &whoCares))
				{									// If hit the top/bottom of platform�
					if (theEnemies[h].mode == kFalling)
					{								// Was the enemy a falling egg?
													// Bounce egg (with some inelasticity).
						theEnemies[i].hVel -= (theEnemies[i].hVel >> 3);
													// When the eggs velocity is between�
													// +/- 8, consider the egg at rest.
						if ((theEnemies[i].hVel < 8) && (theEnemies[i].hVel > -8))
						{
							if (theEnemies[i].hVel > 0)
								theEnemies[i].hVel--;
							else if (theEnemies[i].hVel < 0)
								theEnemies[i].hVel++;
						}
					}
													// Specifically, did enemy hit the top?
					if (theEnemies[h].v > theEnemies[h].wasV)
					{								// Enemy heading down (hit platform top).
						offset = theEnemies[h].dest.bottom - platformRects[i].top;
													// Move enemy up off platform.
						theEnemies[h].dest.top -= offset;
						theEnemies[h].dest.bottom -= offset;
						theEnemies[h].v = theEnemies[h].dest.top << 4;
						theEnemies[h].wasV = theEnemies[h].v;
						if (theEnemies[h].vVel > kDontFlapVel)
							doEnemyScrapeSound = TRUE;
													// "Bounce" enemy.
						if (theEnemies[h].vVel > 0)
							theEnemies[h].vVel = -(theEnemies[h].vVel >> 1);
						else
							theEnemies[h].vVel = theEnemies[h].vVel >> 1;
						if ((theEnemies[h].vVel < 8) && (theEnemies[h].vVel > -8) && 
								(theEnemies[h].hVel == 0) && (theEnemies[h].mode == kFalling))
						{						// Here we handle an egg come to rest.
							if (((theEnemies[h].dest.right - 8) > platformRects[i].right) && 
									(theEnemies[h].hVel == 0))
							{					// Special case where egg right on edge.
								theEnemies[h].hVel = 32;
							}
							else if (((theEnemies[h].dest.left + 8) < platformRects[i].left) && 
									(theEnemies[h].hVel == 0))
							{					// Special case where egg right on edge.
								theEnemies[h].hVel = -32;
							}
							else				// If egg not on the edge of platform�
							{					// switch to "timer" mode.
								theEnemies[h].mode = kEggTimer;
								theEnemies[h].frame = (numOwls * 96) + 128;
								theEnemies[h].vVel = 0;
							}
						}
					}
					if (theEnemies[h].v < theEnemies[h].wasV)
					{							// Enemy was rising - hit bottom of platform.
						offset = theEnemies[h].dest.top - platformRects[i].bottom;
												// Slide enemy off platform.
						theEnemies[h].dest.top -= offset;
						theEnemies[h].dest.bottom -= offset;
						theEnemies[h].v = theEnemies[h].dest.top << 4;
						theEnemies[h].wasV = theEnemies[h].v;
												// Play collision sound.
						doEnemyScrapeSound = TRUE;
												// "Bounce" enemy downward from platform.
						if (theEnemies[h].vVel < 0)
							theEnemies[h].vVel = -(theEnemies[h].vVel >> 2);
						else
							theEnemies[h].vVel = theEnemies[h].vVel >> 2;
						if ((theEnemies[h].vVel < 8) && (theEnemies[h].vVel > -8) && 
								(theEnemies[h].hVel == 0) && (theEnemies[h].mode == kFalling))
						{
							theEnemies[h].mode = kEggTimer;
							theEnemies[h].frame = (numOwls * 96) + 128;
							theEnemies[h].vVel = 0;
						}
					}
				}
			}
		}
	}
}

//--------------------------------------------------------------  CheckEnemyRoofCollision

// Like the player counterpart, this function checks to see if an enemy has hit�
// the ceiling or the lava.  It handles the consequences of both cases.

void CheckEnemyRoofCollision (short i)
{
	short		offset;
	
	if (theEnemies[i].dest.top < (kRoofHeight - 2))
	{					// If enemy has hit the ceiling�
		offset = kRoofHeight - theEnemies[i].dest.top;
						// Move enemy down to a "legal" altitude.
		theEnemies[i].dest.top += offset;
		theEnemies[i].dest.bottom += offset;
		theEnemies[i].v = theEnemies[i].dest.top << 4;
						// Play a collision sound.
		doEnemyScrapeSound = TRUE;
						// Bounce enemy downward.
		theEnemies[i].vVel = -(theEnemies[i].vVel >> 2);
	}
	else if (theEnemies[i].dest.top > kLavaHeight)
	{					// If enemy has fallen into lava�
						// kill that enemy.
		theEnemies[i].mode = kDeadAndGone;
		deadEnemies++;
						// Play a splash sound.
		PlayExternalSound(kSplashSound, kSplashPriority);
						// Call up another from the ranks.
		InitEnemy(i, TRUE);
	}
}

//--------------------------------------------------------------  HandleIdleEnemies

// The following functions handle the various enemy modes.  Enemies are�
// considered to be in a specific mode and each mode is handled differently.
// Idle enemies are ones who are "invisible" - not yet born.  While idle, a�
// timer is ticking down - when it reaches zero, the enemy appears.

void HandleIdleEnemies (short i)
{
	theEnemies[i].frame--;					// Decrement timer.
	if (theEnemies[i].frame <= 0)			// If timer is zero or less�
	{
		theEnemies[i].mode = kSpawning;		// enemy is "born".
		theEnemies[i].wasH = theEnemies[i].h;
		theEnemies[i].wasV = theEnemies[i].v;
		theEnemies[i].hVel = 0;
		theEnemies[i].vVel = 0;
		theEnemies[i].frame = 0;
		SetEnemyAttributes(i);				// Initialize enemy attributes.
		PlayExternalSound(kSpawnSound, kSpawnPriority);
	}
}

//--------------------------------------------------------------  HandleFlyingEnemies

// Once an enemy takes off from a platform, they will always be in flying mode�
// unless they should be killed.  This function handles the flying mode.

void HandleFlyingEnemies (short i)
{
	short		dist;
	Boolean		shouldFlap;
								// Take into account gravity pulling enemy down.
	theEnemies[i].vVel += kGravity;
								// Get absolute difference in enemy/player altitude.
	dist = thePlayer.dest.top - theEnemies[i].dest.top;
	if (dist < 0)
		dist = -dist;
								// See if the player is within the enemy's "seek" range.
	if ((dist < theEnemies[i].heightSmell) && 
			((thePlayer.mode == kFlying) || (thePlayer.mode == kWalking)))
	{							// Enemy will actively seek the player.
		if (thePlayer.dest.left < theEnemies[i].dest.left)
		{						// Determine if quicker to go left or right to get player.
			dist = theEnemies[i].dest.left - thePlayer.dest.left;
			if (dist < 320)		// Closest route is to the left.
				theEnemies[i].facingRight = FALSE;
			else				// Closest route is to the right.
				theEnemies[i].facingRight = TRUE;
		}
		else if (thePlayer.dest.left > theEnemies[i].dest.left)
		{						// Determine if quicker to go left or right to get player.
			dist = thePlayer.dest.left - theEnemies[i].dest.left;
			if (dist < 320)		// Closest route is to the right.
				theEnemies[i].facingRight = TRUE;
			else				// Closest route is to the left.
				theEnemies[i].facingRight = FALSE;
		}
								// Seek an altitude 16 pixels above player.
		if (((theEnemies[i].v + 16) > thePlayer.v) && (evenFrame))
			shouldFlap = TRUE;
		else
			shouldFlap = FALSE;
	}
	else						// Else, player not within enemy's "seek" altitude.
	{							// Flap if necessary to maintain "patrol altitude".
		if ((theEnemies[i].v > theEnemies[i].targetAlt) && (evenFrame))
			shouldFlap = TRUE;
		else
			shouldFlap = FALSE;
	}
	
	if (shouldFlap)				// If the enemy has determined that it needs to flap�
	{							// Give the enemy lift & play the flap sound.
		theEnemies[i].vVel -= theEnemies[i].flapImpulse;
		doEnemyFlapSound = TRUE;
	}
								// Enemy never hovers - must move right or left.
	if (theEnemies[i].facingRight)
	{							// If enemy facing right - move enemy to the right.
		theEnemies[i].hVel += kEnemyImpulse;
		if (theEnemies[i].hVel > theEnemies[i].maxHVel)
			theEnemies[i].hVel = theEnemies[i].maxHVel;
								// Determine correct graphic for enemy.
		switch (theEnemies[i].kind)
		{
			case kOwl:
			if (shouldFlap)
				theEnemies[i].srcNum = 12;
			else
				theEnemies[i].srcNum = 13;
			break;
			
			case kWolf:
			if (shouldFlap)
				theEnemies[i].srcNum = 16;
			else
				theEnemies[i].srcNum = 17;
			break;
			
			case kJackal:
			if (shouldFlap)
				theEnemies[i].srcNum = 20;
			else
				theEnemies[i].srcNum = 21;
			break;
		}
		
	}
	else						// If enemy not facing right (left) move to the left.
	{
		theEnemies[i].hVel -= kEnemyImpulse;
		if (theEnemies[i].hVel < -theEnemies[i].maxHVel)
			theEnemies[i].hVel = -theEnemies[i].maxHVel;
								// Determine correct graphic for enemy.
		switch (theEnemies[i].kind)
		{
			case kOwl:
			if (shouldFlap)
				theEnemies[i].srcNum = 14;
			else
				theEnemies[i].srcNum = 15;
			break;
			
			case kWolf:
			if (shouldFlap)
				theEnemies[i].srcNum = 18;
			else
				theEnemies[i].srcNum = 19;
			break;
			
			case kJackal:
			if (shouldFlap)
				theEnemies[i].srcNum = 22;
			else
				theEnemies[i].srcNum = 23;
			break;
		}
	}
										// Move enemy horizontally based on hori velocity.
	theEnemies[i].h += theEnemies[i].hVel;
	theEnemies[i].dest.left = theEnemies[i].h >> 4;
	theEnemies[i].dest.right = theEnemies[i].dest.left + 64;
										// Move enemy vertically based on vertical velocity.
	theEnemies[i].v += theEnemies[i].vVel;
	theEnemies[i].dest.top = theEnemies[i].v >> 4;
	theEnemies[i].dest.bottom = theEnemies[i].dest.top + 40;
										// Check for wrap-around.
	if (theEnemies[i].dest.left > 640)
	{									// If off right edge, wrap around to left side.
		OffsetRect(&theEnemies[i].dest, -640, 0);
		theEnemies[i].h = theEnemies[i].dest.left << 4;
		OffsetRect(&theEnemies[i].wasDest, -640, 0);
		theEnemies[i].pass++;			// Increment number of "wrap-arounds" for this enemy.
		if (theEnemies[i].pass > 2)		// After two screen passes (wrap arounds)�
		{								// enemy patrols a new altitude.
			theEnemies[i].targetAlt = AssignNewAltitude();
			theEnemies[i].pass = 0;
		}
	}
	else if (theEnemies[i].dest.right < 0)
	{									// If off left edge, wrap around to right side.
		OffsetRect(&theEnemies[i].dest, 640, 0);
		theEnemies[i].h = theEnemies[i].dest.left << 4;
		OffsetRect(&theEnemies[i].wasDest, 640, 0);
		theEnemies[i].pass++;
		if (theEnemies[i].pass > 2)
		{
			theEnemies[i].targetAlt = AssignNewAltitude();
			theEnemies[i].pass = 0;
		}
	}
										// Throw a touch of friction into the mix.
	theEnemies[i].vVel -= theEnemies[i].vVel >> 4;
										// Keep enemies from moving excessively fast.
	if (theEnemies[i].vVel > theEnemies[i].maxVVel)
		theEnemies[i].vVel = theEnemies[i].maxVVel;
	else if (theEnemies[i].vVel < -theEnemies[i].maxVVel)
		theEnemies[i].vVel = -theEnemies[i].maxVVel;
	
	CheckEnemyRoofCollision(i);			// Check for lava/celing collisions.
	CheckEnemyPlatformHit(i);			// Check for platform collisions.
}

//--------------------------------------------------------------  HandleWalkingEnemy

// This is a brief mode for an enemy.  When an enemy has hatched from an egg, it�
// walks only for 8 game frames at which point it takes off and flies for the rest�
// of its life.

void HandleWalkingEnemy (short i)
{
	if (theEnemies[i].facingRight)		// If enemy facing right, walk to the right.
	{
		theEnemies[i].dest.left += 6;	// Move enemy to right.
		theEnemies[i].dest.right += 6;
		switch (theEnemies[i].kind)		// Determine correct graphic for walking enemy.
		{
			case kOwl:
			theEnemies[i].srcNum = 1 - theEnemies[i].srcNum;
			break;
			
			case kWolf:
			theEnemies[i].srcNum = 9 - theEnemies[i].srcNum;
			break;
			
			case kJackal:
			theEnemies[i].srcNum = 17 - theEnemies[i].srcNum;
			break;
		}
		theEnemies[i].hVel = 6 << 4;
	}
	else								// If enemy not facing right (left), walk to the left.
	{
		theEnemies[i].dest.left -= 6;	// Move enemy to left.
		theEnemies[i].dest.right -= 6;
		switch (theEnemies[i].kind)		// Determine correct graphic for walking enemy.
		{
			case kOwl:
			theEnemies[i].srcNum = 5 - theEnemies[i].srcNum;
			break;
			
			case kWolf:
			theEnemies[i].srcNum = 13 - theEnemies[i].srcNum;
			break;
			
			case kJackal:
			theEnemies[i].srcNum = 21 - theEnemies[i].srcNum;
			break;
		}
		theEnemies[i].hVel = -6 << 4;
	}
	
	theEnemies[i].frame++;				// Increment number of frames it has walked for.
	if (theEnemies[i].frame >= 8)		// If over 8, enemy takes off an flies.
	{
		theEnemies[i].mode = kFlying;	// Switch to flying mode.
		theEnemies[i].frame = 0;		// Reset "frame" variable.
		switch (theEnemies[i].kind)		// Determine correct graphic for flying enemy.
		{
			case kOwl:
			if (theEnemies[i].facingRight)
				theEnemies[i].srcNum = 12;
			else
				theEnemies[i].srcNum = 14;
			break;
			
			case kWolf:
			if (theEnemies[i].facingRight)
				theEnemies[i].srcNum = 16;
			else
				theEnemies[i].srcNum = 18;
			break;
			
			case kJackal:
			if (theEnemies[i].facingRight)
				theEnemies[i].srcNum = 20;
			else
				theEnemies[i].srcNum = 22;
			break;
		}
										// Re-size enemy bounds to a "flying" size.
		theEnemies[i].dest.left -= 8;
		theEnemies[i].dest.right += 8;
		theEnemies[i].dest.bottom = theEnemies[i].dest.top + 40;
		theEnemies[i].h = theEnemies[i].dest.left * 16;
		theEnemies[i].v = theEnemies[i].dest.top * 16;
	}
}

//--------------------------------------------------------------  HandleSpawningEnemy

// This is an enemy "rising out of a platform".  Either an egg has just hatched�
// or a brand new enemy has been introduced.  Irregardless, the sphinx is born.
// When the enemy is at its full height, it will begin to walk.

void HandleSpawningEnemy (short i)
{
	theEnemies[i].frame++;				// Advance timer.
	if (theEnemies[i].frame >= 48)		// If timer >= 48, enemy begins to walk.
	{
		theEnemies[i].mode = kWalking;
		theEnemies[i].frame = 0;
		
		switch (theEnemies[i].kind)		// Determine appropriate graphic.
		{
			case kOwl:
			if (theEnemies[i].facingRight)
				theEnemies[i].srcNum = 0;
			else
				theEnemies[i].srcNum = 2;
			break;
			
			case kWolf:
			if (theEnemies[i].facingRight)
				theEnemies[i].srcNum = 4;
			else
				theEnemies[i].srcNum = 6;
			break;
			
			case kJackal:
			if (theEnemies[i].facingRight)
				theEnemies[i].srcNum = 8;
			else
				theEnemies[i].srcNum = 10;
			break;
		}
	}
	else							// If not full height, use "timer" to determine height.
		theEnemies[i].dest.top = theEnemies[i].dest.bottom - theEnemies[i].frame;
}

//--------------------------------------------------------------  HandleFallingEnemy

// A "falling" enemy is an air borne egg.  The enemy was killed, turned into an egg, �
// and the egg is in freefall.  If the egg comes to rest, it will begin a countdown�
// until it is hatched.

void HandleFallingEnemy (short i)
{									// Take into account gravity - accelerate egg down.
	theEnemies[i].vVel += kGravity;
									// Don't allow velocities to skyrocket.
	if (theEnemies[i].vVel > theEnemies[i].maxVVel)
		theEnemies[i].vVel = theEnemies[i].maxVVel;
	else if (theEnemies[i].vVel < -theEnemies[i].maxVVel)
		theEnemies[i].vVel = -theEnemies[i].maxVVel;
	
	if (evenFrame)					// Apply friction on even frames (who knows).
	{								// "Friction" is 1/32nd of the velocity.
		theEnemies[i].hVel -= (theEnemies[i].hVel >> 5);
		if ((theEnemies[i].hVel < 32) && (theEnemies[i].hVel > -32))
		{
			if (theEnemies[i].hVel > 0)
				theEnemies[i].hVel--;
			else if (theEnemies[i].hVel < 0)
				theEnemies[i].hVel++;
		}
	}
									// Move egg horizontally.
	theEnemies[i].h += theEnemies[i].hVel;
	theEnemies[i].dest.left = theEnemies[i].h >> 4;
	theEnemies[i].dest.right = theEnemies[i].dest.left + 24;
									// Move egg vertically.
	theEnemies[i].v += theEnemies[i].vVel;
	theEnemies[i].dest.top = theEnemies[i].v >> 4;
	theEnemies[i].dest.bottom = theEnemies[i].dest.top + 24;
									// Check for wrap around.
	if (theEnemies[i].dest.left > 640)
	{
		OffsetRect(&theEnemies[i].dest, -640, 0);
		theEnemies[i].h = theEnemies[i].dest.left << 4;
		OffsetRect(&theEnemies[i].wasDest, -640, 0);
	}
	else if (theEnemies[i].dest.right < 0)
	{
		OffsetRect(&theEnemies[i].dest, 640, 0);
		theEnemies[i].h = theEnemies[i].dest.left << 4;
		OffsetRect(&theEnemies[i].wasDest, 640, 0);
	}
	
	CheckEnemyRoofCollision(i);	// See if egg hit ceiling or lava.
	CheckEnemyPlatformHit(i);	// Handle platform hit (it is here it determines if�
								// egg has come to rest and should begin countdown).
}

//--------------------------------------------------------------  HandleEggEnemy

// This is the "idle" egg mode.  This is a static egg, sitting peacefully on�
// a platform.  Waiting patiently so it might hatch into a death-sphinx and�
// slaughter the player.

void HandleEggEnemy (short i)
{
	short		center;
	
	theEnemies[i].frame--;				// Decrement the egg timer!
	if (theEnemies[i].frame < 24)		// When it falls below 24, egg starts shrinking.
	{									// Use "frame" to determine height of egg.
		theEnemies[i].dest.top = theEnemies[i].dest.bottom - theEnemies[i].frame;
		if (theEnemies[i].frame <= 0)	// When the egg is completely flat (gone)�
		{								// then BOOM! a sphinx is spawned!
			theEnemies[i].frame = 0;
			PlayExternalSound(kSpawnSound, kSpawnPriority);
			center = (theEnemies[i].dest.left + theEnemies[i].dest.right) >> 1;
										// Resize enemy bounds to new "walking enemy" size.
			theEnemies[i].dest.left = center - 24;
			theEnemies[i].dest.right = center + 24;
			theEnemies[i].wasDest = theEnemies[i].dest;
			theEnemies[i].h = theEnemies[i].dest.left << 4;
			theEnemies[i].v = theEnemies[i].dest.top << 4;
										// Set up all other enemy variables.
			theEnemies[i].wasH = theEnemies[i].h;
			theEnemies[i].wasV = theEnemies[i].v;
			theEnemies[i].hVel = 0;
			theEnemies[i].vVel = 0;
			theEnemies[i].mode = kSpawning;
			theEnemies[i].kind++;
			if (theEnemies[i].kind > kJackal)
				theEnemies[i].kind = kJackal;
			SetEnemyAttributes(i);
		}
	}
}

//--------------------------------------------------------------  MoveEnemies

// This is the "master" enemy function.  It goes through all the enemies�
// and calls the above functions depending upon an enemy's mode.

void MoveEnemies (void)
{
	short		i;
	
	doEnemyFlapSound = FALSE;		// Intially, assume no flap or scrape sounds.
	doEnemyScrapeSound = FALSE;
									// Go through each enemy.
	for (i = 0; i < numEnemies; i++)
	{
		switch (theEnemies[i].mode)
		{							// Handle enemy according to mode it is in.
			case kIdle:				// Enemy not born yet.
			HandleIdleEnemies(i);
			break;
			
			case kFlying:			// Enemy air borne.
			HandleFlyingEnemies(i);
			break;
			
			case kWalking:			// Enemy just born, walking off platform.
			HandleWalkingEnemy(i);
			break;
			
			case kSpawning:			// Enemy growing from a platform.
			HandleSpawningEnemy(i);
			break;
			
			case kFalling:			// Enemy is an egg in flight.
			HandleFallingEnemy(i);
			break;
			
			case kEggTimer:			// Enemy is a patient, idle, silent egg.
			HandleEggEnemy(i);
			break;
			
			case kDeadAndGone:		// Enemy no more - gone for good this level.
			break;
		}
	}
									// If any sounds were flagged, play them.
	if (doEnemyFlapSound)
		PlayExternalSound(kFlap2Sound, kFlap2Priority);
	if (doEnemyScrapeSound)
		PlayExternalSound(kScrape2Sound, kScrape2Priority);
									// See if enough enemies were killed to advance to�
									// next level (wave).
	if ((deadEnemies >= numEnemiesThisLevel) && (countDownTimer == 0))
		countDownTimer = 30;
}

//--------------------------------------------------------------  InitHandLocation

// This simply sets up the hand.  Puts it deep in the lava (off bottom of screen).

void InitHandLocation (void)
{
	SetRect(&theHand.dest, 0, 0, 56, 57);
	OffsetRect(&theHand.dest, 48, 460);
}

//--------------------------------------------------------------  HandleHand

// This is the hand "AI".  The hand, like the sphinx enemies, has modes.

void HandleHand (void)
{
	Rect		whoCares;
	short		hDiff, vDiff, pull, speed;
	
	switch (theHand.mode)
	{
		case kLurking:				// Hand is down, waiting for player to stray near.
		if ((thePlayer.mode == kFlying) && (SectRect(&thePlayer.dest, &grabZone, &whoCares)))
		{							// If player flies near, hand begins to reach out.
			theHand.mode = kOutGrabeth;
			InitHandLocation();
		}
		break;
		
		case kOutGrabeth:			// Hand is either coming after or has a hold of player.
		case kClutching:
		if (SectRect(&thePlayer.dest, &grabZone, &whoCares))
		{							// See if player in the "grab/clutch zone".
			hDiff = theHand.dest.left - thePlayer.dest.left;
			vDiff = theHand.dest.top - thePlayer.dest.top;
									// Ah!  Player caught.  Move player to correct�
									// location relative to the hand (so the player�
									// appears to, in fact, be held).
			if (thePlayer.facingRight)
				hDiff -= 3;
			else
				hDiff -= 21;
			vDiff -= 29;
									// How hard/fast the hand moves depends on level.
			speed = (levelOn >> 3) + 1;
			if (hDiff < 0)
			{
				theHand.dest.left += speed;
				theHand.dest.right += speed;
			}
			else if (hDiff > 0)
			{
				theHand.dest.left -= speed;
				theHand.dest.right -= speed;
			}
			if (vDiff < 0)
			{
				theHand.dest.top += speed;
				theHand.dest.bottom += speed;
			}
			else if (vDiff > 0)
			{
				theHand.dest.top -= speed;
				theHand.dest.bottom -= speed;
			}
									// Determine absolute distance player is from hand.
			if (hDiff < 0)
				hDiff = -hDiff;
			if (vDiff < 0)
				vDiff = -vDiff;
			
			if ((hDiff < 8) && (vDiff < 8))
			{						// If player in the "hot zone", player is nabbed!
				theHand.mode = kClutching;
				thePlayer.clutched = TRUE;
									// Player's movement is severely dampened.
				thePlayer.hVel = thePlayer.hVel >> 3;
				thePlayer.vVel = thePlayer.vVel >> 3;
									// Hand pulls player down (strength is greater on�
									// higher levels).
				pull = levelOn << 2;
				if (pull > 48)		// Set an absolute limit on hand strength.
					pull = 48;
									// Pull player donw!
				thePlayer.vVel += pull;
				theHand.dest.top = thePlayer.dest.top + 29;
				theHand.dest.bottom = theHand.dest.top + 57;
				if (thePlayer.facingRight)
					theHand.dest.left = thePlayer.dest.left + 3;
				else
					theHand.dest.left = thePlayer.dest.left + 21;
				theHand.dest.right = theHand.dest.left + 58;
			}
			else					// If player not in "sweet spot", hand is seeking.
			{
				thePlayer.clutched = FALSE;
				theHand.mode = kOutGrabeth;
			}
		}
		else						// Player not even close to hand�
		{							// Hand sinks back down into lava.
			theHand.dest.top++;
			theHand.dest.bottom++;
									// When hand is off screen, hand resumes lurking.
			if (theHand.dest.top > 460)
				theHand.mode = kLurking;
			else
				theHand.mode = kOutGrabeth;
			thePlayer.clutched = FALSE;
		}
		break;
	}
}

//--------------------------------------------------------------  InitEye

// This initializes all the eye's variables.

void InitEye (void)
{
	SetRect(&theEye.dest, 0, 0, 48, 31);
	OffsetRect(&theEye.dest, 296, 97);
	theEye.mode = kWaiting;
	theEye.frame = (numOwls + 2) * 720;
	theEye.srcNum = 0;
	theEye.opening = 1;
	theEye.killed = FALSE;
	theEye.entering = FALSE;
}

//--------------------------------------------------------------  KillOffEye

// This function handles a "slain" eye!

void KillOffEye (void)
{
	if (theEye.mode == kStalking)
	{
		theEye.killed = TRUE;
		theEye.opening = 1;
		theEye.entering = FALSE;
		if (theEye.srcNum == 0)
			theEye.srcNum = 1;
	}
	else
		InitEye();
}

//--------------------------------------------------------------  HandleEye

// But of course, the eye has modes as well.  This function handles the eye�
// depending upon the mode it is in.

void HandleEye (void)
{
	short		diffH, diffV, speed;
	
	if (theEye.mode == kStalking)		// Eye is alive!
	{
		speed = (levelOn >> 4) + 1;		// How fast it moves depends on level.
		if (speed > 3)
			speed = 3;
										// When eye appears or dies, it is stationary.
		if ((theEye.killed) || (theEye.entering))
		{
			speed = 0;
		}
		else if ((thePlayer.mode != kFlying) && (thePlayer.mode != kWalking))
		{
			diffH = theEye.dest.left - 296;
			diffV = theEye.dest.bottom - 128;
		}
		else
		{
			diffH = theEye.dest.left - thePlayer.dest.left;
			diffV = theEye.dest.bottom - thePlayer.dest.bottom;
		}
										// Find direction to player (no wrap-around for eye).
		if (diffH > 0)
		{
			if (diffH < speed)
				theEye.dest.left -= diffH;
			else
				theEye.dest.left -= speed;
			theEye.dest.right = theEye.dest.left + 48;
		}
		else if (diffH < 0)
		{
			if (-diffH < speed)
				theEye.dest.left -= diffH;
			else
				theEye.dest.left += speed;
			theEye.dest.right = theEye.dest.left + 48;
		}
		if (diffV > 0)
		{
			if (diffV < speed)
				theEye.dest.bottom -= diffV;
			else
				theEye.dest.bottom -= speed;
			theEye.dest.top = theEye.dest.bottom - 31;
		}
		else if (diffV < 0)
		{
			if (-diffV < speed)
				theEye.dest.bottom -= diffV;
			else
				theEye.dest.bottom += speed;
			theEye.dest.top = theEye.dest.bottom - 31;
		}
		
		theEye.frame++;					// Increment eye frame (timer).
										// Determine correct graphic for eye.
		if (theEye.srcNum != 0)
		{
			if (theEye.frame > 3)		// "Eye-closing frame" holds for 3 frames.
			{
				theEye.frame = 0;
				theEye.srcNum += theEye.opening;
				if (theEye.srcNum > 3)
				{
					theEye.srcNum = 3;
					theEye.opening = -1;
					if (theEye.killed)
						InitEye();
				}
				else if (theEye.srcNum <= 0)
				{
					theEye.srcNum = 0;
					theEye.opening = 1;
					theEye.frame = 0;
					theEye.entering = FALSE;
				}
			}
		}
		else if (theEye.frame > 256)
		{
			theEye.srcNum = 1;
			theEye.opening = 1;
			theEye.frame = 0;
		}
										// Get absolute distance from eye to player.
		diffH = theEye.dest.left - thePlayer.dest.left;
		diffV = theEye.dest.bottom - thePlayer.dest.bottom;
		if (diffH < 0)
			diffH = -diffH;
		if (diffV < 0)
			diffV = -diffV;
										// See if player close enough to be killed!
		if ((diffH < 16) && (diffV < 16) && (!theEye.entering) && 
				(!theEye.killed))		// Close enough to call it a kill.
		{
			if (theEye.srcNum == 0)		// If eye was open, player is killed.
			{							// Strike lightning (hit the player).
				if (lightningCount == 0)
				{
					lightH = thePlayer.dest.left + 24;
					lightV = thePlayer.dest.bottom - 24;
					lightningCount = 6;	// Strike 6 times!
				}
										// Player is smokin' bones!
				thePlayer.mode = kFalling;
				if (thePlayer.facingRight)
					thePlayer.srcNum = 8;
				else
					thePlayer.srcNum = 9;
				thePlayer.dest.bottom = thePlayer.dest.top + 37;
				PlayExternalSound(kBoom2Sound, kBoom2Priority);
			}
			else						// If the eye was "blinking", IT was killed!
			{							// Player killed the eye!
				if (lightningCount == 0)
				{						// Strike the eye with lightning!
					lightH = theEye.dest.left + 24;
					lightV = theEye.dest.top + 16;
										// Hit 'er with 15 bolts!
					lightningCount = 15;
				}
				theScore += 2000L;		// A big 2000 points for killing the eye!
				UpdateScoreNumbers();	// Refresh score display.
				PlayExternalSound(kBonusSound, kBonusPriority);
				
				KillOffEye();			// Slay eye!
			}							// Hey, anyone remember that giant eye from�
		}								// Johnny Socko and his Flying Robot?
	}									// As a kid, I thought that was cool!
	else if (theEye.frame > 0)			// Eye has not yet appeared, but waits, lurking!
	{
		theEye.frame--;					// Decrement eye timer.
		if (theEye.frame <= 0)			// When timer hits zero, eye appears!
		{
			theEye.mode = kStalking;	// The eye is after the player!
			if (lightningCount == 0)	// Strike lightning at eye!
			{
				lightH = theEye.dest.left + 24;
				lightV = theEye.dest.top + 16;
				lightningCount = 6;
			}
			theEye.srcNum = 3;
			theEye.opening = 1;
			theEye.entering = TRUE;
		}
	}	
}

//--------------------------------------------------------------  ResolveEnemyPlayerHit

// Okay, a bounds test determined that the player and an enemy have collided.
// This function looks at the two and determines who wins or if it's a draw.

void ResolveEnemyPlayerHit (short i)
{
	short		wasVel, diff, h, v;
	
	if ((theEnemies[i].mode == kFalling) || (theEnemies[i].mode == kEggTimer))
	{						// Okay, if the enemy is an egg�
		deadEnemies++;		// simple - the enemy dies.
		
		theEnemies[i].mode = kDeadAndGone;
		theScore += 500L;	// Add that to our score!
		UpdateScoreNumbers();
		PlayExternalSound(kBonusSound, kBonusPriority);
		InitEnemy(i, TRUE);	// Reset the enemy (I guess you could say they're reincarnated.
	}
	else					// Now, here's a real, live sphinx enemy.
	{						// Get their difference in altitude.
		diff = (theEnemies[i].dest.top + 25) - (thePlayer.dest.top + 19);
		
		if (diff < -2)		// Player is bested.  :(
		{					// Strike player with lightning.
			if (lightningCount == 0)
			{
				lightH = thePlayer.dest.left + 24;
				lightV = thePlayer.dest.bottom - 24;
				lightningCount = 6;
			}
							// Player is bones.
			thePlayer.mode = kFalling;
			if (thePlayer.facingRight)
				thePlayer.srcNum = 8;
			else
				thePlayer.srcNum = 9;
			thePlayer.dest.bottom = thePlayer.dest.top + 37;
			PlayExternalSound(kBoom2Sound, kBoom2Priority);
		}
		else if (diff > 2)	// Yes!  Enemy is killed!
		{					// Well ... we can't kill an enemy who is spawning.
			if ((theEnemies[i].mode == kSpawning) && (theEnemies[i].frame < 16))
				return;
							// Resize enemy bounds (use an egg bounds).
			h = (theEnemies[i].dest.left + theEnemies[i].dest.right) >> 1;
			if (theEnemies[i].mode == kSpawning)
				v = theEnemies[i].dest.bottom - 2;
			else
				v = (theEnemies[i].dest.top + theEnemies[i].dest.bottom) >> 1;
			theEnemies[i].dest.left = h - 12;
			theEnemies[i].dest.right = h + 12;
			if (theEnemies[i].mode == kSpawning)
				theEnemies[i].dest.top = v - 24;
			else
				theEnemies[i].dest.top = v - 12;
			theEnemies[i].dest.bottom = theEnemies[i].dest.top + 24;
			theEnemies[i].h = theEnemies[i].dest.left << 4;
			theEnemies[i].v = theEnemies[i].dest.top << 4;
							// Enemy is a falling egg!
			theEnemies[i].mode = kFalling;
			theEnemies[i].wasDest = theEnemies[i].dest;
			theEnemies[i].wasH = theEnemies[i].h;
			theEnemies[i].wasV = theEnemies[i].v;
							// Give player points based on enemy kind.
			switch (theEnemies[i].kind)
			{
				case kOwl:
				theScore += 500L;
				break;
				
				case kWolf:
				theScore += 1000L;
				break;
				
				case kJackal:
				theScore += 1500L;
				break;
			}
			UpdateScoreNumbers();
			PlayExternalSound(kBoom2Sound, kBoom2Priority);
		}
		else		// Rare case - neither the player nor the enemy get killed.
		{			// They'll bounce off one another.
			if (theEnemies[i].hVel > 0)
				theEnemies[i].facingRight = TRUE;
			else
				theEnemies[i].facingRight = FALSE;
			PlayExternalSound(kScreechSound, kScreechPriority);
		}
		
		wasVel = thePlayer.hVel;
		thePlayer.hVel = theEnemies[i].hVel;
		theEnemies[i].hVel = wasVel;
		wasVel = thePlayer.vVel;
		thePlayer.vVel = theEnemies[i].vVel;
		theEnemies[i].vVel = wasVel;
	}
}

//--------------------------------------------------------------  CheckPlayerEnemyCollision

// This is a simple "bounds test" for determining player/enemy collisions.

void CheckPlayerEnemyCollision (void)
{
	Rect		whoCares, playTest, wrapTest;
	short		i;
	
	playTest = thePlayer.dest;		// Make a copy of player's bounds.
	InsetRect(&playTest, 8, 8);		// Shrink it by 8 pixels all 'round.
	if (thePlayer.wrapping)			// Need to test 2 players if "wraparounding".
		wrapTest = thePlayer.wrap;
	InsetRect(&wrapTest, 8, 8);
									// Test all enemies.
	for (i = 0; i < numEnemies; i++)
	{								// Ignore non-existant enemies.
		if ((theEnemies[i].mode != kIdle) && (theEnemies[i].mode != kDeadAndGone))
		{							// Simple bounds test.
			if (SectRect(&playTest, &theEnemies[i].dest, &whoCares))
			{						// Call function to determine who wins (or tie).
				ResolveEnemyPlayerHit(i);
			}						// If "wrap-arounding", test other rect.
			else if (thePlayer.wrapping)
			{
				if (SectRect(&wrapTest, &theEnemies[i].dest, &whoCares))
					ResolveEnemyPlayerHit(i);
			}
		}
	}
}

