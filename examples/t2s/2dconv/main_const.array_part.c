#define S1(zT6,zT7,zT8,zT9,oT4,iT4,r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4, t5, t6, t7, t8, t9;

/* Start of CLooG code */
for (t1=0;t1<=1;t1++) {
  for (t3=0;t3<=1;t3++) {
    for (t5=max(1,32*t1);t5<=min(32,32*t1+31);t5++) {
      for (t6=0;t6<=2;t6++) {
        for (t7=max(32*t3,2*t6+1);t7<=min(32*t3+31,2*t6+34);t7++) {
          for (t8=max(0,ceild(-2*t6+t7-3,2));t8<=min(16,floord(-2*t6+t7,2));t8++) {
            for (t9=max(max(0,-2*t8+1),-2*t6+t7-2*t8-2);t9<=min(min(1,-2*t8+32),-2*t6+t7-2*t8);t9++) {
              S1(t1,0,t3,0,t8,t9,(2*t8+t9),t5,t6,(-2*t6+t7-2*t8-t9));
            }
          }
        }
      }
    }
  }
}
/* End of CLooG code */
