/* return a 1 saying that ARASHI is running */

pascal OSErr main(selector, response)
OSType	selector;
long	*response;
{
	*response = 0x1;				/* return a value that will re-enable superclock */
	return 0;					
}