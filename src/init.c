
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP generate_bytes_(SEXP n_, SEXP type_);

static const R_CallMethodDef CEntries[] = {

  {"generate_bytes_", (DL_FUNC) &generate_bytes_, 2},
  {NULL , NULL, 0}
};


void R_init_cryptorng(DllInfo *info) {
  R_registerRoutines(
    info,      // DllInfo
    NULL,      // .C
    CEntries,  // .Call
    NULL,      // Fortran
    NULL       // External
  );
  R_useDynamicSymbols(info, FALSE);
}



