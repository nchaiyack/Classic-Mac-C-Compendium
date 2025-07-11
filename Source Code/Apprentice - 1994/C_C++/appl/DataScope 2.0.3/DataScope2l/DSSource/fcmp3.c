# include <stdio.h>
# include <Math.h>
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	DoKernelOperation		Do the convolution kernel operations
	
				p = pointer to input array
				t = pointer to output array
				x = number of x-points in input array
				n = number of points in input array
				k = pointer to kernel array (DataScope format)
				theK = kernel workspace
				m = number of points to a side of the kernel (eg, a 3x3
					kernel has m = 3)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void DoKernelOperation(p,t,x,n,k,theK,m)
	float			*p,*k,*t;
	int				x,n,m;
	extended		*theK;
{
	register float		*tt,*pp,*uu,*qq,*kk;
	register extended	*f;
	int					s;
	void				GetShortage();
	float				GetGenericKernel();
	
	kk = k + (int)(m * (m + 1) - 1);
	f = theK + (int)(m * m - 1);
	
	while (f > theK)	*f-- = (extended)(*kk--) / *k;
	*f = (extended)(*kk) / *k;
	
	s = (int)(m / 2);
	tt = t + (int)(s + s * x);
	pp = p;
	uu = p + (x - m);
	qq = p + (int)(n - m - (m - 1) * x);
	
	while (uu < qq)
		{while (pp < uu)	*tt++ = GetGenericKernel(pp++,x,theK,m-1);
		 *tt = GetGenericKernel(pp,x,theK,m-1);
		 tt += m;
		 uu += x;
		 pp += m;
		}
	while (pp <= uu)		*tt++ = GetGenericKernel(pp++,x,theK,m-1);
	*t = GetGenericKernel(pp,x,theK,m-1);
	
	GetShortage(t,(long)x,(long)(n/x),(long)s,(long)s,(long)s,(long)s);
	
	return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	GetGenericKernel	Get the value of the generic kernel
						operation
						
					p = pointer to 1st element of data for
						kernel
					x = number of x-points in original data
						array
					k = pointer to 1st kernel element
					s = side of the kernel minus 1
				
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
float GetGenericKernel(p,x,k,s)
	register float		*p;
	register extended	*k;
	int					x,s;
{
	register extended	t;
	register float		*u,*q;
	
	t = 0.;
	u = p + (long)(s);
	q = u + (long)(s * x);
	
	while (u < q)
		  {while (p < u)	t += *k++ * (extended)(*p++);
		   t += *k++ * (extended)(*p);
		   u += x;
		   p = u - s;
		  }
	while (p < u)	t += *k++ * (extended)(*p++);
	t += *k * (extended)(*p);
	
	return ((float)(t));
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	GetShortage	fill in the border of a float array
				
				t = pointer to the array
				x = number of x-points in the array
				y = number of y-points in the array
				dxl = left   x-offset
				dxr = right  x-offset
				dyt = top    y-offset
				dyb = bottom y-offset
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	GetShortage(t,x,y,dxl,dxr,dyt,dyb)
	register float		*t;
	long				x,y,dxl,dxr,dyt,dyb;
{
	register float		*p,*u,*w,*q;
	void				CopyColumn(),	CopyRow();

	p = t + (long)(dxl + x * dyt);
	u = p + (long)(x - dxl - dxr - 1);
	q = u + (long)(x * (y - dyt - dyb - 1));
	w = q - (long)(u - p);
	
	if (dxl > 0)
	   { CopyColumn(p,
			        w,
			        (float *)(p - dxl),
			        x,
				    dxl);
		p -= dxl;
		w -= dxl;
	   }
		
	if (dxr > 0)
	   { CopyColumn(u,
			        q,
			        (float *)(u + 1),
			        x,
				    dxr);
		u += dxr;
		q += dxr;
	   }
	
	if (dyt > 0)
	    CopyRow(p,
			    u,
			    t,
			    x,
				dyt);
	
	if (dyb > 0)
	    CopyRow(w,
			    q,
			    (float *)(w + x),
			    x,
				dyb);
	
	 return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	CopyColumn		copy a col of an array into a series
						of cols of an array (dup the col) 
						for float data
				
				p = pointer to first element of input col
				q = pointer to last element of input col
				t = pointer to first element of output cols
				c = number of columns to copy
				x = number of x-points in the original array
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	CopyColumn(p,q,t,x,c)
		register float			*p,*t,*q;
		register long			x;
		long					c;
{
		register long			i;
		long					j;
		
		j = x - c + 1;
		
		while (p < q)
			  {for (i = 1; i < c; i++)	*t++ = *p;
			   *t = *p;
			   p += x;
			   t += j;
			  }
		for (i = 1; i < c; i++)	*t++ = *p;
		*t = *p;
	
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	CopyRow		copy a row of an array into a series
					of rows of an array (dup the row) for
					float data
				
				p = pointer to first element of input row
				q = pointer to last element of input row
				t = pointer to first element of output rows
				x = number of x-points in the original array
				r = number of rows to copy
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void	CopyRow(p,q,t,x,r)
		register float			*p,*t,*q;
		register long			r;
		long					x;
{
		register long			c;
		long					d;
		
		c = (long)(q - p);
		d = x - c;
		
		while (r > 1)
			  {while (p < q)	*t++ = *p++;
			   *t = *p;
			   t += d;
			   p -= c;
			   r--;
			  }
		while (p < q)	*t++ = *p++;
		*t = *p;

		return;
}
