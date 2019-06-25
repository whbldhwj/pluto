data_t_A l_buf_A[16][32];
#define S1(d1,d2,d3,d4,d5,d6) l_buf_A[d5][d6] = fifo_A.read()

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t3=0;t3<=31;t3++) {
      for (t4=2*t1;t4<=2*t1+1;t4++) {
        for (t5=16*t4;t5<=16*t4+15;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
            S1(t1,t2,t3,t4,t5,t6);
          }
        }
      }
    }
  }
}
/* End of CLooG code */
