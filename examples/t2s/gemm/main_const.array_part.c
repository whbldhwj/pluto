#define S1(zT6,zT7,zT8,zT9,zT10,oT5,oT4,oT3,i,j,k) C[i][j] += A[i][k] * B[k][j];

int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11;

/* Start of CLooG code */
for (t6=0;t6<=7;t6++) {
  for (t7=0;t7<=7;t7++) {
    for (t8=0;t8<=31;t8++) {
      for (t9=0;t9<=7;t9++) {
        for (t10=0;t10<=7;t10++) {
          for (t11=0;t11<=1;t11++) {
            S1(0,0,0,0,0,t8,t7,t6,(8*t6+t9),(8*t7+t10),(2*t8+t11));
          }
        }
      }
    }
  }
}
/* End of CLooG code */
