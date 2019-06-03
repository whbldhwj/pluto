data_t_B l_buf_B[(+1*(256)*(512))];
#define S1(d1,d2,d3,d4,d5)	g_buf_B[d4][d5] = B[d4][d5]

int t1, t2, t3, t4, t5;

/* Start of CLooG code */
for (t1=0;t1<=1;t1++) {
  for (t2=0;t2<=1;t2++) {
    for (t3=0;t3<=3;t3++) {
      for (t4=256*t3;t4<=256*t3+255;t4++) {
        for (t5=512*t2;t5<=512*t2+511;t5++) {
          S1(t1,t2,t3,t4,t5);
        }
      }
    }
  }
}
/* End of CLooG code */
