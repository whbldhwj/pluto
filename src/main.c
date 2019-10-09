/*
 * PLUTO: An automatic parallelizer and locality optimizer
 *
 * Copyright (C) 2007-2015 Uday Bondhugula
 *
 * This file is part of Pluto.
 *
 * Pluto is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.

 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * A copy of the GNU General Public Licence can be found in the file
 * `LICENSE' in the top-level directory of this distribution.
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <getopt.h>
#include <libgen.h>

#include <unistd.h>
#include <sys/time.h>

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "osl/scop.h"
#include "osl/generic.h"
#include "osl/extensions/irregular.h"

#include "pluto.h"
#include "transforms.h"
#include "math_support.h"
#include "post_transform.h"
#include "program.h"
#include "version.h"
#include "psa_vsa.h"
#include "psa_dep.h"
#include "psa_array.h"
#include "psa_partition.h"
#include "psa_pe_opt.h"
#include "psa_vsa_dfc.h"
#include "psa_vsa_pe.h"
#include "psa_knobs.h"

#include "clan/clan.h"
#include "candl/candl.h"
#include "candl/scop.h"

#include "pet.h"

PlutoOptions *options;

void usage_message(void) {
  fprintf(stdout, "Usage: polycc <input.c> [options] [-o output]\n");
  fprintf(stdout, "\nOptions:\n");
  fprintf(stdout, "       --pet                     Use libpet for polyhedral "
                  "extraction instead of clan [default - clan]\n");
  fprintf(stdout, "       --isldep                  Use ISL-based dependence "
                  "tester (enabled by default)\n");
  fprintf(
      stdout,
      "       --candldep                Use Candl as the dependence tester\n");
  fprintf(stdout, "       --[no]lastwriter          Remove transitive "
                  "dependences (last conflicting access is computed for "
                  "RAW/WAW)\n");
  fprintf(stdout, "                                 (disabled by default)\n");
  fprintf(stdout,
          "       --islsolve [default]      Use ISL as ILP solver (default)\n");
  fprintf(stdout, "       --pipsolve                Use PIP as ILP solver\n");
#ifdef GLPK
  fprintf(stdout, "       --glpk                    Use GLPK as ILP solver "
                  "(default in case of pluto-lp and pluto-dfp)\n");
#endif
#if defined GLPK || defined GUROBI
  fprintf(stdout,
          "       --lp                      Solve MIP instead of ILP\n");
  fprintf(stdout, "       --dfp                     Use Pluto-lp-dfp instead "
                  "of pluto-ilp [disabled by default]\n");
  fprintf(stdout, "       --ilp                     Use ILP in pluto-lp-dfp "
                  "instead of LP\n");
  fprintf(stdout, "       --lpcolor                 Color FCG based on the "
                  "solutions of the lp-problem [disabled by default]\n");
  fprintf(stdout, "       --clusterscc              Cluster the statemtns of "
                  "an SCC. This is supported only availabe with decoupled "
                  "approach [disabled by default]\n");
#endif
  fprintf(stdout, "\n");
#ifdef GUROBI
  fprintf(stdout,
          "       --gurobi                  Use Gurobi as ILP solver\n");
#endif
  fprintf(stdout, "\n");
  fprintf(stdout,
          "\n  Optimizations          Options related to optimization\n");
  fprintf(stdout, "       --tile                    Tile for locality "
                  "[disabled by default]\n");
  fprintf(stdout, "       --[no]intratileopt        Optimize intra-tile "
                  "execution order for locality [enabled by default]\n");
  fprintf(stdout, "       --l2tile                  Tile a second time "
                  "(typically for L2 cache) [disabled by default] \n");
  fprintf(stdout, "       --parallel                Automatically parallelize "
                  "(generate OpenMP pragmas) [disabled by default]\n");
  fprintf(stdout, "    or --parallelize\n");
  fprintf(stdout, "       --[no]diamond-tile        Performs diamond tiling "
                  "(enabled by default)\n");
  fprintf(stdout, "       --full-diamond-tile       Enables full-dimensional "
                  "concurrent start\n");
  fprintf(stdout, "       --[no]prevector           Mark loops for (icc/gcc) "
                  "vectorization (enabled by default)\n");
  fprintf(stdout, "       --multipar                Extract all degrees of "
                  "parallelism [disabled by default];\n");
  fprintf(stdout, "                                    by default one degree "
                  "is extracted within any schedule sub-tree (if it exists)\n");
  fprintf(stdout, "       --innerpar                Choose pure inner "
                  "parallelism over pipelined/wavefront parallelism [disabled "
                  "by default]\n");
  fprintf(stdout,
          "\n   Fusion                Options to control fusion heuristic\n");
  fprintf(stdout, "       --nofuse                  Do not fuse across SCCs of "
                  "data dependence graph\n");
  fprintf(stdout, "       --maxfuse                 Maximal fusion\n");
  fprintf(stdout, "       --smartfuse [default]     Heuristic (in between "
                  "nofuse and maxfuse)\n");
  fprintf(stdout, "       --typedfuse               Typed fusion. Fuses SCCs "
                  "only when there is no loss of parallelism\n");
  fprintf(stdout, "       --hybridfuse              Typed fusion at outer "
                  "levels and max fuse at inner level\n");
  fprintf(stdout, "       --delayedcut              Delays the cut between "
                  "SCCs of different dimensionalities in dfp approach\n");
  fprintf(stdout, "\n   Index Set Splitting        \n");
  fprintf(stdout, "       --iss                  \n");
  fprintf(
      stdout,
      "\n   Code generation       Options to control Cloog code generation\n");
  fprintf(stdout, "       --nocloogbacktrack        Do not call Cloog with "
                  "backtrack (default - backtrack)\n");
  fprintf(stdout, "       --cloogsh                 Ask Cloog to use simple "
                  "convex hull (default - off)\n");
  fprintf(stdout, "       --codegen-context=<value> Parameters are at least as "
                  "much as <value>\n");
  fprintf(stdout, "\n   Miscellaneous\n");
  fprintf(stdout, "       --rar                     Consider RAR dependences "
                  "too (disabled by default)\n");
  fprintf(
      stdout,
      "       --[no]unroll              Unroll-jam (disabled by default)\n");
  fprintf(
      stdout,
      "       --ufactor=<factor>        Unroll-jam factor (default is 8)\n");
  fprintf(stdout, "       --forceparallel=<bitvec>  6 bit-vector of depths "
                  "(1-indexed) to force parallel (0th bit represents depth "
                  "1)\n");
  fprintf(stdout,
          "       --readscop                Read input from a scoplib file\n");
  fprintf(stdout,
          "       --bee                     Generate pragmas for Bee+Cl@k\n\n");
  fprintf(stdout, "       --indent  | -i            Indent generated code "
                  "(disabled by default)\n");
  fprintf(stdout, "       --silent  | -q            Silent mode; no output as "
                  "long as everything goes fine (disabled by default)\n");
  fprintf(stdout, "       --help    | -h            Print this help menu\n");
  fprintf(stdout, "       --version | -v            Display version number\n");
  fprintf(stdout, "\n   Debugging\n");
  fprintf(stdout, "       --debug                   Verbose/debug output\n");
  fprintf(stdout,
          "       --moredebug               More verbose/debug output\n");
  fprintf(stdout, "\n   T2S\n");
  fprintf(stdout, "       --dsa=<level>             DSA form of input\n");
  fprintf(stdout, "\nTo report bugs, please email "
                  "<pluto-development@googlegroups.com>\n\n");
}

static double rtclock() {
  struct timezone Tzp;
  struct timeval Tp;
  int stat;
  stat = gettimeofday(&Tp, &Tzp);
  if (stat != 0)
    printf("Error return from gettimeofday: %d", stat);
  return (Tp.tv_sec + Tp.tv_usec * 1.0e-6);
}

int main(int argc, char *argv[]) {
  int i;

  double t_start, t_c, t_d, t_t, t_all, t_start_all;

  t_c = 0.0;

  t_start_all = rtclock();

  int option;
  int option_index = 0;

  int nolastwriter = 0;

  char *srcFileName;

  FILE *cloogfp, *outfp;

  struct pet_scop *pscop;

  if (argc <= 1) {
    usage_message();
    return 1;
  }

  options = pluto_options_alloc();

  const struct option pluto_options[] = {
    { "fast-lin-ind-check", no_argument, &options->flic, 1 },
    { "flic", no_argument, &options->flic, 1 },
    { "tile", no_argument, &options->tile, 1 },
    { "notile", no_argument, &options->tile, 0 },
    { "noparallel", no_argument, &options->parallel, 0 },
    { "intratileopt", no_argument, &options->intratileopt, 1 },
    { "nointratileopt", no_argument, &options->intratileopt, 0 },
    { "pet", no_argument, &options->pet, 1 },
    { "diamond-tile", no_argument, &options->diamondtile, 1 },
    { "nodiamond-tile", no_argument, &options->diamondtile, 0 },
    { "full-diamond-tile", no_argument, &options->fulldiamondtile, 1 },
    { "debug", no_argument, &options->debug, true },
    { "moredebug", no_argument, &options->moredebug, true },
    { "rar", no_argument, &options->rar, 1 },
    { "identity", no_argument, &options->identity, 1 },
    { "nofuse", no_argument, &options->fuse, NO_FUSE },
    { "maxfuse", no_argument, &options->fuse, MAXIMAL_FUSE },
    { "smartfuse", no_argument, &options->fuse, SMART_FUSE },
    { "typedfuse", no_argument, &options->fuse, TYPED_FUSE },
    { "hybridfuse", no_argument, &options->hybridcut, 1 },
    { "delayedcut", no_argument, &options->delayed_cut, 1 },
    { "parallel", no_argument, &options->parallel, 1 },
    { "parallelize", no_argument, &options->parallel, 1 },
    { "innerpar", no_argument, &options->innerpar, 1 },
    { "iss", no_argument, &options->iss, 1 },
    { "unroll", no_argument, &options->unroll, 1 },
    { "nounroll", no_argument, &options->unroll, 0 },
    { "bee", no_argument, &options->bee, 1 },
    { "ufactor", required_argument, 0, 'u' },
    { "prevector", no_argument, &options->prevector, 1 },
    { "noprevector", no_argument, &options->prevector, 0 },
    { "codegen-context", required_argument, 0, 'c' },
    { "coeff-bound", required_argument, 0, 'C' },
    { "cloogf", required_argument, 0, 'F' },
    { "cloogl", required_argument, 0, 'L' },
    { "cloogsh", no_argument, &options->cloogsh, 1 },
    { "nocloogbacktrack", no_argument, &options->cloogbacktrack, 0 },
    { "cyclesize", required_argument, 0, 'S' },
    { "forceparallel", required_argument, 0, 'p' },
    { "ft", required_argument, 0, 'f' },
    { "lt", required_argument, 0, 'l' },
    { "multipar", no_argument, &options->multipar, 1 },
    { "l2tile", no_argument, &options->l2tile, 1 },
    { "version", no_argument, 0, 'v' },
    { "help", no_argument, 0, 'h' },
    { "indent", no_argument, 0, 'i' },
    { "silent", no_argument, &options->silent, 1 },
    { "lastwriter", no_argument, &options->lastwriter, 1 },
    { "nolastwriter", no_argument, &nolastwriter, 1 },
    { "nodepbound", no_argument, &options->nodepbound, 1 },
    { "scalpriv", no_argument, &options->scalpriv, 1 },
    { "isldep", no_argument, &options->isldep, 1 },
    { "candldep", no_argument, &options->candldep, 1 },
    { "isldepaccesswise", no_argument, &options->isldepaccesswise, 1 },
    { "isldepstmtwise", no_argument, &options->isldepaccesswise, 0 },
    { "noisldepcoalesce", no_argument, &options->isldepcoalesce, 0 },
    { "readscop", no_argument, &options->readscop, 1 },
    { "pipsolve", no_argument, &options->pipsolve, 1 },
    { "dsa", required_argument, 0, 'd' },
#ifdef GLPK
    { "glpk", no_argument, &options->glpk, 1 },
#endif
#ifdef GUROBI
    { "gurobi", no_argument, &options->gurobi, 1 },
#endif
#if defined GLPK || defined GUROBI
    { "lp", no_argument, &options->lp, 1 },
    { "dfp", no_argument, &options->dfp, 1 },
    { "ilp", no_argument, &options->ilp, 1 },
    { "lpcolor", no_argument, &options->lpcolour, 1 },
    { "clusterscc", no_argument, &options->scc_cluster, 1 },
#endif
    { "islsolve", no_argument, &options->islsolve, 1 },
    { "time", no_argument, &options->time, 1 },
    { 0, 0, 0, 0 }
  };

  /* Read command-line options */
  while (1) {
    option = getopt_long(argc, argv, "bhiqvf:l:F:L:c:o:", pluto_options,
                         &option_index);

    if (option == -1) {
      break;
    }

    switch (option) {
    case 0:
      break;
    case 'F':
      options->cloogf = atoi(optarg);
      break;
    case 'L':
      options->cloogl = atoi(optarg);
      break;
    case 'b':
      options->bee = 1;
      break;
    case 'c':
      options->codegen_context = atoi(optarg);
      break;
    case 'C':
      options->coeff_bound = atoi(optarg);
      if (options->coeff_bound <= 0) {
        printf("ERROR: coeff-bound should be at least 1\n");
        return 2;
      }
      break;
    case 'd':
      options->dsa = atoi(optarg);
      break;
    case 'f':
      options->ft = atoi(optarg);
      break;
    case 'g':
      break;
    case 'h':
      usage_message();
      return 2;
    case 'i':
      /* Handled in polycc */
      break;
    case 'l':
      options->lt = atoi(optarg);
      break;
    case 'm':
      break;
    case 'n':
      break;
    case 'o':
      options->out_file = strdup(optarg);
      break;
    case 'p':
      options->forceparallel = atoi(optarg);
      break;
    case 'q':
      options->silent = 1;
      break;
    case 's':
      break;
    case 'u':
      options->ufactor = atoi(optarg);
      break;
    case 'v':
      printf(
          "PLUTO version %s - An automatic parallelizer and locality optimizer\n\
Copyright (C) 2007--2015  Uday Bondhugula\n\
This is free software; see the source for copying conditions.  There is NO\n\
warranty; not even for MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.\n\n",
          PLUTO_VERSION);
      pluto_options_free(options);
      return 3;
    default:
      usage_message();
      pluto_options_free(options);
      return 4;
    }
  }

  if (optind <= argc - 1) {
    srcFileName = alloca(strlen(argv[optind]) + 1);
    strcpy(srcFileName, argv[optind]);
  } else {
    /* No non-option argument was specified */
    usage_message();
    pluto_options_free(options);
    return 5;
  }

  /* Make options consistent */
  if (options->isldep && options->candldep) {
    printf("[pluto] ERROR: only one of isldep and candldep should be "
           "specified)\n");
    pluto_options_free(options);
    usage_message();
    return 1;
  }

  /* isldep is the default */
  if (!options->isldep && !options->candldep) {
    options->isldep = 1;
  }

  if (options->lastwriter && options->candldep) {
    printf("[pluto] ERROR: --lastwriter is only supported with --isldep\n");
    pluto_options_free(options);
    usage_message();
    return 1;
  }

  if (options->lastwriter && nolastwriter) {
    printf("[pluto] WARNING: both --lastwriter, --nolastwriter are on\n");
    printf("[pluto] disabling --lastwriter\n");
    options->lastwriter = 0;
  }

  /* Make options consistent */
  if (options->diamondtile == 1 && options->tile == 0) {
    options->diamondtile = 0;
  }
  if (options->fulldiamondtile == 1 && options->tile == 0) {
    options->diamondtile = 0;
    options->fulldiamondtile = 0;
  }

  if (options->multipar == 1 && options->parallel == 0) {
    fprintf(stdout,
            "Warning: multipar needs parallel to be on; turning on parallel\n");
    options->parallel = 1;
  }

  if (options->multipar == 1 && options->parallel == 0) {
    fprintf(stdout,
            "Warning: multipar needs parallel to be on; turning on parallel\n");
    options->parallel = 1;
  }

  if (options->gurobi) {
    options->islsolve = 0;
  }
