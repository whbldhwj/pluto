/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_vsa.h"
#include "psa_vsa_dfc.h"
#include "psa_vsa_pe.h"

/* Print out the URE texts as a list */
char **get_vsa_URE_texts(URE **UREs, int URE_num) {
  if (URE_num < 0)
    return NULL;
  char **URE_texts = (char **)malloc(URE_num * sizeof(char *));
  for (int i = 0; i < URE_num; i++) {
    URE_texts[i] = strdup(UREs[i]->text);
  }
  return URE_texts;
}

/* Print out the URE names as a list */
char **get_vsa_URE_names(URE **UREs, int URE_num) {
  if (URE_num <= 0)
    return NULL;
  char **URE_names = (char **)malloc(URE_num * sizeof(char *));
  for (int i = 0; i < URE_num; i++) {
    URE_names[i] = strdup(UREs[i]->name);
  }
  return URE_names;
}

/* Print out the array names as a list */
char **get_vsa_array_names(Array **arrays, int array_num) {
  char **array_names = (char **)malloc(array_num * sizeof(char *));
  for (int i = 0; i < array_num; i++) {
    array_names[i] = strdup(arrays[i]->text);
  }
  return array_names;
}

/*
 * This function extracts the array information from the program
 */
void vsa_array_extract(PlutoProg *prog, VSA *vsa) {
  int array_num = 0;
  for (int i = 0; i < prog->nstmts; i++) {
    Stmt *stmt = prog->stmts[i];
    for (int j = 0; j < stmt->nreads; j++) {
      PlutoAccess *acc = stmt->reads[j];
      if (is_access_scalar(acc))
        continue;
      int p;
      for (p = 0; p < array_num; p++) {
        Array *arr = vsa->arrays[p];
        if (!strcmp(arr->text, acc->name)) {
          break;
        }
      }
      if (p == array_num) {
        array_num++;
        vsa->arrays = realloc(vsa->arrays, array_num * sizeof(Array *));
        vsa->arrays[array_num - 1] = (Array *)malloc(sizeof(Array));
        vsa->arrays[array_num - 1]->text = strdup(acc->name);
        vsa->arrays[array_num - 1]->id = array_num - 1;
        vsa->arrays[array_num - 1]->dim = acc->mat->nrows;
        vsa->arrays[array_num - 1]->data_type = "float"; // By default
      }    
    }
    for (int j = 0; j < stmt->nwrites; j++) {
      PlutoAccess *acc = stmt->writes[j];
      if (is_access_scalar(acc))
        continue;
      int p;
      for (p = 0; p < array_num; p++) {
        Array *arr = vsa->arrays[p];
        if (!strcmp(arr->text, acc->name)) {
          break;
        }
      }
      if (p == array_num) {
        array_num++;
        vsa->arrays = realloc(vsa->arrays, array_num * sizeof(Array *));
        vsa->arrays[array_num - 1] = (Array *)malloc(sizeof(Array));
        vsa->arrays[array_num - 1]->text = strdup(acc->name);
        vsa->arrays[array_num - 1]->id = array_num - 1;
        vsa->arrays[array_num - 1]->dim = acc->mat->nrows;
        vsa->arrays[array_num - 1]->data_type = "float"; // By default
      }
    }
  }
  vsa->array_num = array_num;
}

/* 
 * This function extracts the iterators for T2S program
 * T2S program assumes that all statements are inside one
 * permutable loop bands, and we will only keep the loop
 * iterators and throw the scalar iterators
 */
void vsa_t2s_iter_extract(PlutoProg *prog, VSA *vsa) {
  int band_width = prog->num_hyperplanes;
  for (int i = 0; i < prog->num_hyperplanes; i++) {
    bool loop_hyp = false;
    for (int j = 0; j < prog->nstmts; j++) {
      if (pluto_is_hyperplane_loop(prog->stmts[j], i)) {
        loop_hyp = true;
        break;
      }
    }
    if (!loop_hyp) {
      // scalar hyperplane
      band_width = i;
      break;
    }
  }

  vsa->t2s_iter_num = band_width;
  vsa->t2s_iters = (char **)malloc(sizeof(char *) * band_width);
  char iter[10];
  for (int i = 0; i < vsa->t2s_iter_num; i++) {
    sprintf(iter, "t%d", i + 1);
    vsa->t2s_iters[i] = strdup(iter);
  }
}

/*
 * RAW: iter_read + dep = iter_write
 */ 
void get_var_iters(int acc_id, struct stmt_access_var_pair **acc_var_map, PlutoProg *prog, VSA *vsa) {
  int iter_num = vsa->t2s_iter_num;
  char **iters = vsa->t2s_iters;

  PlutoAccess *acc = acc_var_map[acc_id]->acc;

  for (int i = 0; i < prog->ndeps; i++) {
    Dep *dep = prog->deps[i];
    if (IS_RAW(dep->type)) {
      if (dep->src_acc == acc && acc_var_map[dep->dest_acc->sym_id]->var_name != NULL) {
        // write access 
        for (int iter_id = 0; iter_id < iter_num; iter_id++) {
          int diff;
          if (dep->disvec[iter_id] == DEP_DIS_MINUS_ONE)
            diff = -1;
          else if (dep->disvec[iter_id] == DEP_DIS_ZERO)
            diff = 0;
          else if (dep->disvec[iter_id] == DEP_DIS_PLUS_ONE)
            diff = 1;

          acc_var_map[acc_id]->var_iters[iter_id]->iter_name = strdup(iters[iter_id]);
          acc_var_map[acc_id]->var_iters[iter_id]->iter_offset = acc_var_map[dep->dest_acc->sym_id]->var_iters[iter_id]->iter_offset + diff;
        }
      } else if (dep->dest_acc == acc && acc_var_map[dep->src_acc->sym_id]->var_name != NULL) {
        // read access
        for (int iter_id = 0; iter_id < iter_num; iter_id++) {
          int diff;
          if (dep->disvec[iter_id] == DEP_DIS_MINUS_ONE)
            diff = -1;
          else if (dep->disvec[iter_id] == DEP_DIS_ZERO)
            diff = 0;
          else if (dep->disvec[iter_id] == DEP_DIS_PLUS_ONE)
            diff = 1;

          acc_var_map[acc_id]->var_iters[iter_id]->iter_name = strdup(iters[iter_id]);
          acc_var_map[acc_id]->var_iters[iter_id]->iter_offset = acc_var_map[dep->src_acc->sym_id]->var_iters[iter_id]->iter_offset - diff;
        }
      }
    }
  }
}

