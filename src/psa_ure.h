/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_URE_H
#define _PSA_URE_H

#include "pluto.h"
#include "psa_vsa.h"
#include <cloog/int.h>

#define URE_MERGE

int t2s_compare_stmt_order(Stmt *stmt1, Stmt *stmt2, int band_width);
//void create_RAR_UREs(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa);
PlutoConstraints *get_RAR_domain(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa); 
void create_RAR_UREs(int cc_id, PlutoProg *prog, VSA *vsa);
void create_drain_UREs(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa);
void create_collect_UREs(int cc_id, PlutoProg *prog, VSA *vsa);
char *create_stmt_domain_str(Stmt *stmt, PlutoProg *prog, VSA *vsa);
void stmt_to_UREs(Stmt *stmt, PlutoProg *prog, VSA *vsa);
void stmt_to_UREs_new(Stmt *stmt, PlutoProg *prog, VSA *vsa);
void stmt_to_URE_single(char *domain_str, Stmt *stmt, PlutoProg *prog, VSA *vsa);

char *pluto_constraints_to_t2s_format(const PlutoConstraints *cst, VSA *vsa, int niter, int nparam, char **params);
URE **URE_append(URE **list1, int *num1, URE **list2, int num2);
URE **URE_add(URE **list, int *num, URE *ele);
void URE_init(URE *ure);
void URE_free(URE *ure);
PlutoConstraints *get_anchor_domain(PlutoProg *prog);
char *create_URE_name(char **URE_names, int URE_num, char *var_name);
char *create_URE_text(URE *ure);
char *create_new_acc_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa);
char *create_orig_acc_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa);
char *create_new_acc_ref_str(Stmt *stmt, PlutoAccess *acc, PlutoProg *prog, VSA *vsa);
void vsa_t2s_meta_iter_extract(PlutoProg *prog, VSA *vsa);

void iter_parse_stmt_list(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_stmt *s);
void iter_parse_assignment(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_assignment *a);
void iter_parse_user_stmt(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_user_stmt *a);
void iter_parse_for(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_for *f);
void iter_parse_guard(struct cloogoptions *options, Iter **iters, int iter_num, int *iter_cnt, struct clast_guard *g);

char *clast_expr_to_str(struct cloogoptions *i, struct clast_expr *); 
char *clast_name_to_str(struct clast_name *n);
char *clast_term_to_str(struct cloogoptions *i, struct clast_term *t); 
char *clast_reduction_to_str(struct cloogoptions *i, struct clast_reduction *r);
char *clast_sum_to_str(struct cloogoptions *opt, struct clast_reduction *r);
char *clast_minmax_f_to_str(struct cloogoptions *info, struct clast_reduction *r); 
char *clast_minmax_c_to_str(struct cloogoptions *info, struct clast_reduction *r);
char *clast_binary_to_str(struct cloogoptions *i, struct clast_binary *b);
char *clast_int_to_str(cloog_int_t t); 
#endif