#ifdef GLPK
  if (options->lp && !(options->glpk || options->gurobi)) {
    if (!options->silent) {
      printf("[pluto] LP option available with a LP solver only. Using GLPK "
             "for lp solving\n");
    }
    options->glpk = 1;
  }

  /* By default Pluto-dfp uses lp. */
  if (options->dfp && !options->ilp) {
    options->lp = 1;
  }

  if (options->dfp && !(options->glpk || options->gurobi)) {
    if (!options->silent) {
      printf("[pluto] Dfp framework is currently supported with GLPK and "
             "Gurobi solvers.\n");
      printf("[pluto] Using GLPK for constraint solving [default]. Use "
             "--gurobi to use Gurobi instead of GLPK.\n");
    }
    options->glpk = 1;
  }
  if (options->glpk) {
    /* Turn off islsolve */
    options->islsolve = 0;
  }
#endif

  if (options->dfp && !(options->glpk || options->gurobi)) {
    printf("[pluto] ERROR: DFP framework is currently supported with GLPK or "
           "GUROBI solvers only. Run ./configure --help to for more "
           "information on using different solvers with Pluto.\n");
    pluto_options_free(options);
    usage_message();
    return 1;
  }
  if (options->scc_cluster && !options->dfp) {
    printf("[pluto] Warning: SCC clustering heuristics available with dfp "
           "option (FCG based approach) only. Disabling clustering \n");
  }

  if (options->fuse == TYPED_FUSE && !options->dfp) {
    printf("[Pluto] WARNING: Typed Fuse Available with dfp framework only. "
           "Turning off Typed fuse\n");
    options->fuse = SMART_FUSE;
  }

  /* Make lastwriter default with dfp. This removes transitive dependences and
   * hence reduces FCG construction time */
  if (options->dfp && !options->lastwriter) {
    if (!options->silent) {
      printf("[pluto] Enabling lastwriter dependence analysis with DFP\n");
    }
    options->lastwriter = 1;
  }
  /* Typed fuse is available with clustered FCG approach only */
  if (options->fuse == TYPED_FUSE && options->dfp && !options->scc_cluster) {
    if (!options->silent) {
      printf("[pluto] Typed fuse supported only with clustered FCG approach. "
             "Turning on SCC clustering\n");
    }
    options->scc_cluster = 1;
  }

  /* Extract polyhedral representation from osl scop */
  PlutoProg *prog = NULL;

  osl_scop_p scop = NULL;
  char *irroption = NULL;

  /* Extract polyhedral representation from input program */
  if (options->pet) {
    isl_ctx *pctx = isl_ctx_alloc_with_pet_options();
    pscop = pet_scop_extract_from_C_source(pctx, srcFileName, NULL);

/* Jie Added - Start */
#ifdef PSA_PET_DEBUG
//    /* Print out the PET scop */
//    if (pscop) {
//      pet_scop_emit(stdout, pscop);
//    }
#endif
/* Jie Added - End */

    if (!pscop) {
      fprintf(
          stdout,
          "[pluto] No SCoPs extracted or error extracting SCoPs  using pet\n");
      pluto_options_free(options);
      isl_ctx_free(pctx);
      return 12;
    }
    t_start = rtclock();
    prog = pet_to_pluto_prog(pscop, pctx, options);
    t_d = rtclock() - t_start;

    pet_scop_free(pscop);
    isl_ctx_free(pctx);

    FILE *srcfp = fopen(".srcfilename", "w");
    if (srcfp) {
      fprintf(srcfp, "%s\n", srcFileName);
      fclose(srcfp);
    }
  } else {
    FILE *src_fp;
    /* Extract polyhedral representation from clan scop */
    if (!strcmp(srcFileName, "stdin")) { // read from stdin
      src_fp = stdin;
      osl_interface_p registry = osl_interface_get_default_registry();
      t_start = rtclock();
      scop = osl_scop_pread(src_fp, registry, PLUTO_OSL_PRECISION);
      t_d = rtclock() - t_start;
    } else { // read from regular file

      src_fp = fopen(srcFileName, "r");

      if (!src_fp) {
        fprintf(stderr, "pluto: error opening source file: '%s'\n",
                srcFileName);
        pluto_options_free(options);
        return 6;
      }

      /* Extract polyhedral representation from input program */
      clan_options_p clanOptions = clan_options_malloc();

      if (options->readscop) {
        osl_interface_p registry = osl_interface_get_default_registry();
        t_start = rtclock();
        scop = osl_scop_pread(src_fp, registry, PLUTO_OSL_PRECISION);
        t_d = rtclock() - t_start;
      } else {
        t_start = rtclock();
        scop = clan_scop_extract(src_fp, clanOptions);
        t_d = rtclock() - t_start;
      }
      fclose(src_fp);

      if (!scop || !scop->statement) {
        fprintf(stderr, "Error extracting polyhedra from source file: \'%s'\n",
                srcFileName);
        pluto_options_free(options);
        return 8;
      }
      FILE *srcfp = fopen(".srcfilename", "w");
      if (srcfp) {
        fprintf(srcfp, "%s\n", srcFileName);
        fclose(srcfp);
      }

      clan_options_free(clanOptions);
    }

    /* Convert clan scop to Pluto program */
    prog = scop_to_pluto_prog(scop, options);

    /* Backup irregular program portion in .scop. */
    osl_irregular_p irreg_ext = NULL;
    irreg_ext = osl_generic_lookup(scop->extension, OSL_URI_IRREGULAR);
    if (irreg_ext != NULL)
      irroption = osl_irregular_sprint(irreg_ext); // TODO: test it
    osl_irregular_free(irreg_ext);
  }
  IF_MORE_DEBUG(pluto_prog_print(stdout, prog));

