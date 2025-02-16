##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
# General Helpful Open Utility Library                                                   #
#                                                                                        #
# Copyright (c) 2012-2025                                                                #
#                                                                                        #
# Permission is hereby granted, free of charge, to any person obtaining a copy of this   #
# software and associated documentation files (the "Software"), to deal in the Software  #
# without restriction, including without limitation the rights to use, copy, modify,     #
# merge, publish, distribute, sublicense, and/or sell copies of the Software, and to     #
# permit persons to whom the Software is furnished to do so, subject to the following    #
# conditions:                                                                            #
#                                                                                        #
# The above copyright notice and this permission notice shall be included in all copies  #
# or substantial portions of the Software.                                               #
#                                                                                        #
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,    #
# INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A          #
# PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT     #
# HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF   #
# CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE   #
# OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.                                          #
##########################################################################################

function (set_ghoul_compile_settings target)
  target_compile_features(${target} PUBLIC cxx_std_20)

  set(MSVC_WARNINGS
    "/MP"       # Multi-threading support
    "/W4"       # Highest warning level
    "/w44062"   # missing case label
    "/w44289"   # using for-loop variable outside of loop
    "/w44296"   # expression is always true/false
    "/w45041"   # out-of-line definition for constexpr data is deprecated
    "/w45204"   # virtual class has non-virtual trivial destructor
    "/wd4127"   # conditional expression is constant [raised by: websocketpp]
    "/permissive-"   # Enable conformance mode
    "/Zc:__cplusplus" # Correctly set the __cplusplus macro
  )

  set(CLANG_WARNINGS
    "-Wall"
    "-Wextra"
    "-Wmost"
    "-Wpedantic"

    "-Wabstract-vbase-init"
    "-Walloca"
    "-Wanon-enum-enum-conversion"
    "-Warray-bounds-pointer-arithmetic"
    "-Wassign-enum"
    "-Wbad-function-cast"
    "-Wbinary-literal"
    "-Wbind-to-temporary-copy"
    "-Wbitfield-constant-conversion"
    "-Wbool-conversions"
    "-Wcast-align"
    "-Wcast-qual"
    "-Wcomma"
    "-Wconditional-uninitialized"
    "-Wdate-time"
    "-Wdeprecated-dynamic-exception-spec"
    "-Wdeprecated-this-capture"
    "-Wdivision-by-zero"
    "-Wdtor-name"
    "-Wduplicate-decl-specifier"
    "-Wduplicate-enum"
    "-Wduplicate-method-arg"
    "-Wduplicate-method-match"
    "-Wextra-semi"
    "-Wextra-semi-stmt"
    "-Wfloat-overflow-conversion"
    "-Wfloat-zero-conversion"
    "-Wformat"
    "-Wformat-non-iso"
    "-Wformat-nonliteral"
    "-Wformat-pedantic"
    "-Wformat-type-confusion"
    "-Wheader-hygiene"
    "-Widiomatic-parentheses"
    "-Wimplicit"
    "-Wimplicit-fallthrough"
    "-Wloop-analysis"
    "-Wmain"
    "-Wmethod-signatures"
    "-Wmissing-noreturn"
    "-Wmissing-prototypes"
    "-Wmove"
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Wpointer-arith"
    "-Wpragmas"
    "-Wrange-loop-analysis"
    "-Wreorder"
    "-Wshadow-all"
    "-Wshift-sign-overflow"
    "-Wshorten-64-to-32"
    "-Wsometimes-uninitialized"
    "-Wstring-conversion"
    "-Wsuggest-destructor-override"
    "-Wsuggest-override"
    "-Wtautological-compare"
    "-Wtautological-constant-in-range-compare"
    "-Wtautological-constant-out-of-range-compare"
    "-Wthread-safety"
    "-Wtype-limits"
    "-Wundef"
    "-Wundefined-reinterpret-cast"
    "-Wuninitialized-const-reference"
    "-Wunneeded-internal-declaration"
    "-Wunneeded-member-function"
    "-Wunreachable-code"
    "-Wunreachable-code-break"
    "-Wunreachable-code-return"
    "-Wunused"
    "-Wunused-const-variable"
    "-Wunused-exception-parameter"
    "-Wunused-macros"
    "-Wunused-result"
    "-Wused-but-marked-unused"
    "-Wvariadic-macros"
    "-Wvla"
    "-Wweak-template-vtables"
    "-Wzero-as-null-pointer-constant"

    "-Wno-missing-braces"
    "-Wno-c++98-compat-bind-to-temporary-copy"
  )

  set(GCC_WARNINGS
    "-ggdb"
    "-Wall"
    "-Wextra"
    "-Wpedantic"

    "-Walloc-zero"
    "-Wcast-qual"
    "-Wdate-time"
    "-Wduplicated-branches"
    "-Wduplicated-cond"
    "-Wfloat-equal"
    "-Wformat"
    "-Wlogical-op"
    "-Wmain"
    "-Wno-deprecated-copy"
    "-Wno-float-equal"
    "-Wno-long-long"
    "-Wno-write-strings"
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Woverloaded-virtual"
    "-Wshadow"
    "-Wsuggest-override"
    "-Wtautological-compare"
    "-Wtype-limits"
    "-Wundef"
    "-Wunused"
    "-Wuninitialized"
    "-Wvla"
    "-Wzero-as-null-pointer-constant"
  )

  if (MSVC)
    target_compile_options(${target} PRIVATE ${MSVC_WARNINGS})
    target_compile_definitions(${target}
      # This definition can be removed when the glbinding submodule is updated
      PUBLIC "_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING"
      # This definition can be removed when Cppformat is updated to remove the allocator warning
      PUBLIC "_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING"
      PUBLIC "_CRT_SECURE_NO_WARNINGS"
      PRIVATE "NOMINMAX"
      PRIVATE "WIN32_LEAN_AND_MEAN"
      PRIVATE "VC_EXTRALEAN"
    )

    if (GHOUL_OPTIMIZATION_ENABLE_AVX)
      target_compile_options(${target} PRIVATE "/arch:AVX")
    endif ()
    if (GHOUL_OPTIMIZATION_ENABLE_AVX2)
      target_compile_options(${target} PRIVATE "/arch:AVX2")
    endif ()
    if (GHOUL_OPTIMIZATION_ENABLE_AVX512)
      target_compile_options(${target} PRIVATE "/arch:AVX512")
    endif ()

    if (GHOUL_OPTIMIZATION_ENABLE_OTHER_OPTIMIZATIONS)
      target_compile_options(${target} PRIVATE
        "/Oi" # usage of intrinsic functions
        "/GL" # Whole program optimization
      )
    else ()
      if (GHOUL_ENABLE_EDIT_CONTINUE)
        target_compile_options(${target} PRIVATE
          "/ZI"       # Edit and continue support
        )
      endif ()
    endif ()
  elseif (APPLE AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} PRIVATE ${CLANG_WARNINGS})
    target_compile_definitions(${target} PUBLIC "-DGHOUL_ASSERT")
  elseif (UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} PRIVATE ${CLANG_WARNINGS})
    target_compile_definitions(${target} PUBLIC "-DGHOUL_ASSERT")
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${target} PRIVATE ${GCC_WARNINGS})
    target_compile_definitions(${target} PUBLIC "-DGHOUL_ASSERT")
  else ()
    message("Compiler not handled in set_ghoul_compile_settings.cmake")
  endif ()

  if (APPLE)
    target_compile_definitions(${target} PRIVATE "__gl_h_")
  endif ()
endfunction ()
