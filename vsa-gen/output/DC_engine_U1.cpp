/**
  * This file is automatically gneerated by PolySA CodeGen.
  * Author: Jie Wang
  */

#include "common_header_U1.h"

void U1_Data2CollectData0(
  U1_Data2TransferChannelType buffer[32][16],
  stream<U1_Data2PEChannelType> &fifo_collect,
  int t1,
  int t2,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t3;
    int t4;
    int t5;
    int t6;
    int t7;
    
    for (t4=2*t2;t4<=2*t2+1;t4++) {
      if (t4 == 2*t2 + engine_id) {
        for (t5=2*t1;t5<=2*t1+1;t5++) {
          for (t6=16*t5;t6<=16*t5+15;t6++) {
            for (t7=16*t4;t7<=16*t4+15;t7++) {
            #pragma HLS PIPELINE II=1
              U1_Data2TransferChannelType fifo_data;
              fifo_data = fifo_collect.read();
              buffer[(t6) - 32*t1][(t7) - 16*t4] = fifo_data;
            }
          }
        }
      }
    }
  }
}

void U1_Data2WriteData0(
  U1_Data2TransferChannelType buffer[32][16],
  stream<U1_Data2TransferChannelType> &fifo_transfer_in,
  stream<U1_Data2TransferChannelType> &fifo_transfer_out,
  int t1,
  int t2,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t3;
    int t4;
    int t5;
    int t6;
    for (t4=2*t2;t4<=2*t2+1;t4++) {
      if (t4 == 2*t2 + engine_id){
        for (t5=32*t1;t5<=32*t1+31;t5++) {
          for (t6=16*t4;t6<=16*t4+15;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data2TransferChannelType fifo_data;
            fifo_data = buffer[(t5) - 32*t1][(t6) - 32*t1];
            fifo_transfer_out.write(fifo_data);
          }
        }
      } else if (t4 < 2*t2 + engine_id){
        for (t5=32*t1;t5<=32*t1+31;t5++) {
          for (t6=16*t4;t6<=16*t4+15;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data2TransferChannelType fifo_data = fifo_transfer_in.read();
            fifo_transfer_out.write(fifo_data);
          }
        }
      }
    }
  }
}

void U1_Data2WriteDataFirst(
  U1_Data2TransferChannelType buffer[32][16],
  stream<U1_Data2TransferChannelType> &fifo_transfer_out,
  int t1,
  int t2,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t3;
    int t4;
    int t5;
    int t6;
    for (t4=2*t2;t4<=2*t2+1;t4++) {
      if (t4 == 2*t2 + engine_id){
        for (t5=32*t1;t5<=32*t1+31;t5++) {
          for (t6=16*t4;t6<=16*t4+15;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data2TransferChannelType fifo_data;
            fifo_data = buffer[(t5) - 32*t1][(t6) - 32*t1];
            fifo_transfer_out.write(fifo_data);
          }
        }
      }
    }
  }
}

void U1_DataCollect2Engine0(
  stream<U1_Data2TransferChannelType> &fifo_transfer_in,
  stream<U1_Data2TransferChannelType> &fifo_transfer_out,
  stream<U1_Data2PEChannelType> &fifo_collect,
  unsigned int engine_id
){
#pragma HLS DATA_PACK variable=fifo_transfer_in
#pragma HLS DATA_PACK variable=fifo_transfer_out
#pragma HLS DATA_PACK variable=fifo_collect
#pragma INLINE off

  // local buffers
  U1_Data2TransferChannelType ping_buf[32][16];
  U1_Data2TransferChannelType pong_buf[32][16];
  #pragma HLS RESOURCE variable=ping_buf core=RAM_2P_BRAM
  #pragma HLS RESOURCE variable=pong_buf core=RAM_2P_BRAM
  #pragma HLS DATA_PACK variable=ping_buf
  #pragma HLS DATA_PACK variable=pong_buf
  
  unsigned int round = 0;
  int t1;
  int t2;
  int t1_prev;
  int t2_prev;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      bool first_tile = ((t1 == 0) && (t2 == 0));
      if (round % 2 == 1){
        U1_Data2WriteData0(pong_buf, fifo_transfer_in, fifo_transfer_out, t1, t2, engine_id, !first_tile);
        U1_Data2CollectData0(ping_buf, fifo_collect, t1_prev, t2_prev, engine_id, 1);
      } else {
        U1_Data2WriteData0(ping_buf, fifo_transfer_in, fifo_transfer_out, t1, t2, engine_id, !first_tile);
        U1_Data2CollectData0(pong_buf, fifo_collect, t1_prev, t2_prev, engine_id, 1);
      }
      round++;
      t1_prev = t1;
      t2_prev = t2;
    }
  }
  if (initial_round % 2 == 1){
    U1_Data2WriteData0(pong_buf, fifo_transfer_in, fifo_transfer_out, t1_prev, t2_prev, engine_id, 1);
  } else {
    U1_Data2WriteData0(ping_buf, fifo_transfer_in, fifo_transfer_out, t1_prev, t2_prev, engine_id, 1);
  }
}

void U1_DataCollect2Engine0_wrapper(
  stream<U1_Data2TransferChannelType> &fifo_transfer_in,
  stream<U1_Data2TransferChannelType> &fifo_transfer_out,
  stream<U1_Data2PEChannelType> &fifo_collect,
  unsigned int engine_id
){
  U1_DataCollect2Engine0(fifo_transfer_in, fifo_transfer_out, fifo_collect, engine_id);
}

void U1_DataCollect2EngineLast(
  stream<U1_Data2TransferChannelType> &fifo_transfer_out,
  stream<U1_Data2PEChannelType> &fifo_collect,
  unsigned int engine_id
){
#pragma HLS DATA_PACK variable=fifo_transfer_out
#pragma HLS DATA_PACK variable=fifo_collect
#pragma INLINE off

  // local buffers
  U1_Data2TransferChannelType ping_buf[32][16];
  U1_Data2TransferChannelType pong_buf[32][16];
  #pragma HLS RESOURCE variable=ping_buf core=RAM_2P_BRAM
  #pragma HLS RESOURCE variable=pong_buf core=RAM_2P_BRAM
  #pragma HLS DATA_PACK variable=ping_buf
  #pragma HLS DATA_PACK variable=pong_buf
  
  unsigned int round = 0;
  int t1;
  int t2;
  int t1_prev;
  int t2_prev;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      bool first_tile = ((t1 == 0) && (t2 == 0));
      if (round % 2 == 1){
        U1_Data2WriteDataLast(pong_buf, fifo_transfer_out, t1, t2, engine_id, !first_tile);
        U1_Data2CollectData0(ping_buf, fifo_collect, t1_prev, t2_prev, engine_id, 1);
      } else {
        U1_Data2WriteDataFirst(ping_buf, fifo_transfer_out, t1, t2, engine_id, !first_tile);
        U1_Data2CollectData0(pong_buf, fifo_collect, t1_prev, t2_prev, engine_id, 1);
      }
      round++;
      t1_prev = t1;
      t2_prev = t2;
    }
  }
  if (initial_round % 2 == 1){
    U1_Data2WriteDataFirst(pong_buf, fifo_transfer_out, t1_prev, t2_prev, engine_id, 1);
  } else {
    U1_Data2WriteDataFirst(ping_buf, fifo_transfer_out, t1_prev, t2_prev, engine_id, 1);
  }
}

