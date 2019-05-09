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
  dep->src_acc = d->src_acc;
  dep->dest_acc = d->dest_acc;
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
#ifdef JIE_DEBUG
  fprintf(stdout, "[Debug] dep->id: %d\n", dep->id);
#endif
  dep->disvec = NULL;
  if (d->disvec) {
#ifdef JIE_DEBUG
    fprintf(stdout, "[Debug] duplicate disvec.\n");
#endif
    dep->disvec = (DepDis *)malloc(num_hyperplanes * sizeof(DepDis));
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
  }

// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 1.2.\n");
// #endif
  /* Number of vertices that have already been coloured */
  ngraph->num_coloured_vertices = graph->num_coloured_vertices;

  /* Adjacency matrix */
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

  if (ngraph->sccs)
    free(ngraph->sccs);
  ngraph->sccs = (Scc *)malloc(ngraph->num_sccs * sizeof(Scc));
  for (i = 0; i < ngraph->num_sccs; i++) {
    ngraph->sccs[i].size = graph->sccs[i].size;
    ngraph->sccs[i].max_dim = graph->sccs[i].max_dim;
    ngraph->sccs[i].id = graph->sccs[i].id;
// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] Stop 1.4.\n");
//     fprintf(stdout, "[Debug] size: %d\n", ngraph->sccs[i].size);
//     fprintf(stdout, "[Debug] vertices NULL: %d\n", ngraph->sccs[i].vertices);
//     //fprintf(stdout, "[Debug] vertices :%d\n", ngraph->sccs[i].vertices[0]);
// #endif    
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
// #ifdef JIE_DEBUG
//     fprintf(stdout, "[Debug] Stop 1.5.\n");
// #endif    
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
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 1.6.\n");
// #endif

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
#ifdef JIE_DEBUG
  fprintf(stdout, "[Debug] deps dup.\n");
#endif
  new_prog->deps = (Dep **)malloc(prog->ndeps * sizeof(Dep *));
  for (i = 0; i < prog->ndeps; i++) {
	  new_prog->deps[i] = pluto_dep_prog_dup(prog->deps[i], prog->num_hyperplanes);
  }

  /* Array of dependences */
#ifdef JIE_DEBUG
  fprintf(stdout, "[Debug] transdeps dup.\n");
#endif
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
  if (prog->ddg != NULL)
    new_prog->ddg = graph_dup(prog->ddg);
  else
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
  if (prog->context != NULL)
    new_prog->context = pluto_constraints_dup(prog->context);
  if (prog->decls != NULL)
    new_prog->decls = strdup(prog->decls);
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 8.\n");
// #endif
  /* Codegen context */
  new_prog->codegen_context = pluto_constraints_dup(prog->codegen_context);
// #ifdef JIE_DEBUG
//   fprintf(stdout, "[Debug] Stop 9.\n");
// #endif
  /* Temp autotransform data*/
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

  return new_prog;
}

/* 
 * Distance vector component at level 'level'
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

  /* Neither ZERO, nor PLUS ONE, nor MINUS ONE, has to be STAR */
  return DEP_STAR;    
}

/* 
 * Compute the dependence distance for each mapped hyperplane.
 */
void psa_compute_dep_distances(PlutoProg *prog) {
  int level;

  Dep **deps = prog->deps;
  Dep **transdeps = prog->transdeps;

  /* Clear invalid pointer value in transdeps */
  for (int i = 0; i < prog->ntransdeps; i++) {
    //if (transdeps[i]->disvec != NULL) {
    //  free(transdeps[i]->disvec);
    //}
    transdeps[i]->disvec = NULL;
  }

  for (int i = 0; i < prog->ndeps; i++) {    
    if (deps[i]->disvec != NULL) {
      free(deps[i]->disvec);
    }
    deps[i]->disvec = (DepDis *)malloc(prog->num_hyperplanes * sizeof(DepDis));
    for (level = 0; level < prog->num_hyperplanes; level++) {
      deps[i]->disvec[level] = get_dep_distance(deps[i], prog, level);
    }
#ifdef JIE_DEBUG
    Dep *dep = deps[i];
    fprintf(stdout, "[Debug] id: %d\n", dep->id);
    fprintf(stdout, "[Debug] type: %d\n", dep->type);
    fprintf(stdout, "[Debug] name: %s\n", dep->src_acc->name);
    PlutoConstraints *dpolytope = dep->dpolytope;
    pluto_constraints_pretty_print(stdout, dpolytope);
    for (level = 0; level < prog->num_hyperplanes; level++) {
      fprintf(stdout, "[Debug] dep dis %d: %c\n", level, deps[i]->disvec[level]);
    }
#endif
  }
}

