# Pluto
#
# Copyright (C) Uday Bondhugula 2007
#

SUBDIRS = 

MAINTAINERCLEANFILES = Makefile.in


if EXTERNAL_ISL
# ISL_INCLUDE, ISL_LIBADD were already defined
else
ISL_INCLUDE += -I$(top_srcdir)/isl/include
ISL_LIBADD += $(top_srcdir)/isl/libisl.la
endif

bin_PROGRAMS = pluto

pluto_SOURCES = psa_dep.c constraints.c constraints_polylib.c constraints_isl.c math_support.c gurobi-plugin.c ddg.c transforms.c pluto.c post_transform.c framework.c framework-dfp.c pluto_codegen_if.c polyloop.c ast_transform.c iss.c main.c psa_dep.h constraints.h math_support.h pluto.h program.c program.h tile.c unit_tests.c psa_array.c psa_array.h psa_partition.c psa_partition.h psa_pe_opt.c psa_pe_opt.h psa_vsa.h psa_vsa.c psa_helpers.h distmem.h distmem.c psa_vsa_dfc.c psa_vsa_dfc.h psa_vsa_pe.c psa_vsa_pe.h psa_knobs.h psa_t2s_codegen.c psa_ure.c psa_hls_codegen.c psa_io.c psa_hls_codegen_helpers.c

nodist_EXTRA_pluto_SOURCES = dummy.cc

if PLUTO_DEBUG
OPT_FLAGS = -O3 -funroll-loops -fopenmp
else
OPT_FLAGS = 
DEBUG_FLAGS = -g
endif

pluto_CPPFLAGS = $(OPT_FLAGS) $(DEBUG_FLAGS) -funroll-loops -fopenmp \
   -DSCOPLIB_INT_T_IS_LONGLONG -DCLOOG_INT_GMP -DPLUTO_OPENCL \
   -I../include \
   -I../piplib/include \
   -I../clan/include \
   -I../pet/include \
   $(ISL_INCLUDE) \
   -I../cloog-isl/include \
   -I../openscop/include \
   -I../candl/include \
   -I../polylib/include

pluto_LDFLAGS = -static -fopenmp
pluto_LDADD =  \
   ../piplib/libpiplib_dp.la \
   ../openscop/libosl.la \
   ../clan/libclan.la \
   ../cloog-isl/libcloog-isl.la \
   ../pet/libpet.la \
   $(ISL_LIBADD) \
   ../polylib/libpolylib64.la \
   ../candl/libcandl.la \
   -lm

lib_LTLIBRARIES = libpluto.la

# libpluto doesn't use polylib-based functions
libpluto_la_SOURCES = psa_dep.c constraints.c constraints_isl.c math_support.c ddg.c transforms.c pluto.c post_transform.c framework.c framework-dfp.c polyloop.c iss.c libpluto.c libpluto_dummy.c psa_dep.h constraints.h math_support.h pluto.h program.c program.h tile.c
libpluto_la_CPPFLAGS = $(OPT_FLAGS) $(DEBUG_FLAGS) \
   -DLIB_PLUTO -DSCOPLIB_INT_T_IS_LONGLONG -DCLOOG_INT_GMP \
   -I../include \
   -I../piplib/include \
   -I../clan/include \
   -I../pet/include \
   -I../openscop/include \
   -I../candl/include \
   -I../cloog-isl/include \
   $(ISL_INCLUDE)

# libpluto doesn't use polylib
libpluto_la_LIBADD = \
   ../piplib/libpiplib_dp.la \
   ../pet/libpet.la \
   ../openscop/libosl.la \
   ../clan/libclan.la \
   ../candl/libcandl.la \
   $(ISL_LIBADD) \
   -lm -lgomp
libpluto_la_LDFLAGS = -version-info 1:0:1
