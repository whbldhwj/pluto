/*
 * PolySA: Polyhedral-Based Systolic Array Auto-Compiler
 *
 * Author: Jie Wang
 */
#include "pluto.h"
#include "psa_vsa.h"

/* 
 * This function generates T2S inputs
 */
void psa_t2s_codegen(FILE *fp, const VSA *vsa) {
  /* MISC info */
  fprintf(fp, "// This is automatically generated by PolySA\n\n");

  /* Input declarations */
  fprintf(fp, "// Input declarations\n");
  for (int i = 0; i < vsa->array_num; i++) {
    fprintf(fp, "ImageParam %s(type_of<%s>(), %d, \"%s\");\n", vsa->arrays[i]->text, 
        vsa->arrays[i]->data_type, vsa->arrays[i]->dim, vsa->arrays[i]->text);
  }
  fprintf(fp, "\n");

  /* Global setting */
  fprintf(fp, "// Global setting\n");
  fprintf(fp, "T2S.setting(LoopOrder::Inward, Place::Device);\n\n");

  /* Function declarations */
  fprintf(fp, "// Function declarations\n");
  fprintf(fp, "Func");
  int func_num = vsa->evar_num + vsa->edvar_num + vsa->ivar_num + vsa->idvar_num + 1;
  for (int i = 0; i < vsa->evar_num; i++) {
    fprintf(fp, " %s,", vsa->evar_names[i]);
  }
  for (int i = 0; i < vsa->edvar_num; i++) {
    fprintf(fp, " %s,", vsa->edvar_names[i]);
  }
  for (int i = 0; i < vsa->ivar_num; i++) {
    fprintf(fp, " %s,", vsa->ivar_names[i]);
  }
  for (int i = 0; i < vsa->idvar_num; i++) {
    fprintf(fp, " %s,", vsa->idvar_names[i]);
  }
  fprintf(fp, " APP;\n");
  fprintf(fp, "\n");

  /* Variable declarations */
  fprintf(fp, "// Variable declarations\n");
  fprintf(fp, "Var");
  for (int i = 0; i < vsa->t2s_iter_num; i++) {
    if (i == vsa->t2s_iter_num - 1) {
      fprintf(fp, " %s;\n", vsa->t2s_iters[i]);
    } else {
      fprintf(fp, " %s,", vsa->t2s_iters[i]);
    }
  }
  fprintf(fp, "\n");

  /* UREs */
  fprintf(fp, "// UREs\n");
  for (int i = 0; i < vsa->URE_num; i++) {
    fprintf(fp, "%s\n", vsa->UREs[i]->text);
  }
  fprintf(fp, "\n");

  /* URE domain */
  fprintf(fp, "// Build the initial loop nest\n");
  fprintf(fp, "APP.merge_UREs(");
  for (int i = 0; i < vsa->URE_num; i++) {
    if (i == 0) {
      fprintf(fp, vsa->UREs[i]->name);
    } else {
      fprintf(fp, ", %s", vsa->UREs[i]->name);
    }
  }
  fprintf(fp, ")\n");
  fprintf(fp, "   .domain(");
  for (int i = 0; i < vsa->domain_exp_num; i++) {
    if (i < vsa->domain_exp_num - 1) {
      fprintf(fp, "%s,\n", vsa->domain_exps[i]);
    } else {
      fprintf(fp, "%s", vsa->domain_exps[i]);
    }
  }
  fprintf(fp, ");\n\n");

  /* Optimization */
  
  /* Space-time transformation */

  /* I/O network */

  /* I/O network spec */
}