/*
 * Use the DFS to update the intermediate variables
 */
void update_ivar(int acc_id, struct stmt_access_var_pair **acc_var_map, struct var_pair **adg_var_map, int cc_id, PlutoProg *prog, VSA *vsa) {
  int iter_num = vsa->t2s_iter_num;
  char **iters = vsa->t2s_iters;

  PlutoAccess *acc = acc_var_map[acc_id]->acc;

  if (acc_var_map[acc_id]->var_name == NULL) {
    // test if it's the first access to be updated in the CC
    bool is_first = true;
    for (int dep_id = 0; dep_id < prog->ndeps; dep_id++) {
      Dep *dep = prog->deps[dep_id];
      if (IS_RAW(dep->type)) {
        if ((dep->src_acc == acc && acc_var_map[dep->dest_acc->sym_id]->var_name != NULL) || (dep->dest_acc == acc && acc_var_map[dep->src_acc->sym_id]->var_name != NULL)) {
          is_first = false;
          break;
        }
      }
    }
    if (is_first) {
      vsa->ivar_num++;
      vsa->ivar_names = realloc(vsa->ivar_names, vsa->ivar_num * sizeof(char *));
      vsa->ivar_refs = realloc(vsa->ivar_refs, vsa->ivar_num * sizeof(char *));

      Stmt *stmt = acc_var_map[acc_id]->stmt;
      PlutoAccess *acc = acc_var_map[acc_id]->acc;
      char *arr_name = acc->name;
      char var_name[50];
      sprintf(var_name, "%s_CC%d_I", arr_name, cc_id);
      
      vsa->ivar_names[vsa->ivar_num - 1] = strdup(var_name);
      acc_var_map[acc_id]->var_name = strdup(var_name);
      adg_var_map[cc_id]->var_name = strdup(var_name);
      adg_var_map[cc_id]->ei = 1;
      adg_var_map[cc_id]->d = 0;

      // build the var_ref
      acc_var_map[acc_id]->var_iters = (IterExp **)malloc(iter_num * sizeof(IterExp *));
      for (int iter_id = 0; iter_id < iter_num; iter_id++) {
       acc_var_map[acc_id]->var_iters[iter_id] = (IterExp *)malloc(sizeof(IterExp));
       acc_var_map[acc_id]->var_iters[iter_id]->iter_name = strdup(iters[iter_id]);
       acc_var_map[acc_id]->var_iters[iter_id]->iter_offset = 0;
      }

      char var_ref[50];
      sprintf(var_ref, "%s(%s)", var_name, get_iter_str(acc_var_map[acc_id]->var_iters, iter_num));

      vsa->ivar_refs[vsa->ivar_num - 1] = strdup(var_ref);
      acc_var_map[acc_id]->var_ref = strdup(var_ref);

      // build the drain variable
      if (acc_var_map[acc_id]->d == 1) {
        vsa->idvar_num++;
        vsa->idvar_names = realloc(vsa->idvar_names, vsa->idvar_num * sizeof(char *));
        vsa->idvar_refs = realloc(vsa->idvar_refs, vsa->idvar_num * sizeof(char *));
        char var_name[50];
        sprintf(var_name, "%s_CC%d_ID", arr_name, cc_id);

        vsa->idvar_names[vsa->idvar_num - 1] = strdup(var_name);
        acc_var_map[acc_id]->dvar_name = strdup(var_name);
        adg_var_map[cc_id]->dvar_name = strdup(var_name);
        adg_var_map[cc_id]->d = 1;

        acc_var_map[acc_id]->dvar_iters = (IterExp **)malloc(iter_num * sizeof(IterExp *));
        for (int iter_id = 0; iter_id < iter_num; iter_id++) {
          acc_var_map[acc_id]->dvar_iters[iter_id] = (IterExp *)malloc(sizeof(IterExp));
          acc_var_map[acc_id]->dvar_iters[iter_id]->iter_name = strdup(iters[iter_id]);
          acc_var_map[acc_id]->dvar_iters[iter_id]->iter_offset = 0;
        }

        char var_ref[50];
        sprintf(var_ref, "%s(%s)", var_name, get_iter_str(acc_var_map[acc_id]->dvar_iters, iter_num));

        vsa->idvar_refs[vsa->idvar_num - 1] = strdup(var_ref);
        acc_var_map[acc_id]->dvar_ref = strdup(var_ref);
      }
    } else {
      Stmt **stmt = acc_var_map[acc_id]->stmt;
      PlutoAccess *acc = acc_var_map[acc_id]->acc;
      char *arr_name = acc->name;
      char var_name[50];
      sprintf(var_name, "%s_CC%d_I", arr_name, cc_id);

      acc_var_map[acc_id]->var_name = strdup(var_name);      

      // build the var_ref
      acc_var_map[acc_id]->var_iters = (IterExp **)malloc(iter_num * sizeof(IterExp *));
      for (int iter_id = 0; iter_id < iter_num; iter_id++) {
        acc_var_map[acc_id]->var_iters[iter_id] = (IterExp *)malloc(sizeof(IterExp));
      }
      get_var_iters(acc_id, acc_var_map, prog, vsa);

      char var_ref[50];
      sprintf(var_ref, "%s(%s)", var_name, get_iter_str(acc_var_map[acc_id]->var_iters, iter_num));
      acc_var_map[acc_id]->var_ref = strdup(var_ref);    

      // build the drain variable
      if (acc_var_map[acc_id]->d == 1) {
        char var_name[50];
        sprintf(var_name, "%s_CC%d_ID", arr_name, cc_id);

        acc_var_map[acc_id]->dvar_name = strdup(var_name);
        adg_var_map[cc_id]->dvar_name = strdup(var_name);
        adg_var_map[cc_id]->d = 1;

        acc_var_map[acc_id]->dvar_iters = (IterExp **)malloc(iter_num * sizeof(IterExp *));
        for (int iter_id = 0; iter_id < iter_num; iter_id++) {
          acc_var_map[acc_id]->dvar_iters[iter_id] = (IterExp *)malloc(sizeof(IterExp));
        }

        for (int iter_id = 0; iter_id < iter_num; iter_id++) {
          acc_var_map[acc_id]->dvar_iters[iter_id]->iter_name = strdup(acc_var_map[acc_id]->var_iters[iter_id]->iter_name);
          acc_var_map[acc_id]->dvar_iters[iter_id]->iter_offset = acc_var_map[acc_id]->var_iters[iter_id]->iter_offset;
        }

        char var_ref[50];
        sprintf(var_ref, "%s(%s)", var_name, get_iter_str(acc_var_map[acc_id]->dvar_iters, iter_num));
        acc_var_map[acc_id]->dvar_ref = strdup(var_ref);
      }      
    }
    // update the neighbors
    for (int dep_id = 0; dep_id < prog->ndeps; dep_id++) {
      Dep *dep = prog->deps[dep_id];
      if (IS_RAW(dep->type)) {
        if (dep->src_acc == acc) {
          update_ivar(dep->dest_acc->sym_id, acc_var_map, adg_var_map, cc_id, prog, vsa);
        } else if (dep->dest_acc == acc) {
          update_ivar(dep->src_acc->sym_id, acc_var_map, adg_var_map, cc_id, prog, vsa);
        }
      }
    }
  }
}