PlutoProg **sa_candidates_generation_band(Band *band, int array_dim, 
              PlutoProg *prog, int *nprogs) {
  PlutoProg **progs = NULL;
  unsigned i, j, nloops;

  Ploop **loops;

  int firstD = band->loop->depth;
  int lastD = band->loop->depth + band->width - 1;

  /* Select loops that carried dependence with distance less equal to 1 */
  int *is_space_loop = (int *)malloc(band->width * sizeof(int));
  for (i = firstD; i < firstD + band->width; i++) {    
    for (j = 0; j < prog->ndeps; j++) {
      Dep *dep = prog->deps[j];
      assert(dep->disvec != NULL);
      if (!(dep->disvec[i] == DEP_DIS_ZERO || dep->disvec[i] == DEP_DIS_PLUS_ONE)) {
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
        Band **new_bands;
        unsigned new_nbands;
        new_bands = pluto_get_outermost_permutable_bands(new_prog, &new_nbands);

        Ploop *loop = new_bands[0]->loop;
        /* Make the loop i the outermost loop */
        for (j = 0; j < loop->nstmts; j++) {
          Stmt *stmt = loop->stmts[j];
#ifdef JIE_DEBUG
          fprintf(stdout, "[Debug] stmts nrows: %d\n", stmt->trans->nrows);
          fprintf(stdout, "[Debug] stmts ncols: %d\n", stmt->trans->ncols);
#endif          
          unsigned d;
          for (d = i; d > firstD; d--) {
            pluto_stmt_loop_interchange(stmt, d, d - 1, new_prog);
          }
        }

        pluto_compute_dep_directions(new_prog);
        pluto_compute_dep_satisfaction(new_prog);
        /* Add the new invariant to the list */
        if (progs) {
          progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg));
          progs[*nprogs] = new_prog;
          *nprogs = *nprogs + 1;
        } else {
          progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg));
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
            Band **new_bands;
            unsigned new_nbands;
            new_bands = pluto_get_outermost_permutable_bands(new_prog, &new_nbands);

            Ploop *loop = new_bands[0]->loop;
            /* Make the loop i, j the outermost loops */
            unsigned s;
            for (s = 0; s < loop->nstmts; s++) {
              Stmt *stmt = loop->stmts[s];
              unsigned d;
              for (d = j; d > firstD; d--) {
                pluto_stmt_loop_interchange(stmt, d, d - 1, new_prog);
              }
              for (d = i + 1; d > firstD; d--) {
                pluto_stmt_loop_interchange(stmt, d, d - 1, new_prog);
              }
            }

            pluto_compute_dep_directions(new_prog);
            pluto_compute_dep_satisfaction(new_prog);
            /* Add the new invariant to the list */
            if (progs) {
              progs = (PlutoProg **)realloc(progs, (*nprogs + 1) * sizeof(PlutoProg));
              progs[*nprogs] = new_prog;
              *nprogs = *nprogs + 1;
            } else {
              progs = (PlutoProg **)malloc((*nprogs + 1) * sizeof(PlutoProg));
              progs[*nprogs] = new_prog;
              *nprogs = *nprogs + 1;
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
  psa_compute_dep_distances(prog);

  /* Grasp the outermost permutable loop bands */
  bands = pluto_get_outermost_permutable_bands(prog, &nbands);

  /* Enumerate different combinations of loops in the permutable bands to generate
   * different systolic array candidates 
   */  
  assert (nbands <= 1);
  if (nbands == 0 || nbands > 1) {
    progs = NULL;
    *nprogs_p = 0;
    return progs;
  }

  /* 1D systolic array */
  if (bands[0]->width >= 1) {    
    PlutoProg **new_progs = NULL;
    int new_nprogs = 0;
    new_progs = sa_candidates_generation_band(bands[0], 1, prog, &new_nprogs);
    if (!progs) {
#ifdef JIE_DEBUG
      fprintf(stdout, "[Debug] in 1D branch.\n");
      fprintf(stdout, "[Debug] number of 1D systolic array: %d\n", new_nprogs);
#endif      
      nprogs += new_nprogs;
      progs = new_progs;
    } else {
      progs = (PlutoProg **)realloc(progs, (nprogs + new_nprogs) * sizeof(PlutoProg **));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }      
      nprogs += new_nprogs;
    }
  }
  /* 2D systolic array */
  if (bands[0]->width >= 2) {
    PlutoProg **new_progs = NULL;
    int new_nprogs = 0;
    new_progs = sa_candidates_generation_band(bands[0], 2, prog, &new_nprogs);
#ifdef JIE_DEBUG
    fprintf(stdout, "[Debug] in 2D branch.\n");
    fprintf(stdout, "[Debug] number of 2D systolic array: %d\n", new_nprogs);
#endif    
    if (!progs) {
      nprogs += new_nprogs;
      progs = new_progs;
    } else {
      progs = (PlutoProg **)realloc(progs, (nprogs + new_nprogs) * sizeof(PlutoProg **));
      for (i = nprogs; i < nprogs + new_nprogs; i++) {
        progs[i] = new_progs[i - nprogs];
      }
      nprogs += new_nprogs;
    }
  }

  *nprogs_p = nprogs;
  
  // int nprogs = 1;
  // *nprogs_p = nprogs;  
	// progs = (PlutoProg *)malloc(nprogs * sizeof(PlutoProg*));
	
	// for (i = 0; i < nprogs; i++) {
	// 	progs[i] = pluto_prog_dup(prog);
	// }

	return progs;
}
