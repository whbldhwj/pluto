#ifndef _DISTMEM_H_
#define _DISTMEM_H_

#include "pluto.h"

void get_data_tag(PlutoProg *prog, PlutoAccess *acc, char **data_tag);
struct stmt_access_pair ***
get_read_write_access_with_stmts(Stmt **stmts, int nstmts, int *num_data,
                                 int **nstmts_per_acc);
struct stmt_access_pair ***get_read_access_with_stmts(Stmt **stmts, int nstmts,
                                                      int *num_data,
                                                      int **nstmts_per_acc);
struct stmt_access_pair ***get_write_access_with_stmts(Stmt **stmts, int nstmts,
                                                       int *num_data,
                                                       int **nstmts_per_acc);
static void pluto_mark_statements(PlutoProg *prog);
char *reconstruct_access(PlutoAccess *acc);
void free_char_array_buffers(char **buffer, int size);
Stmt *get_new_anchor_stmt(Stmt **stmts, int nstmts);
char *pluto_dist_gen_declarations(char *arr_name, PlutoProg *prog);
void pluto_dist_update_arr_domain(struct stmt_access_pair **acc_stmts,
                                  int num_accs, PlutoProg *prog,
                                  int *copy_level, int loop_num);
char *get_data_tile_ref_count_stmt_text(Array *arr, PlutoProg *prog);
Stmt **gen_data_tile_ref_count_update_code(struct stmt_access_pair **acc_stmts,
                                           int num_accs, PlutoProg *prog,
                                           int *copy_level, int loop_num);
char *get_tile_pi_check_ref_count_stmt_text(Array *arr, int src_copy_level,
                                            int loop_num, PlutoProg *prog);
Stmt **gen_write_out_tiles_ref_count_code(struct stmt_access_pair **acc_stmts,
                                          int num_accs, PlutoProg *prog,
                                          int *copy_level, int loop_num);
char *get_data_tile_ref_count_init_stmt_text(Array *arr, int src_copy_level,
                                             int loop_num, PlutoProg *prog);
Stmt **gen_data_tile_ref_count_code(struct stmt_access_pair **acc_stmts,
                                    int num_accs, PlutoProg *prog,
                                    int *copy_level, int loop_num);
Stmt **gen_data_tile_alloc_code(struct stmt_access_pair **acc_stmts,
                                int num_accs, PlutoProg *prog, int *copy_level,
                                int loop_num);
Stmt *gen_comm_data_alloc_code(Stmt *anchor_stmt, PlutoConstraints *constraints,
                               int src_copy_level, char *acc_name,
                               PlutoProg *prog, int loop_num);
char *pluto_dist_copy_back_stmt_text(Array *arr, PlutoProg *prog);
char *pluto_dist_copy_back_func_call_text(int loop_num, int src_copy_level,
                                          Array *arr, PlutoProg *prog);
Stmt **gen_copy_back_code(struct stmt_access_pair **racc_stmts, int num_accs,
                          PlutoProg *prog, int *copy_level, int loop_num);
char *pluto_dist_read_init_text(char *arr_name, PlutoProg *prog);
char *get_tile_pi_check_read_in_text(Array *arr, int src_copy_level,
                                     int loop_num, PlutoProg *prog);
Stmt **gen_read_in_code(PlutoConstraints *read_in,
                        struct stmt_access_pair **acc_stmts, int num_accs,
                        PlutoProg *prog, int *copy_level, int loop_num);
Stmt **gen_read_in_code_old(struct stmt_access_pair **racc_stmts, int num_accs,
                            PlutoProg *prog, int *copy_level, int loop_num);
Stmt **gen_write_out_code(struct stmt_access_pair **wacc_stmts, int num_accs,
                          PlutoProg *prog, Stmt *anchor_stmt, int *copy_level,
                          int outer_dist_loop_level, int loop_num,
                          FILE *headerfp);
void print_data_dist_parm_call(char *str, Array *arr);
void print_data_dist_parm_call_from_access(char *str, PlutoAccess *access,
                                           PlutoProg *prog);