/*
 * *******************************************
 * Stage: Array Generation
 * Step: Dependence Analysis
 * *******************************************
 */
/* Jie Added - Start */  
  psa_print_deps(prog);
  fprintf(stdout, "[PSA] Filter out redundant dependences.\n");
  /* Filter out RAR dependences on scalar variables */
  rar_scalar_filter(prog);
  /* Delete the detected RAR dependences by ISL, RAR deps will be added back later */
  if (options->rar) {
    fprintf(stdout, "[PSA] Filter out RAR dependences.\n");
    rar_filter(prog);
  }
/* Jie Added - End */  

  int dim_sum = 0;
  for (i = 0; i < prog->nstmts; i++) {
    dim_sum += prog->stmts[i]->dim;
  }

  if (!options->silent) {
    fprintf(stdout, "[pluto] Number of statements: %d\n", prog->nstmts);
    fprintf(stdout, "[pluto] Total number of loops: %d\n", dim_sum);
    fprintf(stdout, "[pluto] Number of deps: %d\n", prog->ndeps);
    fprintf(stdout, "[pluto] Maximum domain dimensionality: %d\n", prog->nvar);
    fprintf(stdout, "[pluto] Number of parameters: %d\n", prog->npar);
  }

  if (options->iss) {
    pluto_iss_dep(prog);
  }

  /* Jie Added - Start */
