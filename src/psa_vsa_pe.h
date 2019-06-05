/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_VSA_PE_H
#define _PSA_VSA_PE_H

#include "pluto.h"
#include "psa_vsa.h"
#include "assert.h"
#include "distmem.h"
#include "program.h"

void vsa_pe_code_extract(PlutoProg *prog, VSA *vsa);
Stmt **psa_gen_op_read_in_code(
  struct stmt_access_pair **racc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt,
  int array_part_level, int space_level, int time_level,
  int loop_num, VSA *vsa
);
Stmt **psa_gen_res_write_out_code(
  struct stmt_access_pair **wacc_stmts, int num_accs,
  PlutoProg *prog, Stmt *anchor_stmt,
  int array_part_level, int space_level, int time_level,
  int loop_num, VSA *vsa
);
Stmt **psa_gen_compute_code(
  // Stmt ***op_trans_stmts, Stmt ***res_trans_stmts,
  struct stmt_access_pair ***racc_stmts, int *num_raccs, int num_read_data,
  struct stmt_access_pair ***wacc_stmts, int *num_waccs, int num_write_data,
  PlutoProg *prog, Stmt *anchor_stmt,
  int array_part_level, int space_level, int time_level,
  int loop_num, VSA *vsa
);

#endif