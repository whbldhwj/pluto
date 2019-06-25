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

struct codeBlock {
  /* Number of lines */
  int loc;

  /* Code content */
  char **content;
};
typedef struct codeBlock CodeBlock;

struct arrCodeBlock {
  /* Array Name */
  char *arr_name;

  /* Code Content */
  CodeBlock *code;
};
typedef struct arrCodeBlock ArrCodeBlock;

struct iter {
  /* Iterator name */
  char *iter_name;

  /* Iterator bound */
  char *lb;
  char *ub;

  /* Tilable */
  bool tilable;

  /* Tile size */
  int tile_factor;
};
typedef struct iter Iter;

struct psaVSA {
  /* DF_FEED_COUNTER_CODE */
  ArrCodeBlock **df_feed_counter_code;

  /* DF_FEED_ADDR_CAL_CODE */
  ArrCodeBlock **df_feed_addr_cal_code;

  /* DC_COLLECT_COUNTER_CODE */
  ArrCodeBlock **dc_collect_counter_code;

  /* DC_COLLECT_ADDR_CAL_CODE */
  ArrCodeBlock **dc_collect_addr_cal_code;

  /* ARRAY_SIZE */
  int *array_sizes;

  /* DFC_HEAD_BUF_SIZE */
  int *dfc_head_buf_sizes;

  /* OP_NAME */
  char **op_names;

  /* RES_NAME */
  char **res_names;

  /* OP_INTERIOR_IO_ELIMINATE */
  bool *op_io_enable;

  /* RES_INTERIOR_IO_ELIMINATE */
  bool *res_io_enable;

  /* ROW_IL_FACTOR */
  int row_il_factor;

  /* COL_IL_FACTOR */
  int col_il_factor;

  /* GLOBAL_ACCUM_NUM */
  int global_accum_num; // TODO: to be deprecated

  /* LOCAL_ACCUM_NUM */
  int local_accum_num;  // TODO: to be deprecated

  /* LOCAL_REG_NUM */
  int local_reg_num;    // TODO: to be deprecated

  /* OP_ENGINE_NUM */
  int *op_engine_nums;

  /* RES_ENGINE_NUM */
  int *res_engine_nums; 

  /* SA_ROWS */
  int sa_rows;

  /* SA_COLS */
  int sa_cols;

  /* FC_SPLIT_FACTOR */
  int *fc_split_factors; // TODO: to be implemented in the future

  /* FC_GROUP_FACTOR */
  int *fc_group_factors; // TODO: to be implemented in the future

  /* FC_SIMD_FACTOR */
  int *fc_simd_factors; // TODO: to be implemented in the future

  /* ITERATORS */
  Iter **iters;

  /* IL_ENABLE */
  bool il_enable;

  /* SIMD_FACTOR */
  int simd_factor;

  /* OP_NUM */
  int op_num;

  /* RES_NUM */
  int res_num;

  /* OP_DIM */
  int *op_dims;

  /* RES_DIM */
  int *res_dims;

  /* OP_CHANNEL_DIR */
  char **op_channel_dirs;

  /* RES_CHANNEL_DIR */
  char **res_channel_dirs;

  /* DFC_BUF_SIZE */
  int *dfc_buf_sizes;

  /* HEAD_Code */
  ArrCodeBlock **head_code; 

  /* MAC_STAT */
  char *mac_stat; // To be deprecated

  /* LAST_PATCH_CODE */
  CodeBlock *last_patch_code;

  /* LAST_TILE_SIZE */
  CodeBlock *last_tile_size;

  /* TYPE */
  char *type;

  /* ARRAY_PART, SPACE LOOP NUM */
  int array_part_band_width;
  int space_band_width;

  /* ENGINE LOOP NUM */
  //int *op_engine_band_width;
  //int *res_engine_band_width;
  int *engine_band_width;
  
};
typedef struct psaVSA VSA;

void vsa_op_res_extract(PlutoProg *prog, VSA *vsa);
void vsa_channel_dir_extract(PlutoProg *prog, VSA *vsa);
void vsa_engine_num_extract(PlutoProg *prog, VSA *vsa);
void vsa_type_extract(PlutoProg *prog, VSA *vsa);        
void vsa_band_width_extract(PlutoProg *prog, VSA *vsa);
void pluto_prog_to_vsa(PlutoProg *prog, VSA *vsa);
VSA *vsa_alloc();
void vsa_free(VSA *vsa);
void psa_print_string_with_indent(FILE *fp, int indent, char *to_print);
void psa_print_int_with_indent(FILE *fp, int indent, int to_print);
void psa_print_string_list_with_indent(FILE *fp, int indent, char **list, int len);
void psa_print_int_list_with_indent(FILE *fp, int indent, int *list, int len);
void psa_vsa_pretty_print(FILE *fp, const VSA *vsa);

#endif
