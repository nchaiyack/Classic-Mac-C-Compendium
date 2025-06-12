/*/
     Project Arashi: Highscores.h
     Major release: Version 1.1, 7/22/92

     Last modification: Friday, December 11, 1992, 16:58
     Created: Friday, December 14, 1990, 16:35

     Copyright � 1990-1992, Juri Munkki
/*/

/* includes for highscore listing */

typedef struct
{
		long	score;
		char	name[6];
}	Initials;

void WriteCongrats();
void WriteCongrats2();
void WriteNoCongrats();
void WriteScoreNotSaved();
void GameOver();
int Ranking();
int ScrollScores();
void GetInitials();
void GetSetMouse();
void SaveScores(Handle);