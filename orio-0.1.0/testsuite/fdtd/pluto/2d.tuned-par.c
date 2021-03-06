

#include <stdio.h>
#include <sys/time.h>
#include <math.h>


#define ceild(n,d)  ceil(((double)(n))/((double)(d)))
#define floord(n,d) floor(((double)(n))/((double)(d)))
#define max(x,y)    ((x) > (y)? (x) : (y))
#define min(x,y)    ((x) < (y)? (x) : (y))

#define tmax TVAL
#define nx NXVAL
#define ny NYVAL
double ex[nx][ny+1];
double ey[nx+1][ny];
double hz[nx][ny];

void init_arrays()
{
  int i1, i2;
  for (i1=0; i1<nx; i1++)
   for (i2=0; i2<ny+1; i2++)
    ex[i1][i2] = (i1+i2) % 5 + 1;
  for (i1=0; i1<nx+1; i1++)
   for (i2=0; i2<ny; i2++)
    ey[i1][i2] = (i1+i2) % 5 + 1;
  for (i1=0; i1<nx; i1++)
   for (i2=0; i2<ny; i2++)
    hz[i1][i2] = (i1+i2) % 5 + 1;
}

double rtclock()
{
  struct timezone tzp;
  struct timeval tp;
  int stat;
  gettimeofday (&tp, &tzp);
  return (tp.tv_sec + tp.tv_usec*1.0e-6);
}

