#define S1(oT4,iT4,r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=1;t1<=32;t1++) {
  for (t2=0;t2<=2;t2++) {
    for (t3=2*t2+1;t3<=2*t2+34;t3++) {
      for (t4=max(0,ceild(-2*t2+t3-3,2));t4<=min(16,floord(-2*t2+t3,2));t4++) {
        for (t5=max(max(0,-2*t4+1),-2*t2+t3-2*t4-2);t5<=min(min(1,-2*t4+32),-2*t2+t3-2*t4);t5++) {
          S1(t4,t5,(2*t4+t5),t1,t2,(-2*t2+t3-2*t4-t5));
        }
      }
    }
  }
}
/* End of CLooG code */
