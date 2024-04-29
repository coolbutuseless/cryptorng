
test_that("rcrypto works works", {
  
  r1 <- rcrypto(32, type = 'raw')
  r2 <- rcrypto(32, type = 'raw')
  
  expect_true(is.raw(r1))
  expect_true(length(r1) == 32)
  expect_true(!identical(r1, r2))
  
  
  
  
  r1 <- rcrypto(32, type = 'string')
  r2 <- rcrypto(32, type = 'string')
  
  expect_true(is.character(r1))
  expect_true(length(r1) == 1)
  expect_true(nchar(r1) == 64)
  expect_true(!identical(r1, r2))
  
  
})


test_that("n is checked for sanity", {
  expect_error(rcrypto( 0), "positive integer")
  expect_error(rcrypto(-1), "positive integer")
})



test_that("rcrypto_unif() works works", {
  r1 <- rcrypto_unif(32)
  r2 <- rcrypto_unif(32)
  
  expect_true(is.double(r1))
  expect_true(length(r1) == 32)
  expect_true(!identical(r1, r2))
})


test_that("rcrypto_int() works works", {
  r1 <- rcrypto_int(32)
  r2 <- rcrypto_int(32)
  
  expect_true(is.integer(r1))
  expect_true(length(r1) == 32)
  expect_true(!identical(r1, r2))
})