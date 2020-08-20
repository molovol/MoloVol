/***** GCL Generated File *********************/
/* Automatically generated file, do not edit! */
/**********************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <dispatch/dispatch.h>
#include <OpenCL/opencl.h>
#include <OpenCL/gcl_priv.h>
#include "renderkernel.cl.h"

static void initBlocks(void);

// Initialize static data structures
static block_kernel_pair pair_map[1] = {
      { NULL, NULL }
};

static block_kernel_map bmap = { 0, 1, initBlocks, pair_map };

// Block function
void (^matMult_kernel)(const cl_ndrange *ndrange, cl_float* A, cl_char* C) =
^(const cl_ndrange *ndrange, cl_float* A, cl_char* C) {
  int err = 0;
  cl_kernel k = bmap.map[0].kernel;
  if (!k) {
    initBlocks();
    k = bmap.map[0].kernel;
  }
  if (!k)
    gcl_log_fatal("kernel matMult does not exist for device");
  kargs_struct kargs;
  gclCreateArgsAPPLE(k, &kargs);
  err |= gclSetKernelArgMemAPPLE(k, 0, A, &kargs);
  err |= gclSetKernelArgMemAPPLE(k, 1, C, &kargs);
  gcl_log_cl_fatal(err, "setting argument for matMult failed");
  err = gclExecKernelAPPLE(k, ndrange, &kargs);
  gcl_log_cl_fatal(err, "Executing matMult failed");
  gclDeleteArgsAPPLE(k, &kargs);
};

// Initialization functions
static void initBlocks(void) {
  const char* build_opts = "";
  static dispatch_once_t once;
  dispatch_once(&once,
    ^{ int err = gclBuildProgramBinaryAPPLE("/Users/benediktvogler/Projekte/ballpit/renderkernel.cl", "", &bmap, build_opts);
       if (!err) {
          assert(bmap.map[0].block_ptr == matMult_kernel && "mismatch block");
          bmap.map[0].kernel = clCreateKernel(bmap.program, "matMult", &err);
       }
     });
}

__attribute__((constructor))
static void RegisterMap(void) {
  gclRegisterBlockKernelMap(&bmap);
  bmap.map[0].block_ptr = matMult_kernel;
}

