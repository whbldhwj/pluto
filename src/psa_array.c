/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <assert.h>
#include <math.h>

#include "pluto.h"
#include "math_support.h"
#include "constraints.h"
#include "psa_dep.h"
#include "psa_helpers.h"
#include "psa_knobs.h"

#include "osl/macros.h"
#include "osl/scop.h"
#include "osl/body.h"
#include "osl/relation_list.h"
#include "osl/extensions/arrays.h"
#include "osl/extensions/dependence.h"
#include "osl/extensions/loop.h"
#include "osl/extensions/pluto_unroll.h"
#include "osl/extensions/scatnames.h"

#include "cloog/cloog.h"

#include "candl/candl.h"
#include "candl/scop.h"
#include "candl/options.h"
#include "candl/dependence.h"

#include <isl/id.h>
#include <isl/map.h>
#include <isl/mat.h>
#include <isl/set.h>
#include <isl/space.h>
#include <isl/flow.h>
#include <isl/union_map.h>
#include <isl/val.h>

#include "pet.h"

/*
 * Return a clone of HyperplaneProperties
 */
//HyperplaneProperties *hyperplane_properties_dup(const HyperplaneProperties *hProps) {
//  HyperplaneProperties *nhProps = (HyperplaneProperties *)malloc(sizeof(HyperplaneProperties));
//  nhProps->dep_prop = hProps->dep_prop;
//  nhProps->type = hProps->type;
//  nhProps->band_num = hProps->band_num;
//  nhProps->unroll = hProps->unroll;
//  nhProps->prevec = hProps->prevec;
//
//  return nhProps;
//}

/*
 * Return a clone of dep.
 * This method relies on the num_hyperplanes from PlutoProg.
 */
Dep *pluto_dep_prog_dup(Dep *d, int num_hyperplanes) {
  int i;
  Dep *dep = malloc(sizeof(Dep));

  dep->id = d->id;
  dep->src = d->src;
  dep->dest = d->dest;
  dep->src_acc = pluto_access_dup(d->src_acc);
  dep->dest_acc = pluto_access_dup(d->dest_acc);
  dep->dpolytope = pluto_constraints_dup(d->dpolytope);
  dep->bounding_poly = pluto_constraints_dup(d->bounding_poly);

  dep->src_unique_dpolytope =
      d->src_unique_dpolytope ? pluto_constraints_dup(d->src_unique_dpolytope)
                              : NULL;

  dep->depsat_poly =
      d->depsat_poly ? pluto_constraints_dup(d->depsat_poly) : NULL;  
  dep->satvec = NULL; // TODO  
  if (d->satvec) {
    dep->satvec = (int *)malloc(num_hyperplanes * sizeof(int));
    for (i = 0; i < num_hyperplanes; i++) {
      dep->satvec[i] = d->satvec[i];
    }
  }
  dep->type = d->type;
  dep->satisfied = d->satisfied;
  dep->satisfaction_level = d->satisfaction_level;
  dep->dirvec = NULL; // TODO
  if (d->dirvec) {
    dep->dirvec = (DepDir *)malloc(num_hyperplanes * sizeof(DepDir));
    for (i = 0; i < num_hyperplanes; i++) {
      dep->dirvec[i] = d->dirvec[i];
    }
  }
  //dep->disvec
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] dep->id: %d\n", dep->id);
// #endif
  dep->disvec = NULL;
  if (d->disvec) {
// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] duplicate disvec.\n");
// #endif
    dep->disvec = (int *)malloc(num_hyperplanes * sizeof(int));
    for (i = 0; i < num_hyperplanes; i++) {
      dep->disvec[i] = d->disvec[i];
    }
  }
  dep->cst = d->cst ? pluto_constraints_dup(d->cst) : NULL;
  dep->bounding_cst =
      d->bounding_cst ? pluto_constraints_dup(d->bounding_cst) : NULL;

  return dep;
}

/* 
 * Return a clone of plutoOptions
 */
PlutoOptions *pluto_options_dup(const PlutoOptions *options) {
  PlutoOptions *noptions = pluto_options_alloc();  

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 6.1.\n");
// #endif  
  noptions->tile = options->tile;
  noptions->intratileopt = options->intratileopt;  
  noptions->diamondtile = options->diamondtile;
  noptions->pet = options->pet;
  noptions->dynschedule = options->dynschedule;
  noptions->dynschedule_graph = options->dynschedule_graph;
  noptions->dynschedule_graph_old = options->dynschedule_graph_old;  
  noptions->dyn_trans_deps_tasks = options->dyn_trans_deps_tasks;
  noptions->fulldiamondtile = options->fulldiamondtile;

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 6.2.\n");
// #endif    
  noptions->parallel = options->parallel;
  noptions->innerpar = options->innerpar;
  noptions->unroll = options->unroll;
  noptions->ufactor = options->ufactor;
  noptions->prevector = options->prevector;
  noptions->rar = options->rar;
  noptions->fuse = options->fuse;
  noptions->delayed_cut = options->delayed_cut;
  noptions->hybridcut = options->hybridcut;
  noptions->scancount = options->scancount;
  noptions->codegen_context = options->codegen_context;
  noptions->forceparallel = options->forceparallel;
  noptions->multipar = options->multipar;
  noptions->l2tile = options->l2tile;
  noptions->ft = options->ft;
  noptions->lt = options->lt;
  noptions->debug = options->debug;
  noptions->moredebug = options->moredebug;
  //noptions->quite = options->quite;
  noptions->identity = options->identity;
  noptions->bee = options->bee;
  noptions->cloogf = options->cloogf;
  noptions->cloogl = options->cloogl;
  noptions->cloogsh = options->cloogsh;
  noptions->cloogbacktrack = options->cloogbacktrack;
  noptions->isldep = options->isldep;
  noptions->candldep = options->candldep;
  noptions->isldepaccesswise = options->isldepaccesswise;
  noptions->isldepcoalesce = options->isldepcoalesce;
  noptions->lastwriter = options->lastwriter;
  noptions->nodepbound = options->nodepbound;
  noptions->coeff_bound = options->coeff_bound;
  noptions->scalpriv = options->scalpriv;
  noptions->silent = options->silent;
  noptions->readscop = options->readscop;
  noptions->pipsolve = options->pipsolve;
  noptions->islsolve = options->islsolve;
/* Jie Added - Start */  
  noptions->dsa = options->dsa;
/* Jie Added - End */
#ifdef GLPK  
  noptions->glpk = options->glpk;
#endif
#ifdef GUROBI
  noptions->gurobi = options->gurobi;
#endif  
#if defined GLPK || defined GUROBI
  noptions->lp = options->lp;
  noptions->dfp = options->dfp;
  noptions->ilp = options->ilp;
  noptions->lpcolour = options->lpcolour;
  noptions->scc_cluster = options->scc_cluster;
#endif  
  noptions->iss = options->iss;
  //noptions->out_file = strdup(options->out_file);
  noptions->time = options->time;
  noptions->flic = options->flic;

  return noptions;
}

