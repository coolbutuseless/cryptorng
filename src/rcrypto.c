

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
// Pack bytes for return
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP wrap_bytes_for_return(void *buf, size_t N, SEXP type_) {
  
  SEXP res_ = R_NilValue;
  
  const char *type = CHAR(STRING_ELT(type_, 0));
  
  if (strcmp(type, "raw") == 0) {
    res_ = PROTECT(allocVector(RAWSXP, (R_xlen_t)N));
    memcpy(RAW(res_), buf, N);
  } else {
    char *hex = bytes_to_hex(buf, N);
    res_ = PROTECT(allocVector(STRSXP, 1));
    SET_STRING_ELT(res_, 0, mkChar(hex));
    free(hex);
  }
  
  UNPROTECT(1);
  return res_;
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
    error("cryptorng_windows() error: Status = %zu.\n", status);
  }
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Linux use 'Sys_getrandom()'
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#elif defined(__linux__)
  long status = (long)syscall( SYS_getrandom, buf, n, 0 );
  if (status < 0 || status != n) {
    error("cryptorng_linux() error: Status = %zu.\n", status);
  }
  
#else
#error no secrure rcrypto() implemented for this platform
#endif 
}


//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
// Get random bytes from the system RNG  (R Callable)
//
// @param n_ number of bytes
// @param type_ 'raw' or 'string'
//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
SEXP rcrypto_(SEXP n_, SEXP type_) {
  
  if (asInteger(n_) <= 0) {
    error("rcrypto_(): 'n' must be a positive integer");
  }
  size_t n = (size_t)asInteger(n_);
  void *buf = R_alloc(n, 1);
  
  rcrypto(buf, n);
  
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  // Wrap bytes for R and return
  //~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
  SEXP res_ = PROTECT(wrap_bytes_for_return(buf, n, type_));
  UNPROTECT(1);
  return res_;
}





