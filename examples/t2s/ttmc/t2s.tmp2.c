/* Start of CLooG code */
for (t2=1;t2<=8;t2++) {
  for (t3=1;t3<=8;t3++) {
    for (t4=1;t4<=8;t4++) {
      S1(t2,t4,t3);
    }
  }
}
for (t1=8;t1<=64;t1++) {
  for (t2=1;t2<=8;t2++) {
    for (t3=1;t3<=8;t3++) {
      for (t4=1;t4<=8;t4++) {
        for (t5=max(1,ceild(t1-8,7));t5<=min(8,floord(t1-1,7));t5++) {
          S2(t2,t4,t3,t5,(t1-7*t5));
        }
      }
    }
  }
}
/* End of CLooG code */
