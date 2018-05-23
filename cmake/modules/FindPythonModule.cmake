#
# This code defines a helper function find_python_module(). It can be used
# like so:
#	find_python_module(numpy)
# If numpy is found, the variable PY_NUMPY contains the location of the numpy
# module. If the module is required add the keyword "REQUIRED":
#	find_python_module(numpy REQUIRED)

find_package(PythonInterp 3 REQUIRED QUIET)

if (NOT PYTHONINTERP_FOUND)
  message(ERROR "Could not find a valid python executable.")
endif()

function(find_python_module module)
  string(TOUPPER ${module} module_upper)
  if(NOT PYTHON_MODULE_${module_upper})
	if(ARGC GREATER 1 AND ARGV1 STREQUAL "REQUIRED")
	  set(${module}_FIND_REQUIRED TRUE)
	endif()
	# A module's location is usually a directory, but for binary modules
	# it's a .so file.
	execute_process(COMMAND "${PYTHON_EXECUTABLE}" "-c"
	  "import re, ${module}; print(re.compile('/__init__.py.*').sub('',${module}.__file__))"
	  RESULT_VARIABLE _${module}_status
	  OUTPUT_VARIABLE _${module}_location
	  ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE)
	if(NOT _${module}_status)
	  set(PYTHON_MODULE_${module_upper} ${_${module}_location} CACHE STRING
		"Location of Python module ${module}")
	endif(NOT _${module}_status)
  endif(NOT PYTHON_MODULE_${module_upper})
  include(FindPackageHandleStandardArgs)

  find_package_handle_standard_args(PYTHON_MODULE_${module} DEFAULT_MSG PYTHON_MODULE_${module_upper})
endfunction(find_python_module)
