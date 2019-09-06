/* 
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_io.h"

/*
 * This function extracts the I/O information from the program
 */
void vsa_IO_extract(PlutoProg *prog, VSA *vsa) {
  int *num_stmts_per_acc; // indexed by data variable
  int num_read_write_data;
  struct stmt_access_pair ***acc_stmts; // indexed by data variable

  acc_stmts = get_read_write_access_with_stmts(
      prog->stmts, prog->nstmts, &num_read_write_data, &num_stmts_per_acc);

  // total number of access functions in the program
  int total_accs = 0;
  for (int i = 0; i < num_read_write_data; i++) {
    total_accs += num_stmts_per_acc[i];
  }

  // initialize the acc_io_map
  struct stmt_access_io_pair **io_map = NULL;
  int io_map_num_entries = 0;

  // update IO information
  for (int i = 0; i < num_read_write_data; i++) {
    for (int j = 0; j < num_stmts_per_acc[i]; j++) {
      struct stmt_access_pair *acc_stmt = acc_stmts[i][j];
      Stmt *stmt = acc_stmt->stmt;
      PlutoAccess *acc = acc_stmt->acc;
      int acc_id = acc->sym_id;
      for (int d = 0; d < prog->ndeps; d++) {
        Dep *dep = prog->deps[d];
        /* RAR */
        if (IS_RAR(dep->type)) {
          if (dep->src_acc == acc || dep->dest_acc == acc) {
            io_map_num_entries++;
            io_map = (struct stmt_access_io_pair **)realloc(io_map, io_map_num_entries * sizeof(struct stmt_access_io_pair *));
            io_map[io_map_num_entries - 1] = (struct stmt_access_io_pair *)malloc(sizeof(struct stmt_access_io_pair));
            io_map[io_map_num_entries - 1]->stmt = stmt;
            io_map[io_map_num_entries - 1]->acc = acc;
            io_map[io_map_num_entries - 1]->dep = dep;

            if(is_dep_carried_at_space_band(prog, dep)) {
              io_map[io_map_num_entries - 1]->L1_trans_bound = IN_BOUND;
              io_map[io_map_num_entries - 1]->L1_trans_type = EMBEDDED;
              io_map[io_map_num_entries - 1]->L1_trans_dir = get_L1_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L2_trans_bound = IN_BOUND;
              io_map[io_map_num_entries - 1]->L2_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L2_trans_dir = get_L2_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L3_trans_bound = IN_BOUND;
              io_map[io_map_num_entries - 1]->L3_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L3_trans_dir = get_L3_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
            } else if (is_dep_carried_at_time_band(prog, dep)) {
              io_map[io_map_num_entries - 1]->L1_trans_bound = IN_BOUND;
              io_map[io_map_num_entries - 1]->L1_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L1_trans_dir = get_L1_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L2_trans_bound = IN_BOUND;
              io_map[io_map_num_entries - 1]->L2_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L2_trans_dir = get_L2_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L3_trans_bound = IN_BOUND;
              io_map[io_map_num_entries - 1]->L3_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L3_trans_dir = get_L3_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
            }

            // the domain is the same as the statement domain 
            io_map[io_map_num_entries - 1]->domain = pluto_get_new_domain(stmt);
          }
        }
        /* RAW */
        if (IS_RAW(dep->type)) {
          if (dep->src_acc == acc) {
            /* Write access */
            io_map_num_entries++;
            io_map = (struct stmt_access_io_pair **)realloc(io_map, io_map_num_entries * sizeof(struct stmt_access_io_pair *));
            io_map[io_map_num_entries - 1] = (struct stmt_access_io_pair *)malloc(sizeof(struct stmt_access_io_pair));
            io_map[io_map_num_entries - 1]->stmt = stmt;
            io_map[io_map_num_entries - 1]->acc = acc;
            io_map[io_map_num_entries - 1]->dep = dep;

            if (is_dep_carried_at_space_band(prog, dep)) {
              io_map[io_map_num_entries - 1]->L1_trans_bound = OUT_BOUND;
              io_map[io_map_num_entries - 1]->L1_trans_type = EMBEDDED;
              io_map[io_map_num_entries - 1]->L1_trans_dir = get_L1_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L2_trans_bound = OUT_BOUND;
              io_map[io_map_num_entries - 1]->L2_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L2_trans_dir = get_L2_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L3_trans_bound = OUT_BOUND;
              io_map[io_map_num_entries - 1]->L3_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L3_trans_dir = get_L3_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);

              // compute the flow-out set + write-out set      
              // Flow-out set: (array_part + space) not equal src iter
              // Write-out set: waccc iter - WAW src iter
              io_map[io_map_num_entries - 1]->domain = NULL; 
            } else if (is_dep_carried_at_time_band(prog, dep)) {
              io_map[io_map_num_entries - 1]->L1_trans_bound = OUT_BOUND;
              io_map[io_map_num_entries - 1]->L1_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L1_trans_dir = get_L1_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L2_trans_bound = OUT_BOUND;
              io_map[io_map_num_entries - 1]->L2_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L2_trans_dir = get_L2_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              io_map[io_map_num_entries - 1]->L3_trans_bound = OUT_BOUND;
              io_map[io_map_num_entries - 1]->L3_trans_type = SEPARATE;
              io_map[io_map_num_entries - 1]->L3_trans_dir = get_L3_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);

              // compute the flow-out set + write-out set 
              io_map[io_map_num_entries - 1]->domain = NULL;
            }
          } else if (dep->dest_acc == acc){
            /* Read access */
            io_map_num_entries++;
            io_map = (struct stmt_access_io_pair **)realloc(io_map, io_map_num_entries * sizeof(struct stmt_access_io_pair *));
            io_map[io_map_num_entries - 1] = (struct stmt_access_io_pair *)malloc(sizeof(struct stmt_access_io_pair));
            io_map[io_map_num_entries - 1]->stmt = stmt;
            io_map[io_map_num_entries - 1]->acc = acc;
            io_map[io_map_num_entries - 1]->dep = dep;

            if (is_dep_carried_at_space_band(prog, dep)) {
              io_map[io_map_num_entries - 1]->L1_trans_bound = IN_BOUND;
              io_map[io_map_num_entries - 1]->L1_trans_type = EMBEDDED;
              io_map[io_map_num_entries - 1]->L1_trans_dir = get_L1_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              if (is_dep_carried_at_array_part_band(prog, dep)) {
                io_map[io_map_num_entries - 1]->L2_trans_bound = IN_BOUND;
                io_map[io_map_num_entries - 1]->L2_trans_type = SEPARATE;
                io_map[io_map_num_entries - 1]->L2_trans_dir = get_L2_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
                io_map[io_map_num_entries - 1]->L3_trans_bound = IN_BOUND;
                io_map[io_map_num_entries - 1]->L3_trans_type = SEPARATE;
                io_map[io_map_num_entries - 1]->L3_trans_dir = get_L3_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);

              }
              
              // compute the flow-in set
              // Flow-in set: (array part + space) not equal dest iter
              int copy_level = vsa->array_part_band_width + vsa->space_band_width;
              //io_map[io_map_num_entries - 1]->domain = compute_flow_in_of_dep(dep, copy_level, prog);
              io_map[io_map_num_entries - 1]->domain = NULL;
            } else if (is_dep_carried_at_time_band(prog, dep)) {
              io_map[io_map_num_entries - 1]->L1_trans_bound = NO_BOUND;
              io_map[io_map_num_entries - 1]->L1_trans_type = EMBEDDED;
              io_map[io_map_num_entries - 1]->L1_trans_dir = NULL;
              io_map[io_map_num_entries - 1]->L2_trans_bound = UNKNOWN_BOUND;
              io_map[io_map_num_entries - 1]->L2_trans_type = UNKNOWN_TYPE;
              io_map[io_map_num_entries - 1]->L2_trans_dir = NULL;
              io_map[io_map_num_entries - 1]->L3_trans_bound = UNKNOWN_BOUND;
              io_map[io_map_num_entries - 1]->L3_trans_type = UNKNOWN_TYPE;
              io_map[io_map_num_entries - 1]->L3_trans_dir = NULL;

              if (is_dep_carried_at_array_part_band(prog, dep)) {
                io_map[io_map_num_entries - 1]->L1_trans_bound = IN_BOUND;
                io_map[io_map_num_entries - 1]->L1_trans_type = SEPARATE;
                io_map[io_map_num_entries - 1]->L1_trans_dir = get_L1_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
                io_map[io_map_num_entries - 1]->L2_trans_bound = IN_BOUND;
                io_map[io_map_num_entries - 1]->L2_trans_type = SEPARATE;
                io_map[io_map_num_entries - 1]->L2_trans_dir = get_L2_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
                io_map[io_map_num_entries - 1]->L3_trans_bound = IN_BOUND;
                io_map[io_map_num_entries - 1]->L3_trans_type = SEPARATE;
                io_map[io_map_num_entries - 1]->L3_trans_dir = get_L3_trans_dir(prog, dep, vsa, io_map[io_map_num_entries - 1]);
              }

              // compute the flow-in set
              // Flow-in set: (array part + space) not equal dest iter
              io_map[io_map_num_entries - 1]->domain = NULL;
            }
          }
        }
      }
    }
  }

  // Group the access functions based on the array name and transfer direction.
  // Each element in the group shasre the same variable name and data transfer direction.
  // We will allocate one set of data transfer modules (including FIFOs to them).
  int num_data_trans_sets = 0;
  int *num_acc_per_data_trans_sets = NULL;
  int **acc_data_trans_sets; // indexed by data transfer set
  acc_data_trans_sets = get_acc_data_trans_sets(io_map, io_map_num_entries, &num_data_trans_sets, &num_acc_per_data_trans_sets);

  vsa->acc_data_trans_sets = acc_data_trans_sets;
  vsa->num_data_trans_sets = num_data_trans_sets;
  vsa->num_acc_per_data_trans_sets = num_acc_per_data_trans_sets;
  vsa->io_map = io_map;
  vsa->io_map_num_entries = io_map_num_entries;

