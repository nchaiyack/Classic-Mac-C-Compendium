/*/
     Project Arashi: RamFiles.h
     Major release: Version 1.1, 7/22/92

     Last modification: Monday, April 5, 1993, 0:11
     Created: Friday, October 23, 1992, 15:57

     Copyright © 1992-1993, Juri Munkki
/*/

short	IncreaseRamFile(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase);

short	IncreaseByClump(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase,
	long	clumpSize);

short	OddIncreaseRamFile(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase);

short	OddIncreaseByClump(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize,
	long	increase,
	long	clumpSize);

void	PackRamFile(
	Handle	theFile,
	long	*realSize,
	long	*logicalSize);
