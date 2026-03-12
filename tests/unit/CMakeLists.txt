add_executable(uvc-extension-loader-check-test
  uvc-extension-loader-check.cpp
  )

add_test(
  NAME unit-uvc-extension-loader-check
  COMMAND uvc-extension-loader-check-test
  )