/*
 * Return a clone of a graph
 */
Graph *graph_dup(const Graph *graph) {
  int i;

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 1.1.\n");
// #endif
  Graph *ngraph = graph_alloc(graph->nVertices);
  /* vertices */
  for (i = 0; i < ngraph->nVertices; i++) {
    ngraph->vertices[i].id = graph->vertices[i].id;
    ngraph->vertices[i].vn = graph->vertices[i].vn;
    ngraph->vertices[i].fn = graph->vertices[i].fn;
    ngraph->vertices[i].fcg_stmt_offset = graph->vertices[i].fcg_stmt_offset;
    ngraph->vertices[i].scc_id = graph->vertices[i].scc_id;
    ngraph->vertices[i].cc_id = graph->vertices[i].cc_id;
    ngraph->vertices[i].dom_id = graph->vertices[i].dom_id;
  }

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 1.2.\n");
// #endif
  /* Number of vertices that have already been coloured */
  ngraph->num_coloured_vertices = graph->num_coloured_vertices;

  /* Adjacency matrix */
  if (ngraph->adj)
    pluto_matrix_free(ngraph->adj);
  ngraph->adj = pluto_matrix_dup(graph->adj);
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 1.3.\n");
// #endif

  /* scc */
  ngraph->num_sccs = graph->num_sccs;
  ngraph->num_ccs = graph->num_ccs;
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] SCC number: %d\n", graph->num_sccs);
// #endif

  if (graph->num_sccs > 0) {
    if (ngraph->sccs)
      free(ngraph->sccs);
    ngraph->sccs = (Scc *)malloc(graph->num_sccs * sizeof(Scc));
    for (i = 0; i < ngraph->num_sccs; i++) {
      ngraph->sccs[i].size = graph->sccs[i].size;
      ngraph->sccs[i].max_dim = graph->sccs[i].max_dim;
      ngraph->sccs[i].id = graph->sccs[i].id;
      // TODO: vertices could be non-NULL
      ngraph->sccs[i].vertices = NULL;
      /*
      ngraph->sccs[i].vertices = (int *)malloc(ngraph->sccs[i].size * sizeof(int));
      for (j = 0; j < ngraph->sccs[i].size; j++) {
        if (graph->sccs[i].vertices[j] != NULL) {
          ngraph->sccs[i].vertices[j] = graph->sccs[i].vertices[j];
        } else {
          fprintf(stdout, "[Debug] Pointer Null!\n");
        } 
      } 
      */
      ngraph->sccs[i].is_parallel = graph->sccs[i].is_parallel;
      // TODO: sol could be non-NULL
      ngraph->sccs[i].sol = NULL;
      /*
      ngraph->sccs[i].sol = (double *)malloc(ngraph->sccs[i].size * sizeof(double));
      for (j = 0; j < ngraph->sccs[i].size; j++) {
        ngraph->sccs[i].sol[j] = graph->sccs[i].sol[j];
      }
      */
      ngraph->sccs[i].fcg_scc_offset = graph->sccs[i].fcg_scc_offset;
      ngraph->sccs[i].is_scc_coloured = graph->sccs[i].is_scc_coloured;
      ngraph->sccs[i].has_parallel_hyperplane = graph->sccs[i].has_parallel_hyperplane;    
    }
  }

  if (graph->num_ccs > 0) {
    if (ngraph->ccs)
      free(ngraph->ccs);
    ngraph->ccs = (CC *)malloc(graph->num_ccs * sizeof(CC));
    for (i = 0; i < ngraph->num_ccs; i++) {
      ngraph->ccs[i].size = graph->ccs[i].size;
      ngraph->ccs[i].max_dim = graph->ccs[i].max_dim;
      ngraph->ccs[i].id = graph->ccs[i].id;
      if (graph->ccs[i].vertices) {
        ngraph->ccs[i].vertices = (int *)malloc(ngraph->ccs[i].size * sizeof(int));
        for (int j = 0; j < graph->ccs[i].size; j++) {
          ngraph->ccs[i].vertices[j] = graph->ccs[i].vertices[j];
        }
      }
      ngraph->ccs[i].dom_id = graph->ccs[i].dom_id;
      ngraph->ccs[i].type = graph->ccs[i].type;
    }
  }

  ngraph->to_be_rebuilt = graph->to_be_rebuilt;

  return ngraph;
}

/*
 * Return a clone of a Pluto program
 */
PlutoProg *pluto_prog_dup(const PlutoProg *prog) {
  int i;
  PlutoProg *new_prog = (PlutoProg *)pluto_prog_alloc();

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] num_hyperplanes: %d\n", prog->num_hyperplanes);
// #endif

  /* Array of statements */  
  new_prog->nstmts = prog->nstmts;
  new_prog->stmts = (Stmt **)malloc(prog->nstmts * sizeof(Stmt *));
  for (i = 0; i < prog->nstmts; i++) {
	  new_prog->stmts[i] = pluto_stmt_dup(prog->stmts[i]);
  }

  /* Array of dependences */
  new_prog->ndeps = prog->ndeps;
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] deps dup.\n");
// #endif
  new_prog->deps = (Dep **)malloc(prog->ndeps * sizeof(Dep *));
  for (i = 0; i < prog->ndeps; i++) {
	  new_prog->deps[i] = pluto_dep_prog_dup(prog->deps[i], prog->num_hyperplanes);
  }

  /* Array of dependences */
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] transdeps dup.\n");
// #endif
  new_prog->ntransdeps = prog->ntransdeps;
  new_prog->transdeps = (Dep **)malloc(prog->ntransdeps * sizeof(Dep *));
  for (i = 0; i < prog->ntransdeps; i++) {
    new_prog->transdeps[i] = pluto_dep_prog_dup(prog->transdeps[i], prog->num_hyperplanes);
  }

  /* Array of data variable names */
  new_prog->num_data = prog->num_data;
  new_prog->data_names = (char **)malloc(prog->num_data * sizeof(char *));
  for (i = 0; i < prog->num_data; i++) {
    new_prog->data_names[i] = strdup(prog->data_names[i]);
  }

  /* Parameters */
  new_prog->npar = prog->npar;
  new_prog->params = (char **)malloc(prog->npar * sizeof(char *));
  for (i = 0; i < prog->npar; i++) {
    new_prog->params[i] = strdup(prog->params[i]);
  }

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 3.\n");
// #endif

  /* Number of hyperplanes */
  new_prog->num_hyperplanes = prog->num_hyperplanes;

  /* Data dependences graph of the program */
  if (prog->ddg != NULL) {
    if (new_prog->ddg)
      graph_free(new_prog->ddg);
    new_prog->ddg = graph_dup(prog->ddg);
  } else
    new_prog->ddg = NULL;

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 4.\n");
// #endif
  /* Fusion conflict graph of the program */
