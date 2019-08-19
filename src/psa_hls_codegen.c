/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_vsa.h"
#include "psa_hls_codegen.h"

void psa_intel_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name) {
  /* PE Code */
  PlutoProg *new_prog = NULL;
  new_prog = pluto_prog_dup(prog);
  psa_PE_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);

  /* Data Transfer Code */
  new_prog = pluto_prog_dup(prog);
  psa_data_trans_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);

  /* Top Kernel Code */
  new_prog = pluto_prog_dup(prog);
  psa_top_kernel_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);

  /* Header Code */
  new_prog = pluto_prog_dup(prog);
  psa_header_codegen(new_prog, vsa, src_file_name, 0);
  pluto_prog_free(new_prog);
}

void psa_xilinx_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name) {
  /* PE Code */  
  PlutoProg *new_prog = NULL;
  new_prog = pluto_prog_dup(prog);
  psa_PE_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog);

  /* Data Transfer Code */
  new_prog = pluto_prog_dup(prog);
  psa_data_trans_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog);

  /* Top Kernel Code */
  new_prog = pluto_prog_dup(prog);
  psa_top_kernel_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog);

  /* Header Code */
  new_prog = pluto_prog_dup(prog);
  psa_header_codegen(new_prog, vsa, src_file_name, 1);
  pluto_prog_free(new_prog); 
}

void psa_data_trans_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {

}

void psa_top_kernel_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {

}

/* 
 * This function generates PE function code.
 * It will call different APIs depedending on the target platform.
 * 0 - Intel OpenCL
 * 1 - Xilinx HLS
 */
void psa_PE_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {
  /* Get basename */
  char *basec, *bname;
  basec = strdup(src_file_name);
  bname = basename(basec);

  char *dump_file_name;
  dump_file_name = malloc(strlen(bname) - 2 + strlen(".") + strlen("PE.intel") + strlen(".c") + 1);
  strncpy(dump_file_name, bname, strlen(bname) - 2);
  dump_file_name[strlen(bname) - 2] = '\0';
  strcat(dump_file_name, ".");
  strcat(dump_file_name, "PE.intel");
  strcat(dump_file_name, ".c");

  FILE *fp = fopen(dump_file_name, "w");
  if (!fp) {
    fprintf(stdout, "[PSA] Error! File %s can't be opened!\n", dump_file_name);
    return;
  }
  free(basec);
  free(dump_file_name);

  // TODO: Handle irregular space domain in the future
  if (target == 0) {
    fprintf(fp, "__attribute__((max_global_work_dim(0)))\n");
    fprintf(fp, "__attribute__((autorun))");
    fprintf(fp, "__attribtue__((num_compute_units(SYS_ARRAY_NUM_ROWS, SYS_ARRAY_NUM_COLS)))");
  }

  fclose(fp);
}

void psa_header_codegen(PlutoProg *prog, VSA *vsa, char *src_file_name, int target) {
  /* Get basename */
  char *basec, *bname;
  basec = strdup(src_file_name);
  bname = basename(basec);

  char *dump_file_name;
  dump_file_name = malloc(strlen(bname) - 2 + strlen(".") + strlen("intel") + strlen(".h") + 1);
  strncpy(dump_file_name, bname, strlen(bname) - 2);
  dump_file_name[strlen(bname) - 2] = '\0';
  strcat(dump_file_name, ".");
  strcat(dump_file_name, "intel");
  strcat(dump_file_name, ".h");

  FILE *fp = fopen(dump_file_name, "w");
  if (!fp) {
    fprintf(stdout, "[PSA] Error! File %s can't be opened!\n", dump_file_name);
    return;
  }
  free(basec);
  free(dump_file_name);

  /* Macros */
  fprintf(fp, "#define SYS_ARRAY_NUM_ROWS %d\n", vsa->sa_rows);
  fprintf(fp, "$define SYS_ARRAY_NUM_COLS %d\n", vsa->sa_cols);

  fclose(fp); 
}
