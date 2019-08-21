/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_HLS_CODEGEN_HELPER_H
#define _PSA_HLS_CODEGEN_HELPER_H

#include "pluto.h"

char *psa_gen_PE_channel_acc(Vec *trans_dir);
char *psa_gen_acc_str(PlutoAccess *acc, char **names);
char *psa_gen_new_acc_str(Stmt *stmt, PlutoAccess *acc, char **names);

#endif