/*
 * Print out the IterExp in string
 */
char *get_iter_str(IterExp **iters, int iter_num) {
  char *iter_str = "";
  for (int iter_id = 0; iter_id < iter_num; iter_id++) {
    char *iter_name = iters[iter_id]->iter_name;
    int iter_offset = iters[iter_id]->iter_offset;
    char iter_exp[20];
    if (iter_offset == 0) {
      sprintf(iter_exp, "%s", iter_name);
    } else {
      sprintf(iter_exp, "%s + (%d)", iter_name, iter_offset);
    }
    iter_str = concat(iter_str, iter_exp);
    if (iter_id < iter_num - 1) {
      iter_str = concat(iter_str, ", ");
    }
  }
  return iter_str;
}

/*
 * Update the var_name, var_ref, var_iters
 */
void update_var(struct stmt_access_var_pair **acc_var_map, struct var_pair **adg_var_map,
    int total_accs, int cc_id, PlutoProg *prog, VSA *vsa) {
  int iter_num = vsa->t2s_iter_num;
  char **iters = vsa->t2s_iters;

  Graph *adg = prog->adg;
  if (adg->ccs[cc_id].size == 1) {
    // external variable
    vsa->evar_num++;
    vsa->evar_names = realloc(vsa->evar_names, vsa->evar_num * sizeof(char *));
    vsa->evar_refs = realloc(vsa->evar_refs, vsa->evar_num * sizeof(char *));

    for (int i = 0; i < total_accs; i++) {
      Stmt *stmt = acc_var_map[i]->stmt;
      PlutoAccess *acc = acc_var_map[i]->acc;
      int acc_id = acc->sym_id;
      if (acc->cc_id == cc_id) {
        char *arr_name = acc->name;
        // build the var_name
        char var_name[50];
        sprintf(var_name, "%s_CC%d_E", arr_name, cc_id);

        vsa->evar_names[vsa->evar_num - 1] = strdup(var_name);
        acc_var_map[acc_id]->var_name = strdup(var_name);
        adg_var_map[cc_id]->var_name = strdup(var_name);
        adg_var_map[cc_id]->ei = 0;
        adg_var_map[cc_id]->d = 0;

        // build the var_ref
        acc_var_map[acc_id]->var_iters = (IterExp **)malloc(iter_num * sizeof(IterExp *));
        for (int iter_id = 0; iter_id < iter_num; iter_id++) {
          acc_var_map[acc_id]->var_iters[iter_id] = (IterExp *)malloc(sizeof(IterExp));
          acc_var_map[acc_id]->var_iters[iter_id]->iter_name = strdup(iters[iter_id]);
          acc_var_map[acc_id]->var_iters[iter_id]->iter_offset = 0;
        }

        char var_ref[50];
        sprintf(var_ref, "%s(%s)", var_name, get_iter_str(acc_var_map[acc_id]->var_iters, iter_num));

        vsa->evar_refs[vsa->evar_num - 1] = strdup(var_ref);
        acc_var_map[acc_id]->var_ref = strdup(var_ref);

        // build the drain variable
        if (acc_var_map[acc_id]->d == 1) {
          vsa->edvar_num++;
          vsa->edvar_names = realloc(vsa->edvar_names, vsa->edvar_num * sizeof(char *));
          vsa->edvar_refs = realloc(vsa->edvar_refs, vsa->edvar_num * sizeof(char *));
          char var_name[50];
          sprintf(var_name, "%s_CC%d_ED", arr_name, cc_id);

          vsa->edvar_names[vsa->edvar_num - 1] = strdup(var_name);
          acc_var_map[acc_id]->dvar_name = strdup(var_name);
          acc_var_map[cc_id]->dvar_name = strdup(var_name);
          adg_var_map[cc_id]->d = 1;

          acc_var_map[acc_id]->dvar_iters = (IterExp **)malloc(iter_num * sizeof(IterExp *));
          for (int iter_id = 0; iter_id < iter_num; iter_id++) {
            acc_var_map[acc_id]->dvar_iters[iter_id] = (IterExp *)malloc(sizeof(IterExp));
            acc_var_map[acc_id]->dvar_iters[iter_id]->iter_name = strdup(iters[iter_id]);
            acc_var_map[acc_id]->dvar_iters[iter_id]->iter_offset = 0;
          }

          char var_ref[50];
          sprintf(var_ref, "%s(%s)", var_name, get_iter_str(acc_var_map[acc_id]->dvar_iters, iter_num));

          vsa->edvar_refs[vsa->edvar_num - 1] = strdup(var_ref);
          acc_var_map[acc_id]->dvar_ref = strdup(var_ref);
        }
      }
    }
  } else {
    // intermediate variable
    for (int i = 0; i < total_accs; i++) {
      Stmt *stmt = acc_var_map[i]->stmt;
      PlutoAccess *acc = acc_var_map[i]->acc;    
      if (acc->cc_id == cc_id) {
        update_ivar(acc->sym_id, acc_var_map, adg_var_map, cc_id, prog, vsa);
      }
    }
  }
}