// #ifdef JIE_DEBUG
//   if (prog->fcg == NULL) {
//     fprintf(stdout, "[Debug] FCG null!\n");
//   }
// #endif
  if (prog->fcg != NULL)
    new_prog->fcg = graph_dup(prog->fcg);
  else
    new_prog->fcg = NULL;

  if (prog->adg != NULL) {
    if (new_prog->adg)
      graph_free(new_prog->adg);
    new_prog->adg = graph_dup(prog->adg);
  }
  else
    new_prog->adg = NULL;

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 5.\n");
// #endif
  /* Options for Pluto */
  new_prog->options = pluto_options_dup(prog->options);  
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 6.\n");
// #endif
  /* Hyperplane properities */
  new_prog->hProps = (HyperplaneProperties *)malloc(new_prog->num_hyperplanes 
      *sizeof(HyperplaneProperties));
  for (i = 0; i < new_prog->num_hyperplanes; i++) {
    new_prog->hProps[i].dep_prop = prog->hProps[i].dep_prop;
    new_prog->hProps[i].type = prog->hProps[i].type;
    new_prog->hProps[i].psa_type = prog->hProps[i].psa_type;
    new_prog->hProps[i].band_num = prog->hProps[i].band_num;
    new_prog->hProps[i].unroll = prog->hProps[i].unroll;
    new_prog->hProps[i].prevec = prog->hProps[i].prevec;
//    new_prog->hProps[i] = hyperplane_properties_dup(prog->hProps[i]);
  }
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 7.\n");
// #endif
  /* Max (original) domain dimensionality */
  new_prog->nvar = prog->nvar;

  /* Param context */
  if (prog->context != NULL) {
    if (new_prog->context != NULL)
      pluto_constraints_free(new_prog->context);
    new_prog->context = pluto_constraints_dup(prog->context);
  }
  if (prog->decls != NULL) {
    if (new_prog->decls != NULL)
      free(new_prog->decls);
    new_prog->decls = strdup(prog->decls);
  }
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 8.\n");
// #endif
  /* Codegen context */
  if (prog->codegen_context != NULL) {
    if (new_prog->codegen_context != NULL)
      pluto_constraints_free(new_prog->codegen_context);
    new_prog->codegen_context = pluto_constraints_dup(prog->codegen_context);
  }
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 9.\n");
// #endif
  /* Temp autotransform data*/
  if (prog->globcst != NULL)
    new_prog->globcst = pluto_constraints_dup(prog->globcst);
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 10.\n");
// #endif
  /* Hyperplane that was replaced in case concurrent start had been found */
  new_prog->evicted_hyp_pos = prog->evicted_hyp_pos;

  /* scop */
  new_prog->scop = prog->scop;

  /* Number of outermost parallel dimensions to be parameterized */
  new_prog->num_parameterized_loops = prog->num_parameterized_loops;

  /* Number of statements to be colored */
  new_prog->num_stmts_to_be_coloured = prog->num_stmts_to_be_coloured;

  /* Boonlean array indicating whether a dimension is scaled */
  if (prog->scaled_dims != NULL) {
    new_prog->scaled_dims = (int *)malloc(new_prog->nvar * sizeof(int));
    for (i = 0; i < new_prog->nvar; i++) {
      new_prog->scaled_dims[i] = prog->scaled_dims[i];
    }
  }
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 11.\n");
// #endif
  /* Total number of statements coloured per dimension in the FCG */
  if (prog->total_coloured_stmts != NULL) {
    new_prog->total_coloured_stmts = (int *)malloc(new_prog->nvar * sizeof(int));
    for (i = 0;i < new_prog->nvar; i++) {
      new_prog->total_coloured_stmts[i] = prog->total_coloured_stmts[i];
    }
  }
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 12.\n");
// #endif
  /* Total number of coloured dimensions that have been coloured for all statements */
  new_prog->coloured_dims = prog->coloured_dims;

  /* Used to store constraint solving times */
  new_prog->mipTime = prog->mipTime;
  new_prog->ilpTime = prog->ilpTime;
  new_prog->cst_solve_time = prog->cst_solve_time;
  new_prog->cst_write_time = prog->cst_write_time;
  new_prog->scaling_cst_sol_time = prog->scaling_cst_sol_time;
  new_prog->skew_time = prog->skew_time;

  new_prog->fcg_const_time = prog->fcg_const_time;
  new_prog->fcg_colour_time = prog->fcg_colour_time;
  new_prog->fcg_dims_scale_time = prog->fcg_dims_scale_time;
  new_prog->fcg_update_time = prog->fcg_update_time;
  new_prog->fcg_cst_alloc_time = prog->fcg_cst_alloc_time;

  new_prog->num_lp_calls = prog->num_lp_calls;

  /* Systolic array dimension */
  new_prog->array_dim = prog->array_dim;

  /* Systolic array partition dimension */
  new_prog->array_part_dim = prog->array_part_dim;

  /* Systolic array num_rows, num_cols */
  new_prog->array_nrow = prog->array_nrow;
  new_prog->array_ncol = prog->array_ncol;
  
  /* Systolic array row/col interleave factor */
  new_prog->array_il_enable = prog->array_il_enable;
  new_prog->array_il_factor[0] = prog->array_il_factor[0];
  new_prog->array_il_factor[1] = prog->array_il_factor[1];

  /* SIMD */
  new_prog->array_simd_factor = prog->array_simd_factor;

  /* Systolic array interior I/O elimination */
  new_prog->array_io_enable = prog->array_io_enable;

  // Reassociate the dep and stmts
  // This step is vital for following stages which reply on comparing 
  // dep's acc with stmt's acc directly
  reassociate_dep_stmt_acc(new_prog);

  return new_prog;
}

int get_dep_distance_isl(const Dep *dep, const PlutoProg *prog, int level) {
  isl_ctx *ctx = isl_ctx_alloc();
  isl_set *dpoly_set = isl_set_from_pluto_constraints(dep->dpolytope, ctx);
  int npar = prog->npar;
  int src = dep->src;
  int dest = dep->dest;
  Stmt *src_stmt = prog->stmts[src];
  Stmt *dest_stmt = prog->stmts[dest];

  int src_dim = src_stmt->dim;
  int dest_dim = dest_stmt->dim;

  // create isl_map for dependece distance
  // dis = \phi(dest) - \phi(src)
  // [dis | src_iter | dest_iter | param | 1]
  PlutoConstraints *dis_cst = pluto_constraints_alloc(1, 1 + src_dim + dest_dim + npar + 1);
  dis_cst->is_eq[0] = 1;
  dis_cst->val[0][0] = 1;
  for (int i = 1; i < src_dim + 1; i++)
    dis_cst->val[0][i] = src_stmt->trans->val[level][i - 1];
  for (int i = src_dim + 1; i < src_dim + 1 + dest_dim; i++)
    dis_cst->val[0][i] = -dest_stmt->trans->val[level][i - src_dim - 1];
  for (int i = src_dim + 1 + dest_dim; i < src_dim + 1 + dest_dim + npar; i++) {
    dis_cst->val[0][i] = src_stmt->trans->val[level][i - 1 - dest_dim] -
          dest_stmt->trans->val[level][i - 1 - src_dim];
  }
  dis_cst->val[0][src_dim + 1 + dest_dim + npar] = src_stmt->trans->val[level][src_dim + npar] - 
          dest_stmt->trans->val[level][dest_dim + npar];
  dis_cst->nrows = 1;

  isl_map *dis_map = isl_map_from_pluto_constraints(dis_cst, ctx, src_dim + dest_dim, 1, npar);
  isl_set *dis_set = isl_set_apply(dpoly_set, dis_map);

  isl_printer *printer_str = isl_printer_to_str(ctx);
  isl_printer_print_set(printer_str, dis_set);
  char *tmp_str = isl_printer_get_str(printer_str);
  isl_aff *dis_aff = isl_aff_read_from_str(ctx, tmp_str);
  isl_val *dis_val = isl_aff_get_constant_val(dis_aff);
  int dis = isl_val_get_num_si(dis_val);

  isl_printer_free(printer_str);
  free(tmp_str);
  isl_aff_free(dis_aff);
  isl_val_free(dis_val);
  pluto_constraints_free(dis_cst);
  isl_set_free(dis_set);
  isl_ctx_free(ctx);

  return dis;
}

