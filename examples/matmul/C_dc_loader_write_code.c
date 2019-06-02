data_t_C gw_buf_C[(+1*(512)*(512))];
#define S1(d1,d2,d3,d4,d5)	gw_buf_C[d4][d5] = lw_buf_C[d4][d5]

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=1;t1++) {
  for (t2=0;t2<=1;t2++) {
    for (t4=512*t1;t4<=512*t1+511;t4++) {
      for (t5=512*t2;t5<=512*t2+511;t5++) {
        S1(t1,t2,3,t4,t5);
      }
    }
  }
}
/* End of CLooG code */
