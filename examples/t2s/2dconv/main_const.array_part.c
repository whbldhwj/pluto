#define S1(oT4,oT5,oT6,r,s,i,j) Z[r - 1][s - 1] = 0;
#define S2(oT4,oT5,oT6,r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4, t5, t6, t7, t8;

/* Start of CLooG code */
for (t1=0;t1<=4;t1++) {
  for (t3=0;t3<=4;t3++) {
    for (t4=max(0,-8*t1+1);t4<=min(7,-8*t1+32);t4++) {
      for (t5=0;t5<=2;t5++) {
        for (t6=max(0,-8*t3+2*t5+1);t6<=min(7,-8*t3+2*t5+34);t6++) {
          for (t7=max(1,8*t3-2*t5+t6-2);t7<=min(32,8*t3-2*t5+t6);t7++) {
            if ((8*t3 == -t6+t7) && (t5 == 0)) {
              S1(t1,0,t3,(8*t3+t6),(8*t1+t4),0,0);
            }
            S2(t1,0,t3,t7,(8*t1+t4),t5,(8*t3-2*t5+t6-t7));
          }
        }
      }
    }
  }
}
/* End of CLooG code */
