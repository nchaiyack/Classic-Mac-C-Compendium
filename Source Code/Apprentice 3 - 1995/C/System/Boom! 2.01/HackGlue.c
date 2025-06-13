#include <SANE.h>
#undef NAN
#include <fp.h>


double_t sin ( double_t x )
{
	extended80 temp = x;
	Sin(&temp);
	
	return temp;
}

double_t cos ( double_t x )
{
	extended80 temp = x;
	Cos(&temp);
	
	return temp;
}
