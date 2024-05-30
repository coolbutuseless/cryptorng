
<!-- README.md is generated from README.Rmd. Please edit that file -->

# Generate random numbers from your OS cryptographically secure RNG

<!-- badges: start -->

![](https://img.shields.io/badge/cool-useless-green.svg)
[![CRAN](http://www.r-pkg.org/badges/version/cryptorng)](https://cran.r-project.org/package=cryptorng)
[![R-CMD-check](https://github.com/coolbutuseless/cryptorng/actions/workflows/R-CMD-check.yaml/badge.svg)](https://github.com/coolbutuseless/cryptorng/actions/workflows/R-CMD-check.yaml)
<!-- badges: end -->

`{cryptorng}` provides access to your operating system’s
[cryptographically secure pseudorandom number generator
(CSPRNG)](https://en.wikipedia.org/wiki/Cryptographically_secure_pseudorandom_number_generator).

System CSPRNGs provide high-quality, uniform random bytes which are
secure against cryptographic analysis.

These bytes are suitable for use in other crypographic processes e.g.

- seeding another random number generator
- as keys, seeds, nonces, salts etc for encryption

#### What sets `{cryptorng}` apart from other RNGs in R?

- These numbers are as random as you’re going to get on general-purpose
  PCs which don’t have specialist RNG hardware.  
- There’s no seed to set (the OS sets that internally using accumulated
  system entropy)
  - Depending on your system, the CSPRNG may be continually re-seeded as
    more entropy becomes available.
- Multiple threads in parallel won’t read the same bytes.

#### How can I generate random values sampled from a uniform distribution?

``` r
library(cryptorng)
# uniform random bytes
rcrypto(16)
```

    #>  [1] 9d 29 5b 9a 4f 7c fd 4a 1e f1 c1 e1 54 34 30 1d

``` r
# uniform random bytes as a hexadecimal string
rcrypto(16, type = 'chr')
```

    #> [1] "95b9bbed52bb1178797488ede2905a43"

``` r
# random logical values
rcrypto(16, type = 'lgl')
```

    #>  [1] FALSE FALSE FALSE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE  TRUE FALSE
    #> [13] FALSE FALSE FALSE FALSE

``` r
# uniform random integers
rcrypto(16, type = 'int')
```

    #>  [1] -1462110675  -149265429    24479471 -1108190907   891595010   786375080
    #>  [7]  2062005447    56916768  1845302616  -852269330 -2093358023    43116425
    #> [13]  1853964883  1208525937 -1385749686   670879908

``` r
# uniform random doubles in the range [0, 1)
rcrypto(16, type = 'dbl')
```

    #>  [1] 0.6866093 0.1157316 0.4953703 0.5625702 0.8608402 0.7837645 0.9046331
    #>  [8] 0.9356881 0.2529249 0.6325323 0.2894815 0.7857760 0.2388977 0.8496769
    #> [15] 0.2457765 0.4280983

## Installation

This package can be installed from CRAN

``` r
install.packages('cryptorng')
```

You can install the latest development version from
[GitHub](https://github.com/coolbutuseless/cryptorng) with:

``` r
# install.packages('remotes')
remotes::install_github('coolbutuseless/cryptorng')
```

Pre-built source/binary versions can also be installed from
[R-universe](https://r-universe.dev)

``` r
install.packages('cryptorng', repos = c('https://coolbutuseless.r-universe.dev', 'https://cloud.r-project.org'))
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
