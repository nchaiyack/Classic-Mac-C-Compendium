/*/
     Project Arashi: NewTitleEffect.h
     Major release: Version 1.1, 7/22/92

     Last modification: Wednesday, December 9, 1992, 16:31
     Created: Tuesday, March 10, 1992, 11:08

     Copyright � 1992, Juri Munkki
/*/

#define	ARASHIPICT 		131
#define	GAMEOVERPICT	133
#define	HORIZONTAL		1
#define	VERTICAL		2

pascal void MyStdPoly(int grafVerb, PolyHandle thePoly);
void ConvertToPolys(int resId);
void titlemain(int redId, int fadeDirection);
