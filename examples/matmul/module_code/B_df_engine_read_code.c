data_t_B l_buf_B[(+1*(256)*(16))];
#define S1(d1,d2,d3,d4,d5,d6)	l_buf_B[d5][d6] = fifo_B.read()

int t1, t2, t3, t4, t5, t6;

/* Start of CLooG code */
for (t1=0;t1<=1;t1++) {
  for (t2=0;t2<=1;t2++) {
    for (t3=0;t3<=3;t3++) {
      for (t4=32*t2;t4<=32*t2+31;t4++) {
        for (t5=256*t3;t5<=256*t3+255;t5++) {
          for (t6=16*t4;t6<=16*t4+15;t6++) {
            S1(t1,t2,t3,t4,t5,t6);
          }
        }
      }
    }
  }
}
/* End of CLooG code */