void fprint_data_dist_parm_call(FILE *fp, char *acc_name, PlutoProg *prog);
void add_data_dist_parm_decl(FILE *fp, char *acc_name, PlutoProg *prog);
void generate_pack_or_unpack(FILE *packfp, PlutoProg *prog,
                             PlutoConstraints *constraints, char *stmttext,
                             PlutoStmtType stmttype, char **iters,
                             int src_copy_level, int acc_nrows, char *acc_name,
                             char *send_count);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            
Stmt *gen_comm_code_count_recvs(struct stmt_access_pair ***wacc_stmts,
                                int *num_stmts_per_wacc, int num_data,
                                PlutoProg *prog, Stmt *anchor_stmt,
                                int *copy_level, int loop_num);
Stmt *gen_comm_code_async_recv(struct stmt_access_pair ***wacc_stmts,
                               int *num_stmts_per_wacc, int num_data,
                               PlutoProg *prog, Stmt *anchor_stmt,
                               int *copy_level, int loop_num);
Stmt **gen_tasks_code(Stmt **loop_stmts, int nstmts, int *copy_level,
                      PlutoProg *prog, int num_data, int loop_num, int nloops,
                      int *pi_mappings, char *tasks_loops_decl, FILE *outfp,
                      FILE *headerfp);
void gen_compute_task_cloog_code(PlutoProg *prog, int loop_num,
                                 Stmt **loop_stmts, int nstmts,
                                 int src_copy_level, FILE *outfp,
                                 FILE *headerfp);                                                                                     
void data_tile_prog_add_parm(PlutoProg *data_tile_prog, PlutoProg *prog,
                             int src_copy_level);
void data_tile_domain_remove_copy_level_dims(PlutoConstraints *data_tiles,
                                             int src_copy_level,
                                             PlutoProg *prog);
PlutoMatrix *get_data_tile_trans_func(int src_copy_level, int acc_nrows,
                                      PlutoProg *prog);
char **get_data_tile_iterators(Array *arr, int acc_nrows);
char **get_data_scan_iterators(Array *arr, int acc_nrows);
void gen_func_cloog_code(char *func_name, FILE *headerfp, int loop_num,
                         int src_copy_level, int num_data, PlutoAccess **accs,
                         PlutoProg *data_tile_prog, PlutoProg *prog,
                         int writeout_func);
PlutoConstraints *get_data_tile_alloc_constraints(Array *arr, int loop_num,
                                                  int src_copy_level,
                                                  char *acc_name, int acc_nrows,
                                                  PlutoProg *prog);
void gen_data_tile_alloc_cloog_code(PlutoProg *prog, int loop_num,
                                    Stmt **loop_stmts, int nstmts,
                                    int src_copy_level, FILE *outfp,
                                    FILE *headerfp);
PlutoConstraints *
get_data_tile_ref_count_update_constraints(Array *arr, int loop_num,
                                           int src_copy_level, char *acc_name,
                                           int acc_nrows, PlutoProg *prog);
PlutoConstraints *
get_data_tile_ref_count_init_constraints(Array *arr, int loop_num,
                                         int src_copy_level, char *acc_name,
                                         int acc_nrows, PlutoProg *prog);
void gen_data_tile_ref_count_update_cloog_code(PlutoProg *prog, int loop_num,
                                               Stmt **loop_stmts, int nstmts,
                                               int src_copy_level, FILE *outfp,
                                               FILE *headerfp);
PlutoConstraints *get_read_in_constraints(struct stmt_access_pair **racc_stmts,
                                          int num_accs, int src_copy_level,
                                          int loop_num, PlutoProg *prog);
PlutoConstraints *get_read_in_data_tile_alloc_constraints(
    PlutoConstraints *read_in, Array *arr, int loop_num, int src_copy_level,
    char *acc_name, int acc_nrows, PlutoProg *prog);
void gen_read_in_data_tile_alloc_cloog_code(struct stmt_access_pair **acc_stmts,
                                            int num_accs,
                                            PlutoConstraints *read_in,
                                            PlutoProg *prog, int loop_num,
                                            int src_copy_level,
                                            FILE *headerfp);
PlutoConstraints *get_read_in_copy_constraints(PlutoConstraints *read_in,
                                               Array *arr, int loop_num,
                                               int src_copy_level,
                                               char *acc_name, int acc_nrows,
                                               PlutoProg *prog);
