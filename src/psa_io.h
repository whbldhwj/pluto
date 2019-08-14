/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_IO_H
#define _PSA_IO_H

#include "pluto.h"
#include "assert.h"
#include "distmem.h"
#include "program.h"
#include "psa_vsa.h"
#include <cloog/cloog.h>
#include <cloog/clast.h>

//void vsa_var_extract(PlutoProg *prog, VSA *vsa);
//void update_var(struct stmt_access_var_pair **acc_var_map, struct var_pair **adg_var_map, int total_accs, int cc_id, PlutoProg *prog, VSA *vsa);
//void update_ivar(int acc_id, struct stmt_access_var_pair **acc_var_map, struct var_pair **adg_var_map, int cc_id, PlutoProg *prog, VSA *vsa);

void vsa_IO_extract(PlutoProg *prog, VSA *vsa);
int **get_acc_data_trans_sets(struct stmt_access_io_pair **acc_io_map, int num_accs, int *num_sets, int **num_acc_per_set);
bool is_in_same_acc_trans_set(struct stmt_access_io_pair **acc_io_map, const PlutoAccess *acc1, const PlutoAccess *acc2);
bool is_dep_carried_at_space_band(PlutoProg *prog, Dep *dep);
bool is_dep_carried_at_time_band(PlutoProg *prog, Dep *dep);
bool is_dep_carried_at_array_part_band(PlutoProg *prog, Dep *dep);
Vec *get_L1_trans_dir(PlutoProg *prog, Dep *dep, VSA *vsa, struct stmt_access_io_pair *stmt_access_io);
Vec *get_L2_trans_dir(PlutoProg *prog, Dep *dep, VSA *vsa, struct stmt_access_io_pair *stmt_access_io);
Vec *get_L3_trans_dir(PlutoProg *prog, Dep *dep, VSA *vsa, struct stmt_access_io_pair *stmt_access_io);
void acc_io_map_pretty_print(struct stmt_access_io_pair **map, int num_entry);
char *get_trans_bound_str(TransBound bound);
char *get_trans_type_str(TransType type);
/* Vec APIs */
char *get_vec_str(Vec *v);
bool is_vec_equal(const Vec *v1, const Vec *v2);
Vec *vec_alloc(int n);

#endif
