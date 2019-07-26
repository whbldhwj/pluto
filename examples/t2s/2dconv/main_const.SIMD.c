#define S1(oT4,r,s,i,j) Z[r - 1][s - 1] = 0;
#define S2(oT4,r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=1;t1<=32;t1++) {
  for (t2=0;t2<=2;t2++) {
    for (t3=t2;t3<=t2+17;t3++) {
      for (t4=max(1,-2*t2+2*t3-2);t4<=min(32,-2*t2+2*t3+1);t4++) {
        if ((t2 == 0) && (t3 <= floord(t4,2))) {
          S1(t3,t4,t1,0,0);
        }
        for (t5=max(0,2*t2-2*t3+t4);t5<=min(1,2*t2-2*t3+t4+2);t5++) {
          S2(t3,t4,t1,t2,(-2*t2+2*t3-t4+t5));
        }
      }
    }
  }
}
/* End of CLooG code */
