data_t_C l_buf_C[32][32];
#define S1(d1,d2,d3,d4,d5) C[d4][d5] = g_buf_C[d4][d5]
#define S2(d1,d2,d3,d4,d5,d6) g_buf_C[d5][d6] = fifo_C.read()

int t1, t2, t3, t4, t5, t6, t7;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t5=2*t2;t5<=2*t2+1;t5++) {
      for (t6=32*t1;t6<=32*t1+31;t6++) {
        for (t7=16*t5;t7<=16*t5+15;t7++) {
          S2(t1,t2,31,t5,t6,t7);
        }
      }
    }
    for (t5=32*t1;t5<=32*t1+31;t5++) {
      for (t6=32*t2;t6<=32*t2+31;t6++) {
        S1(t1,t2,31,t5,t6);
      }
    }
  }
}
/* End of CLooG code */
