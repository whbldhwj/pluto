/* Start of CLooG code */
for (t1=0;t1<=63;t1++) {
  for (t2=0;t2<=63;t2++) {
    S1(t1,t2);
    for (t3=0;t3<=63;t3++) {
      S3(t1,t2,t3,1);
      S4(t1,t2,t3,1);
      S2(t1,t2,t3);
      S5(t1,t2,t3,1);
      S6(t1,t2,t3,1);
    }
  }
}
/* End of CLooG code */
