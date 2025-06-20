/****************************************************************************
 * TopScores.c
 *
 *		TopScore Stuff
 *
 * JAB 4/28/93 - 5/93
 ****************************************************************************/
#include "CToast.h"

ScoreRecord	scoreRecord[MaxScoreRecords];
short		gLastHighScore=-1;

void LoadTopScores()
{
	Handle	h;
	Ptr		p;
	short	i;
	h = Get1Resource(TopScoreResType,128);
	if (h != NULL) {
		p = *h;
		for (i = 0; i < MaxScoreRecords; ++i) {
			scoreRecord[i].score = *((long *) p);	p += sizeof(long);
			scoreRecord[i].level = *((short *) p);	p += sizeof(short);
			BlockMove(p,scoreRecord[i].name,p[0]+1);
			p += p[0]+1+((p[0]&1)? 0 : 1);
		}
		ReleaseResource(h);
	}
}

void IntegrateScore(long score, short level)
{
	Handle	h,h2;
	Ptr		p;
	short	i,j,id;
	if (score > scoreRecord[MaxScoreRecords-1].score) {
		if (MyRandom(2) == 0)
			PlaySound(S_HighScore1, 5);
		else
			PlaySound(S_HighScore2, 5);
		// Adjust Scores
		for (i = MaxScoreRecords - 1; i >= 0; --i) {
			if (scoreRecord[i].score > score)
				break;
		}
		for (j = MaxScoreRecords - 1; j > i; --j)
			scoreRecord[j] = scoreRecord[j-1];
		++i;
		gLastHighScore = i;
		scoreRecord[i].score = score;
		scoreRecord[i].level = level;
		// Get the person's name...
		GetUserName(gPrefs.userName);
		SavePreferences();
		BlockMove(gPrefs.userName, scoreRecord[i].name, gPrefs.userName[0]+1);

		// Save Scores
		h = NewHandle(sizeof(ScoreRecord) * MaxScoreRecords);
		if (h) {
			while ((h2 = GetResource(TopScoreResType, 128)) != NULL) {
				RmveResource(h2);
				DisposHandle(h2);
			}

			p = *h;
			for (i = 0; i < MaxScoreRecords; ++i) {
				*((long *) p) = scoreRecord[i].score;	p += sizeof(long);
				*((short *) p) = scoreRecord[i].level;	p += sizeof(short);
				BlockMove(scoreRecord[i].name,p,scoreRecord[i].name[0]+1);
				p += p[0]+1+((p[0]&1)? 0 : 1);

			}
			SetHandleSize(h,(long)p-(long) *h);
			AddResource(h,TopScoreResType,128,"\pTop Scores");
			WriteResource(h);
			ReleaseResource(h);
		}
	}
	else
		PlaySound(S_Loser, 5);
}

void DisplayTopScores()
{
	short		i;
	RGBColor	fc,bc;
	Rect		r;
	short		x,y;
	short		topY,leftX;
	static StringPtr	titleStr = "\pCheeze Whizzes";

	TextFont(geneva);
	TextSize(14);
	TextMode(srcOr);
	TextFace(bold);

	if (g12InchMode)
		topY = 264;
	else
		topY = 354;
	
	x = 157-StringWidth(titleStr)/2;
	y = topY;

	fc.red = 0x0000;
	fc.green = 0x0000; 
	fc.blue = 0x9999;
	RGBForeColor(&fc);
	MoveTo(x+2,y+2);
	DrawString(titleStr);

	fc.red = 0x0000;
	fc.green = 0x0000; 
	fc.blue = 0xFFFF;
	RGBForeColor(&fc);
	MoveTo(x+1,y+1);
	DrawString(titleStr);

	fc.red = 0xEEEE;
	fc.green = 0xFEEE; 
	fc.blue = 0xFFFF;
	RGBForeColor(&fc);
	MoveTo(x,y);
	DrawString(titleStr);
	
	TextFont(systemFont);
	TextSize(12);
	
	fc.red = 0x0000;
	fc.green = 0xFFFF; 
	fc.blue = 0x0000;
	RGBForeColor(&fc);
	
	SetRect(&r, 50,359, 265,509);
//	FrameRect(&r);

	for (i = 0; i < MaxScoreRecords; ++i) {
		if (i == gLastHighScore) {
			fc.red = 0x9999;
			fc.green = 0xBBBB; 
			fc.blue = 0xFFFF;
			RGBForeColor(&fc);
		}
		else {

// Black 0x0000     Medium 0x8888     White 0xFFFF   0 1 2 3 4 5 6 7 8 9 A B C D E F
			fc.red = 0x8888;
			fc.green = 0x8888; 
			fc.blue = 0x8888;
			RGBForeColor(&fc);
		}
		PrintfXY(32+24,topY+20+14*i,"%-20.*s",scoreRecord[i].name[0],&scoreRecord[i].name[1]);
		PrintfXY(190,topY+20+14*i,"%d", scoreRecord[i].level);
		PrintfXY(220,topY+20+14*i,"%ld",scoreRecord[i].score);
	}

	fc.red = fc.green = fc.blue = 0x0000;
	RGBForeColor(&fc);
}

#define NameDLOG	128
#define NameField	2

pascal	Boolean	UserNameDialogHook(WindowPtr dp,EventRecord *ep,int *ip)
{
	char	tempChar;
	switch(ep->what) {
		case keyDown:
			tempChar = ep->message & charCodeMask;
			if (tempChar==10 || tempChar==13 || tempChar==3) {
				*ip = OK;
				return TRUE;
			}
			return FALSE;
		case updateEvt:
			break;
		default:
			break;
	}
	return FALSE;
}

void GetUserName(StringPtr name)
{
	GrafPtr		savePort;
	DialogPtr	dp;
	short		itemHit,t;
	Handle		h;
	Rect		r;
	
	GetPort(&savePort);
	if ((dp = GetNewDialog(NameDLOG,NULL,(WindowPtr) -1)) == NULL)
		return;


	ShowWindow(dp);
	SetPort(dp);

	GetDItem(dp,NameField,&t,&h,&r);
	SetIText(h,name);
	SetDItem(dp,NameField,t,h,&r);
	SelIText(dp,NameField,0,32767);

	ShowCursor();

	do {
		ModalDialog((ProcPtr) UserNameDialogHook, &itemHit);
	} while (itemHit != OK);

	HideCursor();

	GetDItem(dp,NameField,&t,&h,&r);
	GetIText(h,name);

	DisposDialog(dp);

	SetPort(savePort);

	MyCopyBits();
}