/* 
 * Distance vector component at level 'level'
 * NOTE: deprecated
 */
DepDis get_dep_distance(const Dep *dep, const PlutoProg *prog, int level) {
  PlutoConstraints *cst;
  int j, src, dest;

  int npar = prog->npar;
  Stmt **stmts = prog->stmts;

  src = dep->src;
  dest = dep->dest;

  Stmt *src_stmt = stmts[dep->src];
  Stmt *dest_stmt = stmts[dep->dest];

  int src_dim = src_stmt->dim;
  int dest_dim = dest_stmt->dim;

  assert(level < stmts[src]->trans->nrows);
  assert(level < stmts[dest]->trans->nrows);

  cst = pluto_constraints_alloc(2 * (2 + dep->dpolytope->nrows),
                                (src_dim + dest_dim) + npar + 1);

  /* Check for zero
   *
   * To test \phi(dest) - \phi(src) = 0, we try
   * 
   * \phi(dest) - \phi(src) >= 1 and
   * \phi(dest) - \phi(src) <= -1
   */
  cst->is_eq[0] = 0;
  for (j = 0; j < src_dim; j++) {
    cst->val[0][j] = -stmts[src]->trans->val[level][j];
  }
  for (j = src_dim; j < src_dim + dest_dim; j++) {
    cst->val[0][j] = stmts[dest]->trans->val[level][j - src_dim];
  }
  for (j = src_dim + dest_dim; j < src_dim + dest_dim + npar; j++) {
    cst->val[0][j] = -stmts[src]->trans->val[level][j - dest_dim] +
                    stmts[dest]->trans->val[level][j - src_dim];
  }
  cst->val[0][src_dim + dest_dim + npar] =
      -stmts[src]->trans->val[level][src_dim + npar] +
      stmts[dest]->trans->val[level][dest_dim + npar] - 1;
  cst->nrows = 1;
  
  pluto_constraints_add(cst, dep->dpolytope);

  bool is_empty = pluto_constraints_is_empty(cst);

  if (is_empty) {
    /* \phi(dest) - \phi(src) <= -1 */
    for (j = 0; j < src_dim; j++) {
      cst->val[0][j] = stmts[src]->trans->val[level][j];
    }
    for (j = src_dim; j < src_dim + dest_dim; j++) {
      cst->val[0][j] = -stmts[dest]->trans->val[level][j - src_dim];
    }
    for (j = src_dim + dest_dim; j < src_dim + dest_dim + npar; j++) {
      cst->val[0][j] = stmts[src]->trans->val[level][j - dest_dim] - 
                       stmts[dest]->trans->val[level][j - src_dim];                       
    }
    cst->val[0][src_dim + dest_dim + npar] = 
        stmts[src]->trans->val[level][src_dim + npar] - 
        stmts[dest]->trans->val[level][dest_dim + npar] - 1;
    cst->nrows = 1;

    pluto_constraints_add(cst, dep->dpolytope);

    is_empty = pluto_constraints_is_empty(cst);

    /* If no solution exists, all points satisfy \phi(dest) - \phi(src) = 0 */
    if (is_empty) {
      pluto_constraints_free(cst);
      return DEP_DIS_ZERO;
    }    
  }

  /* Check for plus one
   *
   * To test \phi(dest) - \phi(src) = 1, we try
   * 
   * \phi(dest) - \phi(src) >= 2 and
   * \phi(dest) - \phi(src) <= 0
   */  
  cst->is_eq[0] = 0;
  for (j = 0; j < src_dim; j++) {
    cst->val[0][j] = -stmts[src]->trans->val[level][j];
  }
  for (j = src_dim; j < src_dim + dest_dim; j++) {
    cst->val[0][j] = stmts[dest]->trans->val[level][j - src_dim];
  }
  for (j = src_dim + dest_dim; j < src_dim + dest_dim + npar; j++) {
    cst->val[0][j] = -stmts[src]->trans->val[level][j - dest_dim] +
                    stmts[dest]->trans->val[level][j - src_dim];
  }
  cst->val[0][src_dim + dest_dim + npar] =
      -stmts[src]->trans->val[level][src_dim + npar] +
      stmts[dest]->trans->val[level][dest_dim + npar] - 2;
  cst->nrows = 1;
  
  pluto_constraints_add(cst, dep->dpolytope);

  is_empty = pluto_constraints_is_empty(cst);

  if (is_empty) {
    /* \phi(dest) - \phi(src) <= 0 */
    for (j = 0; j < src_dim; j++) {
      cst->val[0][j] = stmts[src]->trans->val[level][j];
    }
    for (j = src_dim; j < src_dim + dest_dim; j++) {
      cst->val[0][j] = -stmts[dest]->trans->val[level][j - src_dim];
    }
    for (j = src_dim + dest_dim; j < src_dim + dest_dim + npar; j++) {
      cst->val[0][j] = stmts[src]->trans->val[level][j - dest_dim] - 
                       stmts[dest]->trans->val[level][j - src_dim];                       
    }
    cst->val[0][src_dim + dest_dim + npar] = 
        stmts[src]->trans->val[level][src_dim + npar] - 
        stmts[dest]->trans->val[level][dest_dim + npar];
    cst->nrows = 1;

    pluto_constraints_add(cst, dep->dpolytope);

    is_empty = pluto_constraints_is_empty(cst);

    /* If no solution exists, all points satisfy \phi(dest) - \phi(src) = 1 */
    if (is_empty) {
      pluto_constraints_free(cst);
      return DEP_DIS_PLUS_ONE;
    }    
  }

  /* Check for minus one
   *
   * To test \phi(dest) - \phi(src) = -1, we try
   * 
   * \phi(dest) - \phi(src) >= 0 and
   * \phi(dest) - \phi(src) <= -2
   */  
  cst->is_eq[0] = 0;
  for (j = 0; j < src_dim; j++) {
    cst->val[0][j] = -stmts[src]->trans->val[level][j];
  }
  for (j = src_dim; j < src_dim + dest_dim; j++) {
    cst->val[0][j] = stmts[dest]->trans->val[level][j - src_dim];
  }
  for (j = src_dim + dest_dim; j < src_dim + dest_dim + npar; j++) {
    cst->val[0][j] = -stmts[src]->trans->val[level][j - dest_dim] +
                    stmts[dest]->trans->val[level][j - src_dim];
  }
  cst->val[0][src_dim + dest_dim + npar] =
      -stmts[src]->trans->val[level][src_dim + npar] +
      stmts[dest]->trans->val[level][dest_dim + npar];
  cst->nrows = 1;
  
  pluto_constraints_add(cst, dep->dpolytope);

  is_empty = pluto_constraints_is_empty(cst);

  if (is_empty) {
    /* \phi(dest) - \phi(src) <= -2 */
    for (j = 0; j < src_dim; j++) {
      cst->val[0][j] = stmts[src]->trans->val[level][j];
    }
    for (j = src_dim; j < src_dim + dest_dim; j++) {
      cst->val[0][j] = -stmts[dest]->trans->val[level][j - src_dim];
    }
    for (j = src_dim + dest_dim; j < src_dim + dest_dim + npar; j++) {
      cst->val[0][j] = stmts[src]->trans->val[level][j - dest_dim] - 
                       stmts[dest]->trans->val[level][j - src_dim];                       
    }
    cst->val[0][src_dim + dest_dim + npar] = 
        stmts[src]->trans->val[level][src_dim + npar] - 
        stmts[dest]->trans->val[level][dest_dim + npar] - 2;
    cst->nrows = 1;

    pluto_constraints_add(cst, dep->dpolytope);

    is_empty = pluto_constraints_is_empty(cst);

    /* If no solution exists, all points satisfy \phi(dest) - \phi(src) = -1 */
    if (is_empty) {
      pluto_constraints_free(cst);
      return DEP_DIS_MINUS_ONE;
    }        
  }  

  pluto_constraints_free(cst);
  /* Neither ZERO, nor PLUS ONE, nor MINUS ONE, has to be STAR */
  return DEP_STAR;    
}

