
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP rcrypto_(SEXP n_, SEXP type_);

static const R_CallMethodDef CEntries[] = {

  {"rcrypto_", (DL_FUNC) &rcrypto_, 2},
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



