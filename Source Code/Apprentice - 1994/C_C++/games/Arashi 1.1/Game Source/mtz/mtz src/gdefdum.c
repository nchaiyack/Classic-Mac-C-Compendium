
pascal OSErr main(selector, response)
OSType	selector;
long	*response;
{
	*response = 0x0;				/* return a value that will re-enable superclock */
	return 0;					
}