void psa_compute_dep_distances_isl(PlutoProg *prog) {
  int level;

  Dep **deps = prog->deps;
  Dep **transdeps = prog->transdeps;

  /* Clear invalid pointer value in transdeps */
  for (int i = 0; i < prog->ntransdeps; i++) {
    transdeps[i]->disvec = NULL;
  }

  for (int i = 0; i < prog->ndeps; i++) {
    if (deps[i]->disvec != NULL) {
      free(deps[i]->disvec);
    }
    deps[i]->disvec = (int *)malloc(prog->num_hyperplanes * sizeof(int));
    printf("dep %d src: %d dest: %d type: %d\n", i, deps[i]->src, deps[i]->dest, deps[i]->type);
    for (level = 0; level < prog->num_hyperplanes; level++) {      
       deps[i]->disvec[level] = get_dep_distance_isl(deps[i], prog, level);
       printf("%d\n", deps[i]->disvec[level]);
    }
  }
}

/* 
 * Compute the dependence distance for each mapped hyperplane.
 * NOTE: deprecated
 */
//void psa_compute_dep_distances(PlutoProg *prog) {
//  int level;
//
//  Dep **deps = prog->deps;
//  Dep **transdeps = prog->transdeps;
//
//  /* Clear invalid pointer value in transdeps */
//  for (int i = 0; i < prog->ntransdeps; i++) {
//    //if (transdeps[i]->disvec != NULL) {
//    //  free(transdeps[i]->disvec);
//    //}
//    transdeps[i]->disvec = NULL;
//  }
//
//  for (int i = 0; i < prog->ndeps; i++) {    
//    if (deps[i]->disvec != NULL) {
//      free(deps[i]->disvec);
//    }
//    deps[i]->disvec = (int *)malloc(prog->num_hyperplanes * sizeof(int));
//    for (level = 0; level < prog->num_hyperplanes; level++) {
//      deps[i]->disvec[level] = get_dep_distance(deps[i], prog, level);
//    }
//// #ifdef JIE_DEBUG
////     Dep *dep = deps[i];
////     fprintf(stdout, "[Debug] id: %d\n", dep->id);
////     fprintf(stdout, "[Debug] type: %d\n", dep->type);
////     fprintf(stdout, "[Debug] name: %s\n", dep->src_acc->name);
////     PlutoConstraints *dpolytope = dep->dpolytope;
////     pluto_constraints_pretty_print(stdout, dpolytope);
////     for (level = 0; level < prog->num_hyperplanes; level++) {
////       fprintf(stdout, "[Debug] dep dis %d: %c\n", level, deps[i]->disvec[level]);
////     }
//// #endif
//  }
//}

/* 
 * Generate synchronized array 
 * -- time loop --
 * -- space loop --
 * We first permute the space loop innermost.
 * Next, we exmaine all dependences, for dependences which is strongly satisfied below the 
 * time loop, the first space loop that strongly satisfied the dep will be added to the last 
 * time loop.
 * */