void acc_var_map_pretty_print(struct stmt_access_var_pair **acc_var_map, int num_entry) {
  for (int i = 0; i < 10 + 30 + 30 + 2; i++) {
    fprintf(stdout, "-");
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "%10s|", "acc_id");
  fprintf(stdout, "%30s|", "var_ref");
  fprintf(stdout, "%30s\n", "dvar_ref");
  for (int i = 0; i < num_entry; i++) {
    fprintf(stdout, "%10d|", i);
    fprintf(stdout, "%30s|", acc_var_map[i]->var_ref);
    fprintf(stdout, "%30s\n", acc_var_map[i]->dvar_ref);
  }
  for (int i = 0; i < 10 + 30 + 30 + 2; i++) {
    fprintf(stdout, "-");
  }
  fprintf(stdout, "\n"); 
}

/* 
 * This function extracts the variables in the program and generates info:
 * - evar_num
 * - edvar_num
 * - ivar_num
 * - idvar_num
 * - evar_names
 * - edvar_names
 * - ivar_names
 * - idvar_names
 */
void vsa_var_extract(PlutoProg *prog, VSA *vsa) {
  int *num_stmts_per_acc; // indexed by data variables
  int num_read_write_data;
  struct stmt_access_pair ***acc_stmts; // indexed by data variable
  
  acc_stmts = get_read_write_access_with_stmts(
      prog->stmts, prog->nstmts, &num_read_write_data, &num_stmts_per_acc);    

  // total number of access functions in the program
  int total_accs = 0;
  for (int i = 0; i < num_read_write_data; i++) {
    total_accs += num_stmts_per_acc[i];
  }  

  // initialize the acc_var_map
  struct stmt_access_var_pair **acc_var_map = NULL;
  acc_var_map = (struct stmt_access_var_pair **)malloc(total_accs * sizeof(struct stmt_access_var_pair *)); 
  // initialization
  for (int i = 0; i < total_accs; i++) {
    acc_var_map[i] = (struct stmt_access_var_pair *)malloc(sizeof(struct stmt_access_var_pair));
    acc_var_map[i]->ei = -1;
    acc_var_map[i]->d = -1;
    acc_var_map[i]->var_name = NULL;
    acc_var_map[i]->var_ref = NULL;
    acc_var_map[i]->var_iters = NULL;
    acc_var_map[i]->dvar_name = NULL;
    acc_var_map[i]->dvar_ref = NULL;
    acc_var_map[i]->dvar_iters = NULL;
  }

  // In the context of UREs in T2S, we are assuming all statements are within one permutable band
  int band_width = prog->num_hyperplanes;
  for (int i = 0; i < prog->num_hyperplanes; i++) {
    bool loop_hyp = false;
    for (int j = 0; j < prog->nstmts; j++) {      
      if (pluto_is_hyperplane_loop(prog->stmts[j], i))  {
        loop_hyp = true;
        break;
      }        
    }
    if (!loop_hyp) {
      band_width = i;
    }
  }

  char *iters = "t1";;
  for (int i = 1; i < band_width; i++) {  
    char iter_tmp[6];
    sprintf(iter_tmp, ", t%d", i + 1);
    iters = concat(iters, iter_tmp);
  }

  // Build the access dependence graph. Each node in the graph is one unique access function in the program.
  // Then compute the connected components of the graph, all the accesses in the same componenet will 
  // use the same variable name, the variable is named as [arr_name]_CC[cc_id]_[E/I][D]
  // If there is only access function in the component, then it is an external variable
  // If there are more than one access function in the component, then it is an intermediate variable
  // Additionally, if the access function is a write access, we will add the drain variable correspondingly.

  Graph *adg = adg_create(prog);
  prog->adg= adg;
  adg_compute_cc(prog);

  // initialize the adg_var_map
  struct var_pair **adg_var_map = NULL;
  adg_var_map = (struct var_pair **)malloc(adg->num_ccs * sizeof(struct var_pair *));
  // initialization
  for (int i = 0; i < adg->num_ccs; i++) {
    adg_var_map[i] = (struct var_pair *)malloc(sizeof(struct var_pair));
    adg_var_map[i]->var_name = NULL;
    adg_var_map[i]->dvar_name = NULL;
    adg_var_map[i]->ei = -1;
    adg_var_map[i]->d = -1;
  }

  for (int i = 0; i < num_read_write_data; i++)
    for (int j = 0; j < num_stmts_per_acc[i]; j++) {
      struct stmt_access_pair *acc_stmt = acc_stmts[i][j];
      Stmt *stmt = acc_stmt->stmt;
      PlutoAccess *acc = acc_stmt->acc;
      int acc_id = acc->sym_id;
      acc_var_map[acc_id]->stmt = stmt;
      acc_var_map[acc_id]->acc = acc;
      if (acc_stmt->acc_rw == 1) {
        acc_var_map[acc_id]->d = 1; // add the drain variable for write access
      } else {
        acc_var_map[acc_id]->d = 0; 
      }
      int cc_id = acc->cc_id;
      if (adg->ccs[cc_id].size == 1) {
        acc_var_map[acc_id]->ei = 0; // external variable
      } else {
        acc_var_map[acc_id]->ei = 1; // intermediate variable
      }
    }

  // update the var_name, var_ref, var_iters by DFS in each CC
  for (int i = 0; i < prog->adg->num_ccs; i++) {
    int cc_id = prog->adg->ccs[i].id;
    update_var(acc_var_map, adg_var_map, total_accs, cc_id, prog, vsa);
  }

  vsa->acc_var_map = acc_var_map;
  vsa->adg_var_map = adg_var_map;
#ifdef PSA_VSA_DEBUG
  acc_var_map_pretty_print(acc_var_map, total_accs);
#endif  
}

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
 * Extract op_channel_dir and res_channel_dir from the prog.
 * Currently, we support op_channel_dir and res_channel_dir in:
 * R, Right
 * L, Left,
 * U, Up,
 * D, Down,
 * and combinations of these four above (e.g., RD, Right-down).
 * Examine the RAR dep for operands and the RAW dep for results. 
 */
