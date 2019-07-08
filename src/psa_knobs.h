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
 * Stage: Array Generation
 */
// print out detailed information of each dependence before the Pluto transformation
#define PRINT_DEPS_PREV_TRANSFORM
// print out detailed information of each dependence after the Pluto transformation
//#define PRINT_DEPS_POST_TRANSFORM
// print out the transformed code after Pluto's auto-transformation
#define PRINT_PLUTO_TRANS_PROGRAM
// perform space-time remapping
#define SPACE_TIME_MAPPING

/*
 * Stage: PE Optimization
 */ 
// 
#define LATENCY_HIDING
#define ARRAY_PARTITIONING

/*
 * Stage: Data Transfor Optimization
 */ 

/*
 * Stage: Code Generation
 */

#endif