PlutoProg **sa_candidates_generation_band_sync(Band *band, int array_dim, 
              PlutoProg *prog, int *nprogs) {
  PlutoProg **progs = NULL;
  unsigned i, j, k, nloops;

  Ploop **loops;

  int firstD = band->loop->depth;
  int lastD = band->loop->depth + band->width - 1;

  /* Select loops that carried dependence with distance less equal to 1 */
  int *is_space_loop = (int *)malloc(band->width * sizeof(int));
  for (i = firstD; i < firstD + band->width; i++) {    
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      assert(dep->disvec != NULL);
      if (!(dep->disvec[i] == 0 || dep->disvec[i] == 1)) {
        break;
      }
    }
    is_space_loop[i - firstD] = (j == prog->ndeps);
  }

  /* Perform loop permutation to generate all candidate variants */
  if (array_dim == 1) {
    for (i = firstD; i < firstD + band->width; i++) {
      if (is_space_loop[i - firstD]) {
        PlutoProg *new_prog = pluto_prog_dup(prog);
        /* As new prog is generated, we will need to generate new bands correspondingly */
        /* Make the loop i the innermost loop */
        unsigned d;
        for (d = i; d < lastD; d++) {
          pluto_interchange(new_prog, d, d + 1);
        }
        
        pluto_compute_dep_directions(new_prog);
        pluto_compute_dep_satisfaction(new_prog);

        /* Perform loop skewing */
//        for (int d = 0; d < prog->ndeps; d++) {
//          Dep *dep = prog->deps[d];
//          if (dep->satisfaction_level > lastD - array_dim && dep->satisfaction_level <= lastD) {
//            /* This dependency is first strongly satisfied in the space band,
//             * we will add the space loop to the last time loop */
//            psa_add_loops(new_prog, lastD - array_dim, dep->satisfaction_level);
//
//            pluto_compute_dep_directions(new_prog);
//            pluto_compute_dep_satisfaction(new_prog);
//          }      
//        }
        for (int level = 0; level < array_dim; level++) {
          psa_add_loops(new_prog, lastD - array_dim, level + lastD - array_dim + 1);
        }
        pluto_compute_dep_directions(new_prog);
        pluto_compute_dep_satisfaction(new_prog);
        psa_compute_dep_distances_isl(new_prog);

        /* Update psa_hyp_type */
        psa_detect_hyperplane_types(new_prog, array_dim, 0);
        psa_detect_hyperplane_types_stmtwise(new_prog, array_dim, 0);

        /* Update array dim */
        new_prog->array_dim = array_dim;
        // new_prog->array_part_dim = band->width;
        new_prog->array_part_dim = 0;

        /* Add the new invariant to the list */
        if (progs) {
          progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg *));
          progs[*nprogs] = new_prog;
          *nprogs = *nprogs + 1;
        } else {
          progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg *));
          progs[*nprogs] = new_prog;
          *nprogs = *nprogs + 1;
        }
      }
    }    
  } else if (array_dim == 2) {
    for (i = firstD; i < firstD + band->width; i++) {
      if (is_space_loop[i]) {
        for (j = i + 1; j < firstD + band->width; j++) {
          if (is_space_loop[j]) {
            PlutoProg *new_prog = pluto_prog_dup(prog);
            /* As new prog is generated, we will need to generate new bands correspondingly */
            /* Make the loop i, j the innermost loops */
            unsigned d;
            for (d = i; d < lastD; d++) {
              pluto_interchange(new_prog, d, d + 1);
            }
            for (d = j - 1; d < lastD; d++) {
              pluto_interchange(new_prog, d, d + 1);
            }

            pluto_compute_dep_directions(new_prog);
            pluto_compute_dep_satisfaction(new_prog);

            /* Perform loop skewing */
//            for (int d = 0; d < prog->ndeps; d++) {
//              Dep *dep = prog->deps[d];
//              if (dep->satisfaction_level > lastD - array_dim && dep->satisfaction_level <= lastD) {
//                /* This dependency is first strongly satisfied in the space band,
//                 * we will add the space loop to the last time loop */
//                psa_add_loops(new_prog, lastD - array_dim, dep->satisfaction_level);
//
//                pluto_compute_dep_directions(new_prog);
//                pluto_compute_dep_satisfaction(new_prog);
//              }      
//            }
            for (int level = 0; level < array_dim; level++) {
              psa_add_loops(new_prog, lastD - array_dim, level + lastD - array_dim + 1);
            }
            pluto_compute_dep_directions(new_prog);
            pluto_compute_dep_satisfaction(new_prog);
            psa_compute_dep_distances_isl(new_prog);

            /* Update psa_hyp_type */
            psa_detect_hyperplane_types(new_prog, array_dim, 0);
            psa_detect_hyperplane_types_stmtwise(new_prog, array_dim, 0);

            /* Update array dim */
            new_prog->array_dim = array_dim;
            // new_prog->array_part_dim = band->width;
            new_prog->array_part_dim = 0;

            /* Add the new invariant to the list */
            if (progs) {
              progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg *));
              progs[*nprogs] = new_prog;
              *nprogs = *nprogs + 1;
            } else {
              progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg *));
              progs[*nprogs] = new_prog;
              *nprogs = *nprogs + 1;
            }
          }
        }
      }
    }
  } else if (array_dim == 3) {
    for (i = firstD; i < firstD + band->width; i++) {
      if (is_space_loop[i]) {
        for (j = i + 1; j < firstD + band->width; j++) {
          if (is_space_loop[j]) {
            for (k = j + 1; k < firstD + band->width; k++) {
              if (is_space_loop[k]) {
                PlutoProg *new_prog = pluto_prog_dup(prog);
                /* As new prog is generated, we will need to generate new bands correspondingly */
                /* Make the loop i, j, k the innermost loops */
                unsigned d;
                for (d = i; d < lastD; d++) {
                  pluto_interchange(new_prog, d, d + 1);
                }
                for (d = j - 1; d < lastD; d++) {
                  pluto_interchange(new_prog, d, d + 1);
                }
                for (d = k - 2; d < lastD; d++) {
                  pluto_interchange(new_prog, d, d + 1);
                }

                pluto_compute_dep_directions(new_prog);
                pluto_compute_dep_satisfaction(new_prog);

                /* Perform loop skewing */
//                for (int d = 0; d < prog->ndeps; d++) {
//                  Dep *dep = prog->deps[d];
//                  if (dep->satisfaction_level > lastD - array_dim && dep->satisfaction_level <= lastD) {
//                    /* This dependency is first strongly satisfied in the space band,
//                     * we will add the space loop to the last time loop */
//                    psa_add_loops(new_prog, lastD - array_dim, dep->satisfaction_level);
//
//                    pluto_compute_dep_directions(new_prog);
//                    pluto_compute_dep_satisfaction(new_prog);
//                  }      
//                } 
                for (int level = 0; level < array_dim; level++) {
                  psa_add_loops(new_prog, lastD - array_dim, level + lastD - array_dim + 1);
                }
                pluto_compute_dep_directions(new_prog);
                pluto_compute_dep_satisfaction(new_prog); 
                psa_compute_dep_distances_isl(new_prog);

                /* Update psa_hyp_type */
                psa_detect_hyperplane_types(new_prog, array_dim, 0);
                psa_detect_hyperplane_types_stmtwise(new_prog, array_dim, 0);
    
                /* Update array dim */
                new_prog->array_dim = array_dim;
                // new_prog->array_part_dim = band->width;
                new_prog->array_part_dim = 0;
    
                /* Add the new invariant to the list */
                if (progs) {
                  progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg *));
                  progs[*nprogs] = new_prog;
                  *nprogs = *nprogs + 1;
                } else {
                  progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg *));
                  progs[*nprogs] = new_prog;
                  *nprogs = *nprogs + 1;
                }
              }
            }
          }
        }
      }
    }
  }

  free(is_space_loop);
  return progs;
}

/* Generate asynchronized array 
 * -- space --
 *  -- time --
 */