char *get_read_in_copy_stmt_text(Array *arr, PlutoProg *prog);
void gen_read_in_copy_cloog_code(struct stmt_access_pair **acc_stmts,
                                 int num_accs, PlutoConstraints *read_in,
                                 PlutoProg *prog, int loop_num,
                                 int src_copy_level, FILE *headerfp);                                                                                                                                                                                                                                                                                                                                                                                           
void gen_data_tile_ref_count_init_cloog_code(PlutoProg *prog, int loop_num,
                                             Stmt **loop_stmts, int nstmts,
                                             int src_copy_level,
                                             FILE *headerfp);
PlutoConstraints *
get_data_tile_copy_back_constraints(struct stmt_access_pair **wacc_stmts,
                                    int num_accs, int src_copy_level,
                                    PlutoProg *prog);
void gen_data_tile_copy_back_cloog_code(PlutoProg *prog, int loop_num,
                                        struct stmt_access_pair **wacc_stmts,
                                        int num_accs, int src_copy_level,
                                        FILE *headerfp);
void gen_init_tasks_cloog_code(PlutoProg *prog, Stmt ***tasks_stmts, int nloops,
                               char *tasks_loops_decl, FILE *headerfp);
void gen_pack_send_text_code(PlutoProg *prog, Stmt ***copy_comm_stmts,
                             struct stmt_access_pair ***wacc_stmts,
                             int loop_num, int num_data, int num_comm_stmts,
                             int copy_level, FILE *headerfp, Ploop *loops);
void gen_unpack_text_code(PlutoProg *prog, Stmt ***copy_comm_stmts,
                          struct stmt_access_pair ***wacc_stmts, int loop_num,
                          int num_data, int num_comm_stmts, int copy_level,
                          FILE *headerfp);
void gen_write_out_cloog_code(PlutoProg *prog, PlutoProg *write_out_prog,
                              FILE *headerfp);
void gen_dynschedule_graph_main_text_code(PlutoProg *prog, Ploop **loops,
                                          int nloops, int copy_level[nloops],
                                          FILE *outfp);
void gen_dynschedule_main_text_code(PlutoProg *prog, Ploop **loops, int nloops,
                                    int copy_level[nloops], FILE *outfp);
void gen_dynschedule_graph_header_text_code(PlutoProg *prog, int *copy_level,
                                            int nloops, FILE *headerfp);
void gen_dynschedule_header_text_code(int *copy_level, int nloops,
                                      FILE *headerfp);
Stmt **gen_comm_code_opt_fop(int data_id, struct stmt_access_pair **wacc_stmts,
                             int num_accs, int nloops, int num_data,
                             PlutoProg *prog, Stmt *anchor_stmt,
                             int *copy_level, int outer_dist_loop_level,
                             int loop_num, int *pi_mappings,
                             int *num_comm_stmts, FILE *sigmafp,
                             FILE *headerfp);
void free_stmt_array_buffers(Stmt **buffer, int size);
Stmt **gen_comm_code_opt_foifi(int data_id,
                               struct stmt_access_pair **wacc_stmts,
                               int num_accs, int nloops, int num_data,
                               PlutoProg *prog, Stmt *anchor_stmt,
                               int *copy_level, int outer_dist_loop_level,
                               int loop_num, int *pi_mappings,
                               int *num_comm_stmts, FILE *headerfp);
Stmt **gen_comm_code_opt(int data_id, struct stmt_access_pair **wacc_stmts,
                         int num_accs, int nloops, int num_data,
                         PlutoProg *prog, Stmt *anchor_stmt, int *copy_level,
                         int outer_dist_loop_level, int loop_num,
                         int *pi_mappings, int *num_comm_stmts,
                         FILE *headerfp);
void init_copy_level(PlutoProg *prog, Ploop **loops, int nloops,
                     int *copy_level, int *outer_dist_loop_level);
void pluto_dynschedule_common_codegen(PlutoProg *prog, FILE *sigmafp,
                                      FILE *outfp, FILE *headerfp);
int pluto_dynschedule_graph_codegen(PlutoProg *prog, FILE *sigmafp, FILE *outfp,
                                    FILE *headerfp);
int pluto_dynschedule_codegen(PlutoProg *prog, FILE *sigmafp, FILE *outfp,
                              FILE *headerfp);