//  psa_print_deps(prog);
  /* Create the access dependence graph */
  Graph *adg = adg_create(prog);
  adg_merge_racc(adg, prog);
  prog->adg = adg;
  adg_compute_cc(prog);
  /* Jie Added - End */

  /* Jie Added - Start */
  /* Analyze the data reuse and add RAR dependences */
  PlutoProg **reuse_progs;
  PlutoProg *reuse_prog;
  int num_reuse_progs;
  if (options->dsa != 2) {
    reuse_progs = psa_reuse_adg_analysis(prog, &num_reuse_progs);
    reuse_progs = psa_reuse_filter(reuse_progs, &num_reuse_progs);
  } else {
    reuse_progs = (PlutoProg **)malloc(1 * sizeof(PlutoProg *));
    reuse_progs[0] = pluto_prog_dup(prog);
    num_reuse_progs = 1;
  }
  fprintf(stdout, "[PSA] %d programs generated after reuse analysis.\n", num_reuse_progs);
  /* Jie Added - End */ 

//  for (int reuse_prog_id = 0; reuse_prog_id < num_reuse_progs; reuse_prog_id++) {
  for (int reuse_prog_id = 0; reuse_prog_id < 1; reuse_prog_id++) {
    reuse_prog = reuse_progs[reuse_prog_id];

#ifdef PRINT_DEPS_PREV_TRANSFORM
    psa_print_deps(reuse_prog);
#endif    

#ifdef SPACE_TIME_CHECK
    bool is_uniform = systolic_array_dep_checker_isl(reuse_prog);
    fprintf(stdout, "[PSA] Uniformity before Pluto's Transformation: %d\n", is_uniform);
    if (!is_uniform) {
      continue;
    }
#endif
/*
 * *******************************************
 * Stage: Array Generation
 * Step: Pluto Transformation
 * *******************************************
 */
    if (!options->silent) {
      fprintf(
          stdout,
          "[pluto] Original scheduling [<iter coeff's> <param> <const>]\n\n");
      /* Print out transformations */
      pluto_transformations_pretty_print(reuse_prog);
    }
  
#ifdef PLUTO_TRANSFORM    
    t_start = rtclock();
    fprintf(stdout, "[PSA] ****************************\n");
    fprintf(stdout, "[PSA] Run Pluto's Algorithm.\n");
    /* Auto transformation */
    if (!options->identity) {
      pluto_auto_transform(reuse_prog);
    }
    t_t = rtclock() - t_start;

    for (int s = 0; s < reuse_prog->nstmts; s++) {
      reuse_prog->stmts[s]->untouched = 0;
    }
#endif

    pluto_compute_dep_directions(reuse_prog);
    pluto_compute_dep_satisfaction(reuse_prog);
    pluto_detect_hyperplane_types(reuse_prog);
    pluto_detect_hyperplane_types_stmtwise(reuse_prog);

    for (int n = 0; n < reuse_prog->ndeps; n++) {
      Dep *dep = reuse_prog->deps[n];
      for (int s = 0; s < reuse_prog->num_hyperplanes; s++) {
        if (dep->dirvec[s] != DEP_ZERO && dep->dirvec[s] != DEP_PLUS) {
          fprintf(stdout, "Dep %d at %d less than zero!\n", n, s);
          break;
        }
      }
    }

    if (!options->silent) {
      fprintf(
          stdout,
          "[pluto] Affine transformations [<iter coeff's> <param> <const>]\n\n");
      /* Print out transformations */
      pluto_transformations_pretty_print(reuse_prog);
    }

    /* Jie Added - Start */
#ifdef PRINT_PLUTO_TRANS_PROGRAM
    fprintf(stdout, "[PSA] Dump out the transformed program after Pluto's algorithm.\n");
    pluto_print_program(reuse_prog, srcFileName, "pluto_transform");
#endif
//    for (int n = 0; n < reuse_prog->nstmts; n++) {
//      Stmt *stmt = reuse_prog->stmts[n];
//      pluto_matrix_print(stdout, stmt->trans);
//    }
    /* Jie Added - End */
  
#ifdef PRINT_DEPS_POST_TRANSFORM
    psa_print_deps(reuse_prog);
#endif

/*
 * *******************************************
 * Stage: Array Generation
 * Step: Systolic Array Transformation Legality Checker
 * *******************************************
 */
#ifdef SPACE_TIME_CHECK
    fprintf(stdout, "[PSA] Check space-time mapping legality.\n");
    bool is_legal = systolic_array_legal_checker(reuse_prog);
    if (!is_legal) {
      fprintf(stdout, "[PSA] This program can't be synthesized to systolic array.\n");
      continue;
    }
    fprintf(stdout, "[PSA] Space-time mapping leagalibty: %d\n", is_legal);
#endif

/*
 * *******************************************
 * Stage: Array Generation
 * Step: Space-time Remapping
 * *******************************************
 */
#ifdef SPACE_TIME_MAPPING  
    fprintf(stdout, "[PSA] ****************************\n");
    fprintf(stdout, "[PSA] Explore different systolic array candidates.\n");
    PlutoProg** progs;
    int nprogs;
    progs = sa_candidates_generation(reuse_prog, &nprogs);
    fprintf(stdout, "[PSA] Number of array candidates: %d\n", nprogs);
#endif
  
#ifdef SA_SMART_PICK
    // If SA_SMART_PICK is enabled, PolySA will rank the systolic array candidates based on 
    // heuristics, and place the optimal one in the first place of the list
    fprintf(stdout, "[PSA] ****************************\n");
    fprintf(stdout, "[PSA] Smart pick systolic array candidate.\n");
    sa_candidates_smart_pick(progs, nprogs);    
    for (int i = 1; i < nprogs; i++) {
      pluto_prog_free(progs[i]);
      progs[i] = NULL;      
    }
    nprogs = 1;
    pluto_print_program(progs[0], srcFileName, "smart_pick");    
#endif    

    unsigned prog_id;
    for (prog_id = 0; prog_id < nprogs; prog_id++) {
    /* for (prog_id = 0; prog_id < 1; prog_id++) { */
      VSA *psa_vsa = vsa_alloc();    

      PlutoProg *array_prog = progs[prog_id];  
      pluto_transformations_pretty_print(array_prog);

      /* ARRAY_PART_BAND_WIDTH */
      vsa_band_width_extract(array_prog, psa_vsa);
#ifdef T2S_CODEGEN
      fprintf(stdout, "[PSA] Input code DSA form: %d\n", options->dsa);
      /* T2S_ITERS */
      vsa_t2s_iter_extract(array_prog, psa_vsa);
      /* T2S_VARS */
      vsa_t2s_var_extract(array_prog, psa_vsa);
      /* ARRAYS */
      vsa_array_extract(array_prog, psa_vsa);
      /* UREs */
      vsa_URE_extract(array_prog, psa_vsa);
      /* T2S_META_ITERS */
      vsa_t2s_meta_iter_extract(array_prog, psa_vsa);
#else

#endif      

//      /* OP_NUM, RES_NUM, OP_DIM, RES_DIM, OP_NAME, RES_NAME */      
//      vsa_op_res_extract(prog, psa_vsa);
//      /* Extract OP_CHANNEL_DIR and RES_CHANNEL_DIR */    
//      vsa_channel_dir_extract(prog, psa_vsa);    
//      /* TYPE */
//      vsa_type_extract(prog, psa_vsa);
//      /* Jie Added - End */
    
      /* NOTE: Due to the supernode tiling, we will tile from the lower level to 
       * upper level. Therefore, we will perform the intra-PE optimization first,
       * then, array tiling.
       */
      
      fprintf(stdout, "[PSA] Apply PE Optimization.\n");

/*
* *******************************************
* Stage: PE Optimization
* Step: Latency Hiding / Task Interleaving
* *******************************************
*/
#ifdef LATENCY_HIDING     
      if (options->tile) {
        fprintf(stdout, "[PSA] ****************************\n");
        fprintf(stdout, "[PSA] Apply latency hiding.\n");
        int ret = psa_latency_hiding_optimize(array_prog, psa_vsa);
        if (IS_PSA_SUCCESS(ret)) {
          fprintf(stdout, "[PSA] Completed latency hiding.\n");
          /* Print out transformations */
          pluto_transformations_pretty_print(array_prog);

          /* Update the fields of VSA */
          array_prog->array_il_enable = 1;
        } else {
          fprintf(stdout, "[PSA] Failed latency hiding.\n");
          array_prog->array_il_enable = 0;
        }
      }
  #ifdef PRINT_LATENCY_HIDING_TRANS_PROGRAM
      fprintf(stdout, "[PSA] Dump out the transformed program after latency hiding.\n");
      pluto_print_program(array_prog, srcFileName, "latency_hiding");
  #endif
#endif    

/*
* *******************************************
* Stage: PE Optimization
* Step: SIMD vectorization
* *******************************************
*/
#ifdef SIMD_VECTORIZATION
      if (options->tile) {
        fprintf(stdout, "[PSA] ****************************\n");
        fprintf(stdout, "[PSA] Apply SIMD vectorization.\n");
        int ret = psa_simd_vectorization_optimize(array_prog, psa_vsa);
        if (IS_PSA_SUCCESS(ret)) {
          fprintf(stdout, "[PSA] Completed SIMD vectorization.\n");
          /* Print out transformations */
          pluto_transformations_pretty_print(array_prog);          
        } else {
          fprintf(stdout, "[PSA] Failed SIMD vectorization.\n");
        }
      }
  #ifdef PRINT_SIMD_VECTORIZATION_TRANS_PROGRAM
      fprintf(stdout, "[PSA] Dump out the transformed program after SIMD vectorization.\n");
      pluto_print_program(array_prog, srcFileName, "SIMD");
  #endif
#endif      
  
/*
 * *******************************************
 * Stage: PE Optimization
 * Step: Array Partitioning
 * *******************************************
 */
#ifdef ARRAY_PARTITIONING    
      if (options->tile) {
        fprintf(stdout, "[PSA] ****************************\n");
        fprintf(stdout, "[PSA] Apply array partitioning.\n");
        int ret = psa_array_partition_optimize(array_prog, psa_vsa);
        if (IS_PSA_SUCCESS(ret)) {
          fprintf(stdout, "[PSA] Completed array partitioning.\n");
          /* Print out transformations */
          pluto_transformations_pretty_print(array_prog);
        } else {
          fprintf(stdout, "[PSA] Failed array partitioning.\n");
        }
      }
  #ifdef PRINT_ARRAY_PARTITIONING_TRANS_PROGRAM
      fprintf(stdout, "[PSA] Dump out the transformed program after array partitioning.\n");
      pluto_print_program(array_prog, srcFileName, "array_part");
  #endif
#endif    
 
#ifdef PRINT_DEPS_POST_PE_OPTIMIZATION
      psa_print_trans_deps(array_prog);
#endif 
/*
 * *******************************************
 * Stage: Code Generation
 * Step: Virtual Systolic Array (VSA) Generation
 * *******************************************
 */
#ifdef DUMP_VSA 
      /* Jie Added - Start */
      fprintf(stdout, "[PSA] ***************************************\n");
      fprintf(stdout, "[PSA] Generate Virtual Systolic Array (VSA).\n");    
      /* Complete the rest of VSA fields */
      /* ARRAY_PART_BAND_WIDTH */
      vsa_band_width_extract(array_prog, psa_vsa);
#ifdef T2S_CODEGEN
      /* T2S_ITERS */
      vsa_t2s_iter_extract(array_prog, psa_vsa);
      /* T2S_IO */
      vsa_t2s_IO_extract(array_prog, psa_vsa);
#else 
//      /* IO */
//      vsa_IO_extract(array_prog, psa_vsa);
      /* ARRAY_SHAPE */
      vsa_array_shape_extract(array_prog, psa_vsa);
#endif      
//      pluto_prog_to_vsa(prog, psa_vsa);
      FILE *vsa_fp = fopen("vsa.json", "w");
      if (vsa_fp) {
        psa_vsa_pretty_print(vsa_fp, psa_vsa);
      } else {
        fprintf(stdout, "[PSA] ERROR! File %s can't be opened!\n", "vsa.json");
        return 1;
      }
      fclose(vsa_fp);
#endif

/*
 * *******************************************
 * Stage: Code Generation
 * Step: T2S Input Generation
 * *******************************************
 */
#ifdef T2S_CODEGEN
      fprintf(stdout, "[PSA] ***********************\n");
      fprintf(stdout, "[PSA] Generate T2S inputs.\n");
      /* Get basename */
      char *basec, *bname;
      basec = strdup(srcFileName);
      bname = basename(basec);

      char *dumpFileName;
      dumpFileName = malloc(strlen(bname) - 2 + strlen(".") + strlen("t2s") + strlen(".cpp") + 1);
      strncpy(dumpFileName, bname, strlen(bname) - 2);
      dumpFileName[strlen(bname) - 2] = '\0';
      strcat(dumpFileName, ".");
      strcat(dumpFileName, "t2s");
      strcat(dumpFileName, ".cpp");

      FILE *t2s_fp = fopen(dumpFileName, "w");
      if (t2s_fp) {
        psa_t2s_codegen(t2s_fp, psa_vsa);
      } else {
        fprintf(stdout, "[PSA] ERROR! File %s can't be opened!\n", "t2s.cpp");
        return 1;
      }
      fclose(t2s_fp);
      /* Free Memory */
      free(basec);
      free(dumpFileName);
      /* Free Memory */
#endif

/*
 * *******************************************
 * Stage: Code Generation
 * Step: Intel OpenCL Code Generation
 * *******************************************
 */
#ifdef INTEL_CODEGEN
      fprintf(stdout, "[PSA] ***********************\n");
      fprintf(stdout, "[PSA] Generate Intel OpenCL inputs.\n");
      psa_intel_codegen(array_prog, psa_vsa, srcFileName);
#endif

/*
 * *******************************************
 * Stage: Code Generation
 * Step: Xilinx HLS Code Generation
 * *******************************************
 */
#ifdef XILINX_CODEGEN
      fprintf(stdout, "[PSA] ***********************\n");
      fpritnf(stdout, "[PSA] Generate Xilinx HLS inputs.\n");
      psa_xlinx_codegen(array_prog, psa_vsa, srcFileName);
#endif

      /* Jie Added - End */
  
      // if (options->tile) {
      //   pluto_tile(prog);
      // } else {
      //   if (options->intratileopt) {
      //     pluto_intra_tile_optimize(prog, 0);
      //   }
      // }
    
      // if (options->parallel && !options->tile && !options->identity) {
      //   /* Obtain wavefront/pipelined parallelization by skewing if
      //    * necessary */
      //   unsigned nbands;
      //   Band **bands;
      //   pluto_compute_dep_satisfaction(prog);
      //   bands = pluto_get_outermost_permutable_bands(prog, &nbands);
      //   bool retval = pluto_create_tile_schedule(prog, bands, nbands);
      //   pluto_bands_free(bands, nbands);
    
      //   /* If the user hasn't supplied --tile and there is only pipelined
      //    * parallelism, we will warn the user */
      //   if (retval) {
      //     printf("[pluto] WARNING: pipelined parallelism exists and --tile is not "
      //            "used.\n");
      //     printf("\tUse --tile for better parallelization \n");
      //     fprintf(stdout, "[pluto] After skewing:\n");
      //     pluto_transformations_pretty_print(prog);
      //   }
      // }
    
      // if (options->unroll) {
      //   /* Will generate a .unroll file */
      //   /* plann/plorc needs a .params */
      //   FILE *paramsFP = fopen(".params", "w");
      //   if (paramsFP) {
      //     int i;
      //     for (i = 0; i < prog->npar; i++) {
      //       fprintf(paramsFP, "%s\n", prog->params[i]);
      //     }
      //     fclose(paramsFP);
      //   }
      //   pluto_detect_mark_unrollable_loops(prog);
      // }
 
/*
 * *******************************************
 * Stage: Code Generation
 * Step: CPU Program Generation
 * *******************************************
 */
#ifdef CPU_CODEGEN 
      if (!options->pet && !strcmp(srcFileName, "stdin")) {
        // input stdin == output stdout
        pluto_populate_scop(scop, array_prog, options);
        osl_scop_print(stdout, scop);
      } else { // do the usual Pluto stuff
//        /* NO MORE TRANSFORMATIONS BEYOND THIS POINT */
//        /* Since meta info about loops is printed to be processed by scripts - if
//         * transformations are performed, changed loop order/iterator names will
//         * be missed. */
//        gen_unroll_file(prog);
        fprintf(stdout, "[PSA] ************************\n");
        fprintf(stdout, "[PSA] Generate CPU program.\n");
        fflush(stdout);
        pluto_print_program(array_prog, srcFileName, "cpu");
      }
#endif
    
      pluto_prog_free(array_prog);
      array_prog = NULL;     
      vsa_free(psa_vsa);
    }
    /* Free Memory */    
    free(progs);
    pluto_prog_free(reuse_prog);
    /* Free Memory */
  }
  /* Free Memory */
//  pluto_options_free(options);  
  osl_scop_free(scop);
  free(reuse_progs);
  pluto_prog_free(prog);
  /* Free Memory */

  return 0;
}
