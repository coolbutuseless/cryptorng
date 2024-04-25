
test_that("generate_bytes works works", {
  
  r1 <- generate_bytes(32, type = 'raw')
  r2 <- generate_bytes(32, type = 'raw')
  
  expect_true(is.raw(r1))
  expect_true(length(r1) == 32)
  expect_true(!identical(r1, r2))
  
  
  
  
  r1 <- generate_bytes(32, type = 'string')
  r2 <- generate_bytes(32, type = 'string')
  
  expect_true(is.character(r1))
  expect_true(length(r1) == 1)
  expect_true(nchar(r1) == 64)
  expect_true(!identical(r1, r2))
  
  
})
