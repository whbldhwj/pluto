/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_PE_OPT_H
#define _PSA_PE_OPT_H

#include "pluto.h"
#include "constraints.h"
#include "program.h"
#include "psa_vsa.h"

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

int psa_read_simd_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
);
int psa_read_task_interleave_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
);
int psa_pe_task_interleave_optimize(Band *band, PlutoProg *prog);
bool is_stride_zero_one(Stmt *stmt, PlutoAccess *acc, int depth);
bool psa_loop_is_vectorizable(Ploop *loop, PlutoProg *prog);
int psa_pe_simd_optimize(Band *band, PlutoProg *prog);
int psa_pe_io_eliminate_optimize(Band *band, PlutoProg *prog, VSA *vsa);
void psa_pe_optimize(PlutoProg *prog, VSA *vsa);
#endif