# Copyright 2015 Samsung Electronics Co., Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required (VERSION 2.8.12)
project (JerryCore CXX C ASM)

# Definitions
 # Get version information from git
  execute_process(COMMAND git symbolic-ref -q HEAD
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE JERRY_GIT_BRANCH
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
  execute_process(COMMAND git rev-parse HEAD
                  WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
                  OUTPUT_VARIABLE JERRY_GIT_COMMIT
                  OUTPUT_STRIP_TRAILING_WHITESPACE)
 # Get build date
  execute_process(COMMAND date +%d/%m/%Y
                  OUTPUT_VARIABLE JERRY_BUILD_DATE
                  OUTPUT_STRIP_TRAILING_WHITESPACE)

  set(DEFINES_JERRY
      JERRY_BUILD_DATE="${JERRY_BUILD_DATE}"
      JERRY_COMMIT_HASH="${JERRY_GIT_COMMIT}"
      JERRY_BRANCH_NAME="${JERRY_GIT_BRANCH}")

 # Build modes
  # Debug
   set(DEFINES_JERRY_DEBUG JERRY_ENABLE_PRETTY_PRINTER)

  # Release
   set(DEFINES_JERRY_RELEASE JERRY_NDEBUG)

  # Unit tests
   set(DEFINES_JERRY_UNITTESTS JERRY_ENABLE_PRETTY_PRINTER)

 # Modifiers
  # Full profile
   set(DEFINES_FULL_PROFILE CONFIG_ECMA_NUMBER_TYPE=CONFIG_ECMA_NUMBER_FLOAT64)

  # Compact profile
   set(DEFINES_COMPACT_PROFILE
       CONFIG_ECMA_COMPACT_PROFILE)

  # Minimal compact profile
   set(DEFINES_COMPACT_PROFILE_MINIMAL
       CONFIG_ECMA_COMPACT_PROFILE
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_NUMBER_BUILTIN
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_STRING_BUILTIN
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_BOOLEAN_BUILTIN
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_ERROR_BUILTINS
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_ARRAY_BUILTIN
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_MATH_BUILTIN
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_DATE_BUILTIN
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_JSON_BUILTIN
       CONFIG_ECMA_COMPACT_PROFILE_DISABLE_REGEXP_BUILTIN)

  # Minimal footprint
   set(DEFINES_MINIMAL_FOOTPRINT
       CONFIG_ECMA_LCACHE_DISABLE
       CONFIG_VM_RUN_GC_AFTER_EACH_OPCODE)

 # Memory statistics
  set(DEFINES_MEMORY_STATISTICS MEM_STATS)

 # Valgrind
  set(DEFINES_JERRY_VALGRIND JERRY_VALGRIND)

 # Platform-specific
  # Linux
  # Nuttx
   math(EXPR MEM_HEAP_AREA_SIZE_16K "16 * 1024")
   set(DEFINES_JERRY_NUTTX CONFIG_MEM_HEAP_AREA_SIZE=${MEM_HEAP_AREA_SIZE_16K})
  # MCU
   # stm32f3
    math(EXPR MEM_HEAP_AREA_SIZE_16K "16 * 1024")
    set(DEFINES_JERRY_MCU_STM32F3 CONFIG_MEM_HEAP_AREA_SIZE=${MEM_HEAP_AREA_SIZE_16K})
   # stm32f4
    math(EXPR MEM_HEAP_AREA_SIZE_16K "16 * 1024")
    set(DEFINES_JERRY_MCU_STM32F4 CONFIG_MEM_HEAP_AREA_SIZE=${MEM_HEAP_AREA_SIZE_16K})

# Include directories
 set(INCLUDE_CORE
     ${CMAKE_SOURCE_DIR}/jerry-core
     ${CMAKE_SOURCE_DIR}/jerry-core/lit
     ${CMAKE_SOURCE_DIR}/jerry-core/rcs
     ${CMAKE_SOURCE_DIR}/jerry-core/mem
     ${CMAKE_SOURCE_DIR}/jerry-core/vm
     ${CMAKE_SOURCE_DIR}/jerry-core/ecma/builtin-objects
     ${CMAKE_SOURCE_DIR}/jerry-core/ecma/base
     ${CMAKE_SOURCE_DIR}/jerry-core/ecma/operations
     ${CMAKE_SOURCE_DIR}/jerry-core/parser/collections
     ${CMAKE_SOURCE_DIR}/jerry-core/parser/js
     ${CMAKE_SOURCE_DIR}/jerry-core/jrt)

 # Third-party
  # Valgrind
   set(INCLUDE_THIRD_PARTY_VALGRIND ${CMAKE_SOURCE_DIR}/third-party/valgrind)

# Sources
 # Jerry core
  file(GLOB SOURCE_CORE_API                *.cpp)
  file(GLOB SOURCE_CORE_LIT                lit/*.cpp)
  file(GLOB SOURCE_CORE_RCS                rcs/*.cpp)
  file(GLOB SOURCE_CORE_MEM                mem/*.cpp)
  file(GLOB SOURCE_CORE_VM                 vm/*.cpp)
  file(GLOB SOURCE_CORE_ECMA_BUILTINS      ecma/builtin-objects/*.cpp)
  file(GLOB SOURCE_CORE_ECMA_BASE          ecma/base/*.cpp)
  file(GLOB SOURCE_CORE_ECMA_OPERATIONS    ecma/operations/*.cpp)
  file(GLOB SOURCE_CORE_PARSER_COLLECTIONS parser/collections/*.cpp)
  file(GLOB SOURCE_CORE_PARSER_JS          parser/js/*.cpp)
  file(GLOB SOURCE_CORE_JRT                jrt/*.cpp)

  set(SOURCE_CORE
      jerry.cpp
      ${SOURCE_CORE_API}
      ${SOURCE_CORE_LIT}
      ${SOURCE_CORE_RCS}
      ${SOURCE_CORE_MEM}
      ${SOURCE_CORE_VM}
      ${SOURCE_CORE_ECMA_BUILTINS}
      ${SOURCE_CORE_ECMA_BASE}
      ${SOURCE_CORE_ECMA_OPERATIONS}
      ${SOURCE_CORE_PARSER_COLLECTIONS}
      ${SOURCE_CORE_PARSER_JS}
      ${SOURCE_CORE_JRT})

# Per-option configuration
 # Valgrind
 if("${ENABLE_VALGRIND}" STREQUAL "ON")
  set(DEFINES_JERRY ${DEFINES_JERRY} ${DEFINES_JERRY_VALGRIND})
  set(INCLUDE_CORE ${INCLUDE_CORE} ${INCLUDE_THIRD_PARTY_VALGRIND})
 endif()

 # Log
  if("${ENABLE_LOG}" STREQUAL "ON")
   set(DEFINES_JERRY ${DEFINES_JERRY} JERRY_ENABLE_LOG)
  endif()

# Platform-specific configuration
 set(DEFINES_JERRY ${DEFINES_JERRY} ${DEFINES_JERRY_${PLATFORM_EXT}})

# Targets declaration
 function(declare_targets_for_build_mode BUILD_MODE)
  set(TARGET_NAME ${BUILD_MODE_PREFIX_${BUILD_MODE}})
  set(DEFINES_JERRY ${DEFINES_JERRY} ${DEFINES_JERRY_${BUILD_MODE}})

  function(declare_target_with_modifiers ) # modifiers are passed in ARGN implicit argument
   foreach(MODIFIER ${ARGN})
    set(TARGET_NAME ${TARGET_NAME}${MODIFIER_SUFFIX_${MODIFIER}})
    set(DEFINES_JERRY ${DEFINES_JERRY} ${DEFINES_${MODIFIER}})
   endforeach()

   add_library(${TARGET_NAME}.jerry-core STATIC ${SOURCE_CORE})
   set_property(TARGET ${TARGET_NAME}.jerry-core
                PROPERTY COMPILE_FLAGS "${COMPILE_FLAGS_JERRY} ${CXX_FLAGS_JERRY} ${FLAGS_COMMON_${BUILD_MODE}}")
   target_compile_definitions(${TARGET_NAME}.jerry-core PRIVATE ${DEFINES_JERRY})
   target_include_directories(${TARGET_NAME}.jerry-core PRIVATE ${INCLUDE_CORE})
   target_include_directories(${TARGET_NAME}.jerry-core PRIVATE ${INCLUDE_FDLIBM})
   target_include_directories(${TARGET_NAME}.jerry-core SYSTEM PRIVATE ${INCLUDE_LIBC_INTERFACE})

   if("${BUILD_MODE}" STREQUAL "UNITTESTS")
    target_compile_definitions(${TARGET_NAME}.jerry-core INTERFACE ${DEFINES_JERRY})
    target_include_directories(${TARGET_NAME}.jerry-core INTERFACE ${INCLUDE_CORE})
   endif()
  endfunction()

  foreach(MODIFIERS_LIST ${MODIFIERS_LISTS})
   separate_arguments(MODIFIERS_LIST)

   declare_target_with_modifiers(${MODIFIERS_LIST})
  endforeach()
 endfunction()

 declare_targets_for_build_mode(DEBUG)
 declare_targets_for_build_mode(RELEASE)
 declare_targets_for_build_mode(UNITTESTS)

