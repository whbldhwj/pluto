/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_DEP_H
#define _PSA_DEP_H

#include "pluto.h"
#include "constraints.h"

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

#include "distmem.h"

bool systolic_array_legal_checker(PlutoProg *prog);
bool systolic_array_dep_checker(PlutoProg *prog);
bool systolic_array_dep_checker_isl(PlutoProg *prog);
bool is_dep_constant_at_level(Dep *dep, PlutoProg *prog, int level);
void rar_scalar_filter(PlutoProg *prog);
void rar_filter(PlutoProg *prog);
PlutoProg **psa_reuse_analysis(PlutoProg *prog, int *num_reuse_progs);
PlutoConstraints *construct_rar_dep_polytope(PlutoMatrix *null_space, int idx, Stmt *stmt);
void construct_rar_dep(Dep **deps, PlutoMatrix *null_space, PlutoAccess *acc, Stmt *stmt, PlutoProg *prog);
void reassociate_dep_stmt_acc(PlutoProg *prog);

#endif
