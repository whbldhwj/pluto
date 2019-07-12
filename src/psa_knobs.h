/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_KNOB_H
#define _PSA_KNOB_H

#include "pluto.h"
#include "constraints.h"

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

/*
 * *****************************
 * Stage: Array Generation
 * *****************************
 */
// print out detailed information of each dependence before the Pluto transformation
#define PRINT_DEPS_PREV_TRANSFORM

// print out detailed information of each dependence after the Pluto transformation
//#define PRINT_DEPS_POST_TRANSFORM

// print out the transformed code after Pluto's auto-transformation
#define PRINT_PLUTO_TRANS_PROGRAM

// check the uniformity of the application
#define UNIFORMITY_CHECK

// perform space-time remapping
#define SPACE_TIME_MAPPING

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
// perform latency hiding optimization 
#define LATENCY_HIDING

// print out the transformed code after latency hiding
#define PRINT_LATENCY_HIDING_TRANS_PROGRAM

// dump out latency hiding candidates
#define PRINT_LATENCY_HIDING_MISC

//// perfom array partitioning optimization
//#define ARRAY_PARTITIONING

//// dump out array partitioning candidates
//#define PRINT_ARRAY_PARTITIONING_MISC

// perfrom SIMD vectorization
#define SIMD_VECTORIZATION

// print out the transformed code after SIMD vectorization
#define PRINT_SIMD_VECTORIZATION_TRANS_PROGRAM

// dump out SIMD vectorization candidates
#define PRINT_SIMD_VECTORIZATION_MISC

//// dump out SIMD candidates
//#define PRINT_SIMD_VECTORIZATION_MISC

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
// print out the final transformed code
#define PRINT_FINAL_PROGRAM

#endif