void vsa_channel_dir_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;

  char **op_names = vsa->op_names;
  char **res_names = vsa->res_names;

  char **op_channel_dirs = (char **)malloc(vsa->op_num * sizeof(char *));
  char **res_channel_dirs = (char **)malloc(vsa->res_num * sizeof(char *));

  for (i = 0; i < vsa->op_num; i++) {
    char *op_name = op_names[i];
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      /* RAR dep for operands */
      if (IS_RAR(dep->type) && !strcmp(dep->src_acc->name, op_name)) {
        DepDis *space_dep_dis = (DepDis *)malloc(prog->array_dim * sizeof(DepDis));
        /* Collect dep dis components at space loops */
        int h;
        int space_dim = 0;
        for (h = 0; h < prog->num_hyperplanes; h++) {
          HyperplaneProperties hProps = prog->hProps[h];
          if (IS_PSA_SPACE_LOOP(hProps.psa_type)) {
            space_dep_dis[space_dim] = dep->disvec[h];
            space_dim++;
          }
        }

        /* Analyze the direction */
        if (prog->array_dim == 1) {
          if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            op_channel_dirs[i] = "R";
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            op_channel_dirs[i] = "L";
          } else if (space_dep_dis[0] == DEP_DIS_ZERO) {
            op_channel_dirs[i] = "D";
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loops!\n");
          }
        } else if (prog->array_dim == 2) {
          if (space_dep_dis[0] == DEP_DIS_ZERO) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {              
              /* 0,1 */
              op_channel_dirs[i] = "R";
            } else if (space_dep_dis[i] == DEP_DIS_ZERO) {
              /* 0,0 */
              /* TODO: In place - will be decided in the I/O elimination */
              op_channel_dirs[i] = "I"; 
            } else if (space_dep_dis[i] == DEP_DIS_MINUS_ONE) {
              /* 0,-1 */
              op_channel_dirs[i] = "L";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* 1,1 */
              op_channel_dirs[i] = "DR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* 1,0 */
              op_channel_dirs[i] = "D";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* 1,-1 */
              op_channel_dirs[i] = "DL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* -1,1 */
              op_channel_dirs[i] = "UR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* -1,0 */
              op_channel_dirs[i] = "U";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* -1,-1 */
              op_channel_dirs[i] = "UL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
          }
        }

        free(space_dep_dis);
      }
    }
  }

  for (i = 0; i < vsa->res_num; i++) {
    char *res_name = res_names[i];
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      /* RAW dep for results */
      if (IS_RAW(dep->type) && !strcmp(dep->src_acc->name, res_name)) {
        DepDis *space_dep_dis = (DepDis *)malloc(prog->array_dim * sizeof(DepDis));
        /* Collect dep dis components at space loop */
        int h;
        int space_dim = 0;
        for (h = 0; h < prog->num_hyperplanes; h++) {
          HyperplaneProperties hProps = prog->hProps[h];
          if (IS_PSA_SPACE_LOOP(hProps.psa_type)) {
            space_dep_dis[space_dim] = dep->disvec[h];
            space_dim++;
          }
        }

        /* Analyze the direction */
        if (prog->array_dim == 1) {
          if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            res_channel_dirs[i] = "R";
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            res_channel_dirs[i] = "L";
          } else if (space_dep_dis[0] == DEP_DIS_ZERO) {
            res_channel_dirs[i] = "D";
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loops!\n");
          }
        } else if (prog->array_dim == 2) {
          if (space_dep_dis[0] == DEP_DIS_ZERO) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {              
              /* 0,1 */
              res_channel_dirs[i] = "R";
            } else if (space_dep_dis[i] == DEP_DIS_ZERO) {
              /* 0,0 */
              /* TODO: In place - will be decided in the I/O elimination */
              res_channel_dirs[i] = "I"; 
            } else if (space_dep_dis[i] == DEP_DIS_MINUS_ONE) {
              /* 0,-1 */
              res_channel_dirs[i] = "L";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_PLUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* 1,1 */
              res_channel_dirs[i] = "DR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* 1,0 */
              res_channel_dirs[i] = "D";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* 1,-1 */
              res_channel_dirs[i] = "DL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else if (space_dep_dis[0] == DEP_DIS_MINUS_ONE) {
            if (space_dep_dis[1] == DEP_DIS_PLUS_ONE) {
              /* -1,1 */
              res_channel_dirs[i] = "UR";
            } else if (space_dep_dis[1] == DEP_DIS_ZERO) {
              /* -1,0 */
              res_channel_dirs[i] = "U";
            } else if (space_dep_dis[1] == DEP_DIS_MINUS_ONE) {
              /* -1,-1 */
              res_channel_dirs[i] = "UL";
            } else {
              fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
            }
          } else {
            fprintf(stdout, "[PSA] Error! Wrong dependence component at space loop!\n");
          }
        }

        free(space_dep_dis);
      }
    }
  }

  vsa->op_channel_dirs = op_channel_dirs;
  vsa->res_channel_dirs = res_channel_dirs;
}

/*
 * Calculate the op/res_engine_num
 */
void vsa_engine_num_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;

  vsa->op_engine_nums = (int *)malloc(vsa->op_num * sizeof(int));
  vsa->res_engine_nums = (int *)malloc(vsa->res_num * sizeof(int));

  int op_num = vsa->op_num;
  int res_num = vsa->res_num;

  int sa_rows = vsa->sa_rows;
  int sa_cols = vsa->sa_cols;

  char **op_channel_dirs = vsa->op_channel_dirs;
  char **res_channel_dirs = vsa->res_channel_dirs;

  for (i = 0; i < op_num; i++) {
    char *op_channel_dir = op_channel_dirs[i];
    if (!strcmp(op_channel_dir, "U") || !strcmp(op_channel_dir, "D")) {
      vsa->op_engine_nums[i] = sa_cols;
    } else if (!strcmp(op_channel_dir, "L") || !strcmp(op_channel_dir, "R")) {
      vsa->op_engine_nums[i] = sa_rows;
    } else {
      /* TODO: What about I? */
      vsa->op_engine_nums[i] = sa_rows + sa_cols;
    }
  } 

  for (i = 0; i < res_num; i++) {
    char *res_channel_dir = res_channel_dirs[i];
    if (!strcmp(res_channel_dir, "U") || !strcmp(res_channel_dir, "D")) {
      vsa->res_engine_nums[i] = sa_cols;
    } else if (!strcmp(res_channel_dir, "L") || !strcmp(res_channel_dir, "R")) {
      vsa->res_engine_nums[i] = sa_rows;
    } else {
      /* TODO: What about I? */
      vsa->res_engine_nums[i] = sa_rows + sa_cols;
    }
  } 
}

