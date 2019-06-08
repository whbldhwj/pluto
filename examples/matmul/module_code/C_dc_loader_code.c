data_t_C l_buf_C[(+1*(32)*(32))];
#define S1(t1,t2,t3,t4,t5)	C[d4][d5] = g_buf_C[d4][d5]
#define S2(t1,t2,t3,t4,t5,t6,t7,t8,t9,t10,t11)	g_buf_C[t10][t11] = fifo_C.read()

int t1, t2, t3, t4, t5, t6, t7, t8, t9, t10, t11, t12;

/* Start of CLooG code */
for (t1=0;t1<=31;t1++) {
  for (t2=0;t2<=31;t2++) {
    for (t5=-2*t2-1;t5<=-2*t2;t5++) {
      for (t6=-2*t1-1;t6<=-2*t1;t6++) {
        for (t8=-16*t6;t8<=-16*t6+15;t8++) {
          for (t9=-16*t5;t9<=-16*t5+15;t9++) {
            S2(t1,t2,31,t5,t6,1023,t8,t9,1023,t8,t9);
          }
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
