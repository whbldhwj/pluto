#define S1(oT6,oT7,oT8,oT5,oT4,oT3,i,j,k) C[i][j] = 0;
#define S2(oT6,oT7,oT8,oT5,oT4,oT3,i,j,k) C[i][j] = (C[i][j] + (A[i][k] * B[k][j]));

int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;

/* Start of CLooG code */
for (t1=0;t1<=3;t1++) {
  for (t2=0;t2<=3;t2++) {
    for (t3=0;t3<=3;t3++) {
      for (t4=0;t4<=1;t4++) {
        for (t5=0;t5<=1;t5++) {
          for (t6=0;t6<=1;t6++) {
            for (t7=0;t7<=1;t7++) {
              for (t8=0;t8<=1;t8++) {
                if ((t3 == 0) && (t6 == 0)) {
                  S1(t1,t2,0,0,(2*t2+t5),(2*t1+t4),(4*t1+2*t4+t7),(4*t2+2*t5+t8),0);
                }
                for (t9=0;t9<=1;t9++) {
                  S2(t1,t2,t3,(2*t3+t6),(2*t2+t5),(2*t1+t4),(4*t1+2*t4+t7),(4*t2+2*t5+t8),(4*t3+2*t6+t9));
                }
              }
            }
          }
        }
      }
    }
  }
}
/* End of CLooG code */
