
       subroutine norm(vals,rows,cols,nrows,ncols,maxr,maxc,p)
	   integer maxr,maxc,p
	   integer nrows(0:p),ncols(0:p)
	   real vals(maxc,maxr,0:p)
	   real rows(maxr,0:p), cols(maxc,0:p)

c
c  this example takes the first parameter array #1 and computes
c  the answer and places that in answer #0
c

       do 100 j=1,nrows(0)
		   do 100 i=1,ncols(0)
			   vals(i,j,0) = vals(i,j,1)/255.0
100    continue
       return
	   end
