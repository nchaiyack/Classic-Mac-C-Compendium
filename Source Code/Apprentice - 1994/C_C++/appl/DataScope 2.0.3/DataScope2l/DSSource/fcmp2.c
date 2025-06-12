# include <stdio.h>
# include <Math.h>

/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_lap5c		5-point laplacian, constant stepsize
	
					p  = pointer to source array
					t  = pointer to target array
					x  = number of x-points
					n  = number of points in source array
					dx = 1 / (delta-x * delta-x)
					dy = 1 / (delta-y * delta-y)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_lap5c(p,t,x,n,dx,dy)
		float		*p,*t;
		int			x,n;
		extended	dx,dy;
{
		register float		*t22,*x22,*x12,*x21,*x23,*x32,*q,*u;
		extended			dxy;

		t22 = t + x + 1;
		x12 = p + 1;
		x21 = p + x;
		x22 = x21 + 1;
		x23 = x22 + 1;
		x32 = x22 + x;
		u   = p + x - 1 + x;
		q   = p + n - 1 - x;
		dxy = - 2. * (dx + dy);
		
/*
		****************************************************
		calculate values for interior points
		****************************************************
*/
		while (u < q)
			  {while (x23 < u)
			  		 *t22++ = (float)(dx * ((extended)(*x21++)  +
					 					    (extended)(*x23++)) +
									  dy * ((extended)(*x12++)  +
									  		(extended)(*x32++)) +
									  dxy * (extended)(*x22++));
			   *t22 = (float)(dx * ((extended)(*x21)  +
					 				(extended)(*x23)) +
							  dy * ((extended)(*x12)  +
									(extended)(*x32)) +
							  dxy * (extended)(*x22));
			   u   += x;
			   t22 += 3;
			   x12 += 3;
			   x21 += 3;
			   x22 += 3;
			   x23 += 3;
			   x32 += 3;
			  }
		while (x23 < q)
			  *t22++ = (float)(dx * ((extended)(*x21++)  +
					 				 (extended)(*x23++)) +
							   dy * ((extended)(*x12++)  +
									 (extended)(*x32++)) +
							   dxy * (extended)(*x22++));
		*t22 = (float)(dx * ((extended)(*x21)  +
					 		 (extended)(*x23)) +
					   dy * ((extended)(*x12)  +
							 (extended)(*x32)) +
					   dxy * (extended)(*x22));
/*
		****************************************************
		copy 1st column from second column (except for 1st
		and last rows)
		****************************************************
*/
		t22 = t + x;
		x22 = t22 + 1;
		q   = t + n - x - x;
		while (t22 < q)
			  {*t22 = *x22;
			   t22 += x;
			   x22 += x;
			  }
		*t22 = *x22;
/*
		****************************************************
		copy last column from next-to-last column (except for
		1st and last rows)
		****************************************************
*/
		t22 = t + x - 1 + x;
		x22 = t22 - 1;
		q   = t + n - 1 - x;
		while (t22 < q)
			  {*t22 = *x22;
			   t22 += x;
			   x22 += x;
			  }
		*t22 = *x22;
/*
		****************************************************
		copy top row from second row
		****************************************************
*/
		t22 = t;
		x22 = t + x;
		q   = t + x - 1;
		while (t22 < q)	*t22++ = *x22++;
		*t22 = *x22;
/*
		****************************************************
		copy bottom row from next-to-bottom row
		****************************************************
*/
		t22 = t + n - x;
		x22 = t22 - x;
		q   = t + n - 1;
		while (t22 < q)	*t22++ = *x22++;
		*t22 = *x22;
		
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_lap9c		9-point laplacian, constant stepsize
	
					p = pointer to first element of input array
					t = pointer to first element of target array
					x = number of elements in each row of the input array
					n = number of elements in the input array
					d = stepsize factor = 1 / (4 * delta * delta)
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_lap9c(p,t,x,n,d)
		float				*p,*t;
		int					x,n;
		register extended	d;
{
		register float		*t22,*x11,*x12,*x13,
								 *x21,*x22,*x23,
								 *x31,*x32,*x33,
							*u,*q;

		t22 = t + x + 1;
		x11 = p;
		x12 = x11 + 1;
		x13 = x11 + 2;
		x21 = x11 + x;
		x22 = x21 + 1;
		x23 = x21 + 2;
		x31 = x21 + x;
		x32 = x31 + 1;
		x33 = x31 + 2;
		
		u   = x21 + x - 1;
		q   = p   + n - 1 - x;
/*
		****************************************************
		calculate values for interior points
		****************************************************
*/
		while (u < q)
			  {while (x23 < u)
			  		 *t22++ = (float)((extended)(*x11++) +
					 				  (extended)(*x13++) +
									  (extended)(*x31++) +
									  (extended)(*x33++) +
							    2. * ((extended)(*x12++) +
							   		  (extended)(*x21++) +
									  (extended)(*x23++) +
									  (extended)(*x32++)) -
								12. * (extended)(*x22++));

			  *t22 = (float)((extended)(*x11) + (extended)(*x13) +
							 (extended)(*x31) + (extended)(*x33) +
					   2. * ((extended)(*x12) + (extended)(*x21) +
							 (extended)(*x23) + (extended)(*x32)) -
					   12. * (extended)(*x22));
			   u   += x;
			   t22 += 3;
			   x11 += 3;	x12 += 3;	x13 += 3;
			   x21 += 3;	x22 += 3;	x23 += 3;
			   x31 += 3;	x32 += 3;	x33 += 3;
			  }
		while (x23 < q)
			  *t22++ = (float)((extended)(*x11++) +
					 		   (extended)(*x13++) +
							   (extended)(*x31++) +
							   (extended)(*x33++) +
						 2. * ((extended)(*x12++) +
							   (extended)(*x21++) +
							   (extended)(*x23++) +
							   (extended)(*x32++)) -
						 12. * (extended)(*x22++));

		*t22 = (float)((extended)(*x11) + (extended)(*x13) +
					   (extended)(*x31) + (extended)(*x33) +
				 2. * ((extended)(*x12) + (extended)(*x21) +
					   (extended)(*x23) + (extended)(*x32)) -
				 12. * (extended)(*x22));
/*
		****************************************************
		copy 1st column from second column (except for 1st
		and last rows)
		****************************************************
*/
		t22 = t + x;
		x22 = t22 + 1;
		q   = t + n - x - x;
		while (t22 < q)
			  {*t22 = *x22;
			   t22 += x;
			   x22 += x;
			  }
		*t22 = *x22;
/*
		****************************************************
		copy last column from next-to-last column (except for
		1st and last rows)
		****************************************************
*/
		t22 = t + x - 1 + x;
		x22 = t22 - 1;
		q   = t + n - 1 - x;
		while (t22 < q)
			  {*t22 = *x22;
			   t22 += x;
			   x22 += x;
			  }
		*t22 = *x22;
/*
		****************************************************
		copy top row from second row
		****************************************************
*/
		t22 = t;
		x22 = t + x;
		q   = t + x - 1;
		while (t22 < q)	*t22++ = *x22++;
		*t22 = *x22;
/*
		****************************************************
		copy bottom row from next-to-bottom row
		****************************************************
*/
		t22 = t + n - x;
		x22 = t22 - x;
		q   = t + n - 1;
		while (t22 < q)	*t22++ = *x22++;
		*t22 = *x22;
		
		return;
}
/*
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
	fn_medn9	3 by 3 median filtering
			
				p  = pointer to first element in the input array
				t  = pointer to first element in the output array
				x  = number of x-points in the input array
				n  = number of points in the input array
	~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
*/
void fn_medn9(p,t,x,n)
	float				*p,*t;
	int					x,n;
{
	void				fn_sort();
	register float		*t22,*x31,*x32,*x33,
							 *x22,*x21,*x23,
							 *x11,*x12,*x13,
						*q,*u;
	float				*ppp,pp[10],*pb;

	t22 = t + x + 1;
	pb  = pp + 1;
	
	x11 = p;		x12 = x11 + 1;	x13 = x11 + 2;
	x21 = x11 + x;	x22 = x21 + 1;	x23 = x21 + 2;
	x31 = x21 + x;	x32 = x31 + 1;	x33 = x31 + 2;
	
	u   = x31 + x - 1;
	q   = p + n - 1;
	
	while (u < q)
		  {while (x33 <= u)
		  		 {ppp = pb;
				  *ppp++ = *x11++;	*ppp++ = *x12++;	*ppp++ = *x13++;
				  *ppp++ = *x21++;	*ppp++ = *x22++;	*ppp++ = *x23++;
				  *ppp++ = *x31++;	*ppp++ = *x32++;	*ppp   = *x33++;
				  	fn_sort(9,pp);
				  *t22++ = *(pp + 5);
				 }
				 
		   x11 += 2;	x12 += 2;	x13 += 2;				 
		   x21 += 2;	x22 += 2;	x23 += 2;				 
		   x31 += 2;	x32 += 2;	x33 += 2;
		   t22 += 2;
		   u += x;
		  
		  }
	while (x33 < q)
		  {ppp = pb;
		   *ppp++ = *x11++;	*ppp++ = *x12++;	*ppp++ = *x13++;
		   *ppp++ = *x21++;	*ppp++ = *x22++;	*ppp++ = *x23++;
		   *ppp++ = *x31++;	*ppp++ = *x32++;	*ppp   = *x33++;
		   	fn_sort(9,pp);
		   *t22++ = *(pp + 5);
		  }
	
	ppp = pb;
	*ppp++ = *x11;	*ppp++ = *x12;	*ppp++ = *x13;
	*ppp++ = *x21;	*ppp++ = *x22;	*ppp++ = *x23;
	*ppp++ = *x31;	*ppp++ = *x32;	*ppp   = *x33;
		fn_sort(9,pp);
	*t22 = *(pp + 5);

/*
		****************************************************
		copy 1st column from second column (except for 1st
		and last rows)
		****************************************************
*/
		t22 = t + x;
		x22 = t22 + 1;
		q   = t + n - x - x;
		while (t22 < q)
			  {*t22 = *x22;
			   t22 += x;
			   x22 += x;
			  }
		*t22 = *x22;
/*
		****************************************************
		copy last column from next-to-last column (except for
		1st and last rows)
		****************************************************
*/
		t22 = t + x - 1 + x;
		x22 = t22 - 1;
		q   = t + n - 1 - x;
		while (t22 < q)
			  {*t22 = *x22;
			   t22 += x;
			   x22 += x;
			  }
		*t22 = *x22;
/*
		****************************************************
		copy top row from second row
		****************************************************
*/
		t22 = t;
		x22 = t + x;
		q   = t + x - 1;
		while (t22 < q)	*t22++ = *x22++;
		*t22 = *x22;
/*
		****************************************************
		copy bottom row from next-to-bottom row
		****************************************************
*/
		t22 = t + n - x;
		x22 = t22 - x;
		q   = t + n - 1;
		while (t22 < q)	*t22++ = *x22++;
		*t22 = *x22;		  

	return;
}


void fn_sort(n,ra)
int n;
float ra[];
{
        int l,j,ir,i;
        float rra;

        l=(n >> 1)+1;
        ir=n;
        for (;;) {
                if (l > 1)
                        rra=ra[--l];
                else {
                        rra=ra[ir];
                        ra[ir]=ra[1];
                        if (--ir == 1) {
                                ra[1]=rra;
                                return;
                        }
                }
                i=l;
                j=l << 1;
                while (j <= ir) {
                        if (j < ir && ra[j] < ra[j+1]) ++j;
                        if (rra < ra[j]) {
                                ra[i]=ra[j];
                                j += (i=j);
                        }
                        else j=ir+1;
                }
                ra[i]=rra;
        }
}