#ifdef PSA_VSA_DEBUG
  acc_io_map_pretty_print(io_map, io_map_num_entries);
#endif
 
}

/* 
 * This function groups accesses into different transfer sets. 
 * All the access functions in the same tranasfer set share one common set of data transfer modules (including FIFOs)
 * The access functions are grouped into one set if:
 * - They share the same array name
 * - They share the same trans direction
 */
int **get_acc_data_trans_sets(struct stmt_access_io_pair **acc_io_map, int num_accs, int *num_sets, int **num_acc_per_set) {
  int **acc_sets = NULL;
  for (int i = 0; i < num_accs; i++) {
    PlutoAccess *acc = acc_io_map[i]->acc;
    if (*num_sets == 0) {
      *num_sets += 1;
      acc_sets = (int **)malloc(*num_sets * sizeof(int *));
      acc_sets[0] = (int *)malloc(1 * sizeof(int));
      acc_sets[0][0] = acc->sym_id;
      *num_acc_per_set = (int *)malloc(*num_sets * sizeof(int));
      *num_acc_per_set[0] = 1;
      // update acc_io_map
      acc_io_map[i]->trans_set = *num_sets - 1;
    } else {
      // search if there is any matching set
      int j;
      for (j = 0; j < *num_sets; j++) {
        PlutoAccess *acc_cmp = acc_io_map[acc_sets[j][0]]->acc;
        if (is_in_same_acc_trans_set(acc_io_map, acc, acc_cmp)) {
          *num_acc_per_set[j] += 1;
          acc_sets[j] = realloc(acc_sets[j], *num_acc_per_set[j] * sizeof(int));
          acc_sets[j][*num_acc_per_set[j] - 1] = acc->sym_id;
          // update acc_io_map
          acc_io_map[i]->trans_set = j;
          break;
        } 
      }
      if (j == *num_sets) {
        // allocate new sets;
        *num_sets += 1;
        acc_sets = realloc(acc_sets, *num_sets * sizeof(int *));
        acc_sets[*num_sets - 1] = (int *)malloc(1 * sizeof(int));
        acc_sets[*num_sets - 1][0] = acc->sym_id;        
        *num_acc_per_set = realloc(*num_acc_per_set, *num_sets * sizeof(int));
        (*num_acc_per_set)[*num_sets - 1] = 1;
        // update acc_io_map
        acc_io_map[i]->trans_set = *num_sets - 1;
      }
    }
  }

  return acc_sets;
}

