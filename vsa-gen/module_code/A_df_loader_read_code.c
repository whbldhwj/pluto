data_t_A l_buf_A[(+1*(32)*(32))];
#define S1(d1,d2,d3,d4,d5)	g_buf_A[d4][d5] = A[d4][d5]

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t3=0;t3<=31;t3++) {
      for (t4=32*t1;t4<=32*t1+31;t4++) {
        for (t5=32*t3;t5<=32*t3+31;t5++) {
          S1(t1,t2,t3,t4,t5);
        }
      }
    }
  }
}
/* End of CLooG code */
