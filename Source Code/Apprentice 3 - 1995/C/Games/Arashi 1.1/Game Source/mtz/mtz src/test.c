#include <stdio.h>
#include <Quickdraw.h>
#include <VA.h>

main()
{
 while(!Button())
 	printf("%d \n", VARandom() );
}