/*
 * Detect the array type: local, global
 */
void vsa_type_extract(PlutoProg *prog, VSA *vsa) {
  int i, j;
  for (i = 0; i < vsa->res_num; i++) {
    char *res_name = vsa->res_names[i];
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      /* RAW dep for results */
      if (IS_RAW(dep->type) && !strcmp(dep->src_acc->name, res_name)) {
        DepDis *space_dep_dis = (DepDis *)malloc(prog->array_dim * sizeof(DepDis));
        /* Collect dep dis components at space loop */
        int h;
        int space_dim = 0;
        for (h = 0; h < prog->num_hyperplanes; h++) {
          HyperplaneProperties hProps = prog->hProps[h];
          if (IS_PSA_SPACE_LOOP(hProps.psa_type)) {
            space_dep_dis[space_dim] = dep->disvec[h];
            space_dim++;
          }
        }

        if (prog->array_dim == 1) {
          if (space_dep_dis[0] != DEP_DIS_ZERO)
            break;          
        } else if (prog->array_dim == 2) {
          if (space_dep_dis[0] != DEP_DIS_ZERO || space_dep_dis[1] != DEP_DIS_ZERO)
            break;
        }
      }
    }
  }

  if ((i == vsa->res_num) && (j == prog->ndeps)) {
    vsa->type = "local";
  } else {
    vsa->type = "global";
  }  
}

/* 
 * Extract necessary information from PlutoProg to complete the fields of VSA.
 */
void pluto_prog_to_vsa(PlutoProg *prog, VSA *vsa) {
  int i, j;  

//  /* SA_ROWS */
//  vsa->sa_rows = prog->array_nrow;
//
//  /* SA_COLS */
//  vsa->sa_cols = prog->array_ncol;
//
//  /* IL_ENABLE */
//  vsa->il_enable = prog->array_il_enable;
//
//  /* ROW_IL_FACTOR */
//  vsa->row_il_factor = prog->array_il_factor[0];
//
//  /* COL_IL_FACTOR */
//  vsa->col_il_factor = prog->array_il_factor[1];
//
//  /* SIMD_FACTOR */
//  vsa->simd_factor = prog->array_simd_factor;
//
//  /* OP_ENGINE_NUM, RES_ENGINE_NUM */ 
//  vsa_engine_num_extract(prog, vsa);   
//
//  /* FC_SPLIT_FACTOR */
//  // TODO: add this feature in the future
//  vsa->fc_split_factors = (int *)malloc((vsa->op_num + vsa->res_num) * sizeof(int));
//  for (i = 0; i < vsa->op_num + vsa->res_num; i++) {
//    vsa->fc_split_factors[i] = 1;
//  }
//
//  /* FC_GROUP_FACTOR */
//  // TODO: add this feature in the future
//  vsa->fc_group_factors = (int *)malloc((vsa->op_num + vsa->res_num) *sizeof(int));
//  for (i = 0; i < vsa->op_num + vsa->res_num; i++) {
//    vsa->fc_group_factors[i] = 1;
//  }
//
//  /* FC_SIMD_FACTOR */
//  // TODO: add this feature in the future
//  vsa->fc_simd_factors = (int *)malloc((vsa->op_num + vsa->res_num) *sizeof(int));
//  for (i = 0; i < vsa->op_num; i++) {
//    vsa->fc_simd_factors[i] = vsa->simd_factor;
//  }
//  for (i = vsa->op_num; i < vsa->op_num + vsa->res_num; i++) {
//    vsa->fc_simd_factors[i] = 1;
//  }
//
  /* ARRAY_PART_BAND_WIDTH */
  vsa_band_width_extract(prog, vsa);  

//  /* DF Code */
//  PlutoProg *new_prog;
//  new_prog = pluto_prog_dup(prog);
//  vsa_df_code_extract(new_prog, vsa);
//  pluto_prog_free(new_prog);
//
//  /* DC Code */
//  new_prog = pluto_prog_dup(prog);
//  vsa_dc_code_extract(new_prog, vsa);
//  pluto_prog_free(new_prog);
//
//  /* PE Code */
//  new_prog = pluto_prog_dup(prog);
//  vsa_pe_code_extract(new_prog, vsa);
//  pluto_prog_free(new_prog);

  /* Added for T2s */

  /* T2S_ITERS */
  vsa_t2s_iter_extract(prog, vsa);

  /* T2S_VARS */
  vsa_var_extract(prog, vsa);

  /* ARRAYS */
  vsa_array_extract(prog, vsa);

  /* URES */
  vsa_URE_extract(prog, vsa);

  /* T2S_META_ITERS */
  vsa_t2s_meta_iter_extract(prog, vsa);
}

/*
 * Extract the number of loops in the array_part band, space band, and engine band
 */
