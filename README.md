
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
- as keys, seeds, nonces, salts etc for encryption

#### What sets `{cryptorng}` apart from other RNGs in R?

These bytes are as random as you’re going to get on general-purpose PCs
which don’t have a hardware RNG. There’s no seed to set (the OS sets
that internally using accumulated system entropy), and multiple threads
in parallel won’t read the same bytes.

``` r
library(cryptorng)

# Generate random bytes
rcrypto(16)
```

    #>  [1] 97 42 40 16 af 40 ac 37 ea 1e e5 26 f4 c2 d4 92

``` r
rcrypto(16, type = 'string')
```

    #> [1] "8f4624858d748b37785cf5b170169319"

``` r
# Generate uniform random numbers in the range [0, 1]
# This function does not generate NA values
rcrypto_unif(5)
```

    #> [1] 0.66273988 0.60932209 0.94762338 0.56791628 0.07343798

``` r
# Generate some random integers. 
# This function dose not generate NA values
rcrypto_int(5)
```

    #> [1]  -663423129  1205202332  -420409181 -1482573832 -1686519074

``` r
# Seed the standard R random number generator
set.seed(rcrypto_int(5))
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

| OS           | CSPRNG                            | Notes                                                                                                   |
|--------------|-----------------------------------|---------------------------------------------------------------------------------------------------------|
| macOS, \*BSD | `arc4random_buf()`                | macOS [arc4rand.c](http://www.opensource.apple.com/source/Libc/Libc-1044.10.1/gen/FreeBSD/arc4random.c) |
| Linux        | `SYS_getrandom()` via `syscall()` |                                                                                                         |
| Windows      | `BCryptGenRandom()`               |                                                                                                         |

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
