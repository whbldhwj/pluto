data_t_B l_buf_B[32][16];
#define S1(d1,d2,d3,d4,d5,d6,d7,d8,d9,d10,d11) fifo_B.write(l_buf_B[d10][d11])

int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t3=0;t3<=31;t3++) {
      for (t4=2*t2;t4<=2*t2+1;t4++) {
        for (t5=2*t1;t5<=2*t1+1;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
            for (t7=16*t5;t7<=16*t5+15;t7++) {
              for (t8=16*t4;t8<=16*t4+15;t8++) {
                S1(t1,t2,t3,t4,t5,t6,t7,t8,t6,t6,t8);
              }
            }
          }
        }
      }
    }
  }
}
/* End of CLooG code */
