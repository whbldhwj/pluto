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
#include "psa_vsa.h"
#include "psa_knobs.h"

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
int psa_array_partition_optimize(PlutoProg *prog, VSA *vsa);
int psa_array_partition_optimize_band(PlutoProg *prog, Band *band);
int psa_array_partition_tile_band(PlutoProg *prog, Band *band, int *tile_sizes);
void psa_array_partitioning_misc_pretty_print(const PlutoProg *prog, int num_array_part_loops, Ploop **array_part_loops);
int psa_tile_band_constant(PlutoProg *prog, Band *band, int *tile_sizes, 
    PlutoHypType htype, PSAHypType psa_htype);

#endif