/* 
 * This function examines if two accesses belong to the same transfer_set
 * The criteria are:
 * - The same array name
 * - The same trans direction
 */
bool is_in_same_acc_trans_set(struct stmt_access_io_pair **acc_io_map, const PlutoAccess *acc1, const PlutoAccess *acc2) {
  /* Array name */
  if (strcmp(acc1->name, acc2->name)) 
    return 0;
  /* Trans direction */
  if (!is_vec_equal(acc_io_map[acc1->sym_id]->L1_trans_dir, acc_io_map[acc2->sym_id]->L1_trans_dir))
    return 0;
  if (!is_vec_equal(acc_io_map[acc1->sym_id]->L2_trans_dir, acc_io_map[acc2->sym_id]->L2_trans_dir))
    return 0;
  if (!is_vec_equal(acc_io_map[acc1->sym_id]->L3_trans_dir, acc_io_map[acc2->sym_id]->L3_trans_dir))
    return 0;

  return 1;
}

/* Examine if the dependence is carried by space loops */
bool is_dep_carried_at_space_band(PlutoProg *prog, Dep *dep) {
  DepDir *dirvec = dep->dirvec;
  for (int h = 0; h < prog->num_hyperplanes; h++) {
    if (IS_PSA_SPACE_LOOP(prog->hProps[h].psa_type)) {
      if (dirvec[h] != DEP_ZERO) {
        return 1;
      }
    }
  }
  return 0;
}

