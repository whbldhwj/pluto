#define S1(oT5,oT6,oT7,oT4,r,s,i,j) Z[r - 1][s - 1] = 0;
#define S2(oT5,oT6,oT7,oT4,r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4, t5, t6, t7, t8, t9;

/* Start of CLooG code */
for (t1=0;t1<=4;t1++) {
  for (t3=0;t3<=2;t3++) {
    for (t4=max(0,-8*t1+1);t4<=min(7,-8*t1+32);t4++) {
      for (t5=0;t5<=2;t5++) {
        for (t6=max(0,-8*t3+t5);t6<=min(7,-8*t3+t5+17);t6++) {
          for (t7=max(1,16*t3-2*t5+2*t6-2);t7<=min(32,16*t3-2*t5+2*t6+1);t7++) {
            if ((t3 <= floord(-2*t6+t7,16)) && (t5 == 0)) {
              S1(t1,0,t3,(8*t3+t6),t7,(8*t1+t4),0,0);
            }
            for (t8=max(0,-16*t3+2*t5-2*t6+t7);t8<=min(1,-16*t3+2*t5-2*t6+t7+2);t8++) {
              S2(t1,0,t3,(8*t3+t6),t7,(8*t1+t4),t5,(16*t3-2*t5+2*t6-t7+t8));
            }
          }
        }
      }
    }
  }
}
/* End of CLooG code */
