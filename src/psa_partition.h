/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_PARTITION_H
#define _PSA_PARTITION_H

#include "pluto.h"
#include "constraints.h"
#include "program.h"

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

int psa_read_tile_sizes(
  int *tile_sizes,
  int num_tile_dims, Stmt **stmts, int nstmts, int first_loop
);
void psa_tile_band(PlutoProg *prog, Band *band, int *tile_sizes);
void psa_tile_outermost_permutable_band(PlutoProg *prog, Band *band);
void psa_permute_outermost_tile_band(PlutoProg *prog, Band *band);
void psa_array_partition(PlutoProg *prog);

#endif