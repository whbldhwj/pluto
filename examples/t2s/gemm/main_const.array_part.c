#define S1(oT5,oT6,oT7,oT4,oT3,i,j,k) C[i][j] = A[i][k] * B[k][j];
#define S2(oT5,oT6,oT7,oT4,oT3,i,j,k) C[i][j] += A[i][k] * B[k][j];

int t1, t2, t3, t4, t5, t6, t7, t8;

/* Start of CLooG code */
for (t1=0;t1<=1;t1++) {
  for (t2=0;t2<=1;t2++) {
    for (t4=0;t4<=7;t4++) {
      for (t5=0;t5<=7;t5++) {
        for (t7=0;t7<=3;t7++) {
          for (t8=0;t8<=3;t8++) {
            S1(t1,t2,0,(8*t2+t5),(8*t1+t4),(32*t1+4*t4+t7),(32*t2+4*t5+t8),0);
          }
        }
      }
    }
  }
}
/* End of CLooG code */
