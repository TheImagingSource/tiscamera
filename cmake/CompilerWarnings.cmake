# from here:
#
# https://github.com/lefticus/cppbestpractices/blob/master/02-Use_the_Tools_Avai
# lable.md

function(set_project_warnings project_name)
  option(WARNINGS_AS_ERRORS "Treat compiler warnings as errors" FALSE)

  set(MSVC_WARNINGS
      /W4 # Baseline reasonable warnings
      /w14242 # 'identfier': conversion from 'type1' to 'type1', possible loss
              # of data
      /w14254 # 'operator': conversion from 'type1:field_bits' to
              # 'type2:field_bits', possible loss of data
      /w14263 # 'function': member function does not override any base class
              # virtual member function
      /w14265 # 'classname': class has virtual functions, but destructor is not
              # virtual instances of this class may not be destructed correctly
      /w14287 # 'operator': unsigned/negative constant mismatch
      /we4289 # nonstandard extension used: 'variable': loop control variable
              # declared in the for-loop is used outside the for-loop scope
      /w14296 # 'operator': expression is always 'boolean_value'
      /w14311 # 'variable': pointer truncation from 'type1' to 'type2'
      /w14545 # expression before comma evaluates to a function which is missing
              # an argument list
      /w14546 # function call before comma missing argument list
      /w14547 # 'operator': operator before comma has no effect; expected
              # operator with side-effect
      /w14549 # 'operator': operator before comma has no effect; did you intend
              # 'operator'?
      /w14555 # expression has no effect; expected expression with side- effect
      /w14619 # pragma warning: there is no warning number 'number'
      /w14640 # Enable warning on thread un-safe static member initialization
      /w14826 # Conversion from 'type1' to 'type_2' is sign-extended. This may
              # cause unexpected runtime behavior.
      /w14905 # wide string literal cast to 'LPSTR'
      /w14906 # string literal cast to 'LPWSTR'
      /w14928 # illegal copy-initialization; more than one user-defined
              # conversion has been implicitly applied
  )

  set(CLANG_WARNINGS
    -Wall
    -Wextra # reasonable and standard
    -Wshadow # warn the user if a variable declaration shadows one from a
             # parent context
    -Wnon-virtual-dtor # warn the user if a class with virtual functions has a
                       # non-virtual destructor. This helps catch hard to
                       # track down memory errors
    -Wunused # warn on anything being unused
    -Woverloaded-virtual # warn if you overload (not override) a virtual
                         # function
    -Wpedantic # warn if non-standard C++ is used
    -Wnull-dereference # warn if a null dereference is detected
    -Wdouble-promotion # warn if float is implicit promoted to double
    -Wformat=2 # warn on security issues around functions that format output
               # (ie printf)

    -Wnull-dereference  # Warn if the compiler detects paths that trigger erroneous or undefined behavior due to dereferencing a null pointer. 
    -Wduplicated-cond # Warn about duplicated conditions in an if-else-if chain
    -Wduplicated-branches # Warn when an if-else has identical branches.

    # disabled because these lead to false positives
    -Wswitch
    #-Wswitch-enum   # Warn whenever a switch statement has an index of enumerated type and lacks a case for one or more of the named codes of that enumeration. 
                    # case labels outside the enumeration range also provoke warnings when this option is used. The only difference between -Wswitch and this option is 
                    # that this option gives a warning about an omitted enumeration code even if there is a default label.
    -Wsuggest-override # Warn about overriding virtual functions that are not marked with the override keyword.
    -Wsuggest-final-methods # Warn about virtual methods where code quality would be improved if the method were declared with the C++11 final specifier, or, if possible, its type were declared in an anonymous namespace or with the final specifier.

    # disabled by christopher 2020/02/20

    # -Wold-style-cast # warn for c-style casts
    # -Wcast-align # warn for potential performance problem casts
    # -Wconversion # warn on type conversions that may lose data
    # -Wsign-conversion # warn on sign conversions

    # disabled because of compiler version problems
    #    -Wextra-semi # Warn about redundant semicolon after in-class function definition.
  )

  if (WARNINGS_AS_ERRORS)
    set(CLANG_WARNINGS ${CLANG_WARNINGS} -Werror)
    set(MSVC_WARNINGS ${MSVC_WARNINGS} /WX)
  endif()

  set(GCC_WARNINGS
      ${CLANG_WARNINGS}
      -Wmisleading-indentation # warn if identation implies blocks where blocks
                               # do not exist
      -Wduplicated-cond # warn if if / else chain has duplicated conditions
      -Wduplicated-branches # warn if if / else branches have duplicated code
      -Wlogical-op # warn about logical operations being used where bitwise were
                   # probably wanted
#      -Wuseless-cast # warn if you perform a cast to the same type Gstreamer does not really like this
  )

  if(MSVC)
    set(PROJECT_WARNINGS ${MSVC_WARNINGS})
  elseif(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(PROJECT_WARNINGS ${CLANG_WARNINGS})
  else()
    set(PROJECT_WARNINGS ${GCC_WARNINGS})
  endif()

  target_compile_options(${project_name} INTERFACE ${PROJECT_WARNINGS})

endfunction()
