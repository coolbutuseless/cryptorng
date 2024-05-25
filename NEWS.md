
# cryptorng 0.1.3  2024-05-25

* `rcrypto()` now supports more return types:
    * `raw` raw vector of bytes
    * `chr` hexadecimal string
    * `lgl` random logical values
    * `int` uniform random integers
    * `dbl` uniform random doubles in the range [0, 1]

# cryptorng 0.1.2  2024-04-28

* Fixes for CRAN
    * Reintroduce `()` for functions in `DESCRIPTION`

# cryptorng 0.1.1  2024-04-26

* Fixes for CRAN
    * Shorten title to <= 65 characters
    * Remove `()` from non-R functions in `DESCRIPTION`
* Rename `generate_bytes()` to `rcrypto()` to match R's naming for RNGs
* Check for `n <= 0`

# cryptorng 0.1.0  2024-04-25

* Initial release
