/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_vsa.h"

/*
 * This function analyzes the read/write access info from the program and extracts
 * information about operands and results, including:
 * - op_names, res_names
 * - op_num, res_num
 * - op_dims, res_dims
 */
void vsa_op_res_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;
  char **read_names = NULL;
  int *read_dims = NULL;
  int num_read_names = 0;
  char **write_names = NULL;
  int *write_dims = NULL;
  int num_write_names = 0;

  /* Add all read and write names from the program */
  for (i = 0; i < prog->nstmts; i++) {
    Stmt *stmt = prog->stmts[i];
    for (j = 0; j < stmt->nreads; j++) {
      PlutoAccess *read_acc = stmt->reads[j];
      if (read_acc->mat->nrows == 1) {
        int col;
        for (col = 0; col < read_acc->mat->ncols; col++) {
          if (read_acc->mat->val[0][col] != 0)
            break;          
        }
        if (col == read_acc->mat->ncols) {
          /* scalar */
          continue;
        }
      } 
      char *name = read_acc->name;
      int n;
      /* Check if the name has already been added. */
      for (n = 0; n < num_read_names; n++) {
        if (!strcmp(read_names[n], name))
          break;
      }
      if (n == num_read_names) {             
        num_read_names++;
        read_names = (char **)realloc(read_names, num_read_names * sizeof(char *));
        read_names[num_read_names - 1] = strdup(name);            
        read_dims = (int *)realloc(read_dims, num_read_names * sizeof(int));
        read_dims[num_read_names - 1] = read_acc->mat->nrows;
      }            
    }  

    for (j = 0; j < stmt->nwrites; j++) {
      PlutoAccess *write_acc = stmt->writes[j];
      char *name = write_acc->name;
      int n;
      /* Check if the name has already been added. */
      for (n = 0; n < num_write_names; n++) {
        if (!strcmp(write_names[n], name))
          break;          
      }
      if (n == num_write_names) {
        num_write_names++;
        write_names = (char **)realloc(write_names, num_write_names * sizeof(char *));
        write_names[num_write_names - 1] = strdup(name);
        write_dims = (int *)realloc(write_dims, num_write_names * sizeof(int));
        write_dims[num_write_names - 1] = write_acc->mat->nrows;
      }
    }
  }

  /* Operand array will only read accesses,
   * the rest wiil be the result array.
   * NOTE: Currently we assume each array comes with only one unique reference.
   */
  char **op_names = NULL;
  char **res_names = NULL;
  int op_num = 0;
  int res_num = 0;
  int *op_dims = NULL;
  int *res_dims = NULL;
  
  for (i = 0; i < num_read_names; i++) {
    char *r_name = read_names[i];
    for (j = 0; j < num_write_names; j++) {
      char *w_name = write_names[j];
      if (!strcmp(r_name, w_name)) 
        break;      
    }
    if (j == num_write_names) {
      /* check if the name already exists in op_names */
      int n;
      for (n = 0; n < op_num; n++) {
        if (!strcmp(r_name, op_names[n]))
          break;
      }
      if (n == op_num) {
        op_num++;
        op_names = (char **)realloc(op_names, op_num * sizeof(char *));
        op_names[op_num - 1] = strdup(r_name);
        op_dims = (int *)realloc(op_dims, op_num * sizeof(int));
        op_dims[op_num - 1] = read_dims[i];
      }
    }
  }

  /* All the rest array names that are not op_names will be inserted into res_names */
  for (i = 0; i < num_read_names; i++) {
    char *r_name = read_names[i];
    for (j = 0; j < op_num; j++) {
      if (!strcmp(r_name, op_names[j])) {
        break;
      }
    }
    if (j == op_num) {
      res_num++;
      res_names = (char **)realloc(res_names, res_num * sizeof(char *));
      res_names[res_num - 1] = strdup(r_name);
      res_dims = (int *)realloc(res_dims, res_num * sizeof(int));
      res_dims[res_num - 1] = read_dims[i];              
    }
  }
  for (i = 0; i < num_write_names; i++) {
    char *w_name = write_names[i];
    for (j = 0; j < res_num; j++) {
      if (!strcmp(w_name, res_names[j])) {
        break;
      }
    }
    if (j == res_num) {
      res_num++;
      res_names = (char **)realloc(res_names, res_num * sizeof(char *));
      res_names[res_num - 1] = strdup(w_name);
      res_dims = (int *)realloc(res_dims, res_num * sizeof(int));
      res_dims[res_num - 1] = write_dims[i];                      
    }
  }

  vsa->op_num = op_num;
  vsa->res_num = res_num;
  vsa->op_dims = op_dims;
  vsa->res_dims = res_dims;
  vsa->op_names = op_names;
  vsa->res_names = res_names;

  //free(read_names);
  //free(write_names);
  //free(read_dims);
  //free(write_names);
}

/* 
 * Extract necessary information from PlutoProg to complete the fields of VSA.
 */
