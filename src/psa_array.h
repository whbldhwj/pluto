/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_ARRAY_H
#define _PSA_ARRAY_H

#include "pluto.h"
#include "constraints.h"
#include "program.h"

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

//HyperplaneProperties *hyperplane_properties_dup(const HyperplaneProperties *hProps);
Dep *pluto_dep_prog_dup(Dep *d, int num_hyperplanes);
PlutoOptions *pluto_options_dup(const PlutoOptions *options);
Graph *graph_dup(const Graph *graph);
PlutoProg *pluto_prog_dup(const PlutoProg *prog);
DepDis get_dep_distance(const Dep *dep, const PlutoProg *prog, int level);
void psa_compute_dep_distances(PlutoProg *prog);
PlutoProg **sa_candidates_generation_band_sync(Band *band, int array_dim,
              PlutoProg *prog, int *nprogs);
PlutoProg **sa_candidates_generation_band_async(Band *band, int array_dim, 
              PlutoProg *prog, int *nprogs);
PlutoProg **sa_candidates_generation(PlutoProg *prog, int *nprogs_p);
void sa_candidates_smart_pick(PlutoProg **progs, int nprogs);

#endif
