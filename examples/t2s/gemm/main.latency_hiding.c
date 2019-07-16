#define S1(oT5,iT5,oT3,iT3,i,j,k) C[i][j] += A[i][k] * B[k][j];

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
if ((I >= 1) && (J >= 1) && (K >= 1)) {
  for (t1=0;t1<=floord(I-1,8);t1++) {
    for (t2=0;t2<=floord(J-1,8);t2++) {
      for (t3=0;t3<=K-1;t3++) {
        for (t4=0;t4<=min(7,-8*t1+I-1);t4++) {
          for (t5=0;t5<=min(7,-8*t2+J-1);t5++) {
            S1(t2,t5,t1,t4,(8*t1+t4),(8*t2+t5),t3);
          }
        }
      }
    }
  }
}
/* End of CLooG code */