/* Examine if the dependece is carried by time loops */
bool is_dep_carried_at_time_band(PlutoProg *prog, Dep *dep) {
  return !is_dep_carried_at_space_band(prog, dep);
}

/* Examine if the dependence is carried by array partitioning loops */
bool is_dep_carried_at_array_part_band(PlutoProg *prog, Dep *dep) {
  DepDir *dirvec = dep->dirvec;
  for (int h = 0; h < prog->num_hyperplanes; h++) {
    if (IS_PSA_ARRAY_PART_LOOP(prog->hProps[h].psa_type)) {
      if (dirvec[h] != DEP_ZERO) {
        return 1;
      }
    }
  }
  return 0;
}

/* Analyze the data transfer direction for L1 transfer modules */
Vec *get_L1_trans_dir(PlutoProg *prog, Dep *dep, VSA *vsa, struct stmt_access_io_pair *stmt_access_io) {
  Vec *dir = vec_alloc(vsa->space_band_width);
  int *disvec = dep->disvec;
  int space_hyp_start = vsa->array_part_band_width;

  if (stmt_access_io->L1_trans_type == EMBEDDED) {
    for (int h = space_hyp_start; h < space_hyp_start + dir->length; h++) {
      dir->val[h - space_hyp_start] = disvec[h];
    }
  } else if (stmt_access_io->L1_trans_type == SEPARATE){
    /* Create I/O manually */
    // Currently we will add (1) for 1D, and (1, 0) for 2D by default
    // This could possibly be exposed out as an knob
    for (int i = 0; i < dir->length; i++) {
      dir->val[i] = 0;
    }
    dir->val[0] = 1;
  }

  return dir;
}

/* 
 * Analyze the data transfer direction for L2 transfer modules 
 * 
 */
Vec *get_L2_trans_dir(PlutoProg *prog, Dep *dep, VSA *vsa, struct stmt_access_io_pair *stmt_access_io) {
  Vec *dir = vec_alloc(1);
  dir->val[0] = 1;

  return dir;
}

/* Analsyze the data transfer direction for L3 transfer modules */
Vec *get_L3_trans_dir(PlutoProg *prog, Dep *dep, VSA *vsa, struct stmt_access_io_pair *stmt_access_io) {
  Vec *dir = vec_alloc(1);
  dir->val[0] = 0;

  return dir;
}

