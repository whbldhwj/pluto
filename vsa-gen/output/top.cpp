/**
  * This file is automatically gneerated by PolySA CodeGen.
  * Author: Jie Wang
  */

#include "common_header_U1.h"

void U1_top_kernel(
  U1_data_t0 A[I][K],
  U1_data_t1 B[J][K],
  U1_data_t2 C[I][J]
) {
#pragma HLS DATAFLOW
  // fifo declarations
  stream<U1_Data0PEChannelType> fifo0_feed_0_0;
  #pragma HLS STREAM variable=fifo0_feed_0_0 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_0_1;
  #pragma HLS STREAM variable=fifo0_feed_0_1 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_0_2;
  #pragma HLS STREAM variable=fifo0_feed_0_2 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_1_0;
  #pragma HLS STREAM variable=fifo0_feed_1_0 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_1_1;
  #pragma HLS STREAM variable=fifo0_feed_1_1 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_1_2;
  #pragma HLS STREAM variable=fifo0_feed_1_2 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_2_0;
  #pragma HLS STREAM variable=fifo0_feed_2_0 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_2_1;
  #pragma HLS STREAM variable=fifo0_feed_2_1 depth=2
  stream<U1_Data0PEChannelType> fifo0_feed_2_2;
  #pragma HLS STREAM variable=fifo0_feed_2_2 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_0_0;
  #pragma HLS STREAM variable=fifo1_feed_0_0 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_0_1;
  #pragma HLS STREAM variable=fifo1_feed_0_1 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_0_2;
  #pragma HLS STREAM variable=fifo1_feed_0_2 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_1_0;
  #pragma HLS STREAM variable=fifo1_feed_1_0 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_1_1;
  #pragma HLS STREAM variable=fifo1_feed_1_1 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_1_2;
  #pragma HLS STREAM variable=fifo1_feed_1_2 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_2_0;
  #pragma HLS STREAM variable=fifo1_feed_2_0 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_2_1;
  #pragma HLS STREAM variable=fifo1_feed_2_1 depth=2
  stream<U1_Data1PEChannelType> fifo1_feed_2_2;
  #pragma HLS STREAM variable=fifo1_feed_2_2 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_0_0;
  #pragma HLS STREAM variable=fifo2_collect_0_0 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_0_1;
  #pragma HLS STREAM variable=fifo2_collect_0_1 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_0_2;
  #pragma HLS STREAM variable=fifo2_collect_0_2 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_1_0;
  #pragma HLS STREAM variable=fifo2_collect_1_0 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_1_1;
  #pragma HLS STREAM variable=fifo2_collect_1_1 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_1_2;
  #pragma HLS STREAM variable=fifo2_collect_1_2 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_2_0;
  #pragma HLS STREAM variable=fifo2_collect_2_0 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_2_1;
  #pragma HLS STREAM variable=fifo2_collect_2_1 depth=2
  stream<U1_Data2PEChannelType> fifo2_collect_2_2;
  #pragma HLS STREAM variable=fifo2_collect_2_2 depth=2
  stream<U1_Data0TransferChannelType> fifo0_transfer_0;
  #pragma HLS STREAM variable=fifo0_transfer_0 depth=2
  stream<U1_Data0TransferChannelType> fifo0_transfer_1;
  #pragma HLS STREAM variable=fifo0_transfer_1 depth=2
  stream<U1_Data0TransferChannelType> fifo0_transfer_2;
  #pragma HLS STREAM variable=fifo0_transfer_2 depth=2
  stream<U1_Data1TransferChannelType> fifo1_transfer_0;
  #pragma HLS STREAM variable=fifo1_transfer_0 depth=2
  stream<U1_Data1TransferChannelType> fifo1_transfer_1;
  #pragma HLS STREAM variable=fifo1_transfer_1 depth=2
  stream<U1_Data1TransferChannelType> fifo1_transfer_2;
  #pragma HLS STREAM variable=fifo1_transfer_2 depth=2
  stream<U1_Data2TransferChannelType> fifo2_transfer_0;
  #pragma HLS STREAM variable=fifo2_transfer_0 depth=2
  stream<U1_Data2TransferChannelType> fifo2_transfer_1;
  #pragma HLS STREAM variable=fifo2_transfer_1 depth=2
  stream<U1_Data2TransferChannelType> fifo2_transfer_2;
  #pragma HLS STREAM variable=fifo2_transfer_2 depth=2
  stream<U1_Data0PEChannelType> PE_0_0_fifo0_local;
  #pragma HLS STREAM variable=PE_0_0_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_0_0_fifo1_local;
  #pragma HLS STREAM variable=PE_0_0_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_0_0_fifo2_local;
  #pragma HLS STREAM variable=PE_0_0_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_0_1_fifo0_local;
  #pragma HLS STREAM variable=PE_0_1_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_0_1_fifo1_local;
  #pragma HLS STREAM variable=PE_0_1_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_0_1_fifo2_local;
  #pragma HLS STREAM variable=PE_0_1_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_0_2_fifo0_local;
  #pragma HLS STREAM variable=PE_0_2_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_0_2_fifo1_local;
  #pragma HLS STREAM variable=PE_0_2_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_0_2_fifo2_local;
  #pragma HLS STREAM variable=PE_0_2_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_1_0_fifo0_local;
  #pragma HLS STREAM variable=PE_1_0_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_1_0_fifo1_local;
  #pragma HLS STREAM variable=PE_1_0_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_1_0_fifo2_local;
  #pragma HLS STREAM variable=PE_1_0_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_1_1_fifo0_local;
  #pragma HLS STREAM variable=PE_1_1_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_1_1_fifo1_local;
  #pragma HLS STREAM variable=PE_1_1_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_1_1_fifo2_local;
  #pragma HLS STREAM variable=PE_1_1_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_1_2_fifo0_local;
  #pragma HLS STREAM variable=PE_1_2_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_1_2_fifo1_local;
  #pragma HLS STREAM variable=PE_1_2_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_1_2_fifo2_local;
  #pragma HLS STREAM variable=PE_1_2_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_2_0_fifo0_local;
  #pragma HLS STREAM variable=PE_2_0_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_2_0_fifo1_local;
  #pragma HLS STREAM variable=PE_2_0_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_2_0_fifo2_local;
  #pragma HLS STREAM variable=PE_2_0_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_2_1_fifo0_local;
  #pragma HLS STREAM variable=PE_2_1_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_2_1_fifo1_local;
  #pragma HLS STREAM variable=PE_2_1_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_2_1_fifo2_local;
  #pragma HLS STREAM variable=PE_2_1_fifo2_local depth=2
  stream<U1_Data0PEChannelType> PE_2_2_fifo0_local;
  #pragma HLS STREAM variable=PE_2_2_fifo0_local depth=2
  stream<U1_Data1PEChannelType> PE_2_2_fifo1_local;
  #pragma HLS STREAM variable=PE_2_2_fifo1_local depth=2
  stream<U1_Data2PEChannelType> PE_2_2_fifo2_local;
  #pragma HLS STREAM variable=PE_2_2_fifo2_local depth=2
  
  // modules
  DataFeed0Head(
    A,
    fifo0_transfer_0
  );

  %sDataFeed%dEngine0_wrapper(
    fifo0_transfer_0,
    fifo0_transfer_1,
    fifo0_feed_0_0,
    0
  );

  %sDataFeed%dEngineLast(
    fifo0_transfer_1,
    fifo0_feed_1_0,
    1
  );

  DataFeed1Head(
    B,
    fifo1_transfer_0
  );

  %sDataFeed%dEngine0_wrapper(
    fifo1_transfer_0,
    fifo1_transfer_1,
    fifo1_feed_0_0,
    0
  );

  %sDataFeed%dEngineLast(
    fifo1_transfer_1,
    fifo1_feed_0_1,
    1
  );

  // PE modules
  U1_op0_transfer_wrapper(
    fifo0_feed_0_0,
    fifo0_feed_0_1,
    PE_0_0_fifo0_local);

  U1_op1_transfer_wrapper(
    fifo1_feed_0_0,
    fifo1_feed_1_0,
    PE_0_0_fifo1_local);

  U1_compute_wrapper(
    PE_0_0_fifo0_local,
    PE_0_0_fifo1_local,
    PE_0_0_fifo2_local
  );

  U1_res_transfer_first_wrapper(
    PE_0_0_fifo2_local,
    fifo2_collect_0_0,
    0,
    0,
  );

  U1_op0_transfer_last_wrapper(
    fifo0_feed_0_1,
    PE_0_1_fifo0_local);

  U1_op1_transfer_wrapper(
    fifo1_feed_0_1,
    fifo1_feed_1_1,
    PE_0_1_fifo1_local);

  U1_compute_wrapper(
    PE_0_1_fifo0_local,
    PE_0_1_fifo1_local,
    PE_0_1_fifo2_local
  );

  U1_res_transfer_first_wrapper(
    PE_0_1_fifo2_local,
    fifo2_collect_0_1,
    0,
    1,
  );

  U1_op0_transfer_wrapper(
    fifo0_feed_1_0,
    fifo0_feed_1_1,
    PE_1_0_fifo0_local);

  U1_op1_transfer_last_wrapper(
    fifo1_feed_1_0,
    PE_1_0_fifo1_local);

  U1_compute_wrapper(
    PE_1_0_fifo0_local,
    PE_1_0_fifo1_local,
    PE_1_0_fifo2_local
  );

  U1_res_transfer_wrapper(
    PE_1_0_fifo2_local,
    fifo2_collect_0_0,
    fifo2_collect_1_0,
    1,
    0
  );

  U1_op0_transfer_last_wrapper(
    fifo0_feed_1_1,
    PE_1_1_fifo0_local);

  U1_op1_transfer_last_wrapper(
    fifo1_feed_1_1,
    PE_1_1_fifo1_local);

  U1_compute_wrapper(
    PE_1_1_fifo0_local,
    PE_1_1_fifo1_local,
    PE_1_1_fifo2_local
  );

  U1_res_transfer_wrapper(
    PE_1_1_fifo2_local,
    fifo2_collect_0_1,
    fifo2_collect_1_1,
    1,
    1
  );

  %sDataCollect%dEngineLast(
    fifo2_transfer_0,
    fifo2_collect_1_0,
    1
  );
  U1_DataCollect2Engine0_wrapper(
    fifo2_transfer_0,
    fifo2_transfer_1,
    fifo2_collect_1_1,
    0
  );

  U1_DataCollect2Head(
    C,
    fifo2_transfer_1
  );

}