int main()
{
  init_arrays();

  double annot_t_start=0, annot_t_end=0, annot_t_total=0;
  int annot_i;

  for (annot_i=0; annot_i<REPS; annot_i++)
  {
    annot_t_start = rtclock();
    
  

int t, i, j, k, l, m, n, ii, jj;

	#define S1(zT0,zT1,t,j)	{ey[0][j]=t;}
	#define S2(zT0,zT1,zT2,t,i,j)	{ey[i][j]=ey[i][j]-((double)(1))/2*(hz[i][j]-hz[i-1][j]);}
	#define S3(zT0,zT1,zT2,t,i,j)	{ex[i][j]=ex[i][j]-((double)(1))/2*(hz[i][j]-hz[i][j-1]);}
	#define S4(zT0,zT1,zT2,t,i,j)	{hz[i][j]=hz[i][j]-((double)(7))/10*(ey[1+i][j]+ex[i][1+j]-ex[i][j]-ey[i][j]);}

	int c1, c2, c3, c4, c5, c6, c7;

	register int lb, ub, lb1, ub1, lb2, ub2;
	register int lbv, ubv;

for (c1=-1;c1<=floord(2*tmax+ny-2,32);c1++) {
	lb1=max(max(ceild(32*c1-tmax+1,32),ceild(32*c1-31,64)),0);
	ub1=min(min(floord(32*c1+ny+31,64),floord(tmax+ny-1,32)),floord(32*c1+31,32));
#pragma omp parallel for shared(c1,lb1,ub1) private(c2,c3,c4,c5,c6,c7)
	for (c2=lb1; c2<=ub1; c2++) {
    for (c3=max(max(max(max(ceild(32*c2-ny-30,32),0),ceild(64*c1-96*c2-61,32)),ceild(32*c1-32*c2-31,32)),ceild(32*c1-1024*c2-1891,992));c3<=min(min(floord(32*c2+nx+30,32),floord(tmax+nx-1,32)),floord(32*c1-32*c2+nx+31,32));c3++) {
      if ((c1 <= floord(32*c2+32*c3-nx,32)) && (c2 <= floord(32*c3-nx+ny,32)) && (c3 >= ceild(nx,32))) {
        for (c5=max(32*c2,32*c3-nx+1);c5<=min(32*c3-nx+ny,32*c2+31);c5++) {
          S4(c1-c2,-c1+c2+c3,-c1+2*c2,32*c3-nx,nx-1,-32*c3+c5+nx-1) ;
        }
      }
      if ((c1 <= floord(64*c2-ny,32)) && (c2 >= max(ceild(32*c3-nx+ny+1,32),ceild(ny,32)))) {
        for (c6=max(32*c3,32*c2-ny+1);c6<=min(32*c2+nx-ny,32*c3+31);c6++) {
          S4(c1-c2,-c1+c2+c3,-c1+2*c2,32*c2-ny,-32*c2+c6+ny-1,ny-1) ;
        }
      }
      if (c1 == c2+c3) {
        for (c4=max(max(32*c2-ny+1,0),32*c3);c4<=min(min(32*c3+30,32*c2-ny+31),tmax-1);c4++) {
          for (c5=32*c2;c5<=c4+ny-1;c5++) {
            S1(c1-c2,-c1+2*c2,c4,-c4+c5) ;
            S3(c1-c2,0,-c1+2*c2,c4,0,-c4+c5) ;
            for (c6=c4+1;c6<=32*c3+31;c6++) {
              S2(c1-c2,0,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
              S3(c1-c2,0,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
              S4(c1-c2,0,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
            }
          }
          for (c6=c4+1;c6<=32*c3+31;c6++) {
            S4(c1-c2,0,-c1+2*c2,c4,-c4+c6-1,ny-1) ;
          }
        }
      }
      if (c1 == c2+c3) {
        for (c4=max(max(32*c3,0),32*c2-ny+32);c4<=min(min(32*c3+30,tmax-1),32*c2-1);c4++) {
          for (c5=32*c2;c5<=32*c2+31;c5++) {
            S1(c1-c2,-c1+2*c2,c4,-c4+c5) ;
            S3(c1-c2,0,-c1+2*c2,c4,0,-c4+c5) ;
            for (c6=c4+1;c6<=32*c3+31;c6++) {
              S2(c1-c2,0,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
              S3(c1-c2,0,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
              S4(c1-c2,0,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
            }
          }
        }
      }
      if (c1 == c2+c3) {
        for (c4=max(max(32*c2,32*c3),0);c4<=min(min(32*c2+30,32*c3+30),tmax-1);c4++) {
          S1(c1-c2,-c1+2*c2,c4,0) ;
          for (c6=c4+1;c6<=32*c3+31;c6++) {
            S2(c1-c2,0,-c1+2*c2,c4,-c4+c6,0) ;
          }
          for (c5=c4+1;c5<=32*c2+31;c5++) {
            S1(c1-c2,-c1+2*c2,c4,-c4+c5) ;
            S3(c1-c2,0,-c1+2*c2,c4,0,-c4+c5) ;
            for (c6=c4+1;c6<=32*c3+31;c6++) {
              S2(c1-c2,0,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
              S3(c1-c2,0,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
              S4(c1-c2,0,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
            }
          }
        }
      }
      for (c4=max(max(max(32*c1-32*c2,0),32*c2-ny+1),32*c3-nx+1);c4<=min(min(min(32*c3-nx+31,32*c1-32*c2+31),tmax-1),32*c2-ny+31);c4++) {
        for (c5=32*c2;c5<=c4+ny-1;c5++) {
          for (c6=32*c3;c6<=c4+nx-1;c6++) {
            S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
          }
          S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,nx-1,-c4+c5-1) ;
        }
        for (c6=32*c3;c6<=c4+nx;c6++) {
          S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,ny-1) ;
        }
      }
      for (c4=max(max(max(0,32*c3-nx+1),32*c1-32*c2),32*c2-ny+32);c4<=min(min(min(tmax-1,32*c1-32*c2+31),32*c2-1),32*c3-nx+31);c4++) {
        for (c5=32*c2;c5<=32*c2+31;c5++) {
          for (c6=32*c3;c6<=c4+nx-1;c6++) {
            S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
          }
          S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,nx-1,-c4+c5-1) ;
        }
      }
      for (c4=max(max(max(32*c3-nx+32,32*c1-32*c2),0),32*c2-ny+1);c4<=min(min(min(32*c3-1,32*c1-32*c2+31),tmax-1),32*c2-ny+31);c4++) {
        for (c5=32*c2;c5<=c4+ny-1;c5++) {
          for (c6=32*c3;c6<=32*c3+31;c6++) {
            S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
          }
        }
        for (c6=32*c3;c6<=32*c3+31;c6++) {
          S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,ny-1) ;
        }
      }
      for (c4=max(max(max(32*c2,0),32*c3-nx+1),32*c1-32*c2);c4<=min(min(min(tmax-1,32*c1-32*c2+31),32*c2+30),32*c3-nx+31);c4++) {
        for (c6=32*c3;c6<=c4+nx-1;c6++) {
          S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,0) ;
        }
        for (c5=c4+1;c5<=32*c2+31;c5++) {
          for (c6=32*c3;c6<=c4+nx-1;c6++) {
            S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
          }
          S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,nx-1,-c4+c5-1) ;
        }
      }
      for (c4=max(max(max(0,32*c1-32*c2),32*c3-nx+32),32*c2-ny+32);c4<=min(min(min(32*c3-1,tmax-1),32*c1-32*c2+31),32*c2-1);c4++) {


/*@ begin Loop(
	  transform Composite(
	   tile = [('c5',T1,'ii'),('c6',T2,'jj')],
           permut = [PERMUTS],
           unrolljam = [('c5',U1),('c6',U2)],
	   vector = (VEC, ['ivdep','vector always'])
	  )
	  
        for (c5=32*c2;c5<=32*c2+31;c5++)
          for (c6=32*c3;c6<=32*c3+31;c6++) {
            S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
          }
        

) @*/{
  for (c6=32*c3; c6<=32*c3+28; c6=c6+4) {
    register int cbv_1, cbv_2;
    cbv_1=32*c2;
    cbv_2=32*c2+31;
#pragma ivdep
#pragma vector always
    for (c5=cbv_1; c5<=cbv_2; c5=c5+1) {
      S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5);
      S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+1,-c4+c5);
      S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+2,-c4+c5);
      S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+3,-c4+c5);
      S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5);
      S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+1,-c4+c5);
      S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+2,-c4+c5);
      S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+3,-c4+c5);
      S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1);
      S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5-1);
      S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+1,-c4+c5-1);
      S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6+2,-c4+c5-1);
    }
  }
  for (; c6<=32*c3+31; c6=c6+1) {
    register int cbv_3, cbv_4;
    cbv_3=32*c2;
    cbv_4=32*c2+31;
#pragma ivdep
#pragma vector always
    for (c5=cbv_3; c5<=cbv_4; c5=c5+1) {
      S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5);
      S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5);
      S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1);
    }
  }
}
/*@ end @*/

      }
      for (c4=max(max(max(32*c2,32*c3-nx+32),0),32*c1-32*c2);c4<=min(min(min(32*c3-1,tmax-1),32*c1-32*c2+31),32*c2+30);c4++) {
        for (c6=32*c3;c6<=32*c3+31;c6++) {
          S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,0) ;
        }
        for (c5=c4+1;c5<=32*c2+31;c5++) {
          for (c6=32*c3;c6<=32*c3+31;c6++) {
            S2(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S3(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6,-c4+c5) ;
            S4(c1-c2,-c1+c2+c3,-c1+2*c2,c4,-c4+c6-1,-c4+c5-1) ;
          }
        }
      }
      if ((-c1 == -c2-c3) && (c1 <= min(floord(64*c3-1,32),floord(32*c3+tmax-32,32)))) {
        S1(c3,c1-2*c3,32*c1-32*c3+31,0) ;
        for (c6=32*c1-32*c3+32;c6<=32*c3+31;c6++) {
          S2(c3,0,c1-2*c3,32*c1-32*c3+31,-32*c1+32*c3+c6-31,0) ;
        }
      }
      if ((-c1 == -c2-c3) && (c1 >= ceild(64*c2-31,32)) && (c1 <= min(floord(32*c2+tmax-32,32),floord(64*c2-1,32)))) {
        S1(c1-c2,-c1+2*c2,32*c1-32*c2+31,0) ;
        for (c5=32*c1-32*c2+32;c5<=32*c2+31;c5++) {
          S1(c1-c2,-c1+2*c2,32*c1-32*c2+31,-32*c1+32*c2+c5-31) ;
          S3(c1-c2,0,-c1+2*c2,32*c1-32*c2+31,0,-32*c1+32*c2+c5-31) ;
        }
      }
      if ((-c1 == -c2-c3) && (c1 <= min(floord(32*c2+tmax-32,32),2*c2-1))) {
        for (c5=32*c2;c5<=min(32*c2+31,32*c1-32*c2+ny+30);c5++) {
          S1(c1-c2,-c1+2*c2,32*c1-32*c2+31,-32*c1+32*c2+c5-31) ;
          S3(c1-c2,0,-c1+2*c2,32*c1-32*c2+31,0,-32*c1+32*c2+c5-31) ;
        }
      }
      if ((-c1 == -2*c2) && (-c1 == -2*c3) && (c1 <= floord(tmax-32,16))) {
        if (c1%2 == 0) {
          S1(c1/2,0,16*c1+31,0) ;
        }
      }
      if ((c1 >= 2*c2) && (c2 <= min(c3-1,floord(tmax-32,32)))) {
        for (c6=32*c3;c6<=min(32*c3+31,32*c2+nx+30);c6++) {
          S2(c1-c2,-c1+c2+c3,-c1+2*c2,32*c2+31,-32*c2+c6-31,0) ;
        }
      }
    }
  }
 }




    annot_t_end = rtclock();
    annot_t_total += annot_t_end - annot_t_start;
  }
  
  annot_t_total = annot_t_total / REPS;
  printf("%f\n", annot_t_total);
  
  return 1;
}
                                                                   
