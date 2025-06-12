c   
c  Example FORTRAN source for generating a movie sequence of images
c  to be viewed with NCSA DataScope 1.2.
c
c  Works with NCSA DataScope 1.2 and the dscall.o library included and
c  documented with DataScope.  The ds_send call transfers the dataset to
c  the client Macintosh.
c
c  Has been tested on:
c     UNICOS
c     Sun UNIX 4.0 - change the array dimensions to 20x20 or it takes forever
c
c  Remember that on SUN4 systems, you cannot run external routines
c  unless your IP number is hardcoded into the system table as a remote
c  host; this is because DataScope uses the rexecd socket, where this
c  restriction applies.
c
c  On UNICOS:
c     cc -DUNICOS -c dscall.c
c     cf77 wavex.f dscall.o -o wavex -lnet
c
c  On Suns:
c     cc -DSUN -c dscall.c
c     f77 wavex.f dscall.o -o wavex
c
c
c  original version:  Alan Craig, 1989
c  DataScope version:  Tim Krauskopf, 1989
c
c  this code is in the public domain
c

       real dat(50,50),diff
       character*1 hdf(50,50)
       character*1 pal(768)
       character*80 fname,host
       character*4 tmp
       integer idab(50,50),shape(2)
c
c  Change this host line to indicate YOUR Macintosh.  Use the 
c  IP address in most cases, e.g.
c            host = '192.17.20.10'
c  or use the "name" associated with your Mac, e.g.
c            host = 'mymac.uiuc.edu'
c
       host = '192.17.20.10'
       call ds_open(host)
	   
	   
       xn=0.0
       yn=0.0
       pi=3.1415926
       c=0.1
       be=0.1
       xsize = 50 
       ysize = 50 
       ce=0.5
       bot = 99999999
       top= -999999999
       fname = '        '
       do 10 i=1,10,2
		  print*,'frame = ',i

          do 50 j = 1,xsize
           do 21 jj=1,ysize
             x = j / xsize 
             y = jj / ysize
             t = i
             f4 = 0.0
            
              do 30 n = -15,15 
               do 31 m= -15,15

                fa = cos(sqrt((float(m))**2 + (float(n))**2) * pi *c *t)
                f2 = cos(n * pi * xn)
                f3 = cos(n * pi * x)
                g2 = cos(m * pi * yn)
                g3 = cos(m * pi * y)
                f4 = f4 + ( fa * f2 * f3 * g2 * g3)

31             continue
30           continue
            
             dat(j,jj) = f4

21        continue
50        continue

c	This section finds the range of values in a single frame

          if (i .eq. 1) then 
            do 91 im=1 , xsize
             do 90 in=1 , ysize
                if (dat(im,in) .lt. bot) then 
                  bot = dat(im,in)
                endif
                if (dat(im,in) .gt. top) then 
                   top = dat(im,in)
                endif
90          continue
91        continue
             diff = top - bot
              ratio = 255.0 / diff
           endif


c	This section constructs a file name for each frame based on the 
c	iteration number

          write (tmp,37) i
37        format (I4)
          do 92 ll=1,4
              if ((tmp(ll:ll)) .eq. ' ') then
                 tmp(ll:ll) = '0'
              endif
92        continue
          fname='wav.'//tmp

c	This line writes out a single frame

       call ds(fname,dat,50,50,top,bot)

10     continue

	call ds_close
       stop
       end


c
c  See NCSA DataScope documentation for more information on the 
c  ds_send call.  Uses routines in dscall.o, provided with DataScope.
c
c
       subroutine ds(name,vals,nrows,ncols,max,min) 
       integer nrows,ncols,dummy
       real vals(ncols,nrows),rows(50),cols(50),max,min
       character *80 host,name,flags

       do 500 i=1,nrows
500	  rows(i) = float(i)

       do 501 i=1,ncols
501	  cols(i) = float(i)

       max = 220.0
       min = -150.0
       flags = 'RG'

       call ds_send1(name,flags,max,min,nrows,ncols,rows,cols,vals)

       return
       end
