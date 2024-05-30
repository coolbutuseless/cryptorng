

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>

#if defined(_WIN32)  
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <bcrypt.h>
#elif defined(__linux__)
#include <sys/syscall.h>
#endif


#include <R.h>
#include <Rinternals.h>
#include <Rdefines.h>

//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Bytes to hex
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
char *bytes_to_hex(uint8_t *buf, size_t len) {
  static const char hexmap[] = {'0', '1', '2', '3', '4', '5', '6', '7',
                                '8', '9', 'a', 'b', 'c', 'd', 'e', 'f'};
  
  char *str = (char *)calloc(len * 2 + 1, 1);
  if (str == NULL) {
    error("bytes_to_hex() couldn't allocate %zu bytes", len * 2 + 1);
  }
  str[0] = '\0';
  
  for (size_t i = 0; i < len; i++) {
    str[2 * i]     = hexmap[(buf[i] & 0xF0) >> 4];
    str[2 * i + 1] = hexmap[ buf[i] & 0x0F];
  }
  
  str[len * 2] = '\0';
  return str;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get random bytes from the system RNG  (C Callable)
//
// @param buf pre-allocated buffer in which to put the random bytes
// @param n number of bytes.  Note: when a system RNG runs out of entropy
//        it may return fewer bytes than expected. This function throws an 
//        error in this situation
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
void rcrypto(void *buf, size_t n) {
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // macOS and BSD support arc4random_buf()
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#if defined(__APPLE__) || defined(__FreeBSD__) || defined(__OpenBSD__) || defined(__NetBSD__)
  // void arc4random_buf(void buf[.n], size_t n);
  arc4random_buf(buf, n); 
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Windows: use BCryptGenRandom
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined(_WIN32)  
  // NTSTATUS BCryptGenRandom(
  //     [in, out] BCRYPT_ALG_HANDLE hAlgorithm,
  //     [in, out] PUCHAR            pbBuffer,
  //     [in]      ULONG             cbBuffer,
  //     [in]      ULONG             dwFlags
  // );
  // dwFlags = BCRYPT_USE_SYSTEM_PREFERRED_RNG - Use the system-preferred random 
  // number generator algorithm. The hAlgorithm parameter must be NULL. 
  size_t status = (size_t)BCryptGenRandom( NULL, ( PUCHAR ) buf, n, BCRYPT_USE_SYSTEM_PREFERRED_RNG );
  // Return value is 'NTSTATUS' value. STATUS_SUCCESS = 0.
  if (status != 0) {
    error("rcrypto() windows error: Status = %zu.\n", status);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Linux use 'Sys_getrandom()'
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined(__linux__)
  long status = (long)syscall( SYS_getrandom, buf, n, 0 );
  if (status < 0 || status != n) {
    error("rcrypto() linux error: Status = %zu.\n", status);
  }
  
#else
#error no secure rcrypto() implemented for this platform
#endif 
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get random bytes from the system RNG  (R Callable)
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rcrypto_raw_(SEXP n_) {
  
  if (asInteger(n_) <= 0) {
    error("rcrypto_(): 'n' must be a positive integer");
  }
  size_t n = (size_t)asInteger(n_);
  SEXP res_ = PROTECT(allocVector(RAWSXP, (R_xlen_t)n));
  
  rcrypto(RAW(res_), n);
  
  UNPROTECT(1);
  return res_;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get random bytes from the system RNG as hexadecimal string
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rcrypto_chr_(SEXP n_) {
  
  if (asInteger(n_) <= 0) {
    error("rcrypto_(): 'n' must be a positive integer");
  }
  size_t n = (size_t)asInteger(n_);
  void *buf = R_alloc(n, 1);
  
  rcrypto(buf, n);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wrap bytes for R and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = R_NilValue;
  
  char *hex = bytes_to_hex(buf, n);
  res_ = PROTECT(allocVector(STRSXP, 1));
  SET_STRING_ELT(res_, 0, mkChar(hex));
  free(hex);
    
  UNPROTECT(1);
  return res_;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get random bytes from the system RNG  as logical values
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rcrypto_lgl_(SEXP n_) {
  
  if (asInteger(n_) <= 0) {
    error("rcrypto_(): 'n' must be a positive integer");
  }
  size_t n = (size_t)asInteger(n_);
  uint8_t *buf = (uint8_t *)R_alloc(n, 1);
  
  rcrypto(buf, n);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wrap bytes for R and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(allocVector(LGLSXP, (R_xlen_t)n));
  
  int32_t *ptr = LOGICAL(res_);
  for (size_t i = 0; i < n; i++) {
    ptr[i] = buf[i] > 127;
  }  

  UNPROTECT(1);
  return res_;
}



//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get random int32_t 
//
// @param n_ number of integers
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rcrypto_int_(SEXP n_) {
  
  if (asInteger(n_) <= 0) {
    error("rcrypto_int_(): 'n' must be a positive integer");
  }
  size_t n = (size_t)asInteger(n_);
  SEXP res_ = PROTECT(allocVector(INTSXP, (R_xlen_t)n));
  
  rcrypto((void *)INTEGER(res_), n * sizeof(int));
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Remove NAs
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  int *ptr = INTEGER(res_);
  for (size_t i = 0; i < n; i++) {
    while (ptr[i] == NA_INTEGER) {
      rcrypto((void *)(ptr + i), 1 * sizeof(int));
    }
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  UNPROTECT(1);
  return res_;
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get random floats in the range [0, 1)
//
// @param n_ number of floats
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rcrypto_dbl_(SEXP n_) {
  
  if (asInteger(n_) <= 0) {
    error("rcrypto_unif_(): 'n' must be a positive integer");
  }
  size_t n = (size_t)asInteger(n_);
  SEXP res_ = PROTECT(allocVector(REALSXP, (R_xlen_t)n));
  double *dptr = REAL(res_);
  uint64_t *iptr = (uint64_t *)dptr;
  
  rcrypto((void *)dptr, n * sizeof(double));
  
  // Convert 64-bit unsigned integer to double in range [0, 1)
  // Like xoroshiro does: https://prng.di.unimi.it/
  for (size_t i = 0; i < n; i++) {
    dptr[i] = (double)(iptr[i] >> 11) * 0x1.0p-53;
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // tidy and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  UNPROTECT(1);
  return res_;
}