void vsa_band_width_extract(PlutoProg *prog, VSA *vsa) {
  Band **bands;
  int nbands;
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);
  assert(nbands == 1);

  unsigned i, h;
  for (i = 0; i < nbands; i++) {
    Band *band_cur = bands[i];
    unsigned first_array_part_hyp = 0;
    unsigned first_space_hyp, first_time_hyp;
    bool first_array_part_hyp_found = false;
    bool first_space_hyp_found = false;
    bool first_time_hyp_found = false;
    for (h = 0; h < prog->num_hyperplanes; h++) {
      if (IS_PSA_ARRAY_PART_LOOP(prog->hProps[h].psa_type) &&
        !first_array_part_hyp_found) {
        first_array_part_hyp = h;
        first_array_part_hyp_found = true;        
      }
      if (IS_PSA_SPACE_LOOP(prog->hProps[h].psa_type) && !first_space_hyp_found) {
        first_space_hyp = h;
        first_space_hyp_found = true;
      }
      if (IS_PSA_TIME_LOOP(prog->hProps[h].psa_type) && !first_time_hyp_found) {
        first_time_hyp = h;
        first_time_hyp_found = true;
      }
    }    
    vsa->array_part_band_width = first_space_hyp - first_array_part_hyp;
    vsa->space_band_width = first_time_hyp - first_space_hyp;

//    /* Engine band */
//    //vsa->op_engine_band_width = (int *)malloc(vsa->op_num * sizeof(int));
//    //vsa->res_engine_band_width = (int *)malloc(vsa->res_num * sizeof(int));
//    vsa->engine_band_width = (int *)malloc((vsa->op_num + vsa->res_num) * sizeof(int));
//
//    int array_dim = prog->array_dim;
//    int ref_idx;
//    for (ref_idx = 0; ref_idx < vsa->op_num; ref_idx++) {
//      char *acc_channel_dir = vsa->op_channel_dirs[ref_idx];
//      if (array_dim == 2) {        
//        vsa->engine_band_width[ref_idx] = 1;        
//      } else if (array_dim == 1) {
//        if (!strcmp(acc_channel_dir, "D")) {
//          vsa->engine_band_width[ref_idx] = 1;
//        } else if (!strcmp(acc_channel_dir, "R")) {
//          vsa->engine_band_width[ref_idx] = 0;
//        } else {
//          fprintf(stdout, "[PSA] Error! Not supported!\n");
//          exit(1);
//        }
//      }
//    }
//
//    for (ref_idx = 0; ref_idx < vsa->res_num; ref_idx++) {
//      char *acc_channel_dir = vsa->res_channel_dirs[ref_idx];
//      if (array_dim == 2) {
//        vsa->engine_band_width[ref_idx + vsa->op_num] = 1;
//      } else if (array_dim == 1) {
//        if (!strcmp(acc_channel_dir, "D")) {
//          vsa->engine_band_width[ref_idx + vsa->op_num] = 1;
//        } else if (!strcmp(acc_channel_dir, "R")) {
//          vsa->engine_band_width[ref_idx + vsa->op_num] = 0;
//        } else {
//          fprintf(stdout, "[PSA] Error! Not supported!\n");
//          exit(1);
//        }
//      }
//    }
  }
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
  vsa->fc_simd_factors = NULL;
  vsa->iters = NULL;
  vsa->iter_num = 0;
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
  vsa->array_part_band_width = 0;
  vsa->space_band_width = 0;
  vsa->engine_band_width = NULL;  

  /* T2S */
  vsa->evar_num = 0;
  vsa->edvar_num = 0;
  vsa->ivar_num = 0;
  vsa->idvar_num = 0;
  vsa->evar_names = NULL;
  vsa->edvar_names = NULL;
  vsa->evar_refs = NULL;
  vsa->edvar_refs = NULL;
  vsa->ivar_names = NULL;
  vsa->idvar_names = NULL;
  vsa->ivar_refs = NULL;
  vsa->idvar_refs = NULL;
  vsa->acc_var_map = NULL;
  vsa->adg_var_map = NULL;
  vsa->t2s_iter_num = -1;
  vsa->t2s_iters = NULL;
  vsa->t2s_meta_iters = NULL;
  vsa->array_num = -1;
  vsa->arrays = NULL;

  vsa->URE_num = 0;
  vsa->UREs = NULL;
  vsa->domain_exp_num = 0;
  vsa->domain_exps = NULL;
  vsa->t2s_IO_func_num = 0;
  vsa->t2s_IO_func_names = NULL;
  vsa->t2s_IO_build_num = 0;
  vsa->t2s_IO_build_calls = NULL;

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

