
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
#' Generate random numbers using platform-specific cryptographically secure
#' pseudorandom number generators
#' 
#' @param n Number of random numbers to generate.
#'        Note: if the entropy pool is exhausted on your
#'        system it may not be able to provide the requested number of bytes -
#'        in this case an error is thrown.
#' @param type Type of returned values - 'raw', 'chr', 'lgl', 'int' or 'dbl'. 
#'        Default: 'raw'
#'        \describe{
#'          \item{'raw'}{Uniform random bytes from the CSPRNG returned as a raw vector}
#'          \item{'chr'}{Uniform random bytes from the CRPRNG returned as a hexadecimal string}
#'          \item{'lgl'}{Uniform random bytes return as random logical values}
#'          \item{'int'}{Combines 4 random bytes to create uniform random integers.  This output is
#'                       further filtered to remove any NA values which may occur}
#'          \item{'dbl'}{Combines 8 random bytes to create uniform random numbers in the range [0, 1)}
#'        }
#' 
#' 
#' @section Details for \code{type = 'dbl'}:
#' An 8-byte double-precision floating point number is obtained by first
#' concatenating 8 random bytes into an 8-byte unsigned integer (i.e. \code{uint64_t}).
#' 
#' This \code{uint64_t} value is converted to an 8-byte double using: 
#' \code{(x >> 11) * 0x1.0p-53}.
#' 
#' 
#' @section Details for \code{type = 'int'}:
#' A 4-byte random R integer value is obtained by concatenating 4 random bytes.
#' These integer values are then filtered to exclude the special \code{NA_integer}
#' value used by R.
#' 
#' 
#' @section Platform notes:
#' The method used for generating random values varies depending on the 
#' operating system (OS):
#'  
#' \itemize{
#'   \item{For macOS and BSDs: \code{arc4random_buf()}}
#'   \item{For linux: \code{syscall(SYS_getrandom())}}
#'   \item{For win32: \code{BCryptGenRandom()}}
#' }
#'
#' All these random number generators are internally seeded by the OS using entropy 
#' gathered from multiple sources and are considered cryptographically secure.
#'
#' @return Depending on the \code{type} argument: a hexadecimal string, a 
#' raw vector, a logical vector, an integer vector or a numeric vector.  
#' 
#' @export
#' @examples
#' rcrypto(16, type = 'raw')
#' rcrypto(16, type = 'chr')
#' rcrypto(16, type = 'lgl')
#' rcrypto(16, type = 'int')
#' rcrypto(16, type = 'dbl')
#~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
rcrypto <- function(n, type = 'raw') {
  switch(
    type,
    raw = .Call(rcrypto_raw_, n),
    chr = .Call(rcrypto_chr_, n),
    lgl = .Call(rcrypto_lgl_, n),
    int = .Call(rcrypto_int_, n),
    dbl = .Call(rcrypto_dbl_, n),
    stop("rcrypto(): not a valid type: ", type)
  )
}
