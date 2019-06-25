/**
  * This file is automatically gneerated by PolySA CodeGen.
  * Author: Jie Wang
  */

#include "common_header_U1.h"

void U1_Data0FeedData0(
  U1_Data0TransferChannelType buffer[16][32],
  stream<U1_Data0PEChannelType> &fifo_feed,
  int t1,
  int t2,
  int t3,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t4;
    int t5;
    int t6;
    int t7;
    int t8;
    int t9;
    int t10;
    int t11;
    
    for (t4=2*t1;t4<=2*t1+1;t4++) {
      if (t4 == 2*t1 + engine_id) {
        for (t5=2*t2;t5<=2*t2+1;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
            for (t7=16*t4;t7<=16*t4+15;t7++) {
              for (t8=16*t5;t8<=16*t5+15;t8++) {
              #pragma HLS PIPELINE II=1
                U1_Data0TransferChannelType fifo_data;
                fifo_data = buffer[(t7) - 16*t4][(t6) - 32*t3];
                fifo_feed.write(fifo_data);
              }
            }
          }
        }
      }
    }
  }
}

void U1_Data1FeedData0(
  U1_Data1TransferChannelType buffer[32][16],
  stream<U1_Data1PEChannelType> &fifo_feed,
  int t1,
  int t2,
  int t3,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t4;
    int t5;
    int t6;
    int t7;
    int t8;
    int t9;
    int t10;
    int t11;
    
    for (t4=2*t2;t4<=2*t2+1;t4++) {
      if (t4 == 2*t2 + engine_id) {
        for (t5=2*t1;t5<=2*t1+1;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
            for (t7=16*t5;t7<=16*t5+15;t7++) {
              for (t8=16*t4;t8<=16*t4+15;t8++) {
              #pragma HLS PIPELINE II=1
                U1_Data1TransferChannelType fifo_data;
                fifo_data = buffer[(t6) - 32*t3][(t8) - 16*t4];
                fifo_feed.write(fifo_data);
              }
            }
          }
        }
      }
    }
  }
}

void U1_Data0ReadData0(
  U1_Data0TransferChannelType buffer[16][32],
  stream<U1_Data0TransferChannelType> &fifo_transfer_in,
  stream<U1_Data0TransferChannelType> &fifo_transfer_out,
  int t1,
  int t2,
  int t3,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t4;
    int t5;
    int t6;
    
    for (t4=2*t1;t4<=2*t1+1;t4++) {
      if (t4 == 2*t1 + engine_id){
        for (t5=16*t4;t5<=16*t4+15;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data0TransferChannelType fifo_data = fifo_transfer_in.read();
            buffer[(t5) - 16*t4][(t6) - 32*t3] = fifo_data;
          }
        }
      } else if (t4 > 2*t1 + engine_id){
        for (t5=16*t4;t5<=16*t4+15;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data0TransferChannelType fifo_data = fifo_transfer_in.read();
            fifo_transfer_out.write(fifo_data);
          }
        }
      }
    }
  }
}

void U1_Data0ReadDataLast(
  U1_Data0TransferChannelType buffer[16][32],
  stream<U1_Data0TransferChannelType> &fifo_transfer_in,
  int t1,
  int t2,
  int t3,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t4;
    int t5;
    int t6;
    
    for (t4=2*t1;t4<=2*t1+1;t4++) {
      if (t4 == 2*t1 + engine_id){
        for (t5=16*t4;t5<=16*t4+15;t5++) {
          for (t6=32*t3;t6<=32*t3+31;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data0TransferChannelType fifo_data = fifo_transfer_in.read();
            buffer[(t5) - 16*t4][(t6) - 32*t3] = fifo_data;
          }
        }
      }
    }
  }
}

void U1_Data1ReadData0(
  U1_Data1TransferChannelType buffer[32][16],
  stream<U1_Data1TransferChannelType> &fifo_transfer_in,
  stream<U1_Data1TransferChannelType> &fifo_transfer_out,
  int t1,
  int t2,
  int t3,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t4;
    int t5;
    int t6;
    
    for (t4=2*t2;t4<=2*t2+1;t4++) {
      if (t4 == 2*t2 + engine_id){
        for (t5=32*t3;t5<=32*t3+31;t5++) {
          for (t6=16*t4;t6<=16*t4+15;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data1TransferChannelType fifo_data = fifo_transfer_in.read();
            buffer[(t5) - 32*t3][(t6) - 16*t4] = fifo_data;
          }
        }
      } else if (t4 > 2*t2 + engine_id){
        for (t5=32*t3;t5<=32*t3+31;t5++) {
          for (t6=16*t4;t6<=16*t4+15;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data1TransferChannelType fifo_data = fifo_transfer_in.read();
            fifo_transfer_out.write(fifo_data);
          }
        }
      }
    }
  }
}