//  /* OP_NAME */
//  psa_print_string_with_indent(fp, 2, "\"OP_NAME\": [\n");
//  psa_print_string_list_with_indent(fp, 4, vsa->op_names, vsa->op_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* RES_NAME */
//  psa_print_string_with_indent(fp, 2, "\"RES_NAME\": [\n");
//  psa_print_string_list_with_indent(fp, 4, vsa->res_names, vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* OP_DIM */
//  psa_print_string_with_indent(fp, 2, "\"OP_DIM\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->op_dims, vsa->op_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* RES_DIM */
//  psa_print_string_with_indent(fp, 2, "\"RES_DIM\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->res_dims, vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* OP_CHANNEL_NUM */
//  psa_print_string_with_indent(fp, 2, "\"OP_CHANNEL_NUM\": ");  
//  psa_print_int_with_indent(fp, 0, vsa->op_num);
//  psa_print_string_with_indent(fp, 0, ",\n");
//
//  /* RES_CHANNEL_NUM */
//  psa_print_string_with_indent(fp, 2, "\"RES_CHANNEL_NUM\": ");
//  psa_print_int_with_indent(fp, 0, vsa->res_num);
//  psa_print_string_with_indent(fp, 0, ",\n");
//
//  /* FC_SPLIT_FACTOR */
//  psa_print_string_with_indent(fp, 2, "\"FC_SPLIT_FACTOR\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->fc_split_factors, vsa->op_num + vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* FC_GROUP_FACTOR */
//  psa_print_string_with_indent(fp, 2, "\"FC_GROUP_FACTOR\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->fc_group_factors, vsa->op_num + vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* FC_SIMD_FACTOR */
//  psa_print_string_with_indent(fp, 2, "\"FC_SIMD_FACTOR\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->fc_simd_factors, vsa->op_num + vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");  
//
//  /* OP_CHANNEL_DIR */
//  psa_print_string_with_indent(fp, 2, "\"OP_CHANNEL_DIR\": [\n");
//  psa_print_string_list_with_indent(fp, 4, vsa->op_channel_dirs, vsa->op_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* RES_CHANNEL_DIR */
//  psa_print_string_with_indent(fp, 2, "\"RES_CHANNEL_DIR\": [\n");
//  psa_print_string_list_with_indent(fp, 4, vsa->res_channel_dirs, vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* IL_ENABLE */
//  psa_print_string_with_indent(fp, 2, "\"IL_ENABLE\": ");
//  psa_print_int_with_indent(fp, 0, vsa->il_enable);
//  psa_print_string_with_indent(fp, 0, ",\n");
//
//  /* ROW_IL_FACTOR */
//  psa_print_string_with_indent(fp, 2, "\"ROW_IL_FACTOR\": ");
//  psa_print_int_with_indent(fp, 0, vsa->row_il_factor);
//  psa_print_string_with_indent(fp, 0, ",\n");
//
//  /* COL_IL_FACTOR */
//  psa_print_string_with_indent(fp, 2, "\"COL_IL_FACTOR\": ");
//  psa_print_int_with_indent(fp, 0, vsa->col_il_factor);
//  psa_print_string_with_indent(fp, 0, ",\n");
//
//  /* SIMD_FACTOR */
//  psa_print_string_with_indent(fp, 2, "\"SIMD_FACTOR\": ");
//  psa_print_int_with_indent(fp, 0, vsa->simd_factor);
//  psa_print_string_with_indent(fp, 0, ",\n");
//
//  /* SA_ROWS */
//  psa_print_string_with_indent(fp, 2, "\"SA_ROWS\": ");
//  psa_print_int_with_indent(fp, 0, vsa->sa_rows);
//  psa_print_string_with_indent(fp, 0, ",\n");  
//
//  /* SA_COLS */
//  psa_print_string_with_indent(fp, 2, "\"SA_COLS\": ");
//  psa_print_int_with_indent(fp, 0, vsa->sa_cols);
//  psa_print_string_with_indent(fp, 0, ",\n");  
//
//  /* OP_ENGINE_NUM */
//  psa_print_string_with_indent(fp, 2, "\"OP_ENGINE_NUM\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->op_engine_nums, vsa->op_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* RES_ENGINE_NUM */
//  psa_print_string_with_indent(fp, 2, "\"RES_ENGINE_NUM\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->res_engine_nums, vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");
//
//  /* TYPE */
//  psa_print_string_with_indent(fp, 2, "\"TYPE\": \"");
//  psa_print_string_with_indent(fp, 0, vsa->type);
//  psa_print_string_with_indent(fp, 0, "\",\n");
//
//  /* ARRAY_PART_BAND_WIDTH */
//  psa_print_string_with_indent(fp, 2, "\"ARRAY_PART_BAND_WIDTH\": ");
//  psa_print_int_with_indent(fp, 0, vsa->array_part_band_width);
//  psa_print_string_with_indent(fp, 0, ",\n");  
//
//  /* SPACE_BAND_WIDTH */
//  psa_print_string_with_indent(fp, 2, "\"SPACE_BAND_WIDTH\": ");
//  psa_print_int_with_indent(fp, 0, vsa->space_band_width);
//  psa_print_string_with_indent(fp, 0, ",\n");
//
//  /* ENGINE_BAND_WIDTH */
//  psa_print_string_with_indent(fp, 2, "\"ENGINE_BAND_WIDTH\": [\n");
//  psa_print_int_list_with_indent(fp, 4, vsa->engine_band_width, vsa->op_num + vsa->res_num);
//  psa_print_string_with_indent(fp, 2, "],\n");

  /****************************************/
  /* Following are T2S attibutes */
  /****************************************/
  /* EVAR_NUM */
  psa_print_string_with_indent(fp, 2, "\"EVAR_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->evar_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* EDVAR_NUM */
  psa_print_string_with_indent(fp, 2, "\"EDVAR_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->edvar_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* IVAR_NUM */
  psa_print_string_with_indent(fp, 2, "\"IVAR_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->ivar_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* IDVAR_NUM */
  psa_print_string_with_indent(fp, 2, "\"IDVAR_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->idvar_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* EVAR_NAMES */
  psa_print_string_with_indent(fp, 2, "\"EVAR_NAMES\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->evar_names, vsa->evar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* EDVAR_NAMES */
  psa_print_string_with_indent(fp, 2, "\"EDVAR_NAMES\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->edvar_names, vsa->edvar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* EVAR_REFS */
  psa_print_string_with_indent(fp, 2, "\"EVAR_REFS\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->evar_refs, vsa->evar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* EDVAR_REFS */
  psa_print_string_with_indent(fp, 2, "\"EDVAR_REFS\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->edvar_refs, vsa->edvar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* IVAR_NAMES */
  psa_print_string_with_indent(fp, 2, "\"IVAR_NAMES\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->ivar_names, vsa->ivar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* IDVAR_NAMES */
  psa_print_string_with_indent(fp, 2, "\"IDVAR_NAMES\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->idvar_names, vsa->idvar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* IVAR_REFS */
  psa_print_string_with_indent(fp, 2, "\"IVAR_REFS\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->ivar_refs, vsa->ivar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* IDVAR_REFS */
  psa_print_string_with_indent(fp, 2, "\"IDVAR_REFS\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->idvar_refs, vsa->idvar_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* T2S_ITER_NUM */
  psa_print_string_with_indent(fp, 2, "\"T2S_ITER_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->t2s_iter_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* T2S_ITERS */
  psa_print_string_with_indent(fp, 2, "\"T2S_ITERS\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->t2s_iters, vsa->t2s_iter_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* ARRAY_NUM */
  psa_print_string_with_indent(fp, 2, "\"ARRAY_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->array_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* ARRAYS */
  psa_print_string_with_indent(fp, 2, "\"ARRAYS\": [\n");
  psa_print_string_list_with_indent(fp, 4, get_vsa_array_names(vsa->arrays, vsa->array_num), vsa->array_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* URE_NUM */
  psa_print_string_with_indent(fp, 2, "\"URE_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->URE_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* URES */
  psa_print_string_with_indent(fp, 2, "\"URES\": [\n");
  psa_print_string_list_with_indent(fp, 4, get_vsa_URE_texts(vsa->UREs, vsa->URE_num), vsa->URE_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  /* DOMAIN_EXP_NUM */
  psa_print_string_with_indent(fp, 2, "\"DOMAIN_EXP_NUM\": ");
  psa_print_int_with_indent(fp, 0, vsa->domain_exp_num);
  psa_print_string_with_indent(fp, 0, ",\n");

  /* DOMAIN_EXPS */
  psa_print_string_with_indent(fp, 2, "\"DOMAIN_EXPS\": [\n");
  psa_print_string_list_with_indent(fp, 4, vsa->domain_exps, vsa->domain_exp_num);
  psa_print_string_with_indent(fp, 2, "],\n");

  fprintf(fp, "}\n");
}
