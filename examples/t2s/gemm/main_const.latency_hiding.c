#define S1(oT4,oT3,i,j,k) C[i][j] = 0;
#define S2(oT4,oT3,i,j,k) C[i][j] += A[i][k] * B[k][j];

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=0;t1<=15;t1++) {
  for (t2=0;t2<=15;t2++) {
    for (t3=0;t3<=63;t3++) {
      for (t4=0;t4<=3;t4++) {
        for (t5=0;t5<=3;t5++) {
          if (t3 == 0) {
            S1(t2,t1,(4*t1+t4),(4*t2+t5),0);
          }
          S2(t2,t1,(4*t1+t4),(4*t2+t5),t3);
        }
      }
    }
  }
}
/* End of CLooG code */
