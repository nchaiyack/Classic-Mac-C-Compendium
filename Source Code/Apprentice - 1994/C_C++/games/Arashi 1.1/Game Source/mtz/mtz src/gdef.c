
pascal OSErr main(selector, response)
OSType	selector;
long	*response;
{
	*response = 0x12;				/* return a value that will disable superclock */
	return 0;					
}