int pluto_distmem_codegen(PlutoProg *prog, FILE *cloogfp, FILE *sigmafp,
                          FILE *outfp, FILE *headerfp);
void pluto_add_distmem_decls(PlutoProg *prog, FILE *headerfp);
void init_packfile();
void pluto_dist_initalize_array_domains(PlutoProg *prog, int nloops);
void init_pi_mappings(PlutoProg *prog, Ploop **loops, int nloops,
                      int *pi_mappings);
void pluto_dynschedule_common_parallelize(PlutoProg *prog, FILE *sigmafp,
                                          FILE *headerfp, Ploop **loops,
                                          int nloops, int *pi_mappings,
                                          int *copy_level);
int pluto_dynschedule_parallelize(PlutoProg *prog, FILE *sigmafp,
                                  FILE *headerfp, FILE *pifp);
char *get_add_vertex_weight_stmt_text(int phase_level, int **is_scalar_dim,
                                      int loop_num);
Stmt **gen_vertex_wgt_stmt_code(struct stmt_access_pair **acc_stmts,
                                int num_accs, PlutoProg *prog, int *copy_level,
                                int *phase_levels, int **is_scalar_dim,
                                int loop_num);
char *get_add_edge_weight_stmt_text();
Stmt **gen_edge_wgt_stmt_code(struct stmt_access_pair ***acc_stmts,
                              int num_accs, Stmt *anchor_stmt, PlutoProg *prog,
                              int *copy_level, int non_scalar_copy_level,
                              int **is_scalar_dim, int max_copy_level,
                              int loop_num);
void gen_comm_vol_code(struct stmt_access_pair **wacc_stmts, int num_accs,
                       int nloops, PlutoProg *prog, Stmt *anchor_stmt,
                       int *copy_level, int **is_scalar_dim, int loop_num,
                       int *pi_mappings, FILE *headerfp, FILE *outfp);
void gen_edge_wgt_func_code(struct stmt_access_pair **wacc_stmts, int naccs,
                            int *copy_level, int *scalar_dim_copy_level,
                            int **is_scalar_dim, int max_copy_level,
                            PlutoProg *prog, int loop_num, int nloops,
                            int *pi_mappings, FILE *outfp, FILE *headerfp);
void fprint_clear_comm_cost(FILE *outfp, int max_copy_level, int nloops);
void fprint_get_node_func(FILE *outfp, int max_copy_level, int num_loops,
                          int *copy_level);
void fprint_partition_func(FILE *outfp);
void fprint_debug_graph_func(FILE *outfp);
void fprint_update_edges_func(FILE *outfp, int max_copy_level, int nloops);
char *get_total_num_phases(int nloops, Ploop **loops, int *copy_level,
                           int *outer_dist_loop_level, int **is_scalar_dim,
                           PlutoProg *prog);
int *get_num_non_scalar_dims(int nloops, Ploop **loops, int *copy_level,
                             int **is_scalar_dim);
int pluto_dist_gen_compute_pi(int nloops, Ploop **loops, int *copy_level,
                              PlutoProg *prog, int *pi_mappings, FILE *outfp,
                              FILE *headerfp);
int pluto_dist_gen_intial_code(int nloops, Ploop **loops, int *copy_level,
                               PlutoProg *prog, FILE *headerfp);
void gen_copy_back_function(Stmt ****copy_back_stmts, int *num_write_data,
                            int num_copy_back_stmts, int nloops,
                            PlutoProg *prog, FILE *headerfp);
int pluto_sharedmem_data_dist_codegen(PlutoProg *prog, Ploop **loops,
                                      int nloops, int copy_level[nloops],
                                      FILE *outfp);
int pluto_shared_memory_data_dist(PlutoProg *prog, FILE *headerfp,
                                  FILE *outfp);
void pluto_dist_generate_copy_back_code(Ploop **loops, int nloops,
                                        int *copy_level, FILE *headerfp,
                                        PlutoProg *prog);
int pluto_distmem_parallelize(PlutoProg *prog, FILE *sigmafp, FILE *headerfp,
                              FILE *pifp);                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                

static PlutoConstraints *get_context_equality(int copy_level, int src_nrows,
                                              int ncols);                              
#endif