/**
  * This file is automatically gneerated by PolySA CodeGen.
  * Author: Jie Wang
  */

#include "common_header_U1.h"

void U1_DataFeed0Head(
  U1_data_t0 *A,
  stream<U1_Data0TransferChannelType> &fifo_transfer_out
){
#pragma HLS INLINE off
#pragma HLS DATA_PACK variable=fifo_transfer_out

  // local buffers
  U1_data_t0 A_buf[32][32];

  // iterators
  int t1, t2, t3, t4, t5, t6, t7;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      for (t3=0;t3<=31;t3++) {
        for (t5=32*t1;t5<=32*t1+31;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
            A_buf[(t5) - 32*t1][(t6) - 32*t3] = A[(t5)][(t6)];
          }
        }
        for (t5=2*t1;t5<=2*t1+1;t5++) {
          for (t6=16*t5;t6<=16*t5+15;t6++) {
            for (t7=32*t3;t7<=32*t3+31;t7++) {
              U1_Data0TransferChannelType fifo_data;
              fifo_data.data = A_buf[(t6) - 32*t1][(t7) - 32*t3];
              fifo_transfer_out.write(fifo_data);
            }
          }
        }
      }
    }
  }
}

void U1_DataFeed1Head(
  U1_data_t1 *B,
  stream<U1_Data1TransferChannelType> &fifo_transfer_out
){
#pragma HLS INLINE off
#pragma HLS DATA_PACK variable=fifo_transfer_out

  // local buffers
  U1_data_t1 B_buf[32][32];

  // iterators
  int t1, t2, t3, t4, t5, t6, t7;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      for (t3=0;t3<=31;t3++) {
        for (t5=32*t3;t5<=32*t3+31;t5++) {
          for (t6=32*t2;t6<=32*t2+31;t6++) {
            B_buf[(t5) - 32*t3][(t6) - 32*t2] = B[(t5)][(t6)];
          }
        }
        for (t5=2*t2;t5<=2*t2+1;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
            for (t7=16*t5;t7<=16*t5+15;t7++) {
              U1_Data1TransferChannelType fifo_data;
              fifo_data.data = B_buf[(t6) - 32*t3][(t7) - 32*t2];
              fifo_transfer_out.write(fifo_data);
            }
          }
        }
      }
    }
  }
}