void U1_Data1ReadDataLast(
  U1_Data1TransferChannelType buffer[32][16],
  stream<U1_Data1TransferChannelType> &fifo_transfer_in,
  int t1,
  int t2,
  int t3,
  int engine_id,
  bool en,
){
#pragma HLS INLINE off

  if (en) {
    // iterator declarations
    int t4;
    int t5;
    int t6;
    
    for (t4=2*t2;t4<=2*t2+1;t4++) {
      if (t4 == 2*t2 + engine_id){
        for (t5=32*t3;t5<=32*t3+31;t5++) {
          for (t6=16*t4;t6<=16*t4+15;t6++) {
          #pragma HLS PIPELINE II=1
            U1_Data1TransferChannelType fifo_data = fifo_transfer_in.read();
            buffer[(t5) - 32*t3][(t6) - 16*t4] = fifo_data;
          }
        }
      }
    }
  }
}

void U1_DataFeed0Engine0(
  stream<U1_Data0TransferChannelType> &fifo_transfer_in,
  stream<U1_Data0TransferChannelType> &fifo_transfer_out,
  stream<U1_Data0PEChannelType> &fifo_feed,
  unsigned int engine_id
){
#pragma HLS DATA_PACK variable=fifo_transfer_in
#pragma HLS DATA_PACK variable=fifo_transfer_out
#pragma HLS DATA_PACK variable=fifo_feed
#pragma INLINE off

  // local buffers
  U1_Data0TransferChannelType ping_buf[16][32];
  U1_Data0TransferChannelType pong_buf[16][32];
  #pragma HLS RESOURCE variable=ping_buf core=RAM_2P_BRAM
  #pragma HLS RESOURCE variable=pong_buf core=RAM_2P_BRAM
  #pragma HLS DATA_PACK variable=ping_buf
  #pragma HLS DATA_PACK variable=pong_buf
  
  unsigned int round = 0;
  int t1;
  int t2;
  int t3;
  int t1_prev;
  int t2_prev;
  int t3_prev;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      for (t3=0;t3<=31;t3++) {
        bool first_tile = ((t1 == 0) && (t2 == 0) && (t3 == 0));
        if (round % 2 == 1){
          U1_Data0ReadData0(pong_buf, fifo_transfer_in, fifo_transfer_out, t1, t2, t3, engine_id, 1);
          U1_Data0FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        } else {
          U1_Data0ReadData0(ping_buf, fifo_transfer_in, fifo_transfer_out, t1, t2, t3, engine_id, 1);
          U1_Data0FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        }
        round++;
        t1_prev = t1;
        t2_prev = t2;
        t3_prev = t3;
      }
    }
  }
  if (initial_round % 2 == 1){
    U1_Data0FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  } else {
    U1_Data0FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  }
}

void U1_DataFeed0Engine0_wrapper(
  stream<U1_Data0TransferChannelType> &fifo_transfer_in,
  stream<U1_Data0TransferChannelType> &fifo_transfer_out,
  stream<U1_Data0PEChannelType> &fifo_feed,
  unsigned int engine_id
){
  U1_DataFeed0Engine0(fifo_transfer_in, fifo_transfer_out, fifo_feed, engine_id);
}

void U1_DataFeed0EngineLast(
  stream<U1_Data0TransferChannelType> &fifo_transfer_in,
  stream<U1_Data0PEChannelType> &fifo_feed,
  unsigned int engine_id
){
#pragma HLS DATA_PACK variable=fifo_transfer_in
#pragma HLS DATA_PACK variable=fifo_feed
#pragma INLINE off

  // local buffers
  U1_Data0TransferChannelType ping_buf[16][32];
  U1_Data0TransferChannelType pong_buf[16][32];
  #pragma HLS RESOURCE variable=ping_buf core=RAM_2P_BRAM
  #pragma HLS RESOURCE variable=pong_buf core=RAM_2P_BRAM
  #pragma HLS DATA_PACK variable=ping_buf
  #pragma HLS DATA_PACK variable=pong_buf
  
  unsigned int round = 0;
  int t1;
  int t2;
  int t3;
  int t1_prev;
  int t2_prev;
  int t3_prev;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      for (t3=0;t3<=31;t3++) {
        bool first_tile = (t1 == 0) && (t2 == 0) && (t3 == 0);
        if (round % 2 == 1){
          U1_Data0ReadDataLast(pong_buf, fifo_transfer_in, t1, t2, t3, engine_id, 1);
          U1_Data0FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        } else {
          U1_Data0ReadDataLast(ping_buf, fifo_transfer_in, t1, t2, t3, engine_id, 1);
          U1_Data0FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        }
        round++;
        t1_prev = t1;
        t2_prev = t2;
        t3_prev = t3;
      }
    }
  }
  if (initial_round % 2 == 1){
    U1_Data0FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  } else {
    U1_Data0FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  }
}

