/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_VSA_DFC_H
#define _PSA_VSA_DFC_H

#include "pluto.h"
#include "assert.h"
#include "distmem.h"
#include "program.h"
#include "psa_array.h"
#include "psa_vsa.h"

void vsa_df_code_extract(PlutoProg *prog, VSA *vsa);
void vsa_dc_code_extract(PlutoProg *prog, VSA *vsa);
Band **psa_get_part_space_bands(PlutoProg *prog, int *nbands);
Ploop **psa_get_intra_tile_dist_loops(Band *band, PlutoProg *prog, int *nloops);
Stmt **psa_gen_write_out_code(
  struct stmt_access_pair **wacc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt, 
  int array_part_level, int space_level, int time_level, 
  int loop_num, VSA *vsa
);
Stmt **psa_gen_read_in_code(
  struct stmt_access_pair **racc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt, 
  int array_part_level, int space_level, int time_level, 
  int loop_num, VSA *vsa
);
void pluto_stmt_loop_reverse(Stmt *stmt, int level, PlutoProg *prog);
void pluto_reverse(PlutoProg *prog, int level);
void pluto_prog_dc_transform(PlutoProg *prog, VSA *vsa, 
                             int *dfc_engine_level, int *dfc_loader_level,
                             struct stmt_access_pair *acc_stmt,
                             int array_part_level, int space_level, int time_level);
void psa_init_level(PlutoProg *prog, Ploop **loops, int nloops,
                    int *array_part_level, int *space_level, int *time_level);
PlutoConstraints *psa_convex_hull(PlutoConstraints *in);
static __isl_give isl_space *set_names(__isl_take isl_space *space,
                                       enum isl_dim_type type, char **names);
void generate_dfc_code(
  VSA *vsa, PlutoProg *prog,
  PlutoConstraints *read_in_write_out, int copy_level,
  char *acc_name, int acc_nrows,
  char *buf_size,
  char *stmt_text,
  char *module_name
);
void pluto_prog_df_transform(PlutoProg *prog, VSA *vsa, 
                             int *dfc_engine_level, int *dfc_loader_level,
                             struct stmt_access_pair *acc_stmt,
                             int array_part_level, int space_level, int time_level);

#endif