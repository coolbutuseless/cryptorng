
<!-- README.md is generated from README.Rmd. Please edit that file -->

# `{cryptorng}` Generate random bytes from your OS cryptographically secure RNG

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
[![R-CMD-check](https://github.com/coolbutuseless/cryptorng/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/coolbutuseless/cryptorng/actions/workflows/R-CMD-check.yaml)
<!-- badges: end -->

`{cryptorng}` provides access to OS-provided [cryptographically secure
pseudorandom number generators
(CSPRNG)](https://en.wikipedia.org/wiki/Cryptographically_secure_pseudorandom_number_generator).

**Summary:** OS-provided CSPRNGs provide high-quality random bytes.

These bytes are suitable for use in other crypographic processes e.g.

- seeding another random number generator
- as keys, seeds, nonces, salts etc for other
- generating an encryption key.

``` r
library(cryptorng)

# Generate random bytes
rcrypto(16)
```

    #>  [1] 91 06 5a 61 38 75 db fa 0f 8d 8f cf 54 57 e1 09

``` r
rcrypto(16, type = 'string')
```

    #> [1] "4b8f4a386eaa0ae12b8d9a9eb5b3047e"

``` r
# Generate some random integers  (Beware of NAs using this approach)
N <- 5
rand_ints <- na.omit(readBin(rcrypto(N * 4), integer(), N))
rand_ints
```

    #> [1] 1467300025  582430056 -792231472   88556227  902676152

``` r
# Seed the standard R random number generator
set.seed(rand_ints[1])
```

## Installation

You can install from
[GitHub](https://github.com/coolbutuseless/cryptorng) with:

``` r
# install.package('remotes')
remotes::install_github('coolbutuseless/cryptorng')
```

## Technical bits

Major operating systems (e.g. macOS, Linux, Windows) now come with
CSPRNG functionality built-in.  
These RNGs (Random Number Generators) are cryptographically secure -
meaning that

- bytes generated pass statistical randomness tests
- it is impossible to reconstruct the prior stream of numbers if the
  current internal state of the RNG is revealed

These are still only **pseudo**-random number generators though, as the
production of numbers is deterministic given the seed. So a high entropy
seed which is kept secret is considered standard practice.

For these OS-provided CSPRNGs, the entropy (and the seeding) is provided
by the OS itself - using such things as hardware RNGs, timing jitter,
network traffic, disk activity. The initial seed is never revealed to
the user, and
[*reseeding*](https://en.wikipedia.org/wiki//dev/random#BSD_systems) may
take place to ensure that additional entropy is used when available.

The C function for generating random values varies depending on the OS:

| OS           | CSPRNG                            |
|--------------|-----------------------------------|
| macOS, \*BSD | `arc4random_buf()`                |
| Linux        | `SYS_getrandom()` via `syscall()` |
| Windows      | `BCryptGenRandom()`               |

All of these random number generators are internally seeded by the OS
using entropy gathered from multiple sources and use random number
algorithms which are considered cryptographically secure.

## C code

This package and code is MIT licensed. Please feel free to
incorporate/adapt this code into your own project.

The following is a snapshot of the core function which calls the
appropriate CSPRNG for your system.

Note: If your system is not supported, please open an
[issue](https://github.com/coolbutuseless/cryptorng/issues) with
information on your systems CSPRNG and/or `/dev/random` information.

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
