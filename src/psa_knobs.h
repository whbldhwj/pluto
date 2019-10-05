/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_KNOB_H
#define _PSA_KNOB_H

//#define DEMO0
#define DEMO1

/*
 * *****************************
 * Stage: Array Generation
 * *****************************
 */
// print out detailed information of each dependence before the Pluto transformation
#define PRINT_DEPS_PREV_TRANSFORM

// print out detailed information of each dependence after the Pluto transformation
//#define PRINT_DEPS_POST_TRANSFORM

// perform pluto's algorithm
#define PLUTO_TRANSFORM

// print out the transformed code after Pluto's auto-transformation
#define PRINT_PLUTO_TRANS_PROGRAM

// check the legality of space-time transformation
#define SPACE_TIME_CHECK

// check the uniformity of the application
#define UNIFORMITY_CHECK

// perform space-time remapping
#define SPACE_TIME_MAPPING

// sync or async
#ifdef DEMO1
  #define SYNC_ARRAY
//  #define SYNC_ARRAY_WAVEFRONT
#endif
#ifdef DEMO0
  #define ASYNC_ARRAY
#endif

// upper bound of permissible systolic array dimension
#define SA_DIM_UB 2

// pick up the optimal array variant based on heuristics
#define SA_SMART_PICK

// print out the transformed code after smart pick
#define PRINT_SA_SMART_PICK_PROGRAM

/*
 * *****************************
 * Stage: PE Optimization
 * *****************************
 */ 
#ifdef DEMO0
  // perform latency hiding optimization 
  #define LATENCY_HIDING
  
  // print out the transformed code after latency hiding
  #define PRINT_LATENCY_HIDING_TRANS_PROGRAM
  
  // dump out latency hiding candidates
  #define PRINT_LATENCY_HIDING_MISC
  
  // perfrom SIMD vectorization
  #define SIMD_VECTORIZATION
  
  // print out the transformed code after SIMD vectorization
  #define PRINT_SIMD_VECTORIZATION_TRANS_PROGRAM
  
  // dump out SIMD vectorization candidates
  #define PRINT_SIMD_VECTORIZATION_MISC

  // perfom array partitioning optimization
  #define ARRAY_PARTITIONING
  
  // print out the transformed code after array partitioning
  #define PRINT_ARRAY_PARTITIONING_TRANS_PROGRAM
  
  // dump out array partitioning candidates
  #define PRINT_ARRAY_PARTITIONING_MISC 
#endif

// print out detailed information of each dependence after the PE optimization
#define PRINT_DEPS_POST_PE_OPTIMIZATION

/*
 * ************************************
 * Stage: Data Transfor Optimization
 * ************************************
 */ 

/*
 * ************************
 * Stage: Code Generation
 * ************************
 */
// dump out virtual systolic array
#define DUMP_VSA
// print out the final transformed CPU code
#define CPU_CODEGEN
// generate T2S code
#ifdef DEMO1
  #define T2S_CODEGEN
#endif
// generate Intel OpenCL code
//#define INTEL_CODEGEN
// generate Xilinx HLS code
//#define XILINX_CODEGEN

#endif
