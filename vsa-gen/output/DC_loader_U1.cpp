/**
  * This file is automatically gneerated by PolySA CodeGen.
  * Author: Jie Wang
  */

#include "common_header_U1.h"

void U1_DataCollect2Head(
  U1_data_t2 *C,
  stream<U1_Data2TransferChannelType> &fifo_transfer_in
){
#pragma HLS INLINE off
#pragma HLS DATA_PACK variable=fifo_transfer_in

  // local buffers
  U1_data_t2 C_buf[32][32];

  // iterators
  int t1, t2, t3, t4, t5, t6, t7;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      for (t5=2*t2;t5<=2*t2+1;t5++) {
        for (t6=32*t1;t6<=32*t1+31;t6++) {
          for (t7=16*t5;t7<=16*t5+15;t7++) {
            U1_Data2TransferChannelType fifo_data = fifo_transfer_in.read();
            C_buf[(t6) - 32*t1][(t7) - 32*t2] = fifo_data.data;
          }
        }
      }
      for (t5=32*t1;t5<=32*t1+31;t5++) {
        for (t6=32*t2;t6<=32*t2+31;t6++) {
          C[(t5)][(t6)] = C_buf[(t5) - 32*t1][(t6) - 32*t2];
        }
      }
    }
  }
}

