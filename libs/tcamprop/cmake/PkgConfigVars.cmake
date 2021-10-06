# - determine variables defined in pkg-config files
#
# Usage:
#   pkg_check_variable(<PKG_NAME> <VARIABLE_NAME>)
#
# Checks for a variable in the given package and translates to a call such as
# `pkg-config --variable=<VARIABLE_NAME> <PKG_NAME>`. The output is a cached
# variable named
#
#   <PKG_NAME>_<VARIABLE_NAME>
#
# Note that both names are uppercased and any dashes replaced by underscores.
#

find_package(PkgConfig REQUIRED QUIET)

function(pkg_check_variable _pkg _name)
    string(TOUPPER ${_pkg} _pkg_upper)
    string(TOUPPER ${_name} _name_upper)
    string(REPLACE "-" "_" _pkg_upper ${_pkg_upper})
    string(REPLACE "-" "_" _name_upper ${_name_upper})
    set(_output_name "${_pkg_upper}_${_name_upper}")

    execute_process(COMMAND ${PKG_CONFIG_EXECUTABLE} --variable=${_name} ${_pkg}
                    OUTPUT_VARIABLE _pkg_result
                    OUTPUT_STRIP_TRAILING_WHITESPACE)

    set("${_output_name}" "${_pkg_result}" CACHE STRING "pkg-config variable
    ${_name} of ${_pkg}")
endfunction()
