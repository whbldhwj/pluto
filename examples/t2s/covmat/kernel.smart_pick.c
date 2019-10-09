#define S1(i,j) V[i][j] = 0;
#define S2(i,j,k) V[i][j] = (V[i][j] + ((d[k][i] * d[k][j]) / 8));

int t1, t2, t3;

/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=8;t2++) {
    S1(t2,t1);
    for (t3=1;t3<=8;t3++) {
      S2(t2,t1,t3);
    }
  }
}
/* End of CLooG code */
