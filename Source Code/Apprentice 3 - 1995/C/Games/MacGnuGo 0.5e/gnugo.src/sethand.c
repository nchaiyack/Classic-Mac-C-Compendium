#include "comment.header"

#define BLACKSTONE 2

extern unsigned char p[19][19];
extern int MAXX, MAXY;

void sethand(int i)
     /* set up handicap pieces */
{
  int half, q;
  
  half = (MAXX + 1)/2 - 1;
  if (MAXX < 13)
    q = 2;
  else
    q = 3;
  if (MAXX < 7) q = 1;
  
  if (i > 0)
    {
      p[q][MAXY - q - 1] = BLACKSTONE;
      if (i > 1)
	{
	  p[MAXX - q - 1][q] = BLACKSTONE;
	  if (i > 2)
	    {
	      p[q][q] = BLACKSTONE;
	      if (i > 3)
		{
		  p[MAXX - q - 1][MAXY - q - 1] = BLACKSTONE;
		  if (i == 5)
		    p[half][half] = BLACKSTONE;
		  else
		    if (i > 5)
		      {
			p[q][half] = BLACKSTONE;
			p[MAXX - q - 1][half] = BLACKSTONE;
			if (i == 7)
			  p[half][half] = BLACKSTONE;
			else
			  if (i > 7)
			    {
			      p[half][MAXY - q - 1] = BLACKSTONE;
			      p[half][q] = BLACKSTONE;
			      if (i > 8)
				p[half][half] = BLACKSTONE;
			      if (i > 9)
				{p[q - 1][q - 1] = BLACKSTONE;
				 if (i > 10)
				   {p[MAXX - q][MAXY - q] = BLACKSTONE;
				    if (i > 11)
				      {p[q - 1][MAXY - q] = BLACKSTONE;
				       if (i > 12)
					 {p[MAXX - q][q - 1] = BLACKSTONE;
					  if (i > 13)
					    {p[(q + half)/2][(q + half)/2] = BLACKSTONE;
					     if (i > 14)
					       {p[(MAXX - q + half)/2][(MAXY - q + half)/2] = BLACKSTONE;
						if (i > 15)
						  {p[(q + half)/2][(MAXY - q + half)/2] = BLACKSTONE;
						   if (i > 16)
						     p[(MAXX - q + half)/2][(q + half)/2] = BLACKSTONE;
						 }
					      }
					   }
					}
				     }
				  }
			       }
			    }
		      }
		}
	    }
	}
    }
}  /* end sethand */
