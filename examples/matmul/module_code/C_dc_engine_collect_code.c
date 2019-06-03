data_t_C l_buf_C[(+1*(512)*(16))];
#define S1(d1,d2,d3,d4,d5,d6)	l_buf_C[l_count_C++] = fifo_C.read()

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=0;t1<=1;t1++) {
  for (t2=0;t2<=1;t2++) {
    for (t4=-32*t2-31;t4<=-32*t2;t4++) {
      for (t5=512*t1;t5<=512*t1+511;t5++) {
        for (t6=-16*t4;t6<=-16*t4+15;t6++) {
          S1(t1,t2,3,t4,t5,t6);
        }
      }
    }
  }
}
/* End of CLooG code */
