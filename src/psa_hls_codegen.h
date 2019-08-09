/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_HLS_CODEGEN_H
#define _PSA_HLS_CODEGEN_H

#include "pluto.h"
#include "assert.h"

void psa_intel_codegen(FILE *fp, const VSA *vsa);
void psa_xilinx_codegen(FILE *fp, const VSA *vsa);

#endif