PlutoProg **sa_candidates_generation_band_async(Band *band, int array_dim, 
              PlutoProg *prog, int *nprogs) {
  PlutoProg **progs = NULL;
  unsigned i, j, k, nloops;

  Ploop **loops;

  int firstD = band->loop->depth;
  int lastD = band->loop->depth + band->width - 1;

  /* Select loops that carried dependence with distance less equal to 1 */
  int *is_space_loop = (int *)malloc(band->width * sizeof(int));
  for (i = firstD; i < firstD + band->width; i++) {    
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      assert(dep->disvec != NULL);
      if (!(dep->disvec[i] == 0 || dep->disvec[i] == 1)) {
        break;
      }
    }
    is_space_loop[i - firstD] = (j == prog->ndeps);
  }

  /* Perform loop permutation to generate all candidate variants */
  if (array_dim == 1) {
    for (i = firstD; i < firstD + band->width; i++) {
      if (is_space_loop[i - firstD]) {
        PlutoProg *new_prog = pluto_prog_dup(prog);
        /* As new prog is generated, we will need to generate new bands correspondingly */
        //Band **new_bands;
        //unsigned new_nbands;
        //new_bands = pluto_get_outermost_permutable_bands(new_prog, &new_nbands);

        //Ploop *loop = new_bands[0]->loop;
        /* Make the loop i the outermost loop */
        unsigned d;
        for (d = i; d > firstD; d--) {
          pluto_interchange(new_prog, d, d - 1);
        }

        pluto_compute_dep_directions(new_prog);
        pluto_compute_dep_satisfaction(new_prog);
        psa_compute_dep_distances_isl(new_prog);

        /* Update psa_hyp_type */
        psa_detect_hyperplane_types(new_prog, array_dim, 0);
        psa_detect_hyperplane_types_stmtwise(new_prog, array_dim, 0);

        /* Update array dim */
        new_prog->array_dim = array_dim;
        // new_prog->array_part_dim = band->width;
        new_prog->array_part_dim = 0;

        /* Add the new invariant to the list */
        if (progs) {
          progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg *));
          progs[*nprogs] = new_prog;
          *nprogs = *nprogs + 1;
        } else {
          progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg *));
          progs[*nprogs] = new_prog;
          *nprogs = *nprogs + 1;
        }
      }
    }    
  } else if (array_dim == 2) {
    for (i = firstD; i < firstD + band->width; i++) {
      if (is_space_loop[i]) {
        for (j = i + 1; j < firstD + band->width; j++) {
          if (is_space_loop[j]) {
            PlutoProg *new_prog = pluto_prog_dup(prog);
            /* As new prog is generated, we will need to generate new bands correspondingly */
            //Band **new_bands;
            //unsigned new_nbands;
            //new_bands = pluto_get_outermost_permutable_bands(new_prog, &new_nbands);

            //Ploop *loop = new_bands[0]->loop;
            /* Make the loop i, j the outermost loops */
            unsigned d;
            for (d = j; d > firstD; d--) {
              pluto_interchange(new_prog, d, d - 1);
            }
            for (d = i + 1; d > firstD; d--) {
              pluto_interchange(new_prog, d, d - 1);
            }

            pluto_compute_dep_directions(new_prog);
            pluto_compute_dep_satisfaction(new_prog);
            psa_compute_dep_distances_isl(new_prog);

            /* Update psa_hyp_type */
            psa_detect_hyperplane_types(new_prog, array_dim, 0);
            psa_detect_hyperplane_types_stmtwise(new_prog, array_dim, 0);

            /* Update array dim */
            new_prog->array_dim = array_dim;
            // new_prog->array_part_dim = band->width;
            new_prog->array_part_dim = 0;

            /* Add the new invariant to the list */
            if (progs) {
              progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg *));
              progs[*nprogs] = new_prog;
              *nprogs = *nprogs + 1;
            } else {
              progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg *));
              progs[*nprogs] = new_prog;
              *nprogs = *nprogs + 1;
            }
          }
        }
      }
    }
  } else if (array_dim == 3) {
    for (i = firstD; i < firstD + band->width; i++) {
      if (is_space_loop[i]) {
        for (j = i + 1; j < firstD + band->width; j++) {
          if (is_space_loop[j]) {
            for (k = j + 1; k < firstD + band->width; k++) {
              if (is_space_loop[k]) {
                PlutoProg *new_prog = pluto_prog_dup(prog);
                /* As new prog is generated, we will need to generate new bands correspondingly */
                //Band **new_bands;
                //unsigned new_nbands;
                //new_bands = pluto_get_outermost_permutable_bands(new_prog, &new_nbands);
    
                //Ploop *loop = new_bands[0]->loop;
                /* Make the loop i, j, k the outermost loops */
                unsigned d;
                for (d = k; d > firstD; d--) {
                  pluto_interchange(new_prog, d, d - 1);
                }
                for (d = j + 1; d > firstD; d--) {
                  pluto_interchange(new_prog, d, d - 1);
                }
                for (d = i + 2; d > firstD; d--) {
                  pluto_interchange(new_prog, d, d - 1);
                }
    
                pluto_compute_dep_directions(new_prog);
                pluto_compute_dep_satisfaction(new_prog); 
                psa_compute_dep_distances_isl(new_prog);

                /* Update psa_hyp_type */
                psa_detect_hyperplane_types(new_prog, array_dim, 0);
                psa_detect_hyperplane_types_stmtwise(new_prog, array_dim, 0);
    
                /* Update array dim */
                new_prog->array_dim = array_dim;
                // new_prog->array_part_dim = band->width;
                new_prog->array_part_dim = 0;
    
                /* Add the new invariant to the list */
                if (progs) {
                  progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg *));
                  progs[*nprogs] = new_prog;
                  *nprogs = *nprogs + 1;
                } else {
                  progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg *));
                  progs[*nprogs] = new_prog;
                  *nprogs = *nprogs + 1;
                }
              }
            }
          }
        }
      }
    }
  }

  free(is_space_loop);
  return progs;
}

/*
 * Enumerate different combinations of the loops in the outermost permutable loop
 * band to generate different systolic array candidates. 
 * Select permutable loops in the outermost permutable loop bands with all dependence
 * components (<= 1) and generate all combinations for different array layouts. 
 * (2D and 1D systolic array)
 * Separate arrays that need interior I/O elimination and those don't.
 */
PlutoProg **sa_candidates_generation(PlutoProg *prog, int *nprogs_p) {
	PlutoProg **progs = NULL;
  int nprogs = 0;

  int i;
  unsigned nbands;
  Band **bands;
  /* Compute projected dependence components */
  psa_compute_dep_distances_isl(prog);

  /* Grasp the outermost permutable loop bands */
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);
#ifdef PSA_ARRAY_DEBUG
  fprintf(stdout, "[PSA] nbands: %d\n", nbands);
#endif

  /* Enumerate different combinations of loops in the permutable bands to generate
   * different systolic array candidates 
   */  
  assert (nbands <= 1);
  if (nbands == 0 || nbands > 1) {
    progs = NULL;
    *nprogs_p = 0;
    return progs;
  }

#ifdef ASYNC_ARRAY
  fprintf(stdout, "[PSA] Generate asynchronized array.\n");
#endif
#ifdef SYNC_ARRAY
  fprintf(stdout, "[PSA] Generate syncrhonized array.\n");
  fprintf(stdout, "[PSA] Notice! Sync array generation is only supported for T2S codegen.\n");
#endif  