void acc_io_map_pretty_print(struct stmt_access_io_pair **map, int num_entry) {
  for (int i = 0; i < 10 + 20 * 9 + 10 + 10; i++) {
    fprintf(stdout, "-");
  }
  fprintf(stdout, "\n");
  fprintf(stdout, "%10s|", "acc_id");
  fprintf(stdout, "%20s|", "L1_trans_bound");
  fprintf(stdout, "%20s|", "L1_trans_type");
  fprintf(stdout, "%20s|", "L1_trans_dir");
  fprintf(stdout, "%20s|", "L2_trans_bound");
  fprintf(stdout, "%20s|", "L2_trans_type");
  fprintf(stdout, "%20s|", "L2_trans_dir");
  fprintf(stdout, "%20s|", "L3_trans_bound");
  fprintf(stdout, "%20s|", "L3_trans_type");
  fprintf(stdout, "%20s|", "L3_trans_dir");
  fprintf(stdout, "%10s\n", "trans_set");

  for (int i = 0; i < num_entry; i++) {
    fprintf(stdout, "%10d|", i);
    fprintf(stdout, "%20s|", get_trans_bound_str(map[i]->L1_trans_bound));
    fprintf(stdout, "%20s|", get_trans_type_str(map[i]->L1_trans_type));
    fprintf(stdout, "%20s|", get_vec_str(map[i]->L1_trans_dir));
    fprintf(stdout, "%20s|", get_trans_bound_str(map[i]->L2_trans_bound));
    fprintf(stdout, "%20s|", get_trans_type_str(map[i]->L2_trans_type));
    fprintf(stdout, "%20s|", get_vec_str(map[i]->L2_trans_dir));
    fprintf(stdout, "%20s|", get_trans_bound_str(map[i]->L3_trans_bound));
    fprintf(stdout, "%20s|", get_trans_type_str(map[i]->L3_trans_type));
    fprintf(stdout, "%20s|", get_vec_str(map[i]->L3_trans_dir));
    fprintf(stdout, "%10d\n", map[i]->trans_set);
  }

  for (int i = 0; i < 10 + 20 * 9 + 10 + 10; i++) {
    fprintf(stdout, "-");
  }
  fprintf(stdout, "\n");
}

char *get_trans_bound_str(TransBound bound) {
  switch (bound) {
    case IN_BOUND:
      return "IN_BOUND";
      break;
    case OUT_BOUND:
      return "OUT_BOUND";
      break;
    case NO_BOUND:
      return "NO_BOUND";
      break;
    case UNKNOWN_BOUND:
      return "UNKNOWN_BOUND";
      break;
  }
}

char *get_trans_type_str(TransType type) {
  switch (type) {
    case EMBEDDED:
      return "EMBEDDED";
      break;
    case SEPARATE:
      return "SEPARATE";
      break;
    case UNKNOWN_TYPE:
      return "UNKNOWN_TYPE";
      break;
  }
}

/* Print out the contents of a vector in the form as (X, X) */
char *get_vec_str(Vec *v) {
  if (v == NULL || v->length == 0)
    return "";

  char* str = "(";
  for (int i = 0; i < v->length; i++) {
    if (i > 0) {
      str = concat(str, " ,");
    }
    char val[10];
    sprintf(val, "%d", v->val[i]);
    str = concat(str, val);
  }
  str = concat(str, ")");
  return str;
}

/* Compare if two vectors are equal */
bool is_vec_equal(const Vec *v1, const Vec *v2) {
  if (v1 == NULL || v2 == NULL)
    return 0;

  if (v1->length != v2->length)
    return 0;
  for (int i = 0; i < v1->length; i++) {
    if (v1->val[i] != v2->val[i])
      return 0;
  }

  return 1;
}

Vec *vec_alloc(int n) {
  Vec *vec = (Vec *)malloc(sizeof(Vec));
  vec->length = n;
  vec->val = (int *)malloc(n * sizeof(int));

  return vec;
}
