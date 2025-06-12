/* some glue code so static & global constructors will be called, & to glue to make
	some c++ routines avail from c */

#include	<String.h>
#include	<Machine Dependencies.h>
#include	<KerbConf.h>

extern "C" {
void	c_main();
}

void	main()
{
	OpenNet();
	c_main();
	}

extern "C" {

void	cplus_cleanup()
{
	CloseNet();
	}

void	forgetsession(char *name, char *inst, char *realm)
{
	gKerbConf.DeleteSession(name, inst, realm);
	}

}
 
