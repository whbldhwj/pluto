/* 
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#include "psa_hls_codegen_helpers.h"

/*
 * Return channel access string
 * e.g., [row][col + 1]
 */
char *psa_gen_PE_channel_acc(Vec *trans_dir) {
  int array_dim = trans_dir->length;
  char *acc_str = "";
  if (array_dim == 1) {
    acc_str = concat(acc_str, "[row][col");
    switch (trans_dir->val[0]) {
      case -1:
        acc_str = concat(acc_str, " - 1]");
        break;
      case 0:
        acc_str = concat(acc_str, "]");
        break;
      case 1:
        acc_str = concat(acc_str, " + 1]");
        break;
    }
  } else if (array_dim == 2) {
    acc_str = concat(acc_str, "[row");
    switch (trans_dir->val[0]) {
      case -1:
        acc_str = concat(acc_str, " - 1]");
        break;
      case 0:
        acc_str = concat(acc_str, "]");
        break;
      case 1:
        acc_str = concat(acc_str, " + 1]");
        break;
    }
    acc_str = concat(acc_str, "[col");
    switch (trans_dir->val[1]) {
      case -1:
        acc_str = concat(acc_str, " - 1]");
        break;
      case 0:
        acc_str = concat(acc_str, "]");
        break;
      case 1:
        acc_str = concat(acc_str, " + 1]");
        break;
    }
  }

  return acc_str;
}

/*
 * Return array access string
 * e.g., A[i][k]
 */
char *psa_gen_acc_str(PlutoAccess *acc, char **names) {
  char *acc_str = "";
  acc_str = concat(acc_str, acc->name);
  for (int row = 0; row < acc->mat->nrows; row++) {
    acc_str = concat(acc_str, "[");
    bool first_exp = true;
    for (int col = 0; col < acc->mat->ncols - 1; col++) {
      char tmp_str[20];
      if (acc->mat->val[row][col]) {
        switch (acc->mat->val[row][col]) {
          case 1:
            sprintf(tmp_str, "%s", names[col]);
            break;
          case -1:
            sprintf(tmp_str, "-%s", names[col]);        
            break;
          default:
            sprintf(tmp_str, "%d * %s", acc->mat->val[row][col], names[col]);
            break;
        }
        if (first_exp) {
          acc_str = concat(acc_str, tmp_str);
          first_exp = false;
        } else {
          acc_str = concat(acc_str, " + ");
          acc_str = concat(acc_str, tmp_str);
        }
      }
    }
    // constant
    if (acc->mat->val[row][acc->mat->ncols - 1]) {
      char tmp_str[20];
      sprintf(tmp_str, "%d", acc->mat->val[row][acc->mat->ncols - 1]);
      if (first_exp) {
        acc_str = concat(acc_str, tmp_str);
        first_exp = false;
      } else {
        acc_str = concat(acc_str, " + ");
        acc_str = concat(acc_str, tmp_str);
      }
    }
    acc_str = concat(acc_str, "]");
  }

  return acc_str;
}

/*
 * Return array access string after transformation
 * e.g., A[i][k]
 */
char *psa_gen_new_acc_str(Stmt *stmt, PlutoAccess *acc, char **names) {
  int *divs;
  PlutoMatrix *new_acc_mat = pluto_get_new_access_func(stmt, acc->mat, &divs);
  
  char *acc_str = "";
  acc_str = concat(acc_str, acc->name);
  for (int row = 0; row < acc->mat->nrows; row++) {
    acc_str = concat(acc_str, "[");
    if (divs[row] != 1) {
      acc_str = concat(acc_str, "(");
    }
    bool first_exp = true;
    for (int col = 0; col < acc->mat->ncols - 1; col++) {
      char tmp_str[20];
      if (acc->mat->val[row][col]) {
        switch (acc->mat->val[row][col]) {
          case 1:
            sprintf(tmp_str, "%s", names[col]);
            break;
          case -1:
            sprintf(tmp_str, "-%s", names[col]);        
            break;
          default:
            sprintf(tmp_str, "%d * %s", acc->mat->val[row][col], names[col]);
            break;
        }
        if (first_exp) {
          acc_str = concat(acc_str, tmp_str);
          first_exp = false;
        } else {
          acc_str = concat(acc_str, " + ");
          acc_str = concat(acc_str, tmp_str);
        }
      }
    }
    // constant
    if (acc->mat->val[row][acc->mat->ncols - 1]) {
      char tmp_str[20];
      sprintf(tmp_str, "%d", acc->mat->val[row][acc->mat->ncols - 1]);
      if (first_exp) {
        acc_str = concat(acc_str, tmp_str);
        first_exp = false;
      } else {
        acc_str = concat(acc_str, " + ");
        acc_str = concat(acc_str, tmp_str);
      }
    }
    if (divs[row] != 1) {
      acc_str = concat(acc_str, ") / ");
      char tmp_str[10];
      sprintf(tmp_str, "%d", divs[row]);
      acc_str = concat(acc_str, tmp_str);
    }
    acc_str = concat(acc_str, "]");
  }

  pluto_matrix_free(new_acc_mat);
  free(divs);

  return acc_str;
}
