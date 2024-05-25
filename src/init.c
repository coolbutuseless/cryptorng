
// #define R_NO_REMAP
#include <R.h>
#include <Rinternals.h>

extern SEXP rcrypto_raw_(SEXP n_);
extern SEXP rcrypto_chr_(SEXP n_);
extern SEXP rcrypto_lgl_(SEXP n_);
extern SEXP rcrypto_int_(SEXP n_);
extern SEXP rcrypto_dbl_(SEXP n_);

static const R_CallMethodDef CEntries[] = {
  
  {"rcrypto_raw_", (DL_FUNC) &rcrypto_raw_, 1},
  {"rcrypto_chr_", (DL_FUNC) &rcrypto_chr_, 1},
  {"rcrypto_lgl_", (DL_FUNC) &rcrypto_lgl_, 1},
  {"rcrypto_int_", (DL_FUNC) &rcrypto_int_, 1},
  {"rcrypto_dbl_", (DL_FUNC) &rcrypto_dbl_, 1},
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



