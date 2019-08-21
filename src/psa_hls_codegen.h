/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#ifndef _PSA_HLS_CODEGEN_H
#define _PSA_HLS_CODEGEN_H

#include "pluto.h"
#include "assert.h"
#include "psa_hls_codegen_helpers.h"

void psa_intel_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name);
void psa_xilinx_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name);

void psa_data_trans_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target);
void psa_top_kernel_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target);
void psa_PE_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target);
void psa_header_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target);

#endif
