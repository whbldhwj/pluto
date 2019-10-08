/* Start of CLooG code */
for (t1=1;t1<=8;t1++) {
  for (t2=1;t2<=8;t2++) {
    for (t4=1;t4<=8;t4++) {
      S1(t2,t4,t1);
    }
    for (t3=8;t3<=64;t3++) {
      for (t4=1;t4<=8;t4++) {
        for (t5=max(1,ceild(t3-8,7));t5<=min(8,floord(t3-1,7));t5++) {
          S2(t2,t4,t1,t5,(t3-7*t5));
        }
      }
    }
  }
}
/* End of CLooG code */
