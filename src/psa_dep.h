/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 * 
 * Author: Jie Wang
 */
#ifndef _PSA_DEP_H
#define _PSA_DEP_H

#include "pluto.h"
#include "constraints.h"

#include "clan/clan.h"
#include "candl/candl.h"

#include "pet.h"
#include "osl/scop.h"

bool systolic_array_dep_checker(PlutoProg *prog);
void rar_scalar_filter(PlutoProg *prog);

#endif