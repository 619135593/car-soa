# Distributed under the OSI-approved BSD 3-Clause License.  See accompanying
# file Copyright.txt or https://cmake.org/licensing for details.

cmake_minimum_required(VERSION 3.5)

file(MAKE_DIRECTORY
  "/home/zz/car/build/_deps/json-src"
  "/home/zz/car/build/_deps/json-build"
  "/home/zz/car/build/_deps/json-subbuild/json-populate-prefix"
  "/home/zz/car/build/_deps/json-subbuild/json-populate-prefix/tmp"
  "/home/zz/car/build/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp"
  "/home/zz/car/build/_deps/json-subbuild/json-populate-prefix/src"
  "/home/zz/car/build/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp"
)

set(configSubDirs )
foreach(subDir IN LISTS configSubDirs)
    file(MAKE_DIRECTORY "/home/zz/car/build/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp/${subDir}")
endforeach()
if(cfgdir)
  file(MAKE_DIRECTORY "/home/zz/car/build/_deps/json-subbuild/json-populate-prefix/src/json-populate-stamp${cfgdir}") # cfgdir has leading slash
endif()
