/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_URE_H
#define _PSA_URE_H

#include "pluto.h"
#include "psa_vsa.h"

int t2s_compare_stmt_order(Stmt *stmt1, Stmt *stmt2, int band_width);
URE **create_RAR_UREs(PlutoAccess *acc, PlutoProg *prog, VSA *vsa, int *URE_num);
URE **create_drain_UREs(PlutoAccess *acc, PlutoProg *prog, VSA *vsa, int *URE_num);
URE **stmt_to_UREs(Stmt *stmt, PlutoProg *prog, VSA *vsa, int *URE_num);
URE **URE_append(URE **list1, int *num1, URE **list2, int num2);
char *create_URE_name(char **URE_names, int URE_num, char *var_name);
#endif
