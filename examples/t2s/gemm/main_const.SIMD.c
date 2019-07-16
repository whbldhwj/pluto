#define S1(oT5,oT4,oT3,i,j,k) C[i][j] += A[i][k] * B[k][j];

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=0;t1<=7;t1++) {
  for (t2=0;t2<=7;t2++) {
    for (t3=0;t3<=31;t3++) {
      for (t4=0;t4<=7;t4++) {
        for (t5=0;t5<=7;t5++) {
          for (t6=0;t6<=1;t6++) {
            S1(t3,t2,t1,(8*t1+t4),(8*t2+t5),(2*t3+t6));
          }
        }
      }
    }
  }
}
/* End of CLooG code */
