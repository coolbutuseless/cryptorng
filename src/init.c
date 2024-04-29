
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP rcrypto_(SEXP n_, SEXP type_);
extern SEXP rcrypto_unif_(SEXP n_);
extern SEXP rcrypto_int_(SEXP n_);

static const R_CallMethodDef CEntries[] = {
  
  {"rcrypto_", (DL_FUNC) &rcrypto_, 2},
  {"rcrypto_unif_", (DL_FUNC) &rcrypto_unif_, 1},
  {"rcrypto_int_", (DL_FUNC) &rcrypto_int_, 1},
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