#if SA_DIM_UB >= 1
  /* 1D systolic array */
  if (bands[0]->width >= 1) {   
    fprintf(stdout, "[PSA] Explore 1D systolic array... ");
    PlutoProg **new_progs = NULL;
    int new_nprogs = 0;
#ifdef ASYNC_ARRAY    
    new_progs = sa_candidates_generation_band_async(bands[0], 1, prog, &new_nprogs);
#endif
#ifdef SYNC_ARRAY
    new_progs = sa_candidates_generation_band_sync(bands[0], 1, prog, &new_nprogs);
#endif    
    if (!progs) {
// #ifdef JIE_DEBUG
//       fprintf(stdout, "[Debug] in 1D branch.\n");
//       fprintf(stdout, "[Debug] number of 1D systolic array: %d\n", new_nprogs);
// #endif      
      progs = (PlutoProg **)malloc((nprogs + new_nprogs) * sizeof(PlutoProg *));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }
      nprogs += new_nprogs;      
    } else {
      progs = (PlutoProg **)realloc(progs, (nprogs + new_nprogs) * sizeof(PlutoProg *));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }      
      nprogs += new_nprogs;
    }
    fprintf(stdout, "%d variants found.\n", new_nprogs);
    /* Free Memory */
    free(new_progs);
    /* Free Memory */
  }
#endif
#if SA_DIM_UB >= 2
  /* 2D systolic array */
  if (bands[0]->width >= 2) {
    fprintf(stdout, "[PSA] Explore 2D systolic array... ");
    PlutoProg **new_progs = NULL;
    int new_nprogs = 0;
#ifdef JIE_DEBUG
    fprintf(stdout, "[Debug] prog ntransdeps: %d\n", prog->ntransdeps);
    fprintf(stdout, "[Debug] prog transdep[2] nrows: %d\n", prog->transdeps[2]->dpolytope->nrows);
    fprintf(stdout, "[Debug] prog transdep[2] ncols: %d\n", prog->transdeps[2]->dpolytope->ncols);
#endif
#ifdef ASYNC_ARRAY    
    new_progs = sa_candidates_generation_band_async(bands[0], 2, prog, &new_nprogs);
#endif
#ifdef SYNC_ARRAY
    new_progs = sa_candidates_generation_band_sync(bands[0], 2, prog, &new_nprogs);
#endif    

#ifdef JIE_DEBUG
    fprintf(stdout, "[Debug] new_prog ntransdeps: %d\n", new_progs[0]->ntransdeps);
    fprintf(stdout, "[Debug] new_prog transdep[2] nrows: %d\n", new_progs[0]->transdeps[2]->dpolytope->nrows);
    fprintf(stdout, "[Debug] new_prog transdep[2] ncols: %d\n", new_progs[0]->transdeps[2]->dpolytope->ncols);
#endif    
// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] in 2D branch.\n");
//     fprintf(stdout, "[Debug] number of 2D systolic array: %d\n", new_nprogs);
// #endif    
    if (!progs) {
      progs = (PlutoProg **)malloc((nprogs + new_nprogs) * sizeof(PlutoProg *));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }
      nprogs += new_nprogs;      
    } else {
      progs = (PlutoProg **)realloc(progs, (nprogs + new_nprogs) * sizeof(PlutoProg *));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }
      nprogs += new_nprogs;
    }
    fprintf(stdout, "%d variants found.\n", new_nprogs);
    /* Free Memory */
    free(new_progs);
    /* Free Memory */  
  }
#endif
#if SA_DIM_UB >= 3
  /* 3D systolic array */
  if (bands[0]->width >= 3) {
    fprintf(stdout, "[PSA] Explore 3D systolic array... ");
    PlutoProg **new_progs = NULL;
    int new_nprogs = 0;
#ifdef ASYNC_ARRAY
    new_progs = sa_candidates_generation_band_async(bands[0], 3, prog, &new_nprogs);
#endif
#ifdef SYNC_ARRAY    
    new_progs = sa_candidates_generation_band_sync(bands[0], 3, prog, &new_nprogs);
#endif    
    if (!progs) {
      progs = (PlutoProg **)malloc((nprogs + new_nprogs) * sizeof(PlutoProg *));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }
      nprogs += new_nprogs;      
    } else {
      progs = (PlutoProg **)realloc(progs, (nprogs + new_nprogs) * sizeof(PlutoProg *));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }
      nprogs += new_nprogs;
    }
    fprintf(stdout, "%d variants found.\n", new_nprogs);
    /* Free Memory */
    free(new_progs);
    /* Free Memory */   
  } 
#endif

  *nprogs_p = nprogs;
  
  // int nprogs = 1;
  // *nprogs_p = nprogs;  
	// progs = (PlutoProg *)malloc(nprogs * sizeof(PlutoProg*));
	
	// for (i = 0; i < nprogs; i++) {
	// 	progs[i] = pluto_prog_dup(prog);
	// }

  /* Free Memory */
  pluto_bands_free(bands, nbands);
  /* Free Memory */

	return progs;
}

/*
 * This function select the optimal systolic array based on heuristics
 * The selected optimal systolic array will be placed in the first place
 * in the list.
 * The heuristics is: the more reuse has been carried in the space loops, 
 * the better the design is. This is based on the fact that if the resue 
 * can be carried in the space loops, we could alleviate the efforts of 
 * interior I/O elimination, which brings more design overheads.
 * Following the heuristic, this function adds up number of access functions
 * in the systolic array candidate that have reuse (RAR, RAW) been carried in
 * the space loops. And then rank the design based on this score.
 */ 
void sa_candidates_smart_pick(PlutoProg **progs, int nprogs) {
  int *score = (int*)malloc(nprogs * sizeof(int));
  for (int i = 0; i < nprogs; i++)
    score[i] = 0;
  int max_score = -1;
  int opt_prog_id = -1;

  for (int i = 0; i < nprogs; i++) {
    PlutoProg *prog = progs[i];
    pluto_transformations_pretty_print(prog);

    // scan through all depedences
    int ndeps = prog->ndeps;
    Dep **deps = prog->deps;
    for (int j = 0; j < ndeps; j++) {
      Dep *dep = deps[j];
      if (IS_RAW(dep->type) || IS_RAR(dep->type)) {
        int array_dim = prog->array_dim;
        int space_hyp_start;
        int h;
        for (h = 0; h < prog->num_hyperplanes; h++) {
          if (IS_PSA_SPACE_LOOP(prog->hProps[h].psa_type)) {
            space_hyp_start = h;
            break;
          }
        }

        for (h = space_hyp_start; h < space_hyp_start + array_dim; h++) {
          if (dep->disvec[h] != 0)
            break;
        }
        if (h < space_hyp_start + array_dim)
          score[i]++;
      }
    }

    // rank the score
    if (score[i] > max_score) {
      max_score = score[i];
      opt_prog_id = i;
    }
  }

  // move the optimal prog to the first place
  fprintf(stdout, "[PSA] The %dth design is picked.\n", opt_prog_id + 1);
  fprintf(stdout, "[PSA] The optimal design is placed in the first place.\n");
  PlutoProg *tmp_prog = NULL;
  tmp_prog = progs[0];
  progs[0] = progs[opt_prog_id];
  progs[opt_prog_id] = tmp_prog;

  free(score);
}
