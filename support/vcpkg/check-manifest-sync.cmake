##########################################################################################
#                                                                                        #
# GHOUL                                                                                  #
# General Helpful Open Utility Library                                                   #
#                                                                                        #
# Copyright (c) 2012-2026                                                                #
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

# Verifies that the `ghoul-deps` meta-port declares exactly the same dependencies and
# features as the root vcpkg.json. Superprojects that add Ghoul through add_subdirectory
# cannot use Ghoul's manifest (vcpkg only reads the manifest of the top-level project) and
# instead depend on `ghoul-deps`, so the two lists have to stay identical.
#
# Run with:  cmake -P support/vcpkg/check-manifest-sync.cmake

cmake_minimum_required(VERSION 3.19)

get_filename_component(GHOUL_ROOT_DIR "${CMAKE_CURRENT_LIST_DIR}/../.." ABSOLUTE)
set(MANIFEST "${GHOUL_ROOT_DIR}/vcpkg.json")
set(META_PORT "${CMAKE_CURRENT_LIST_DIR}/ports/ghoul-deps/vcpkg.json")

function (flatten_dependencies outVar dependencies)
  set(result "")
  string(JSON count LENGTH "${dependencies}")
  if (count GREATER 0)
    math(EXPR lastIndex "${count} - 1")
    foreach (i RANGE ${lastIndex})
      string(JSON entryType TYPE "${dependencies}" ${i})
      string(JSON entry GET "${dependencies}" ${i})

      if (entryType STREQUAL "STRING")
        list(APPEND result "${entry}")
        continue ()
      endif ()

      string(JSON name GET "${entry}" "name")

      string(JSON features ERROR_VARIABLE featuresError GET "${entry}" "features")
      set(featureList "")
      if (featuresError STREQUAL "NOTFOUND")
        string(JSON featureCount LENGTH "${features}")
        if (featureCount GREATER 0)
          math(EXPR lastFeature "${featureCount} - 1")
          foreach (j RANGE ${lastFeature})
            string(JSON feature GET "${features}" ${j})
            list(APPEND featureList "${feature}")
          endforeach ()
          list(SORT featureList)
        endif ()
      endif ()
      list(JOIN featureList "+" featureText)

      string(JSON defaults ERROR_VARIABLE defaultsError GET "${entry}" "default-features")
      if (NOT defaultsError STREQUAL "NOTFOUND")
        set(defaults "ON")
      endif ()

      list(APPEND result "${name}[${featureText}](default-features=${defaults})")
    endforeach ()
  endif ()

  list(SORT result)
  set(${outVar} "${result}" PARENT_SCOPE)
endfunction ()

function (feature_names outVar features)
  string(JSON count LENGTH "${features}")
  set(result "")
  if (count GREATER 0)
    math(EXPR lastIndex "${count} - 1")
    foreach (i RANGE ${lastIndex})
      string(JSON name MEMBER "${features}" ${i})
      list(APPEND result "${name}")
    endforeach ()
    list(SORT result)
  endif ()
  set(${outVar} "${result}" PARENT_SCOPE)
endfunction ()

file(READ "${MANIFEST}" manifestJson)
file(READ "${META_PORT}" metaPortJson)

set(errors "")

string(JSON manifestDependencies GET "${manifestJson}" "dependencies")
string(JSON metaPortDependencies GET "${metaPortJson}" "dependencies")
flatten_dependencies(manifestDeps "${manifestDependencies}")
flatten_dependencies(metaPortDeps "${metaPortDependencies}")
if (NOT manifestDeps STREQUAL metaPortDeps)
  list(APPEND errors "  dependencies\n    vcpkg.json:  ${manifestDeps}\n    ghoul-deps:  ${metaPortDeps}")
endif ()

string(JSON manifestFeatures GET "${manifestJson}" "features")
string(JSON metaPortFeatures GET "${metaPortJson}" "features")
feature_names(manifestFeatureNames "${manifestFeatures}")
feature_names(metaPortFeatureNames "${metaPortFeatures}")
if (NOT manifestFeatureNames STREQUAL metaPortFeatureNames)
  list(APPEND errors "  feature names\n    vcpkg.json:  ${manifestFeatureNames}\n    ghoul-deps:  ${metaPortFeatureNames}")
else ()
  foreach (feature ${manifestFeatureNames})
    string(JSON manifestFeatureDependencies GET "${manifestFeatures}" "${feature}" "dependencies")
    string(JSON metaPortFeatureDependencies GET "${metaPortFeatures}" "${feature}" "dependencies")
    flatten_dependencies(manifestFeatureDeps "${manifestFeatureDependencies}")
    flatten_dependencies(metaPortFeatureDeps "${metaPortFeatureDependencies}")
    if (NOT manifestFeatureDeps STREQUAL metaPortFeatureDeps)
      list(APPEND errors "  feature '${feature}'\n    vcpkg.json:  ${manifestFeatureDeps}\n    ghoul-deps:  ${metaPortFeatureDeps}")
    endif ()
  endforeach ()
endif ()

if (errors)
  list(JOIN errors "\n" errorText)
  message(FATAL_ERROR
    "vcpkg.json and support/vcpkg/ports/ghoul-deps/vcpkg.json have diverged:\n${errorText}\n"
  )
endif ()

message(STATUS "vcpkg.json and ghoul-deps are in sync")
