/***********************************************************************/
/*********************************************************************
* This software is Copyright (C) 1988 by Steven Dorner and the
* University of Illinois Board of Trustees, and by CSNET.  No warranties of
* any kind are expressed or implied.  No support will be provided.
* This software may not be redistributed without prior consent of CSNET.
* You may direct questions to dorner@garcon.cso.uiuc.edu.
**********************************************************************/

#define ROTORSZ 256
#define MASK 0377

#include <string.h>
#include <crypt.h>

char  t1[ROTORSZ];
char  t2[ROTORSZ];
char  t3[ROTORSZ];

int   Encrypting = 0;
static int ccnt, nchars, n1, n2;
char   *Visible();
int crypt_init(char *pw);
int encode(char *out, char *buf, int n);
int threecpy(unsigned char *to, unsigned char *from);


crypt_start(pass)
char   *pass;
{

  n1 = 0;
  n2 = 0;
  ccnt = 0;
  nchars = 0;
  Encrypting = 1;
  crypt_init(pass);
}

crypt_init(pw)
char   *pw;
{
  int   ic, i, k, temp;
  unsigned random;
  char  buf[13];
  int  seed;
  char   *crypt();

  /* must reinitialize the arrays */
  for (i = 0; i < ROTORSZ; i++)
    t1[i] = t2[i] = t3[i] = 0;

  strncpy(buf, crypt(pw, pw), 13);

  seed = 123;
  for (i = 0; i < 13; i++)
    seed = seed * buf[i] + i;
  for (i = 0; i < ROTORSZ; i++)
    t1[i] = i;
  for (i = 0; i < ROTORSZ; i++)
  {
    seed = 5 * seed + buf[i % 13];
    random = seed % 65521;
    k = ROTORSZ - 1 - i;
    ic = (random & MASK) % (k + 1);
    random >>= 8;
    temp = t1[k];
    t1[k] = t1[ic];
    t1[ic] = temp;
    if (t3[k] != 0)
      continue;
    ic = (random & MASK) % k;
    while (t3[ic] != 0)
      ic = (ic + 1) % k;
    t3[k] = ic;
    t3[ic] = k;
  }
  for (i = 0; i < ROTORSZ; i++)
    t2[t1[i] & MASK] = i;
}

/***********************************************************************
* encrypts a string
* first byte of string is encoded length of string
* returns length of encoded string
***********************************************************************/
encryptit(to, from)
char   *to;
char   *from;
{
  char  scratch[4096];
  char   *sp;

  sp = scratch;
  for (; *from; from++)
  {
    *sp++ = t2[(t3[(t1[(*from + n1) & MASK] + n2) & MASK] - n2) & MASK] - n1;
    n1++;
    if (n1 == ROTORSZ)
    {
      n1 = 0;
      n2++;
      if (n2 == ROTORSZ)
	n2 = 0;
    }
  }
  return (encode(to, scratch, sp - scratch));
}

/*
 * * Basic 1 character encoding function to make a char printing. 
 */
#define ENC(c) (((unsigned char)(c) & 077) + '#')

encode(out, buf, n)       /* output a line of binary (up to 45 chars)
		 * in */
int   n;            /* a readable format, converting 3 chars to 4  */
char   *buf;
char   *out;
{
  int   i;
  char   *outStart;

  outStart = out;
  *out++ = ENC(n);

  for (i = 0; i < n; buf += 3, i += 3, out += 4)
    threecpy((unsigned char *)out, (unsigned char *)buf);

  /* null terminate */
  *out = '\0';
  return (out - outStart);
}

threecpy(to, from)
unsigned char *to;
unsigned char *from;
{
  int   c1, c2, c3, c4;

  c1 = *from >> 2;
  c2 = (*from << 4) & 060 | (from[1] >> 4) & 017;
  c3 = (from[1] << 2) & 074 | (from[2] >> 6) & 03;
  c4 = from[2] & 077;
  *to++ = ENC(c1);
  *to++ = ENC(c2);
  *to++ = ENC(c3);
  *to = ENC(c4);
}
