/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_VSA_H
#define _PSA_VSA_H

#include "pluto.h"
#include "assert.h"
#include "distmem.h"
#include "program.h"
#include "psa_ure.h"
#include <cloog/cloog.h>
#include <cloog/clast.h>

void vsa_op_res_extract(PlutoProg *prog, VSA *vsa);
void vsa_channel_dir_extract(PlutoProg *prog, VSA *vsa);
void vsa_engine_num_extract(PlutoProg *prog, VSA *vsa);
void vsa_type_extract(PlutoProg *prog, VSA *vsa);        
void vsa_band_width_extract(PlutoProg *prog, VSA *vsa);
void vsa_iter_extract(PlutoProg *prog, VSA *vsa);
void vsa_array_shape_extract(PlutoProg *prog, VSA *vsa);
void pluto_prog_to_vsa(PlutoProg *prog, VSA *vsa);
VSA *vsa_alloc();
void vsa_free(VSA *vsa);
void psa_print_string_with_indent(FILE *fp, int indent, char *to_print);
void psa_print_int_with_indent(FILE *fp, int indent, int to_print);
void psa_print_string_list_with_indent(FILE *fp, int indent, char **list, int len);
void psa_print_int_list_with_indent(FILE *fp, int indent, int *list, int len);
void psa_vsa_pretty_print(FILE *fp, const VSA *vsa);
/* T2S Added */
void vsa_array_extract(PlutoProg *prog, VSA *vsa);
void vsa_t2s_var_extract(PlutoProg *prog, VSA *vsa);
void psa_t2s_codegen(FILE *fp, const VSA *vsa);
char **t2s_space_time_pprint(VSA *vsa, int *line_num);
char **get_vsa_array_names(Array **arrays, int array_num);
char **get_vsa_URE_texts(URE **UREs, int URE_num);
char **get_vsa_URE_names(URE **UREs, int URE_num);
void vsa_URE_extract(PlutoProg *prog, VSA *vsa);
void vsa_t2s_iter_extract(PlutoProg *prog, VSA *vsa);
void get_var_iters(int acc_id, struct stmt_access_var_pair **acc_var_map, PlutoProg *prog, VSA *vsa);
void update_t2s_ivar(int acc_id, struct stmt_access_var_pair **acc_var_map, struct var_pair **adg_var_map, int cc_id, PlutoProg *prog, VSA *vsa);
char *get_iter_str(IterExp **iters, int iter_num);
void update_t2s_var(struct stmt_access_var_pair **acc_var_map, struct var_pair **adg_var_map, int total_accs, int cc_id, PlutoProg *prog, VSA *vsa);
void vsa_t2s_IO_extract(PlutoProg *prog, VSA *vsa);
#endif
