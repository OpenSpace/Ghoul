##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
# General Helpful Open Utility Library                                                   #
#                                                                                        #
# Copyright (c) 2012-2022                                                                #
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
  target_compile_features(${target} PRIVATE cxx_std_17)

  set(MSVC_WARNINGS
    "/MP"       # Multi-threading support
    "/W4"       # Highest warning level
    "/w44062"   # enumerator 'identifier' in a switch of enum 'enumeration' is not handled
    "/wd4127"   # conditional expression is constant
    "/wd4201"   # nonstandard extension used : nameless struct/union
    "/w44255"   # 'function': no function prototype given: converting '()' to '(void)'
    "/w44263"   # 'function': member function does not override any base class virtual member function
    "/w44264"   # 'virtual_function': no override available for virtual member function from base 'class'; function is hidden
    "/w44265"   # 'class': class has virtual functions, but destructor is not virtual
    "/w44266"   # 'function': no override available for virtual member function from base 'type'; function is hidden
    "/w44289"   # nonstandard extension used : 'var' : loop control variable declared in the for-loop is used outside the for-loop scope
    "/w44296"   # 'operator': expression is always false
    "/w44311"   # 'variable' : pointer truncation from 'type' to 'type'
    "/w44339"   # 'type' : use of undefined type detected in CLR meta-data - use of this type may lead to a runtime exception
    "/w44342"   # behavior change: 'function' called, but a member operator was called in previous versions
    "/w44350"   # behavior change: 'member1' called instead of 'member2'
    "/w44431"   # missing type specifier - int assumed. Note: C no longer supports default-int
    "/w44471"   # a forward declaration of an unscoped enumeration must have an underlying type (int assumed)
    "/w44545"   # expression before comma evaluates to a function which is missing an argument list
    "/w44546"   # function call before comma missing argument list
    "/w44547"   # 'operator': operator before comma has no effect; expected operator with side-effect
    "/w44548"   # expression before comma has no effect; expected expression with side-effect
    "/w44549"   # 'operator': operator before comma has no effect; did you intend 'operator'?
    "/w44555"   # expression has no effect; expected expression with side-effect
    # This is disabled until GLM is updated to version 0.9.9 that removes occurrance of this warning
    # "/w44574"   # 'identifier' is defined to be '0': did you mean to use '#if identifier'?
    "/w44608"   # 'symbol1' has already been initialized by another union member in the initializer list, 'symbol2'
    "/w44628"   # digraphs not supported with -Ze. Character sequence 'digraph' not interpreted as alternate token for 'char'
    "/w44640"   # 'instance': construction of local static object is not thread-safe
    "/w44905"   # wide string literal cast to 'LPSTR'
    "/w44906"   # string literal cast to 'LPWSTR'
    "/w44986"   # 'symbol': exception specification does not match previous declaration
    "/w44988"   # 'symbol': variable declared outside class/function scope
    "/std:c++latest" # Latest C++ standard
    "/permissive-"   # Enable conformance mode
    "/Zc:twoPhase-"  # Used to prevent C:\Program Files (x86)\Windows Kits\8.1\Include\um\combaseapi.h(229): error C2187: syntax error: 'identifier' was unexpected here
                    # This is a bug in Visual Studio 15.3 and can be removed with the next version:
                    # https://developercommunity.visualstudio.com/content/problem/94419/vs-2017-153-with-permissive-shows-error-c2187-in-c.html
    "/Zc:strictStrings-"    # Windows header don't adhere to this
    "/Zc:__cplusplus" # Correctly set the __cplusplus macro
  )

  set(CLANG_WARNINGS
    "-stdlib=libc++"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Wabstract-vbase-init"
    "-Walloca"
    "-Wanon-enum-enum-conversion"
    "-Warray-bounds-pointer-arithmetic"
    "-Wassign-enum"
    "-Wbad-function-cast"
    "-Wbinary-literal"
    "-Wbind-to-temporary-copy"
    "-Wbit-int-extension"
    "-Wbitfield-constant-conversion"
    "-Wbitwise-op-parentheses"
    "-Wbool-conversions"
    "-Wbool-operation"
    "-Wcast-align"
    "-Wcast-function-type"
    "-Wcast-qual"
    "-Wcomma"
    "-Wcomplex-component-init"
    "-Wconditional-uninitialized"
    "-Wcovered-switch-default"
    "-Wdate-time"
    "-Wdelete-non-virtual-dtor"
    "-Wdeprecated-copy"
    "-Wdeprecated-copy-dtor"
    "-Wdeprecated-dynamic-exception-spec"
    "-Wdeprecated-this-capture"
    "-Wdivision-by-zero"
    "-Wdollar-in-identifier-extension"
    "-Wdtor-name"
    "-Wduplicate-decl-specifier"
    "-Wduplicate-enum"
    "-Wduplicate-method-arg"
    "-Wduplicate-method-match"
    "-Wempty-init-stmt"
    "-Wextra-semi"
    "-Wextra-semi-stmt"
    "-Wfloat-equal"
    "-Wfloat-overflow-conversion"
    "-Wfloat-zero-conversion"
    "-Wfor-loop-analysis"
    "-Wformat-non-iso"
    "-Wformat-nonliteral"
    "-Wformat-pedantic"
    "-Wformat-type-confusion"
    "-Wheader-hygiene"
    "-Widiomatic-parentheses"
    "-Wimplicit-fallthrough"
    "-Wimport-preprocessor-directive-pedantic"
    "-Winfinite-recursion"
    "-Wkeyword-macro"
    "-Wlanguage-extension-token"
    "-Wlogical-op-parantheses"
    "-Wloop-analysis"
    "-Wmain"
    "-Wmethod-signatures"
    "-Wmicrosoft-end-of-file"
    "-Wmicrosoft-enum-value"
    "-Wmicrosoft-fixed-enum"
    "-Wmicrosoft-flexible-array"
    "-Wmicrosoft-redeclare-static"
    "-Wmisleading-indentation"
    "-Wmismatched-tags"
    "-Wmissing-field-initializers"
    "-Wmissing-noreturn"
    "-Wmissing-prototypes"
    "-Wmove"
    "-Wnewline-eof"
    "-Wno-missing-braces"
    "-Wnon-virtual-dtor"
    "-Wnull-pointer-subtraction"
    "-Wold-style-cast"
    "-Woverloaded-virtual"
    "-Wpessimizing-move"
    "-Wpointer-arith"
    "-Wpragmas"
    "-Wrange-loop-analysis"
    "-Wredundant-move"
    "-Wreorder"
    "-Wself-move"
    "-Wsemicolon-before-method-body"
    "-Wshadow-all"
    "-Wshift-sign-overflow"
    "-Wshorten-64-to-32"
    "-Wsign-compare"
    "-Wsigned-enum-bitfield"
    "-Wsometimes-uninitialized"
    "-Wstring-concatenation"
    "-Wstring-conversion"
    "-Wsuggest-destructor-override"
    "-Wsuggest-override"
    "-Wswitch-enum"
    "-Wtautological-compare"
    "-Wtautological-constant-in-range-compare"
    "-Wtautological-constant-out-of-range-compare"
    "-Wthread-safety"
    "-Wtype-limits"
    "-Wundef"
    "-Wundefined-func-template"
    "-Wundefined-reinterpret-cast"
    "-Wuninitialized-const-reference"
    "-Wunneeded-internal-declaration"
    "-Wunneeded-member-function"
    "-Wunreachable-code"
    "-Wunreachable-code-break"
    "-Wunreachable-code-return"
    "-Wunused"
    "-Wunused-but-set-parameter"
    "-Wunused-const-variable"
    "-Wunused-exception-parameter"
    "-Wunused-macros"
    "-Wunused-result"
    "-Wused-but-marked-unused"
    "-Wvariadic-macros"
    "-Wvla"
    "-Wweak-template-vtables"
    "-Wzero-as-null-pointer-constant"
    "-Wzero-length-array"
  )

  set(GCC_WARNINGS
    "-ggdb"
    "-Wall"
    "-Wextra"
    "-Wpedantic"
    "-Walloc-zero"
    "-Wcast-qual"
    "-Wdate-time"
    "-Wduplicated-cond"
    "-Wlogical-op"
    "-Wno-deprecated-copy"
    "-Wno-float-equal"
    "-Wno-long-long"
    "-Wno-write-strings"
    "-Wnon-virtual-dtor"
    "-Wold-style-cast"
    "-Woverloaded-virtual"
    "-Wshadow"
    "-Wsuggest-override"
    "-Wundef"
    "-Wuseless-cast"
    "-Wzero-as-null-pointer-constant"
  )

  if (MSVC)
    target_compile_options(${target} PRIVATE ${MSVC_WARNINGS})
    if (GHOUL_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE "/WX")
    endif ()

    target_compile_definitions(${target}
      # This definition can be removed when the glbinding submodule is updated
      PUBLIC "_SILENCE_CXX17_ADAPTOR_TYPEDEFS_DEPRECATION_WARNING"
      # This definition can be removed when Cppformat is updated to remove the allocator warning
      PUBLIC "_SILENCE_CXX17_OLD_ALLOCATOR_MEMBERS_DEPRECATION_WARNING"
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

    if (GHOUL_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE "-Werror")
    endif ()

  elseif (UNIX AND CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(${target} PRIVATE ${CLANG_WARNINGS})
    
    target_link_libraries(${target} PRIVATE "c++" "c++abi")
    
    if (GHOUL_WARNINGS_AS_ERRORS)
        target_compile_options(${target} PRIVATE "-Werror")
    endif ()
  elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
    target_compile_options(${target} PRIVATE ${GCC_WARNINGS})

    if (GHOUL_WARNINGS_AS_ERRORS)
      target_compile_options(${target} PRIVATE "-Werror")
    endif ()
  else ()
    message("Compiler not handled in set_ghoul_compile_settings.cmake")
  endif ()

  if (APPLE)
    target_compile_definitions(${target} PRIVATE "__gl_h_")
  endif ()
endfunction ()
