data_t_A l_buf_A[1][1];
data_t_B l_buf_B[1][1];
data_t_C l_buf_C[16][16];
#define S1(zT6,zT7,zT8,zT5,zT4,zT3,i,j,k) C[i][j] = beta*C[i][j] + alpha*A[i][k] * B[k][j];
#define S2(d1,d2,d3,d4,d5,d6,d7,d8,d9) l_buf_A[d10][d11] = fifo_A.read()
#define S3(d1,d2,d3,d4,d5,d6,d7,d8,d9) l_buf_B[d10][d11] = fifo_B.read()
#define S4(d1,d2,d3,d4,d5,d6,d7,d8,d9) fifo_C.write(l_buf_C[d10][d11])

int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t3=0;t3<=31;t3++) {
      for (t4=2*t1;t4<=2*t1+1;t4++) {
        for (t5=2*t2;t5<=2*t2+1;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
            for (t7=16*t4;t7<=16*t4+15;t7++) {
              for (t8=16*t5;t8<=16*t5+15;t8++) {
                S1(t1,t2,t3,t6,t5,t4,t7,t8,t6);
                S2(t1,t2,t3,t4,t5,t6,t7,t8,t6);
                S3(t1,t2,t3,t4,t5,t6,t7,t8,t6);
                if ((t3 == 31) && (t6 == 1023)) {
                  S4(t1,t2,31,t4,t5,1023,t7,t8,1023);
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
