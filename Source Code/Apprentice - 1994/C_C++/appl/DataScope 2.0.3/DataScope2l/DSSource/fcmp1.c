# include <stdio.h>
# include <Math.h>

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_coef2	get coefficients used in computing varying-stepped
				second order derivatives
				
				p2 = pointer to first element of source vector
				t  = pointer to first element of target vector
				n  = number of elements in the source vector
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_coef2(p2,t,n)	register float		*p2;
							register extended	*t;
							int					n;
{
		register extended	z;
		register float		*p1,*p3,*q;
		register extended	*u,two;
		   
		p1 = p2;
		p3 = p2 + 1;
		q  = p2 + n - 1;
		u  = t + n;
		two = 2.;
		 	
		z    = 1. / ((extended)(*p3++) - (extended)(*p2++));
		*t++ = z * z;
		*u++ = - 1.;

		while (p3 < q)
			  {z	= two / ((extended)(*p3) - (extended)(*p1));
			   *t++ = z / ((extended)(*p3) - (extended)(*p2));
			   *u++ = - ((extended)(*p3++) - (extended)(*p2)) /
			   		    ((extended)(*p2) - (extended)(*p1++));
			   p2++;
			  }
			  
		z	= two / ((extended)(*p3) - (extended)(*p1));
		*t++ = z / ((extended)(*p3) - (extended)(*p2));
		*u++ = - ((extended)(*p3) - (extended)(*p2)) /
			   	 ((extended)(*p2) - (extended)(*p1));
		
		z  = 1. / ((extended)(*p3) - (extended)(*p2));
		*t = z * z;
		*u = - 1.;
		
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_coef1	get coefficients used in computing varying-stepped
				first order derivatives
				
				p2 = pointer to first element of source vector
				t  = pointer to first element of target vector
				n  = number of elements in the source vector
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_coef1(p2,t,n)	register float		*p2;
							register extended	*t;
							int					n;
{
		register extended	z;
		register float		*p1,*p3,*q;
		register extended	*u;
		
		p1 = p2;
		p3 = p2 + 1;
		q  = p2 + n - 1;
		u  = t + n;
		 	
		*t++ = .5 / ((extended)(*p3) - (extended)(*p2));
		*u++ = 1.;
		if (n == 2)
		   {*t = *(t - 1);
		    *u = *(u - 1);
			return;
		   }
		p2++;
		p3++;
		while (p3 < q)
			  {z = ((extended)(*p2) - (extended)(*p1)) / 
			  	   ((extended)(*p3) - (extended)(*p2));
			   *t++ = z / ((extended)(*p3++) - (extended)(*p1++));
			   *u++ = 1. / (z * z);
			   p2++;
			  }
			  
		z = ((extended)(*p2) - (extended)(*p1)) / 
			 ((extended)(*p3) - (extended)(*p2));
		*t++ = z / ((extended)(*p3) - (extended)(*p1));
		*u++ = 1. / (z * z);
		
		*t = .5 / ((extended)(*p3) - (extended)(*p2));
		*u = 1.;
		
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_shr		shift array one to the right
	
				p = pointer to first element of source array
				q = pointer to last  element of source array
				t = pointer to first element of target array
				x = number of elements in each row
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_shr(p,q,t,x)	register float	*p,*t,*q;
						register int	x;
{
		register float	*u;
		
		if (x == 1)
		   {while (p < q)	*t++ = *p++;
		    *t = *p;
		   }
		else
			{u = q - (x - 1);
			 t += q - p;
			 q--;
			 while (u > p)
			 	   {while (q > u) *t-- = *q--;
				    *t-- = *q;
					*t-- = *q--;
					q--;
					u -= x;
				   }
			 while (q > p) *t-- = *q--;
			 *t-- = *q;
			 *t = *q;
			}
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_shd		shift array down one
	
				p = pointer to first element of source array
				q = pointer to last  element of source array
				t = pointer to first element of target array
				x = number of elements in each row
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_shd(p,q,t,x)	register float	*p,*t,*q;
						register int	x;
{
		
		if (x == q - p + 1)
		   {while (p < q)	*t++ = *p++;
		    *t = *p;
		   }
		else
			{t += q - p;
			 q -= x;
			 while (p < q)	*t-- = *q--;
			 *t-- = *q--;
			 q += x;
			 while (p < q)	*t-- = *q--;
			 *t = *q;
			}
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_shl		shift array one to the left
	
				p = pointer to first element of source array
				q = pointer to last  element of source array
				t = pointer to first element of target array
				x = number of elements in each row
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_shl(p,q,t,x)	register float	*p,*t,*q;
						register int	x;
{
		register float	*u;
		
		if (x == 1)
		   {while (p < q)	*t++ = *p++;
		    *t = *p;
		   }
		else
			{u = p + x - 1;
			 p++;
			 while (u < q)
			 	   {while (p < u) *t++ = *p++;
				    *t++ = *p;
					*t++ = *p++;
					p++;
					u += x;
				   }
			 while (p < q) *t++ = *p++;
			 *t++ = *p;
			 *t = *p;
			}
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_shu		shift array up one
	
				p = pointer to first element of source array
				q = pointer to last  element of source array
				t = pointer to first element of target array
				x = number of elements in each row
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_shu(p,q,t,x)	register float	*p,*t,*q;
						register int	x;
{
		
		if (x == q - p + 1)
		   {while (p < q)	*t++ = *p++;
		    *t = *p;
		   }
		else
			{p += x;
			 while (p < q)	*t++ = *p++;
			 *t++ = *p++;
			 p -= x;
			 while (p < q)	*t++ = *p++;
			 *t = *p;
			}
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_max		return max of an array
	
				p = pointer to first element of array
				n = number of elements in the array
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
float	fn_max(p,n)		register float	*p;
						register int	n;
{
		register float	t,*q;
		
		q = p + n - 1;
		t = *q;
		while (p < q)
			  {if (*p > t)	t = *p++;
			   else		  	p++;
			  }
		return(t);
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_min		return min of an array
	
				p = pointer to array
				n = number of array elements
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
float	fn_min(p,n)		register float	*p;
						register int	n;
{
		register float		t,*q;
		
		q = p + n - 1;
		t = *q;
		while (p < q)
			  {if (*p < t)	t = *p++;
			   else		  	p++;
			  }
		return(t);
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_mean		return mean of an array
	
				p = pointer to array
				n = number of array elements
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
float	fn_mean(p,n)	register float	*p;
						register int	n;
{
		register float	*q;
		register extended	t;
		
		q = p + n - 1;
		t = 0.;
		while (p < q) t += (extended)(*p++);
		t += (extended)(*p);
		return ((float)(t / (extended)(n)));
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_sdev		return standard deviation of an array
	
				p = pointer to array
				n = number of array elements
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
float	fn_sdev(p,n)	register float	*p;
						register int	n;
{
		register float		*q;
		register extended	t,u;
		
		q = p + n - 1;
		t = 0.;
		u = t;
		
		while (p < q)
			  {t += (extended)(*p) * (extended)(*p);
			   u += (extended)(*p++);
			  }
		t += (extended)(*p) * (extended)(*p);
		u += (extended)(*p++);
		
		t -= u * u / (extended)(n);
		t /= (extended)(n - 1);
		return ((float)(sqrt(t)));
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	float fn_stpsd	calc standard deviation of the increments of a vector
	
					p = pointer to first element of vector
					n = number of elements in the vector
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
float	fn_stpsd(p,n)	register float	*p;
						register int	n;
{
	register extended	s,u;
	register float		*q,*t;
	
	q = p + n - 1;
	t = p + 1;
	
	u = (extended)(*q) - (extended)(*p);
	s = - u * u / (extended)(n - 1);
	
	while (t < q)
		  {u = (extended)(*t++) - (extended)(*p++);
		   s += u * u;
		  }
	u = (extended)(*t) - (extended)(*p);
	s += u * u;
	
	s /= (extended)(n - 2);
	return((float)(sqrt(s)));
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void fn_dx1c	1st derivative with respect to x, constant stepsize
	
					p = pointer to first element of input array
					t = pointer to first element of target array
					x = number of elements in each row of the input array
					n = number of elements in the input array
					d = stepsize factor = .5 / delta-x
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_dx1c(p,t,x,n,d)
		float				*p,*t;
		int					x,n;
		register extended	d;
{
		register float		*t2,*p1,*p3,*u,*q;

		p1 = p;
		p3 = p + 2;
		u  = p + x - 1;
		q  = p + n - 1;
		t2 = t + 1;
/*
		****************************************************
		calculate values for interior points
		****************************************************
*/
		while (u < q)
			  {while (p3 < u)
			  		 *t2++ = (float)(d * ((extended)(*p3++) -
					 					  (extended)(*p1++)));
			   *t2 = (float)(d * ((extended)(*p3) - (extended)(*p1)));
			   u  += x;
			   t2 += 3;
			   p1 += 3;
			   p3 += 3;
			  }
		while (p3 < q)
			  *t2++ = (float)(d * ((extended)(*p3++) - (extended)(*p1++)));
		*t2 = (float)(d * ((extended)(*p3) - (extended)(*p1)));
/*
		****************************************************
		copy 1st column from second column
		****************************************************
*/
		t2 = t;
		p1 = t + 1;
		p3 = t + n - x;
		while (t2 < p3)
			  {*t2 = *p1;
			   t2 += x;
			   p1 += x;
			  }
		*t2 = *p1;
/*
		****************************************************
		copy last column from next-to-last column
		****************************************************
*/
		t2 = t + x - 1;
		p1 = t2 - 1;
		p3 = t + n - 1;
		while (t2 < p3)
			  {*t2 = *p1;
			   t2 += x;
			   p1 += x;
			  }
		*t2 = *p1;

		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void fn_dx2c	2nd derivative with respect to x, constant stepsize
	
					p = pointer to first element of input array
					t = pointer to first element of target array
					x = number of elements in each row of the input array
					n = number of elements in the input array
					d = stepsize factor = 1 / (delta-x * delta-x)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_dx2c(p,t,x,n,d)
		float				*p,*t;
		int					x,n;
		register extended	d;
{
		register float		*t2,*p2,*p1,*p3,*u,*q;

		p1 = p;
		p2 = p + 1;
		p3 = p + 2;
		u  = p + x - 1;
		q  = p + n - 1;
		t2 = t + 1;
/*
		****************************************************
		calculate values for interior points
		****************************************************
*/
		while (u < q)
			  {while (p3 < u)
			  		 {*t2++ = (float)(d * ((extended)(*p3++) +
					 					   (extended)(*p1++) -
										   (extended)(*p2)   -
										   (extended)(*p2)   ));
					  p2++;
					 }
			   *t2 = (float)(d * ((extended)(*p3) + (extended)(*p1) -
			   					  (extended)(*p2) - (extended)(*p2)));
			   u  += x;
			   t2 += 3;
			   p1 += 3;
			   p2 += 3;
			   p3 += 3;
			  }
		while (p3 < q)
			  {*t2++ = (float)(d * ((extended)(*p3++) +
					 				(extended)(*p1++) -
									(extended)(*p2)   -
									(extended)(*p2)   ));
			   p2++;
			  }
		*t2 = (float)(d * ((extended)(*p3) + (extended)(*p1) -
			   			   (extended)(*p2) - (extended)(*p2)));
/*
		****************************************************
		copy 1st column from second column
		****************************************************
*/
		t2 = t;
		p1 = t + 1;
		p3 = t + n - x;
		while (t2 < p3)
			  {*t2 = *p1;
			   t2 += x;
			   p1 += x;
			  }
		*t2 = *p1;
/*
		****************************************************
		copy last column from next-to-last column
		****************************************************
*/
		t2 = t + x - 1;
		p1 = t2 - 1;
		p3 = t + n - 1;
		while (t2 < p3)
			  {*t2 = *p1;
			   t2 += x;
			   p1 += x;
			  }
		*t2 = *p1;

		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void fn_dy1c	1st derivative with respect to y, constant stepsize
	
					p = pointer to first element of input array
					t = pointer to first element of target array
					x = number of elements in each row of the input array
					n = number of elements in the input array
					d = stepsize factor = .5 / delta-y
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_dy1c(p,t,x,n,d)
		float				*p,*t;
		int					x,n;
		register extended	d;
{
		register float		*t2,*p1,*p3,*q;

		p1 = p;
		p3 = p + x + x;
		q  = p + n - 1;
		t2 = t + x;
/*
		****************************************************
		calculate values for interior points
		****************************************************
*/
		while (p3 < q)
			  *t2++ = (float)(d * ((extended)(*p3++) - (extended)(*p1++)));
		*t2 = (float)(d * ((extended)(*p3) - (extended)(*p1)));
/*
		****************************************************
		copy top row from second row
		****************************************************
*/
		t2 = t;
		p1 = t + x;
		p3 = t + x - 1;
		while (t2 < p3)	*t2++ = *p1++;
		*t2 = *p1;
/*
		****************************************************
		copy bottom row from next-to-bottom row
		****************************************************
*/
		p3 = t + n - 1;
		t2 = p3 - x + 1;
		p1 = t2 - x;
		while (t2 < p3)	*t2++ = *p1++;
		*t2 = *p1;

		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void fn_dy2c	2nd derivative with respect to y, constant stepsize
	
					p = pointer to first element of input array
					t = pointer to first element of target array
					x = number of elements in each row of the input array
					n = number of elements in the input array
					d = stepsize factor = 1 / (delta-y * delta-y)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_dy2c(p,t,x,n,d)
		float				*p,*t;
		int					x,n;
		register extended	d;
{
		register float		*t2,*p1,*p2,*p3,*q;

		p1 = p;
		p2 = p1 + x;
		p3 = p2 + x;
		q  = p + n - 1;
		t2 = t + x;
/*
		****************************************************
		calculate values for interior points
		****************************************************
*/
		while (p3 < q)
			  {*t2++ = (float)(d * ((extended)(*p3++) + (extended)(*p1++)
			  					  - (extended)(*p2) - (extended)(*p2)));
			   p2++;
			  }
		*t2 = (float)(d * ((extended)(*p3) + (extended)(*p1)
			  			 - (extended)(*p2) - (extended)(*p2)));
/*
		****************************************************
		copy top row from second row
		****************************************************
*/
		t2 = t;
		p1 = t + x;
		p3 = t + x - 1;
		while (t2 < p3)	*t2++ = *p1++;
		*t2 = *p1;
/*
		****************************************************
		copy bottom row from next-to-bottom row
		****************************************************
*/
		p3 = t + n - 1;
		t2 = p3 - x + 1;
		p1 = t2 - x;
		while (t2 < p3)	*t2++ = *p1++;
		*t2 = *p1;

		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void fn_dyv		1st derivative with respect to y, varying stepsize
	
					p = pointer to first element of input array
					t1 = pointer to first element of target array
					x = number of x-points
					y = number of y-points
					m1 = coefficient vector
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_dyv(p,t1,x,y,m1)
		float				*p,*t1;
		int					x,y;
		extended			*m1;
{
		register float		*t,*p1,*p2,*p3,*q,*u;
		register extended	*m2;
		
		t = t1;
		p1 = p;
		p2 = p1;
		p3 = p2 + x;
		u  = p1 + x + x - 1;
		q  = p1 + (int)(x * y - 1);
		m2 = m1 + y;
		
		t  += (x - 1);
		p3 += (x - 1);
		p2 += x;
		
		u += x;
		while (u < q)
			  {p3++;
			   t++;
			   m1++;
			   m2++;
			   while (p3 < u)
			  		 {*t++ = (float)(*m1 * ((extended)(*p3++) -
					 		 (extended)(*p2) + *m2 *
							 ((extended)(*p2) - (extended)(*p1++))));
					  p2++;
					 }
			   *t = (float)(*m1 * ((extended)(*p3) - (extended)(*p2) +
			   				*m2 * ((extended)(*p2) - (extended)(*p1++))));
			   p2++;
			   u += x;
			  }
		p3++;
		t++;
		m1++;
		m2++;
		while (p3 < q)
			  {*t++ = (float)(*m1 * ((extended)(*p3++) -
			  				 (extended)(*p2) + *m2 *
							 ((extended)(*p2) - (extended)(*p1++))));
			   p2++;
			  }
		*t = (float)(*m1 * ((extended)(*p3) - (extended)(*p2) +
					 *m2 * ((extended)(*p2) - (extended)(*p1++))));
/*
		****************************************************
		copy top row from second row
		****************************************************
*/
		t = t1;
		p = t1 + x;
		u = t1 + x - 1;
		while (t < u)	*t++ = *p++;
		*t = *p;
/*
		****************************************************
		copy bottom row from next-to-bottom row
		****************************************************
*/
		u = t1 + (int)(x * y) - 1;
		t = u - x + 1;
		p = t - x;
		while (t < u)	*t++ = *p++;
		*t = *p;

		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	void fn_dxv		1st derivative with respect to x, varying stepsize
	
					p = pointer to first element of input array
					t1 = pointer to first element of target array
					x = number of x-points
					y = number of y-points
					m1 = coefficient vector
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	fn_dxv(p,t1,x,y,m1)
		register float		*p,*t1;
		int					x,y;
		register extended	*m1;
{
		register float		*t,*q,*u,*v,*w;
		register extended	*m2;
		
		t = t1;
		q = p + (int)(x * y - 1);
		m2 = m1 + x;

		v = p;
		w = p;
		p++;
		   
		u = v + x - 1;
		while (u < q)
			  {t++;
			   m1++;
			   p++;
			   w++;
			   m2++;
			   while (p < u)
					 {*t++ = (float)(*m1++ * ((extended)(*p++) -
					 		 (extended)(*w) + *m2++ * ((extended)(*w) -
							 (extended)(*v++))));
					  w++;
					 }
			   *t++ = (float)(*m1++ * ((extended)(*p) - (extended)(*w) +
			   		  *m2++ * ((extended)(*w) - (extended)(*v++))));
			   if (y == 1) return;
			   p++;	p++;
			   v++;	v++;
			   t++;
			   w++; w++;
			   u += x;
			   m1 -= (x - 1);
			   m2 -= (x - 1);
			  }

		t++;
		m1++;
		p++;
		w++;
		m2++;
		while (p < u)
			  {*t++ = (float)(*m1++ * ((extended)(*p++) -
					 		 (extended)(*w) + *m2++ * ((extended)(*w) -
							 (extended)(*v++))));
			   w++;
			  }
		*t++ = (float)(*m1++ * ((extended)(*p) - (extended)(*w) +
			   		  *m2++ * ((extended)(*w) - (extended)(*v++))));
/*
		****************************************************
		copy 1st column from second column
		****************************************************
*/
		u = t1;
		v = t1 + 1;
		w = t1 + (int)(x * y) - x;
		while (u < w)
			  {*u = *v;
			   u += x;
			   v += x;
			  }
		*u = *v;
/*
		****************************************************
		copy last column from next-to-last column
		****************************************************
*/
		u = t1 + x - 1;
		v = u - 1;
		w = t1 + (int)(x * y) - 1;
		while (u < w)
			  {*u = *v;
			   u += x;
			   v += x;
			  }
		*u = *v;

		return;
}
/*
		****************************************************
		Transpose the x and y-ness of the array.
		****************************************************
*/
void
fn_transpose(src,dst,x,y)
	float *src,*dst;
	int x,y;
{
	register float *f;
	register int i,j;
	
	
	for (i=0; i<y; i++) {
		f = dst + i;					/* start of new column, going down */

		for (j=0; j<x; j++) {			/* go in order of source array */
			*f = *src++;
			f += y;						/* increment to next row in dest */
		}
	}

}