void U1_DataFeed1Engine0(
  stream<U1_Data1TransferChannelType> &fifo_transfer_in,
  stream<U1_Data1TransferChannelType> &fifo_transfer_out,
  stream<U1_Data1PEChannelType> &fifo_feed,
  unsigned int engine_id
){
#pragma HLS DATA_PACK variable=fifo_transfer_in
#pragma HLS DATA_PACK variable=fifo_transfer_out
#pragma HLS DATA_PACK variable=fifo_feed
#pragma INLINE off

  // local buffers
  U1_Data1TransferChannelType ping_buf[32][16];
  U1_Data1TransferChannelType pong_buf[32][16];
  #pragma HLS RESOURCE variable=ping_buf core=RAM_2P_BRAM
  #pragma HLS RESOURCE variable=pong_buf core=RAM_2P_BRAM
  #pragma HLS DATA_PACK variable=ping_buf
  #pragma HLS DATA_PACK variable=pong_buf
  
  unsigned int round = 0;
  int t1;
  int t2;
  int t3;
  int t1_prev;
  int t2_prev;
  int t3_prev;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      for (t3=0;t3<=31;t3++) {
        bool first_tile = ((t1 == 0) && (t2 == 0) && (t3 == 0));
        if (round % 2 == 1){
          U1_Data1ReadData0(pong_buf, fifo_transfer_in, fifo_transfer_out, t1, t2, t3, engine_id, 1);
          U1_Data1FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        } else {
          U1_Data1ReadData0(ping_buf, fifo_transfer_in, fifo_transfer_out, t1, t2, t3, engine_id, 1);
          U1_Data1FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        }
        round++;
        t1_prev = t1;
        t2_prev = t2;
        t3_prev = t3;
      }
    }
  }
  if (initial_round % 2 == 1){
    U1_Data1FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  } else {
    U1_Data1FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  }
}

void U1_DataFeed1Engine0_wrapper(
  stream<U1_Data1TransferChannelType> &fifo_transfer_in,
  stream<U1_Data1TransferChannelType> &fifo_transfer_out,
  stream<U1_Data1PEChannelType> &fifo_feed,
  unsigned int engine_id
){
  U1_DataFeed1Engine0(fifo_transfer_in, fifo_transfer_out, fifo_feed, engine_id);
}

void U1_DataFeed1EngineLast(
  stream<U1_Data1TransferChannelType> &fifo_transfer_in,
  stream<U1_Data1PEChannelType> &fifo_feed,
  unsigned int engine_id
){
#pragma HLS DATA_PACK variable=fifo_transfer_in
#pragma HLS DATA_PACK variable=fifo_feed
#pragma INLINE off

  // local buffers
  U1_Data1TransferChannelType ping_buf[32][16];
  U1_Data1TransferChannelType pong_buf[32][16];
  #pragma HLS RESOURCE variable=ping_buf core=RAM_2P_BRAM
  #pragma HLS RESOURCE variable=pong_buf core=RAM_2P_BRAM
  #pragma HLS DATA_PACK variable=ping_buf
  #pragma HLS DATA_PACK variable=pong_buf
  
  unsigned int round = 0;
  int t1;
  int t2;
  int t3;
  int t1_prev;
  int t2_prev;
  int t3_prev;
  
  for (t1=0;t1<=31;t1++) {
    for (t2=0;t2<=31;t2++) {
      for (t3=0;t3<=31;t3++) {
        bool first_tile = (t1 == 0) && (t2 == 0) && (t3 == 0);
        if (round % 2 == 1){
          U1_Data1ReadDataLast(pong_buf, fifo_transfer_in, t1, t2, t3, engine_id, 1);
          U1_Data1FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        } else {
          U1_Data1ReadDataLast(ping_buf, fifo_transfer_in, t1, t2, t3, engine_id, 1);
          U1_Data1FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, !first_tile);
        }
        round++;
        t1_prev = t1;
        t2_prev = t2;
        t3_prev = t3;
      }
    }
  }
  if (initial_round % 2 == 1){
    U1_Data1FeedData0(ping_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  } else {
    U1_Data1FeedData0(pong_buf, fifo_feed, t1_prev, t2_prev, t3_prev, engine_id, 1);
  }
}