VSA *pluto_prog_to_vsa(PlutoProg *prog) {
  int i, j;
  VSA *vsa = vsa_alloc();

  /* OP_NUM */
  /* RES_NUM */
  /* OP_DIM */
  /* RES_DIM */
  /* OP_NAME */
  /* RES_NAME */      
  vsa_op_res_extract(prog, vsa);

  /* SA_ROWS */

  /* SA_COLS */

  /* IL_ENABLE */

  /* ROW_IL_FACTOR */

  /* COL_IL_FACTOR */

  /* SIMD_FACTOR */

  /* OP_ENGINE_NUM */

  /* RES_ENGINE_NUM */  

  /* FC_SPLIT_FACTOR */
  // TODO: add this feature in the future
  vsa->fc_split_factors = (int *)malloc((vsa->op_num + vsa->res_num) * sizeof(int));
  for (i = 0; i < vsa->op_num + vsa->res_num; i++) {
    vsa->fc_split_factors[i] = 1;
  }

  /* FC_GROUP_FACTOR */
  // TODO: add this feature in the future
  vsa->fc_group_factors = (int *)malloc((vsa->op_num + vsa->res_num) *sizeof(int));
  for (i = 0; i < vsa->op_num + vsa->res_num; i++) {
    vsa->fc_group_factors[i] = 1;
  }

  /* ITERATORS */

  /* OP_CHANNEL_DIR */

  /* RES_CHANNEL_DIR */

  /* MAT_STAT */

  return vsa;
}

VSA *vsa_alloc() {
  VSA *vsa = (VSA *)malloc(sizeof(VSA));

  vsa->df_feed_counter_code = NULL;
  vsa->df_feed_addr_cal_code = NULL;
  vsa->dc_collect_counter_code = NULL;
  vsa->dc_collect_addr_cal_code = NULL;
  vsa->array_sizes = NULL;
  vsa->dfc_head_buf_sizes = NULL;
  vsa->op_names = NULL;
  vsa->res_names = NULL;
  vsa->row_il_factor = 0;
  vsa->col_il_factor = 0;
  vsa->global_accum_num = 0;
  vsa->local_reg_num = 0;
  vsa->op_engine_nums = NULL;
  vsa->res_engine_nums = NULL;
  vsa->sa_rows = 0;
  vsa->sa_cols = 0;
  vsa->fc_split_factors = NULL;
  vsa->fc_group_factors = NULL;
  vsa->iters = NULL;
  vsa->il_enable = 0;
  vsa->simd_factor = 0;
  vsa->op_num = 0;
  vsa->res_num = 0;
  vsa->op_dims = NULL;
  vsa->res_dims = NULL;
  vsa->op_channel_dirs = NULL;
  vsa->res_channel_dirs = NULL;
  vsa->dfc_buf_sizes = NULL;
  vsa->head_code = NULL;
  vsa->mac_stat = NULL;
  vsa->last_patch_code = NULL;
  vsa->last_tile_size = NULL;
  vsa->type = NULL;

  return vsa;
}

void vsa_free(VSA *vsa) {

}

void psa_print_string_with_indent(FILE *fp, int indent, char *to_print) {
  fprintf(fp, "%*s%s", indent, "", to_print);
}

void psa_print_int_with_indent(FILE *fp, int indent, int to_print) {
  fprintf(fp, "%*s%d", indent, "", to_print);
}

void psa_print_string_list_with_indent(FILE *fp, int indent, char **list, int len) {
  int i;
  for (i = 0; i < len; i++) {
    psa_print_string_with_indent(fp, indent, "\"");
    psa_print_string_with_indent(fp, 0, list[i]);
    if (i < len - 1) {
      psa_print_string_with_indent(fp, 0, "\",\n");
    } else {
      psa_print_string_with_indent(fp, 0, "\"\n");
    }
  }
}

void psa_print_int_list_with_indent(FILE *fp, int indent, int *list, int len) {
  int i;
  for (i = 0; i < len; i++) {
    psa_print_int_with_indent(fp, indent, list[i]);
    if (i < len - 1) {
      psa_print_string_with_indent(fp, 0, ",\n");
    } else {
      psa_print_string_with_indent(fp, 0, "\n");
    }
  }
}

/* Print out the content of the vsa following the JSON format */
void psa_vsa_pretty_print(FILE *fp, const VSA *vsa) {
  int i;
  fprintf(fp, "{\n");

  /* OP_NAME */
  psa_print_string_with_indent(fp, 2, "\"OP_NAME\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->op_names, vsa->op_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* RES_NAME */
  psa_print_string_with_indent(fp, 2, "\"RES_NAME\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->res_names, vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* OP_DIM */
  psa_print_string_with_indent(fp, 2, "\"OP_DIM\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->op_dims, vsa->op_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* RES_DIM */
  psa_print_string_with_indent(fp, 2, "\"RES_DIM\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->res_dims, vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* OP_CHANNEL_NUM */
  psa_print_string_with_indent(fp, 2, "\"OP_CHANNEL_NUM\": ");  
  psa_print_int_with_indent(fp, 0, vsa->op_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* RES_CHANNEL_NUM */
  psa_print_string_with_indent(fp, 2, "\"RES_CHANNEL_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->res_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* FC_SPLIT_FACTOR */
  psa_print_string_with_indent(fp, 2, "\"FC_SPLIT_FACTOR\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->fc_split_factors, vsa->op_num + vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* FC_GROUP_FACTOR */
  psa_print_string_with_indent(fp, 2, "\"FC_GROUP_FACTOR\": [\n");
  psa_print_int_list_with_indent(fp, 4, vsa->fc_group_factors, vsa->op_num + vsa->res_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  fprintf(fp, "}\n");
}