#define S1(r,s,i,j) Z[r][s] = 0;
#define S2(r,s,i,j) Z[r][s] = (Z[r][s] + (X[r + i][s + j] * W[i][j]));

int t1, t2, t3, t4;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=5;t2<=28;t2++) {
    if (t2 <= 19) {
      if ((t2+1)%2 == 0) {
        S1(((t2-3)/2),t1,1,1);
      }
    }
    for (t3=max(1,ceild(t2-19,3));t3<=min(3,floord(t2-3,3));t3++) {
      for (t4=max(ceild(t2+t3+1,2),t2-t3-8);t4<=min(floord(t2+t3+3,2),t2-t3-1);t4++) {
        S2((t2-t3-t4),t1,t3,(-t2-t3+2*t4));
      }
    }
  }
}
/* End of CLooG code */
