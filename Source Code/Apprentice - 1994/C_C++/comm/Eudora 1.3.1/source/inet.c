#define FILE_NUM 18
/* Copyright (c) 1990-1992 by the University of Illinois Board of Trustees */
#pragma load EUDORA_LOAD
#pragma segment Util
/**********************************************************************
 * DotToNum - turn an address in dotted decimal into an internet address
 * returns True if the conversion was successful.  This routine is
 * somewhat limited, in that it will accept only four octets, and does
 * not permit the abbreviated forms for class A and B networks.
 **********************************************************************/
Boolean DotToNum(UPtr string,long *nPtr)
{
	long address=0;
	Byte b=0;
	UPtr cp;
	int dotcount=0;
	
	/*
	 * allow leading spaces
	 */
	for (cp=string+1;cp<=string+*string;cp++) if (*cp!=' ') break;
	
	/*
	 * the address
	 */
	for (;cp<=string+*string;cp++)
	{
		if (*cp=='.')
		{
			if (++dotcount > 3) return (False); /* only 4 octets allowed */
			address <<= 8;
			address |= b;
			b=0;
		}
		else if (isdigit(*cp))
		{
			b *= 10;
			b |= (*cp - '0');
			if (b>255) return (False);					/* keep it under 256 */
		}
		else if (*cp==' ')										/* allow trailing spaces */
			break;
		else
			return (False); 										/* periods or digits ONLY */
	}
	
	/*
	 * final checks, assignment
	 */
	if (dotcount!=3) return (False);
	address <<= 8;
	address |= b;
	*nPtr = address;
	return(True);
}

/**********************************************************************
 * NumToDot - turn an address into a dotted decimal string
 **********************************************************************/
UPtr NumToDot(unsigned long num,UPtr string)
{
	unsigned char* bp=(unsigned char *)&num;
	UPtr cp=string;
	int count=4;
	int length;
	
	for (count=4;count;count--,bp++)
	{
		NumToString((unsigned long)*bp,cp);
		length = *cp;
		*cp = '.';
		cp += length+1;
	}
	*string = cp-string-1;
	return(string);
} 

