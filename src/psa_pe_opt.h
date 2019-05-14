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

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

int psa_read_task_interleave_tile_sizes(
  int *tile_sizes,
  int num_tile_dims
);
int psa_pe_task_interleave_optimize(Band *band, PlutoProg *prog);
int psa_pe_simd_optimize(Band *band, PlutoProg *prog);
int psa_pe_io_eliminate_optimize(Band *band, PlutoProg *prog);
void psa_pe_optimize(PlutoProg *prog);  
#endif