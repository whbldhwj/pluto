#define S1(i,j) C[i][j] = 0;
#define S2(i,j,k) C[i][j] += (A[i][k] * B[k][j]);
#define S3(d1,d2,d3,d4) regB_1 = read_channel_intel(fifoB_3_PE[row][col]);
#define S4(d1,d2,d3,d4) regA_3 = read_channel_intel(fifoA_4_PE[row][col]);
#define S5(d1,d2,d3,d4) write_channel_intel(fifoB_3_PE[row + 1][col], regB_1);
#define S6(d1,d2,d3,d4) write_channel_intel(fifoA_4_PE[row][col + 1], regA_3);

__attribute__((max_global_work_dim(0)))
__attribute__((autorun))
__attribtue__((num_compute_units(SYS_ARRAY_NUM_ROWS, SYS_ARRAY_NUM_COLS)))
__kernel void PE_kernel()
{
  const int row = get_compute_id(0);
  const int col = get_compute_id(1);
  int t1, t2, t3, t4, t5;

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

}
