/**
  * This file is automatically gneerated by PolySA CodeGen.
  * Author: Jie Wang
  */

// vendor headers
#include "hls_stream.h"
#include "ap_int.h"
#include "ap_fixed.h"

// common headers
#include <stdio.h>
#include <string.h>

using namespace hls;

#define cal_aligned_size(x, y) ((x + y - 1) / y * y)
// Data types
typedef float U1_data_t0;
typedef ap_uint<512> U1_bus_t0;
#define U1_DATA0_WIDTH 32
#define U1_DATA0_PACK_FACTOR (512/U1_DATA0_WIDTH)
typedef float U1_data_t1;
typedef ap_uint<512> U1_bus_t1;
#define U1_DATA1_WIDTH 32
#define U1_DATA1_PACK_FACTOR (512/U1_DATA1_WIDTH)
typedef float U1_data_t2;
typedef ap_uint<512> U1_bus_t2;
#define U1_DATA2_WIDTH 32
#define U1_DATA2_PACK_FACTOR (512/U1_DATA2_WIDTH)
typedef unsigned int uint;
union ufloat{
  float f;
  unsigned int u;
};

// Macros
#define U1_I 64
#define U1_K 64
#define U1_J 64

#define U1_ROW_IL_FACTOR 16
#define U1_COL_IL_FACTOR 16
#define U1_SA_ROWS 2
#define U1_SA_COLS 2
#define U1_SIMD_FACTOR 1
#define U1_DATA0_FC_SIMD_FACTOR 1
#define U1_DATA0_FC_GROUP_FACTOR 1
#define U1_DATA0_FC_SPLIT_FACTOR 1
#define U1_DATA1_FC_SIMD_FACTOR 1
#define U1_DATA1_FC_GROUP_FACTOR 1
#define U1_DATA1_FC_SPLIT_FACTOR 1
#define U1_DATA2_FC_SIMD_FACTOR 1
#define U1_DATA2_FC_GROUP_FACTOR 1
#define U1_DATA2_FC_SPLIT_FACTOR 1

// Functions and structs
struct U1_Data0TransferChannelType{
  U1_Data0TransferChannelType(){}
  U1_Data0TransferChannelType(
    ap_uint<U1_DATA0_WIDTH*U1_DATA0_FC_SIMD_FACTOR> data_t
  ){
    data = data_t;
  }
  ap_uint<U1_DATA0_WIDTH*U1_DATA0_FC_SIMD_FACTOR> data;
};

struct U1_Data1TransferChannelType{
  U1_Data1TransferChannelType(){}
  U1_Data1TransferChannelType(
    ap_uint<U1_DATA1_WIDTH*U1_DATA1_FC_SIMD_FACTOR> data_t
  ){
    data = data_t;
  }
  ap_uint<U1_DATA1_WIDTH*U1_DATA1_FC_SIMD_FACTOR> data;
};

struct U1_Data2TransferChannelType{
  U1_Data2TransferChannelType(){}
  U1_Data2TransferChannelType(
    ap_uint<U1_DATA2_WIDTH*U1_DATA2_FC_SIMD_FACTOR> data_t){
    data = data_t;
  }
  ap_uint<U1_DATA2_WIDTH*U1_DATA2_FC_SIMD_FACTOR> data;
};

struct U1_Data0PEChannelType{
  U1_Data0PEChannelType(){}
  U1_Data0PEChannelType(
    ap_uint<32> data_t
  ){
    data = data_t;
  }
  ap_uint<32> data;
};

typedef ap_uint<32> U1_Data0SIMDType;

struct U1_Data1PEChannelType{
  U1_Data1PEChannelType(){}
  U1_Data1PEChannelType(
    ap_uint<32> data_t
  ){
    data = data_t;
  }
  ap_uint<32> data;
};

typedef ap_uint<32> U1_Data1SIMDType;

struct U1_Data2PEChannelType{
  U1_Data2PEChannelType(){}
  U1_Data2PEChannelType(
    U1_data_t2 data_t){
    data = data_t;
  }
  U1_data_t2 data;
};

void U1_DataFeed0Head(
  U1_data_t0* A,
  stream<U1_Data0TransferChannelType> &fifo_transfer_out
);

void U1_DataFeed1Head(
  U1_data_t1* B,
  stream<U1_Data1TransferChannelType> &fifo_transfer_out
);

void U1_DataCollect2Head(
  U1_data_t2* C,
  stream<U1_Data2TransferChannelType> &fifo_transfer_in
);

void U1_DataFeed0Engine0_wrapper(
  stream<U1_Data0TransferChannelType> &fifo_transfer_in,
  stream<U1_Data0TransferChannelType> &fifo_transfer_out,
  stream<U1_Data0PEChannelType> &fifo_feed,
  unsigned int engine_id
);

void U1_DataFeed0EngineLast(
  stream<U1_Data0TransferChannelType> &fifo_transfer_in,
  stream<U1_Data0PEChannelType> &fifo_feed,
  unsigned int engine_id
);

void U1_DataFeed1Engine0_wrapper(
  stream<U1_Data1TransferChannelType> &fifo_transfer_in,
  stream<U1_Data1TransferChannelType> &fifo_transfer_out,
  stream<U1_Data1PEChannelType> &fifo_feed,
  unsigned int engine_id
);

void U1_DataFeed1EngineLast(
  stream<U1_Data1TransferChannelType> &fifo_transfer_in,
  stream<U1_Data1PEChannelType> &fifo_feed,
  unsigned int engine_id
);

void U1_DataCollect2Engine0_wrapper(
  stream<U1_Data2TransferChannelType> &fifo_transfer_in,
  stream<U1_Data2TransferChannelType> &fifo_transfer_out,
  stream<U1_Data2PEChannelType> &fifo_collect,
  unsigned int engine_id
);

void U1_DataCollect2EngineLast(
  stream<U1_Data2TransferChannelType> &fifo_transfer_out,
  stream<U1_Data2PEChannelType> &fifo_collect,
  unsigned int engine_id
);

void U1_top_kernel(
  U1_data_t0* A,
  U1_data_t1* B,
  U1_data_t2* C
);

template<typename To, typename From>
inline To Reinterpret(const From& val){
  return reinterpret_cast<const To&>(val);
}

template<class data_t, class bus_t, int WIDTH>
data_t data_select(
  bus_t bus_data,
  uint offset
){
  data_t ret;
  ret = Reinterpret<data_t>((ap_uint<WIDTH>)bus_data(WIDTH-1 + offset*WIDTH, offset*WIDTH));
  return ret;
}
