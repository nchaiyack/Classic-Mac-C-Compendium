/*
a quite simple algorithm and a main program to test it.

Given two adjacent memory areas, swap them without losing their
contents. The two areas may be of different sizes. The thing
is done in a single step and without using a large external buffer.
(To do that in many steps is easy, but slow: repeatedly rotate the
whole data set by one until it reachs the required position. To do
that with a large buffer is easy, if enough memory is available: copy
one of the areas to the buffer, move the other one to its final position,
then copy the buffer to the final position of this data. But fulfilling
both the requirements is not easy).


The algorithm is based on this fact: given two reciprocally prime
numbers, then the succession:

a(0)= any number between 0 and n2-1 (included)
a(i+1)= (a(i)+n1) MOD n2

visits once every number between 0 and n2-1 before repeating itself.
Furthermore, one should remember that MCD(n1,n2) = MCD(n1,n1+n2)
Now, do this:
take away one byte and conserve it in a temporary place. Fill the
"hole" in the data set by placing there the byte which must go there
in the final (swapped) configuration, and continue in this way until
the byte needed to fill the hole is the byte conserved outside the
data set.
If dim1 and dim2 are reciprocally prime, this is enough to complete
the job. Otherwise, this has put to the right place all the bytes whose
index MOD MCD(dim1,dim2) is equal to the same expression computed
for the firstly chosen byte. So, simply repeat the job on bytes with
different values of that expression.

The algorithm is simple enough that certainly somebody has
discovered and published it before I conceived it, but I don't
know anything about that...

Written by Gabriele Speranza, 11 July 1993
*/




#include <stdio.h>

void visualizza(char*buf,int n);
int euclide(int m, int n);



void main()
{
char temp,buffer[1000];
int i,j,new_i,dim1,dim2,MCD;

for(;;){
	for(i=0;i<sizeof(buffer);i++)
		buffer[i]=i+' ';


	printf("dim1, dim2:");
	scanf("%d %d",&dim1,&dim2);

	MCD=euclide(dim1,dim2);

	visualizza(buffer,dim1+dim2);
	for(j=0;j<MCD;j++){
		temp=buffer[j];
		i=j;
		for(;;){
			new_i= i+dim1;
			if(new_i>=dim1+dim2) new_i=i-dim2;
			if(new_i==j){
				buffer[i]=temp;
				break;
				}
			buffer[i]=buffer[new_i];
			i=new_i;
			}
		}
	visualizza(buffer,dim1+dim2);
	}
}

static void visualizza(buf,n)
char*buf;
int n;
{
while(n--) printf("%c",*buf++);
printf("\n");
}

static int euclide(m,n)
int m,n;
{
int r;
for(;;){
	r=m % n;
	if(r==0) return n;
	m=n;
	n=r;
	}
}

