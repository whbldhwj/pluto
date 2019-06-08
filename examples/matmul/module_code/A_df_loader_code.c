data_t_A l_buf_A[(+1*(32)*(32))];
#define S1(t1,t2,t3,t4,t5)	g_buf_A[d4][d5] = A[d4][d5]
#define S2(t1,t2,t3,t4,t5,t6)	fifo_A.write(g_buf_A[d5][d6])

int t1, t2, t3, t4, t5, t6, t7;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t3=0;t3<=31;t3++) {
      for (t5=32*t1;t5<=32*t1+31;t5++) {
        for (t6=32*t3;t6<=32*t3+31;t6++) {
          S1(t1,t2,t3,t5,t6);
        }
      }
      for (t5=2*t1;t5<=2*t1+1;t5++) {
        for (t6=16*t5;t6<=16*t5+15;t6++) {
          for (t7=32*t3;t7<=32*t3+31;t7++) {
            S2(t1,t2,t3,t5,t6,t7);
          }
        }
      }
    }
  }
}
/* End of CLooG code */
