#define S1(zT5,zT6,zT7,zT8,zT4,r,s,i,j) Z[r - 1][s - 1] += X[r + i - 1][s + j - 1] * W[i][j];

int t1, t2, t3, t4, t5, t6, t7, t8, t9;

/* Start of CLooG code */
if ((R >= 1) && (S >= 1)) {
  for (t1=0;t1<=floord(S,32);t1++) {
    for (t3=0;t3<=floord(R+6,32);t3++) {
      for (t4=max(0,ceild(t3-2,2));t4<=min(floord(t3,2),floord(R,64));t4++) {
        for (t5=max(1,32*t1);t5<=min(S,32*t1+31);t5++) {
          for (t6=max(0,ceild(32*t3-R-2,2));t6<=2;t6++) {
            for (t7=max(max(32*t3,64*t4+2*t6),2*t6+1);t7<=min(min(32*t3+31,64*t4+2*t6+65),2*t6+R+2);t7++) {
              for (t8=max(ceild(-2*t6+t7-3,2),32*t4);t8<=min(min(floord(R,2),floord(-2*t6+t7,2)),32*t4+31);t8++) {
                for (t9=max(max(1,2*t8),-2*t6+t7-2);t9<=min(min(R,-2*t6+t7),2*t8+1);t9++) {
                  S1(t1,0,t3,t4,t8,t9,t5,t6,(-2*t6+t7-t9));
                }
              }
            }
          }
        }
      }
    }
  }
}
/* End of CLooG code */
