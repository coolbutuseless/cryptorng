
<!-- README.md is generated from README.Rmd. Please edit that file -->

# `{cryptorng}` Generate random bytes from cryptographically secure random number sources

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
[![R-CMD-check](https://github.com/coolbutuseless/cryptorng/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/coolbutuseless/cryptorng/actions/workflows/R-CMD-check.yaml)
<!-- badges: end -->

`{cryptorng}` provides access to OS-provided [cryptographically secure
pseudorandom number generators
(CSPRNG)](https://en.wikipedia.org/wiki/Cryptographically_secure_pseudorandom_number_generator).

A good source of random bytes is essential for generating secure
encryption inputs like keys and nonces (e.g. for
[`rmonocypher`](https://github.com/coolbutuseless/rmonocypher)).

The method used for generating random values varies depending on the OS:

| OS           | CSPRNG                            |
|--------------|-----------------------------------|
| macOS, \*BSD | `arc4random_buf()`                |
| Linux        | `SYS_getrandom()` via `syscall()` |
| Windows      | `BCryptGenRandom()`               |

All these random number generators are internally seeded by the OS using
entropy gathered from multiple sources and use random number algorithms
which are considered cryptographically secure.

## Installation

You can install from
[GitHub](https://github.com/coolbutuseless/cryptorng) with:

``` r
# install.package('remotes')
remotes::install_github('coolbutuseless/cryptorng')
```

## Generate random bytes

``` r
library(cryptorng)

rcrypto(16)
#>  [1] 17 37 86 a0 c4 28 40 24 ad 83 2e 8c 7b af f3 39
rcrypto(16, type = 'string')
#> [1] "e3405fcc36833ff4761938b3563c7c61"
```

# C code

This package and code is MIT licensed. Please feel free to
incorporate/adapt this code into your own project.

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
